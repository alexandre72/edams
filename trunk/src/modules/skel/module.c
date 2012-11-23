/*
 * module.c
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


#include "module.h"



typedef struct _Screen Screen;

EAPI int version_maj_symbol = 0;
EAPI int version_min_symbol = 0;
EAPI int version_mic_symbol = 1;


//
//Return module name.
//
EAPI char *_module_name_get(void)
{
	return  strdup(_("skel"));
}



//
//Print module informations.
//
EAPI void _print_module_infos(void)
{
	INF(_("%s module"), _module_name_get());
	INF(_("\tVersion:%d.%d.%d"),
					version_maj_symbol,
					version_min_symbol,
					version_mic_symbol);
	INF(_("\tBuild on:%s"), __DATE__);
}



//
//Return module description.
//
EAPI char *_module_desc_get(void)
{
	return strdup(_("skel module is only here to be a skeleton for new module!"));
}



//
//Return module internal's id.
//
EAPI char *_module_id_get(void)
{
	return strdup(_("skel"));
}



//
//Return module theme path.
//
EAPI char *_module_theme_get(void)
{
	char p[FILENAME_MAX];
	snprintf(p, sizeof(p), "%s"DIR_SEPARATOR_S"skel.edj", edams_theme_path_get());
	return strdup(p);
}


//
//
//
static Eina_Bool _module_init(void)
{
    INF(_("Alloc module ressources."));
    
    settings = calloc(1, sizeof(Settings));

    if (!settings)
       {
          ERR(_("Couldn't calloc Settings struct!"));
          return EINA_FALSE;
       }

   return EINA_TRUE;
}



//
//
//
void _module_shutdown(void)
{
    INF(_("Free module ressources."));
    FREE(settings);    
    settings = NULL;
}



EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);
