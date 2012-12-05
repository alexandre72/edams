/*
 * preferences_dlg.c
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


#include "edams.h"
#include "path.h"


//
//
//
void
preferences_dlg_new(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win;
	App_Info *app = (App_Info *)data;

   	win  = elm_win_util_standard_add(_("Preferences"), "");
   	elm_win_autodel_set(win, EINA_TRUE);
   	elm_win_center(win, EINA_TRUE, EINA_TRUE);
   	evas_object_show(win);

 	Evas_Object *bg = elm_bg_add(win);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg );
  	evas_object_size_hint_min_set(bg, 600, 450);

   	Evas_Object *prefs = elm_prefs_add(win);
   	evas_object_size_hint_weight_set(prefs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	elm_win_resize_object_add(win, prefs);
	elm_prefs_autosave_set(prefs, EINA_TRUE);
	elm_prefs_file_set(prefs, edams_epb_file_get(), NULL);
	elm_prefs_data_set(prefs, app->prefs_data);
   	evas_object_show(prefs);
}
