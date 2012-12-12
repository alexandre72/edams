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


#include <Evas.h>
#include <Eina.h>

#include "libedams.h"
#include "rooms.h"

//
//Define App UI. Can be used to modify it dynamicaly.
//
typedef struct
{
	Evas_Object *win;
	Evas_Object *waiting_win;
	Evas_Object *toolbar;
	Eina_List *rooms;
	Eina_List *devices;
	Eina_List *meters;
	Elm_Prefs_Data *prefs_data;
	Room *room;
} App_Info;



Evas_Object*_room_naviframe_content(Room *room);

//preferences_dlg.c: Preferences dialog.
void preferences_dlg_new(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);

#endif /* __EDAMS_H__ */
