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

typedef struct _Sensor Sensor;
typedef struct _Room Room;

/* Sensor */
Sensor *sensor_new(unsigned int id, const char * name, const char * type, const char * description, const char * datasheeturl, Evas_Object * picture, const char * soundfile, const char * group, const char * creation);
void sensor_free(Sensor *sensor);

void sensor_id_set(Sensor *sensor, unsigned int id);
unsigned int sensor_id_get(const Sensor *sensor);
void sensor_name_set(Sensor *sensor, const char * name);
const char * sensor_name_get(const Sensor *sensor);
void sensor_type_set(Sensor *sensor, const char * type);
const char * sensor_type_get(const Sensor *sensor);
void sensor_description_set(Sensor *sensor, const char * description);
const char * sensor_description_get(const Sensor *sensor);
void sensor_datasheeturl_set(Sensor *sensor, const char * datasheeturl);
const char * sensor_datasheeturl_get(const Sensor *sensor);
void sensor_picture_set(Sensor *sensor, Evas_Object *picture);
Evas_Object *sensor_picture_get(const Sensor *sensor, Evas *evas, const char *eet_file);
void sensor_soundfile_set(Sensor *sensor, const char * soundfile);
const char * sensor_soundfile_get(const Sensor *sensor);
void sensor_group_set(Sensor *sensor, const char * group);
const char * sensor_group_get(const Sensor *sensor);
void sensor_creation_set(Sensor *sensor, const char * creation);
const char * sensor_creation_get(const Sensor *sensor);

/* Room */
Room *room_new(unsigned int id, const char * name, const char * description, Eina_List * sensors, Evas_Object * photo);
void room_free(Room *room);

const char *room_filename_get(Room *room);
void room_id_set(Room *room, unsigned int id);
unsigned int room_id_get(const Room *room);
void room_name_set(Room *room, const char * name);
const char * room_name_get(const Room *room);
void room_description_set(Room *room, const char * description);
const char * room_description_get(const Room *room);
void room_sensors_add(Room *room, Sensor *sensor);
void room_sensors_del(Room *room, Sensor *sensor);
Sensor *room_sensors_get(const Room *room, unsigned int nth);
unsigned int room_sensors_count(const Room *room);
Eina_List *room_sensors_list_get(const Room *room);
void room_sensors_list_clear(Room *room);
void room_sensors_list_set(Room *room, Eina_List *list);
void room_photo_set(Room *room, Evas_Object *photo);
Evas_Object *room_photo_get(const Room *room, Evas *evas, const char *eet_file);

Room *room_load(Evas *evas, const char *filename);
Eina_Bool room_save(Room *room);

/* Global initializer / shutdown functions */
void rooms_init(void);
void rooms_shutdown(void);

#endif /* __ROOMS_H__ */
