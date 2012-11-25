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



#ifdef HAVE_EVIL
static const char *
_find_program_in_path ()
{
  wchar_t wfilename[PATH_MAX];
  char s[PATH_MAX];

  GetModuleFileNameW (NULL, wfilename, PATH_MAX);

  wcstombs(s, wfilename, wcslen(wfilename)+1);
  return strdup(s);
 }
#endif



// Return FriLogos installation directory. useful to get global installation data path.
// @return char pointer containing installation directory.
const char *
edams_install_path_get (void)
{
#ifdef HAVE_EVIL
	char s[MAX_PATH];
	snprintf(s, sizeof(s), ecore_file_dir_get(_find_program_in_path()));
	return strdup(s);
#else
	return PACKAGE_DIR;
#endif
}


// Return FriLogos user data directory. useful to get user data path.
// @return char pointer containing installation directory.
const char *
edams_data_path_get(void)
{
	char s[PATH_MAX];
#ifdef HAVE_EVIL
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA,  NULL,  0, s);
	strcat(s, DIR_SEPARATOR_S"edams"DIR_SEPARATOR_S);
#else
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"data"DIR_SEPARATOR_S"edams", efreet_data_home_get());
#endif
	return strdup(s);
}



const char *
edams_modules_path_get(void)
{
	char s[PATH_MAX];
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"lib"DIR_SEPARATOR_S"edams"DIR_SEPARATOR_S"modules", edams_install_path_get());
	return strdup(s);
}



const char *
edams_locale_path_get(void)
{
#ifdef HAVE_EVIL
	char s[PATH_MAX];
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"share"DIR_SEPARATOR_S"locale", edams_install_path_get());	
	return strdup(s);
#else
	return PACKAGE_LOCALE_DIR;
#endif
}


//
//
//
const char *
edams_sensors_data_path_get (void)
{
    char s[PATH_MAX];
    snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"sensors", edams_data_path_get());
	return strdup(s);
}


//
//
//
const char *
edams_rooms_data_path_get(void)
{
    char s[PATH_MAX];
    snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"rooms", edams_data_path_get());
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
