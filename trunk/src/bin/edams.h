/*
 * edams.h
 * This file is part of EDAMS
 *
 * Copyright (C) 2012 - Alexandre Dussart
 *
 * EDAMS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EDAMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EDAMS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __EDAMS_H__
#define __EDAMS_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Evas.h>
#include <Eina.h>

#include "utils.h"
#include "location.h"
#include "settings.h"
#include "xpl.h"

#define PID_FILE "/var/run/edams.pid"

/*
 *Define App UI struct.
 */
typedef struct
{
	int argc;
	char **argv;

	Evas_Object *win;
	Evas_Object *toolbar;
	Eina_List *locations;
	Location *location;
	Widget *widget;
} App_Info;


const Eina_List *edams_locations_list_get();
App_Info *edams_app_info_get();
void statusbar_text_set(const char *msg, const char *ic);
void update_naviframe_content(Location *location);
Evas_Object*_location_naviframe_content_set(Location *location);

#endif /* __EDAMS_H__ */
