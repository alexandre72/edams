/*
 * locations.h
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


#ifndef __LOCATION_H__
#define __LOCATION_H__

#include <Eina.h>
#include <Eet.h>
#include <Evas.h>

typedef struct _Widget Widget;
typedef struct _Location Location;

typedef enum _Exposure_Flags
{
	INDOOR = 1,
	OUTDOOR = 2
}Exposure_Flags;


/* Widget */
Widget *widget_new(const char * name, unsigned int device_id);
void widget_free(Widget *widget);

void widget_name_set(Widget *widget, const char * name);
void widget_device_id_set(Widget *widget, unsigned int device_id);
void widget_position_set(Widget *widget, unsigned int position);

const char * widget_name_get(const Widget *widget);
unsigned int widget_device_id_get(const Widget *widget);
unsigned int widget_position_get(const Widget *widget);

void location_widgets_add(Location *location, Widget *widget);
void location_widgets_del(Location *location, Widget *widget);
Widget *location_widgets_get(const Location *location, unsigned int nth);
unsigned int location_widgets_count(const Location *location);
Eina_List *location_widgets_list_get(const Location *location);
void location_widgets_list_clear(Location *location);
void location_widgets_list_set(Location *location, Eina_List *list);

/* Location */
Location *location_new(unsigned int id, const char * name, const char * description);
void location_free(Location *location);
Location *location_load(const char *filename);
Eina_Bool location_save(Location *location);
Eina_Bool location_remove(Location *location);

void location_id_set(Location *location, unsigned int id);
void location_name_set(Location *location, const char * name);
void location_description_set(Location *location, const char * description);
void location_image_set(Location *location, Evas_Object *image);
void location_exposure_set(Location *location, Exposure_Flags exposure);
void location_latitude_set(Location *location, double latitude);
void location_longitude_set(Location *location, double longitude);
void location_cosm_feed_set(Location *location, const char *cosm_feed);

const char *location_filename_get(Location *location);
unsigned int location_id_get(const Location *location);
const char * location_name_get(const Location *location);
const char * location_description_get(const Location *location);
Exposure_Flags location_exposure_get(const Location *location);
double location_latitude_get(const Location *location);
double location_longitude_get(const Location *location);
const char *location_cosm_feed_get(const Location *location);

Eina_List *locations_list_get();
Eina_List * locations_list_free(Eina_List *locations);

/* Global initializer / shutdown functions */
void locations_init(void);
void locations_shutdown(void);

#endif /* __LOCATION_H__ */
