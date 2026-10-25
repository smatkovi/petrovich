/*
 * This file is part of Petrovich - A program to share arbitrary files over the services supported in Maemo 5
 *
 * Copyright (c) 2009 Faheem Pervez <trippin1@gmail.com>. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *      
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *      
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <config.h>
#include <string.h>
#include <hildon-mime.h>

#include "callbacks.h"
#include "utilities.h"
#include "share.h"

void on_window_destroyed (gpointer user_data G_GNUC_UNUSED)
{
	gtk_main_quit ();
}

void on_share_button_clicked (GtkWidget *share_button G_GNUC_UNUSED, Data *data)
{
	gchar *main_window_uri;
	GtkWidget *window;

	g_return_if_fail (data);

	window = hildon_stackable_window_new ();
	hildon_program_add_window (data->program, HILDON_WINDOW (window));
	data->toolbar = hildon_edit_toolbar_new_with_text (dgettext_localised_or_english ("osso-filemanager", "hfil_ti_edit_share", "Choose items to share"), dgettext_localised_or_english ("hildon-libs", "wdgt_bd_share", "Share"));
	data->multiple_file_selection = g_object_new (HILDON_TYPE_FILE_SELECTION, "model", data->hildon_fm_model, "edit-mode", TRUE, "hide-navi", TRUE, "show-folders", FALSE, NULL);

	main_window_uri = hildon_file_selection_get_current_folder_uri (HILDON_FILE_SELECTION (data->main_file_selection));
	hildon_file_selection_set_current_folder_uri (HILDON_FILE_SELECTION (data->multiple_file_selection), main_window_uri, NULL);
	g_free (main_window_uri);

	hildon_window_set_edit_toolbar (HILDON_WINDOW (window), HILDON_EDIT_TOOLBAR (data->toolbar));
	gtk_container_add (GTK_CONTAINER (window), data->multiple_file_selection);
	g_signal_connect_swapped (data->toolbar, "arrow-clicked", G_CALLBACK (gtk_widget_destroy), window);
	g_signal_connect (data->toolbar, "button-clicked", G_CALLBACK (on_file_activated), data);
	gtk_widget_show_all (window);

	gtk_window_fullscreen (GTK_WINDOW (window));
}

static void on_about_dialog_link_activated (GtkAboutDialog *about G_GNUC_UNUSED, const gchar *link_, gpointer data G_GNUC_UNUSED)
{
	hildon_uri_open (link_, hildon_uri_get_default_action (link_, NULL), NULL);
}

void on_about_button_clicked (GtkWidget *about_button G_GNUC_UNUSED, Data *data)
{
	const gchar *authors[] = { "Faheem Pervez <trippin1@gmail.com>", NULL };
	const gchar *documenters[] = { "Thomas Perl (Program Name)", NULL };
	GdkPixbuf *logo;

	g_return_if_fail (data);

	logo = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), SHARING_ICON, 64, 0, NULL);

    gtk_about_dialog_set_url_hook ((GtkAboutDialogActivateLinkFunc) on_about_dialog_link_activated, NULL, NULL);
    gtk_show_about_dialog (GTK_WINDOW (data->main_window), "comments", "File manager-like application allowing for the sending of arbitrary files", "version", PACKAGE_VERSION, "website", "http://maemo.org/downloads/product/Maemo5/petrovich/", "copyright", "(C) 2009 Faheem Pervez", "logo", logo, "license", "GPL v2", "authors", authors, "documenters", documenters, NULL);

	if (logo)
		g_object_unref (logo);
}

void on_current_folder_changed (HildonFileSelection *self, Data *data)
{
	char *root_folder= "files:///";
	char *active_folder = hildon_file_selection_get_current_folder_uri (self);

	!strcmp (active_folder, root_folder) ? gtk_widget_hide(data->share_button) : gtk_widget_show(data->share_button);

	g_free (active_folder);
}

void on_file_activated (GtkWidget *self, Data *data)
{
	GSList *file_list;

	g_return_if_fail (self);
	g_return_if_fail (data);

	if (HILDON_IS_FILE_SELECTION (self))
	{
		gchar *uri;

		file_list = hildon_file_selection_get_selected_uris (HILDON_FILE_SELECTION (self));
		uri = g_slist_nth_data (file_list, 0);

		share_file (data->context, GTK_WINDOW (data->main_window), uri);

		goto cleanup;
	}
	else if (HILDON_IS_EDIT_TOOLBAR (self))
	{
		file_list = hildon_file_selection_get_selected_uris (HILDON_FILE_SELECTION (data->multiple_file_selection));
		if (g_slist_length (file_list) == 0)
			goto cleanup;

		g_signal_emit_by_name (data->toolbar, "arrow-clicked", G_TYPE_NONE);
		share_files (data->context, GTK_WINDOW (data->main_window), file_list);

		goto cleanup;
	}
	else
		g_return_if_reached ();

cleanup:
	g_slist_foreach (file_list, (GFunc) g_free, NULL);
	g_slist_free (file_list);
	return;
}

