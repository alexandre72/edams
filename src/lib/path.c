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
#include "utils.h"

#include <Eina.h>
#include <Efreet.h>



/*
 * Return EDAMS installation directory. useful to get global installation data path.
 */
const char *
edams_install_path_get (void)
{
	return PACKAGE_DIR;
}/*edams_install_path_get*/


/*
 *Return EDAMS user data directory. useful to get user data path.
 */
const char *
edams_data_path_get(void)
{
	char s[PATH_MAX];
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"data"DIR_SEPARATOR_S"edams", efreet_data_home_get());
	return strdup(s);
}/*edams_data_path_get*/

/*
 *
 */
const char *
edams_locale_path_get(void)
{
	return PACKAGE_LOCALE_DIR;
}/*edams_locale_path_get*/


/*
 *
 */
const char *
edams_devices_data_path_get (void)
{
    char s[PATH_MAX];
    snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"devices", edams_data_path_get());
	return strdup(s);
}/*edams_devices_data_path_get*/


/*
 *
 */
const char *
edams_sounds_data_path_get (void)
{
    char s[PATH_MAX];
    snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"sounds", edams_data_path_get());
	return strdup(s);
}/*edams_devices_data_path_get*/

/*
 *
 */
const char *
edams_locations_data_path_get(void)
{
    char s[PATH_MAX];
    snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"locations", edams_data_path_get());
	return strdup(s);
}/*edams_locations_data_path_get*/



/*
 *
 */
const char *
edams_settings_file_get (void)
{
    char s[PATH_MAX];
    snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"settings.eet", edams_data_path_get());
	return strdup(s);
}/*edams_settings_file_get*/


/*
 *
 */
const char *
edams_theme_path_get(void)
{
	char s[PATH_MAX];

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"share"DIR_SEPARATOR_S"edams"DIR_SEPARATOR_S"themes"DIR_SEPARATOR_S"default", edams_install_path_get());
	return strdup(s);
}/*edams_theme_path_get*/

/*
 *
 */
const char *
edams_edje_theme_file_get (void)
{
	char s[PATH_MAX];

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"share"DIR_SEPARATOR_S"edams"DIR_SEPARATOR_S"themes"DIR_SEPARATOR_S"default"DIR_SEPARATOR_S"edams.edj", edams_install_path_get());
	return strdup(s);
}/*edams_edje_theme_file_get*/
