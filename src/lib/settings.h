/*
 * settings.h
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



#ifndef __SETTINGS_H__
#define __SETTINGS_H__

typedef struct
{
	char *cosm_apikey;
	char *gnuplot_path;
	char *map_background;
	Eina_Bool softemu;
	Eina_Bool debug;
} Settings;

const Settings *edams_settings_get(void);
Settings *edams_settings_free(Settings *settings);
void edams_settings_write(Settings *settings);
#endif /* __SETTINGS_H__ */
