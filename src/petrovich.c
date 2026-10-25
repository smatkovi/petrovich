/*
 * Petrovich - A program to share arbitrary files over the services supported in Maemo 5
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <glib.h>
#include <locale.h>

#include <libosso.h>
#include <gtk/gtk.h>
#include <hildon/hildon.h>

#include <hildon/hildon-file-selection.h>
#include <hildon/hildon-file-system-model.h>

#include "data.h"
#include "callbacks.h"
#include "utilities.h"

static void cleanup (Data *data);

static void setup_window (Data *data)
{
	g_return_if_fail (data);

	data->program = HILDON_PROGRAM (hildon_program_get_instance ());

	data->main_window = hildon_stackable_window_new ();
	g_signal_connect_swapped (G_OBJECT (data->main_window), "destroy", G_CALLBACK (on_window_destroyed), NULL);
	hildon_program_add_window (data->program, HILDON_WINDOW (data->main_window));

	data->hildon_fm_model = HILDON_FILE_SYSTEM_MODEL (g_object_new (HILDON_TYPE_FILE_SYSTEM_MODEL, "ref-widget", data->main_window, NULL));
	data->main_file_selection = hildon_file_selection_new_with_model (data->hildon_fm_model);
	g_signal_connect (G_OBJECT (data->main_file_selection), "file-activated", G_CALLBACK (on_file_activated), data);
	g_signal_connect (G_OBJECT (data->main_file_selection), "current-folder-changed", G_CALLBACK (on_current_folder_changed), data);
	gtk_container_add (GTK_CONTAINER (data->main_window), data->main_file_selection);

	gtk_widget_show_all (data->main_window);
}

static void setup_menu (Data *data)
{
	GtkWidget *app_menu, *button;
	g_return_if_fail (data);

	app_menu = hildon_app_menu_new();

	button = gtk_button_new_with_label (dgettext_localised_or_english("hildon-libs", "wdgt_bd_share", "Share"));
	g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (on_share_button_clicked), data);
	hildon_app_menu_append(HILDON_APP_MENU (app_menu), GTK_BUTTON (button));
	data->share_button = button;

	button = gtk_button_new_from_stock (GTK_STOCK_ABOUT);
	g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (on_about_button_clicked), data);
	hildon_app_menu_append(HILDON_APP_MENU (app_menu), GTK_BUTTON (button));

	hildon_window_set_app_menu (HILDON_WINDOW (data->main_window), HILDON_APP_MENU (app_menu));
	gtk_widget_show_all (app_menu);
	gtk_widget_hide (data->share_button);
}

static void cleanup (Data *data)
{
	g_assert (data);

	if (data->context)
		osso_deinitialize (data->context);

	g_free (data);
}

gint main (gint argc, gchar** argv)
{
	gchar app_name[] = PACKAGE;
	Data *data = g_new0 (Data, 1);

	setlocale (LC_ALL, "");

	hildon_gtk_init (&argc, &argv);

	gtk_settings_set_long_property (gtk_settings_get_default (), "gtk-enable-mnemonics", 0, PACKAGE);

	data->context = osso_initialize (OSSO_SERVICE, PACKAGE_VERSION, FALSE, NULL);

	app_name[0] = toupper (app_name[0]);
	g_set_application_name (app_name);

	setup_window (data);
	setup_menu (data);

	gtk_main ();

	cleanup (data);

	return EXIT_SUCCESS;
}
