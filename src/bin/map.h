/*
 * map.h
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


#ifndef __MAP_H
#define __MAP_H

#include <Evas.h>
#include "edams.h"

void map_new(void *data, Evas_Object *obj, void *event_info);
void map_quit();
void map_widget_data_update(App_Info *app, Location *location, Device *device);
void map_location_add(Location *location);
void xpl_control_basic_cmnd_send(Device *device);
#endif
