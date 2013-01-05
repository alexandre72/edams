/*
 * settings.c
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

#include <Eina.h>
#include <Eet.h>
#include "settings.h"
#include "edams.h"
#include "path.h"
#include "utils.h"


#define EET_STRING_SETTINGS_READ(_field, _var) \
   	ret = eet_read(ef, _field, &size); \
   	if(ret) \
   	{ \
		_var = eina_stringshare_add(ret);	\
   		FREE(ret);	\
   	}	\

#define EET_BOOL_SETTINGS_READ(_field, _var) \
   	ret = eet_read(ef, _field, &size); \
   	if(ret) \
   	{ \
		_var = atoi(ret) ? EINA_TRUE : EINA_FALSE; \
   		FREE(ret);	\
   	} \

#define EET_STRING_SETTINGS_WRITE(_field, _var) \
   	eet_write(ef, _field, _var, strlen(_var)+1, 0); \


#define EET_BOOL_SETTINGS_WRITE(_field, _var) \
	if(_var == EINA_TRUE) \
	   	eet_write(ef, _field, "1", strlen("1")+1, 0); \
	else \
	   	eet_write(ef, _field, "0", strlen("0")+1, 0); \


//
//
//
const Settings
*edams_settings_get(void)
{
	Eet_File *ef;

    Settings *settings = calloc(1, sizeof(Settings));

	if(!settings)
	{
		debug(stderr, _("ERROR:Couldn't calloc Settings struct!"));
		return NULL;
	}

	ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_READ);

	char *ret;
	int size;
	settings->cosm_apikey = NULL;
	settings->softemu = EINA_FALSE;
	settings->debug = EINA_FALSE;

	EET_BOOL_SETTINGS_READ("edams/debug", settings->debug);
	EET_BOOL_SETTINGS_READ("edams/softemu", settings->softemu);
	EET_STRING_SETTINGS_READ("map/map_background",settings->map_background);
	EET_STRING_SETTINGS_READ("edams/cosm_apikey",settings->cosm_apikey);
	EET_STRING_SETTINGS_READ("edams/gnuplot_path",settings->gnuplot_path);
	eet_close(ef);

	set_debug_mode(settings->debug);
	debug(stdout, _("Debug is %s"), settings->debug?_("enabled"):_("disabled"));
	debug(stdout, _("Cosm data handling options is %s"), settings->cosm_apikey?_("enabled"):_("disabled"));
	debug(stdout, _("Software emulation is %s"), settings->softemu?_("enabled"):_("disabled"));
	debug(stdout, _("Map background file is '%s'"), settings->map_background);
	debug(stdout, _("Gnuplot path is '%s'"), settings->gnuplot_path);

	return settings;
}

void
edams_settings_write(Settings *settings)
{
	Eet_File *ef;

	ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_WRITE);
	EET_STRING_SETTINGS_WRITE("edams/cosm_apikey", settings->cosm_apikey);
	EET_STRING_SETTINGS_WRITE("edams/gnuplot_path", settings->gnuplot_path);
	EET_STRING_SETTINGS_WRITE("map/map_background", settings->map_background);
	EET_BOOL_SETTINGS_WRITE("edams/softemu", settings->softemu);
	EET_BOOL_SETTINGS_WRITE("edams/debug", settings->debug);
	set_debug_mode(settings->debug);

	eet_close(ef);
}



Settings *edams_settings_free(Settings *settings)
{
	eina_stringshare_del(settings->gnuplot_path);
	eina_stringshare_del(settings->cosm_apikey);
	eina_stringshare_del(settings->map_background);
	FREE(settings);
	return NULL;
}
