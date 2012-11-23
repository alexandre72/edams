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






#include "settings.h"


//
//
//
const char *edams_settings_file_get(void)
{
	char s[PATH_MAX];
	snprintf(s, sizeof(s), "%s%ssettings.eet", edams_data_path_get(), DIR_SEPARATOR_S);
  	 return strdup(s);
}



//
//Read a edams setting from file.
//
int setting_int_read(const char *setting)
{
  	Eet_File *ef;
  	char *ret;
	int size;

    if(!setting)
        return -1;

    if(!(ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_READ)))
    {
        ERR(_("Can't read edams settings!"));
		eet_close(ef);
		return -1;
	}

	ret = eet_read(ef, setting, &size);
   	eet_close(ef);
	if(ret)
	{
	    int i = atoi(ret);
	    //INF(_("Setting %s set to value %d", setting, i);
    	return i;
    }
	else
    {
        ERR("Can't read %s field from edams settings file.", setting);
        return -1;
    }
}



//
//Read a setting from file.
//
int file_setting_int_read(const char *file, const char *setting)
{
  	Eet_File *ef;
  	char *ret;
	int size;

    if(!file || !setting)
        return -1;

    if(!(ef = eet_open(file, EET_FILE_MODE_READ)))
    {
        ERR(_("Can't read %s settings from %s!"), setting, file);
		eet_close(ef);
		return -1;
	}

	ret = eet_read(ef, setting, &size);
   	eet_close(ef);
	if(ret)
	{
	    int i = atoi(ret);
	    //INF(_("Setting %s set to value %d", setting, i);
    	return i;
    }
	else
    {
        ERR(_("Can't read %s settings from %s!"), setting, file);
        return -1;
    }
}



//
//Write a setting to a file.
//
int file_setting_write(const char *file, const char *setting,  const int value)
{
  	Eet_File *ef;
  	char s[100];

    if(!file || !setting)
        return -1;

    eina_convert_itoa(value, s);

    if(!(ef = eet_open(file, EET_FILE_MODE_READ_WRITE)))
    {
        ERR(_("Can't write %s settings from %s!"), setting, file);
		eet_close(ef);
		return -1;
	}
	//INF(_("Setting %s set to value %d", setting, i);
	eet_write(ef, setting, s, strlen(s) + 1, 1);

	eet_sync(ef);
	eet_close(ef);

	return 0;
}




//
//Write a edams setting to file.
//
int setting_write(const char *setting,  const int value)
{
  	Eet_File *ef;
  	char s[100];

    if(!setting)
        return -1;

    eina_convert_itoa(value, s);

    if(!(ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_READ_WRITE)))
    {
        ERR(_("Can't write edams settings!"));
		eet_close(ef);
		return -1;
	}
	//INF(_("Setting %s set to value %d", setting, i);
	eet_write(ef, setting, s, strlen(s) + 1, 1);

	eet_sync(ef);
	eet_close(ef);

	return 0;
}
