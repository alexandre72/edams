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


#include "libedams.h"
#include "modules.h"
#include "rooms.h"


//
//Define App UI. Can be used to modify it dynamicaly.
//
typedef struct
{
	Evas_Object *win;
	Evas_Object *toolbar;
	Eina_List *rooms;
} App_Info;


//about.c: About dialog box.
void about_dialog_new(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);

//init.c
//shudown.c
int edams_init();
int edams_shutdown();

//sensors_browser.c: Sensors browser.
void sensors_browser_new(void *data, Evas_Object *obj, void *event_info);

//preferences_dlg.c: Preferences dialog.
void preferences_dlg_new(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);

//Win32 ported funcs.
#ifdef HAVE_EVIL
extern char *efreet_data_home_get();
#endif

#endif /* __EDAMS_H__ */
