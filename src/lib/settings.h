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

#include <Eet.h>
#include <Ecore_Evas.h>

#include "libedams.h"
#include "path.h"
#include "rooms.h"

//settings.c: Settings functions.
int setting_write(const char *setting, const int value);
int setting_int_read(const char *setting);
int file_setting_write(const char *file, const char *setting,  const int value);
int file_setting_int_read(const char *file, const char *setting);
Eina_Bool setting_bool_read(const char *setting);
const char *edams_settings_file_get(void);

#endif /* __SETTINGS_H__ */
