/*
 * share.c - sending files without Maemo 5's sharing dialog
 *
 * Maemo 5's Petrovich handed the files to Nokia's closed sharing dialog
 * (libsharingdialog). Maemo Leste has none, so this offers what Petrovich
 * was used for:
 *
 *   Bluetooth  to a paired device (BlueZ 5), by OBEX Object Push through
 *              obexd (org.bluez.obex on the session bus)
 *   E-mail     a new message in Modest with the files attached
 *              (libmodest-dbus-client)
 *
 * Copyright (C) 2026 Sebastian Matkovich
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <string.h>
#include <gio/gio.h>
#include <hildon/hildon.h>
#include <libmodest-dbus-client/libmodest-dbus-client.h>

#include "share.h"

enum { SEND_BLUETOOTH = 1, SEND_EMAIL = 2 };

typedef struct {
	GtkWindow *parent;
	GDBusConnection *session;
	GSList *paths;		/* local file paths still to send */
	char *device;		/* Bluetooth address */
	char *obex_session;	/* object path */
	guint watch;
	int sent, failed;
} Transfer;

/* --- Bluetooth -------------------------------------------------------------- */

/* Paired devices: address -> name */
static GHashTable *paired_devices(void)
{
	GHashTable *devs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	GDBusConnection *sys = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	GVariant *r, *ifaces, *props;
	GVariantIter *objs;
	const char *path;

	if (!sys)
		return devs;
	r = g_dbus_connection_call_sync(sys, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager",
					"GetManagedObjects", NULL, G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
					G_DBUS_CALL_FLAGS_NONE, 5000, NULL, NULL);
	if (r) {
		g_variant_get(r, "(a{oa{sa{sv}}})", &objs);
		while (g_variant_iter_next(objs, "{&o@a{sa{sv}}}", &path, &ifaces)) {
			props = g_variant_lookup_value(ifaces, "org.bluez.Device1", G_VARIANT_TYPE("a{sv}"));
			if (props) {
				gboolean paired = FALSE;
				const char *addr = NULL, *name = NULL;
				g_variant_lookup(props, "Paired", "b", &paired);
				g_variant_lookup(props, "Address", "&s", &addr);
				if (!g_variant_lookup(props, "Alias", "&s", &name))
					g_variant_lookup(props, "Name", "&s", &name);
				if (paired && addr)
					g_hash_table_insert(devs, g_strdup(addr), g_strdup(name ? name : addr));
				g_variant_unref(props);
			}
			g_variant_unref(ifaces);
		}
		g_variant_iter_free(objs);
		g_variant_unref(r);
	}
	g_object_unref(sys);
	return devs;
}

static char *pick_device(GtkWindow *parent)
{
	GHashTable *devs = paired_devices();
	GPtrArray *addrs = g_ptr_array_new_with_free_func(g_free);
	GtkWidget *dlg, *sel;
	GHashTableIter it;
	gpointer addr, name;
	char *chosen = NULL;

	if (g_hash_table_size(devs) == 0) {
		hildon_banner_show_information(GTK_WIDGET(parent), NULL,
					       "No paired Bluetooth device. Pair one in the Bluetooth settings first.");
		g_hash_table_destroy(devs);
		g_ptr_array_free(addrs, TRUE);
		return NULL;
	}
	dlg = hildon_picker_dialog_new(parent);
	gtk_window_set_title(GTK_WINDOW(dlg), "Send to");
	sel = hildon_touch_selector_new_text();
	g_hash_table_iter_init(&it, devs);
	while (g_hash_table_iter_next(&it, &addr, &name)) {
		hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(sel), name);
		g_ptr_array_add(addrs, g_strdup(addr));
	}
	hildon_picker_dialog_set_selector(HILDON_PICKER_DIALOG(dlg), HILDON_TOUCH_SELECTOR(sel));
	if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
		int i = hildon_touch_selector_get_active(HILDON_TOUCH_SELECTOR(sel), 0);
		if (i >= 0 && (guint)i < addrs->len)
			chosen = g_strdup(addrs->pdata[i]);
	}
	gtk_widget_destroy(dlg);
	g_hash_table_destroy(devs);
	g_ptr_array_free(addrs, TRUE);
	return chosen;
}

