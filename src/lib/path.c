/*
 * path.c
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


#include "path.h"




// Return FriLogos installation directory. useful to get global installation data path.
// @return char pointer containing installation directory.
const char *
edams_install_path_get (void)
{
	return PACKAGE_DIR;
}


// Return FriLogos user data directory. useful to get user data path.
// @return char pointer containing installation directory.
const char *
edams_data_path_get(void)
{
	char s[PATH_MAX];
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"data"DIR_SEPARATOR_S"edams", efreet_data_home_get());
	return strdup(s);
}


const char *
edams_locale_path_get(void)
{
	return PACKAGE_LOCALE_DIR;
}


//
//
//
const char *
edams_db_path_get (void)
{
    char s[PATH_MAX];
    snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"db", edams_data_path_get());
	return strdup(s);
}


//
//
//
const char *
edams_locations_data_path_get(void)
{
    char s[PATH_MAX];
    snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"locations", edams_data_path_get());
	return strdup(s);
}



//
//
//
const char *
edams_settings_file_get (void)
{
    char s[PATH_MAX];
    snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"settings.eet", edams_data_path_get());
	return strdup(s);
}


//
//
//
const char *
edams_theme_path_get(void)
{
	char s[PATH_MAX];

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"share"DIR_SEPARATOR_S"edams"DIR_SEPARATOR_S"themes"DIR_SEPARATOR_S"default", edams_install_path_get());
	return strdup(s);
}

//
//
//
const char *
edams_edje_theme_file_get (void)
{
	char s[PATH_MAX];

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"share"DIR_SEPARATOR_S"edams"DIR_SEPARATOR_S"themes"DIR_SEPARATOR_S"default"DIR_SEPARATOR_S"edams.edj", edams_install_path_get());
	return strdup(s);
}
