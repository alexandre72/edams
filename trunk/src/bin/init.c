/*
 * init.c
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
#include "utils.h"
#include "path.h"
#include "device.h"



int
edams_init()
{
	char s[PATH_MAX];
	//Elm_Theme *th;

	//TODO:splash screen.
	//Evas_Object *win;
	//win = elm_win_add(NULL, "init", ELM_WIN_SPLASH);
	//elm_win_borderless_set(win, EINA_FALSE);
    	//elm_win_shaped_set(win, EINA_TRUE);
	//elm_win_alpha_set(win, EINA_TRUE);
	//elm_win_title_set(win, _("Initialize"));
	//evas_object_show(win);
	//evas_object_resize(win, 400, 400);

	//If no configurations path then create a new one in user home's.
	strcpy(s, edams_data_path_get());
	if(ecore_file_is_dir(s) == EINA_FALSE)
	{
		msgbox(_("It appears that it's the first time you run EDAMS. To be used, EDAMS needs some database files containing items. I'll copy some default items files, but you can easily remove them and create new ones(highly recommended)!"));

		ecore_file_mkpath(edams_data_path_get());
	}

    //Create locations database directory.
    strcpy(s, edams_locations_data_path_get());
	if(ecore_file_is_dir(s) == EINA_FALSE)
		ecore_file_mkpath(s);

    //Create devices database directory.
    strcpy(s, edams_devices_data_path_get());
	if(ecore_file_is_dir(s) == EINA_FALSE)
		ecore_file_mkpath(s);


    //Create cache directory.
	//snprintf(s, sizeof(s), "%s/edams", efreet_cache_home_get());
	//if(ecore_file_is_dir(s) == 0)
	//	ecore_file_mkpath(s);

    	//Settings elementary options.
   	elm_theme_extension_add(NULL, edams_edje_theme_file_get() );

	//elm_scale_set(1.50);
	//elm_finger_size_set(0);
    elm_language_set("fr_FR.UTF-8");
	//evas_object_del(win);

	//Initialize locations descriptor.
	locations_init();
	devices_init();

    ecore_con_url_pipeline_set(EINA_FALSE);

	return 0;
}
