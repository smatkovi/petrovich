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

#ifndef __CALLBACKS_H_
#define __CALLBACKS_H_

#include <glib.h>

#include <gtk/gtk.h>
#include <hildon/hildon.h>

#include <hildon/hildon-file-selection.h>
#include <hildon/hildon-file-system-model.h>

#include "data.h"

void on_window_destroyed (gpointer user_data G_GNUC_UNUSED);

void on_share_button_clicked (GtkWidget *share_button G_GNUC_UNUSED, Data *data);

void on_about_button_clicked (GtkWidget *about_button G_GNUC_UNUSED, Data *data);

void on_current_folder_changed (HildonFileSelection *self, Data *data);

void on_file_activated (GtkWidget *self, Data *data);

#endif /* __CALLBACKS_H_ */