static void transfer_free(Transfer *t)
{
	if (t->watch)
		g_dbus_connection_signal_unsubscribe(t->session, t->watch);
	if (t->obex_session)
		g_dbus_connection_call(t->session, "org.bluez.obex", "/org/bluez/obex", "org.bluez.obex.Client1",
				       "RemoveSession", g_variant_new("(o)", t->obex_session), NULL,
				       G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
	g_slist_free_full(t->paths, g_free);
	g_free(t->device);
	g_free(t->obex_session);
	g_object_unref(t->session);
	g_free(t);
}

static void send_next(Transfer *t);

static void transfer_changed_cb(GDBusConnection *c, const char *sender, const char *path, const char *iface,
				const char *signal, GVariant *params, gpointer data)
{
	Transfer *t = data;
	GVariant *changed;
	const char *iface_name, *status = NULL;

	(void)c, (void)sender, (void)path, (void)iface, (void)signal;
	g_variant_get(params, "(&s@a{sv}@as)", &iface_name, &changed, NULL);
	if (!strcmp(iface_name, "org.bluez.obex.Transfer1") && g_variant_lookup(changed, "Status", "&s", &status)) {
		if (!strcmp(status, "complete") || !strcmp(status, "error")) {
			if (!strcmp(status, "complete"))
				t->sent++;
			else
				t->failed++;
			g_dbus_connection_signal_unsubscribe(t->session, t->watch);
			t->watch = 0;
			send_next(t);
		}
	}
	g_variant_unref(changed);
}

static void send_next(Transfer *t)
{
	GVariant *r;
	GError *err = NULL;
	const char *transfer;
	char *path, *msg;

	if (!t->paths) {
		msg = t->failed ? g_strdup_printf("Bluetooth: %d sent, %d failed", t->sent, t->failed)
				: g_strdup_printf("Bluetooth: %d %s sent", t->sent, t->sent == 1 ? "file" : "files");
		hildon_banner_show_information(GTK_WIDGET(t->parent), NULL, msg);
		g_free(msg);
		transfer_free(t);
		return;
	}
	path = t->paths->data;
	t->paths = g_slist_delete_link(t->paths, t->paths);
	r = g_dbus_connection_call_sync(t->session, "org.bluez.obex", t->obex_session, "org.bluez.obex.ObjectPush1",
					"SendFile", g_variant_new("(s)", path), G_VARIANT_TYPE("(oa{sv})"),
					G_DBUS_CALL_FLAGS_NONE, 30000, NULL, &err);
	g_free(path);
	if (!r) {
		t->failed++;
		g_clear_error(&err);
		send_next(t);
		return;
	}
	g_variant_get(r, "(&o@a{sv})", &transfer, NULL);
	t->watch = g_dbus_connection_signal_subscribe(t->session, "org.bluez.obex", "org.freedesktop.DBus.Properties",
						      "PropertiesChanged", transfer, NULL, G_DBUS_SIGNAL_FLAGS_NONE,
						      transfer_changed_cb, t, NULL);
	g_variant_unref(r);
}

static void send_bluetooth(GtkWindow *parent, GSList *uris)
{
	Transfer *t;
	GVariantBuilder opts;
	GVariant *r;
	GError *err = NULL;
	char *device = pick_device(parent);

	if (!device)
		return;
	t = g_new0(Transfer, 1);
	t->parent = parent;
	t->device = device;
	t->session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
	for (GSList *l = uris; l; l = l->next) {
		char *p = g_filename_from_uri(l->data, NULL, NULL);
		if (p)
			t->paths = g_slist_append(t->paths, p);
	}
	g_variant_builder_init(&opts, G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_add(&opts, "{sv}", "Target", g_variant_new_string("opp"));
	hildon_banner_show_information(GTK_WIDGET(parent), NULL, "Sending over Bluetooth...");
	r = g_dbus_connection_call_sync(t->session, "org.bluez.obex", "/org/bluez/obex", "org.bluez.obex.Client1",
					"CreateSession", g_variant_new("(sa{sv})", device, &opts),
					G_VARIANT_TYPE("(o)"), G_DBUS_CALL_FLAGS_NONE, 30000, NULL, &err);
	if (!r) {
		char *msg = g_strdup_printf("Bluetooth: %s", err ? err->message : "no connection");
		hildon_banner_show_information(GTK_WIDGET(parent), NULL, msg);
		g_free(msg);
		g_clear_error(&err);
		transfer_free(t);
		return;
	}
	g_variant_get(r, "(o)", &t->obex_session);
	g_variant_unref(r);
	send_next(t);
}

/* --- e-mail ------------------------------------------------------------------ */

static void send_email(osso_context_t *ctx, GtkWindow *parent, GSList *uris)
{
	if (!libmodest_dbus_client_compose_mail(ctx, NULL, NULL, NULL, NULL, NULL, uris))
		hildon_banner_show_information(GTK_WIDGET(parent), NULL, "Could not open the e-mail editor");
}

/* --- the dialog ------------------------------------------------------------ */

void share_files(osso_context_t *ctx, GtkWindow *parent, GSList *uris)
{
	GtkWidget *dlg;
	int response;

	if (!uris)
		return;
	dlg = gtk_dialog_new_with_buttons("Send via", parent, GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
					  "Bluetooth", SEND_BLUETOOTH, "E-mail", SEND_EMAIL, NULL);
	response = gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
	if (response == SEND_BLUETOOTH)
		send_bluetooth(parent, uris);
	else if (response == SEND_EMAIL)
		send_email(ctx, parent, uris);
}

void share_file(osso_context_t *ctx, GtkWindow *parent, const gchar *uri)
{
	GSList one = { (gpointer)uri, NULL };

	share_files(ctx, parent, &one);
}
