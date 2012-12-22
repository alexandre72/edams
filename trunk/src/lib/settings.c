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


//
//
//
const Settings
*edams_settings_get(void)
{
	Eet_File *ef;

	fprintf(stdout, _("Read applications settings...\n"));
    Settings *settings = calloc(1, sizeof(Settings));

	if(!settings)
	{
		fprintf(stderr, _("ERROR:Couldn't calloc Settings struct!\n"));
		return NULL;
	}

	ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_READ);

	char *ret;
	int size;
	settings->cosm_apikey = NULL;
	settings->softemu = EINA_FALSE;
	settings->hardemu = EINA_FALSE;
	settings->debug = EINA_FALSE;

   	ret = eet_read(ef, "edams/cosm_apikey", &size);
   	if(ret)
   	{
		settings->cosm_apikey = eina_stringshare_add(ret);
   		FREE(ret);
   	}

   	ret = eet_read(ef, "edams/softemu", &size);
   	if(ret)
   	{
		settings->softemu = atoi(ret) ? EINA_TRUE : EINA_FALSE;
   		FREE(ret);
   	}

   	ret = eet_read(ef, "edams/hardemu", &size);
   	if(ret)
   	{
		settings->hardemu = atoi(ret) ? EINA_TRUE : EINA_FALSE;
   		FREE(ret);
   	}

   	ret = eet_read(ef, "edams/debug", &size);
   	if(ret)
   	{
		settings->debug = atoi(ret) ? EINA_TRUE : EINA_FALSE;
   		FREE(ret);
   	}

	eet_close(ef);

	fprintf(stdout, _("OPTION:Cosm data handling=>%s\n"), settings->cosm_apikey?_("ENABLE"):_("DISABLE"));
	fprintf(stdout, _("OPTION:Software emulation(snprintf)=>%s\n"), settings->softemu?_("ENABLE"):_("DISABLE"));
	fprintf(stdout, _("OPTION:Hardware emulation(serial loopback)=>%s\n"), settings->hardemu?_("ENABLE"):_("DISABLE"));
	fprintf(stdout, _("OPTION:Debug with printf=>%s\n"), settings->debug?_("ENABLE"):_("DISABLE"));

	return settings;
}

void
edams_settings_write(Settings *settings)
{
	Eet_File *ef;

	ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_WRITE);

   	eet_write(ef, "edams/cosm_apikey", settings->cosm_apikey, strlen(settings->cosm_apikey)+1, 0);

	if(settings->softemu == EINA_TRUE)
	   	eet_write(ef, "edams/softemu", "1", strlen("1")+1, 0);
	else
	   	eet_write(ef, "edams/softemu", "0", strlen("0")+1, 0);

	if(settings->hardemu == EINA_TRUE)
	   	eet_write(ef, "edams/hardemu", "1", strlen("1")+1, 0);
	else
	   	eet_write(ef, "edams/hardemu", "0", strlen("0")+1, 0);

	if(settings->debug == EINA_TRUE)
	   	eet_write(ef, "edams/debugf", "1", strlen("1")+1, 0);
	else
	   	eet_write(ef, "edams/debugf", "0", strlen("0")+1, 0);

	eet_close(ef);
}



Settings *edams_settings_free(Settings *settings)
{

	eina_stringshare_del(settings->cosm_apikey);
	FREE(settings);
	return NULL;
}
