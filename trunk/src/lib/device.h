/*
 * device.h
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


#ifndef __DEVICE_H
#define __DEVICE_H

#include "location.h"

typedef struct _Device Device;

typedef enum _Type_Flags
{
	UNKNOWN			= (0),
	SENSOR			= (1 << 1),
	COMMAND  		= (1 << 2),
	HUMIDITY		= (1 << 10),
	TEMPERATURE		= (1 << 11),
	PRESSURE		= (1 << 13),
	PIR  			= (1 << 14),
	GAS  			= (1 << 15),
	DISTANCE		= (1 << 16),
	SHOCK  			= (1 << 17),
	FLOW  			= (1 << 18),
	MICROPHONE		= (1 << 19),
	CAMERA			= (1 << 13),
	RELAY  			= (1 << 20),
	TRANSISTOR		= (1 << 21),
	SWITCH 			= (1 << 22),
	IC  			= (1 << 23),
	LED  			= (1 << 24),
	LCD  			= (1 << 25),
	BUZZER			= (1 << 26)
}Type_Flags;


Device *device_new(unsigned int id, const char * name);
void device_free(Device *device);
Device *device_clone(const Device *src);
Device *devices_detect(char *s);
Eina_Bool device_save(Device *device);
Device *device_load(const char *filename);

void device_id_set(Device *device, unsigned int id);
void device_name_set(Device *device, const char * name);
void device_type_set(Device *device, Type_Flags type);
void device_description_set(Device *device, const char * description);
void device_datasheeturl_set(Device *device, const char * datasheeturl);
void device_image_set(Device *device, Evas_Object *image);
void device_creation_set(Device *device, const char * creation);
void device_data_set(Device *device, const char * data);
void device_widget_set(Device *device, const char * widget);
void device_units_set(Device *device, const char *units);
void device_unit_format_set(Device *device, const char *unit_format);
void device_unit_symbol_set(Device *device, const char *unit_symbol);

const char *device_filename_get(Device *device);
unsigned int device_id_get(const Device *device);
const char * device_name_get(const Device *device);
Type_Flags device_type_get(const Device *device);
const char * device_description_get(const Device *device);
const char * device_datasheeturl_get(const Device *device);
const char * device_creation_get(const Device *device);
const char * device_data_get(const Device *device);
const char * device_widget_get(const Device *device);
const char * device_units_get(const Device *device);
const char * device_unit_format_get(const Device *device);
const char * device_unit_symbol_get(const Device *device);

Eina_List *devices_database_list_get();
Eina_List *devices_list_free(Eina_List *devices);
Device* devices_list_device_with_id_get(Eina_List *devices, unsigned int id);
int devices_list_sort_cb(const void *d1, const void *d2);

/* Global initializer / shutdown functions */
void devices_init(void);
void devices_shutdown(void);
#endif
