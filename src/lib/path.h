/*
 * path.h
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


#ifndef __PATH_H
#define __PATH_H__


#include "libedams.h"


//path.c:Paths functions.
const char *edams_install_path_get (void);
const char *edams_data_path_get(void);
const char *edams_modules_path_get(void);
const char *edams_locale_path_get(void);
const char *edams_rooms_data_path_get (void);
const char *edams_sensors_data_path_get (void);
const char *edams_serialin_data_path_get (void);
const char *edams_edje_theme_file_get (void);
const char *edams_theme_path_get(void);
#endif /* __PATH_H__ */
