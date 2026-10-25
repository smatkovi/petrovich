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

#include <libintl.h>
#include <string.h>
#include "utilities.h"

G_CONST_RETURN gchar* dgettext_localised_or_english (const gchar* domainname, const gchar* msgid, const gchar *fallback_string)
{
	const gchar *retval;

	g_return_val_if_fail (domainname, NULL);
	g_return_val_if_fail (msgid, NULL);
	g_return_val_if_fail (fallback_string, NULL);

	retval = dgettext (domainname, msgid);

	return G_UNLIKELY (!g_ascii_strncasecmp (retval, msgid, strlen (msgid))) ? fallback_string : retval;
}

