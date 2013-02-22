/*
 * global_view.h
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


#ifndef __GLOBALVIEW_H
#define __GLOBALVIEW_H

#include "location.h"

void global_view_new(void *data, Evas_Object *obj, void *event_info);
void global_view_quit();

void global_view_location_add(Location *location);
void global_view_location_del(Location *location);

void global_view_widget_data_update(Location *location, Widget *widget);
void global_view_new_mail_emit(int num_new, int num_total);
void global_view_osd_write(const char *text, int delay);

static const int TEMP_MIN = -30;
static const int TEMP_MAX = 50;

#endif
