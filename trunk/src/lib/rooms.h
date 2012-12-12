/*
 * rooms.h
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


#ifndef __ROOMS_H__
#define __ROOMS_H__

#include <Eina.h>
#include <Eet.h>
#include <Evas.h>

typedef struct _Device Device;
typedef struct _Room Room;




/* Device */
Device *device_new(unsigned int id, const char * name, const char * type, const char * description, const char * datasheeturl);
void device_free(Device *device);
Device *device_clone(const Device *src);


void device_id_set(Device *device, unsigned int id);
void device_name_set(Device *device, const char * name);
void device_type_set(Device *device, const char * type);
void device_description_set(Device *device, const char * description);
void device_datasheeturl_set(Device *device, const char * datasheeturl);
void device_image_set(Device *device, Evas_Object *image);
void device_creation_set(Device *device, const char * creation);
void device_data_set(Device *device, char * data);
void device_meter_set(Device *device, const char * meter);

const char *device_filename_get(Device *device);
unsigned int device_id_get(const Device *device);
const char * device_name_get(const Device *device);
const char * device_type_get(const Device *device);
const char * device_description_get(const Device *device);
const char * device_datasheeturl_get(const Device *device);
const char * device_creation_get(const Device *device);
const char * device_data_get(const Device *device);
const char * device_meter_get(const Device *device);

Eina_Bool device_save(Device *device);
Device *device_load(const char *filename);

Eina_List *devices_list_get();
Eina_List *devices_list_free(Eina_List *devices);
int devices_list_sort_cb(const void *d1, const void *d2);

/* Room */
Room *room_new(unsigned int id, const char * name, const char * description, Eina_List * devices);
void room_free(Room *room);
void room_devices_add(Room *room, Device *device);
void room_devices_del(Room *room, Device *device);
void room_devices_list_clear(Room *room);
void room_devices_list_set(Room *room, Eina_List *list);

void room_id_set(Room *room, unsigned int id);
void room_name_set(Room *room, const char * name);
void room_description_set(Room *room, const char * description);
unsigned int room_devices_count(const Room *room);
void room_image_set(Room *room, Evas_Object *image);

const char *room_filename_get(Room *room);
unsigned int room_id_get(const Room *room);
const char * room_name_get(const Room *room);
const char * room_description_get(const Room *room);
Device *room_devices_get(const Room *room, unsigned int nth);
Eina_List *room_devices_list_get(const Room *room);

Room *room_load(const char *filename);
Eina_Bool room_save(Room *room);
Eina_Bool room_remove(Room *room);

Eina_List *rooms_list_get();
Eina_List *rooms_list_free(Eina_List *rooms);
Eina_List *rooms_list_room_remove(Eina_List *rooms, Room *room);

/* Global initializer / shutdown functions */
void rooms_init(void);
void rooms_shutdown(void);

#endif /* __ROOMS_H__ */
