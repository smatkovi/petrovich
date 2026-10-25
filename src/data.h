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

#ifndef __DATA_H_
#define __DATA_H_


#include <libosso.h>
#include <gtk/gtk.h>
#include <hildon/hildon.h>

#include <hildon/hildon-file-system-model.h>

typedef struct _Data
{
	osso_context_t *context;
	HildonProgram *program;

	GtkWidget *main_window, *main_file_selection, *share_button;
	GtkWidget *multiple_file_selection, *toolbar;
	HildonFileSystemModel *hildon_fm_model;
} Data;

#endif /* __DATA_H_ */

