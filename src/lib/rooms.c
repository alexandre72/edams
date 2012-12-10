/*
 * rooms.c
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


#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "rooms.h"
#include "libedams.h"
#include "path.h"

struct _Sensor {
    char *__eet_filename;
    unsigned int id;
    const char * name;
    const char * type;
    const char * description;
    const char * datasheeturl;
    char * data;
	const char * meter;
    const char * creation;
	const char * revision;
    unsigned int version;
};

struct _Room {
    char *__eet_filename;
    unsigned int id;
    const char * name;
    const char * description;
    Eina_List * sensors;
    const char * creation;
    const char * revision;
    unsigned int version;
};


static const char SENSOR_ENTRY[] = "sensor";
static const char ROOM_ENTRY[] = "room";

static Eet_Data_Descriptor *_sensor_descriptor = NULL;
static Eet_Data_Descriptor *_room_descriptor = NULL;


static int
_rooms_list_sort_cb(const void *d1, const void *d2)
{
    const char *txt = room_name_get((Room*)d1);
    const char *txt2 = room_name_get((Room*)d2);

    if(!txt) return(1);
    if(!txt2) return(-1);

    return(strcoll(txt, txt2));
}



int
sensors_list_sort_cb(const void *d1, const void *d2)
{
    unsigned int id1 = sensor_id_get((Sensor *)d1);
    unsigned int id2 = sensor_id_get((Sensor *)d2);

	if(id1 == id2)
		return 0;
	else if(id1 < id2)
		return -1;
	else
		return 1;
}



static char *
_filename_create()
{
    int i = 0;
    char s[PATH_MAX];
    time_t timestamp;
	struct tm *t;

	timestamp = time(NULL);
	t = localtime(&timestamp);

	do
	{
	snprintf(s, sizeof(s), "%02d%02d%02d-%03d.eet", (int)t->tm_hour, (int)t->tm_min, (int) t->tm_sec, i);
		i++;
	} while(ecore_file_exists(s) == EINA_TRUE);

	return strdup(s);
}



static inline void
_sensor_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_sensor_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Sensor);
    _sensor_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "id", id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "type", type, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "description", description, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "datasheeturl", datasheeturl, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "creation", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "revision", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "version", version, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "meter", meter, EET_T_STRING);
}

static inline void
_sensor_shutdown(void)
{
    if (!_sensor_descriptor) return;
    eet_data_descriptor_free(_sensor_descriptor);
    _sensor_descriptor = NULL;
}

Sensor *
sensor_new(unsigned int id, const char * name, const char * type, const char * description, const char * datasheeturl)
{
	char s[PATH_MAX];

    Sensor *sensor = calloc(1, sizeof(Sensor));

    if (!sensor)
       {
          fprintf(stderr, "ERROR: could not calloc Sensor\n");
          return NULL;
       }

    sensor->id = id;
    sensor->name = eina_stringshare_add(name ? name : "undefined");
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%s.eet" , edams_sensors_data_path_get(), sensor->name);
    sensor->__eet_filename = eina_stringshare_add(s);
    sensor->type = eina_stringshare_add(type ? type : "undefined");
    sensor->description = eina_stringshare_add(description ? description : "undefined");
    sensor->datasheeturl = eina_stringshare_add(datasheeturl ? datasheeturl : "undefined");
    sensor->meter = eina_stringshare_add("default");
    sensor->data = NULL;

    //Add creation date informations.
	time_t timestamp = time(NULL);
	struct tm *t = localtime(&timestamp);
	snprintf(s, sizeof(s), "%02d-%02d-%d",
				(int)t->tm_mday,
  				(int)t->tm_mon,
  				1900+(int)t->tm_year);

    sensor->creation = eina_stringshare_add(s);
	sensor->revision = NULL;
	sensor->version = 0x0001;


    return sensor;
}

void
sensor_free(Sensor *sensor)
{
	if(sensor)
	{
	    eina_stringshare_del(sensor->__eet_filename);
    	eina_stringshare_del(sensor->name);
    	eina_stringshare_del(sensor->type);
    	eina_stringshare_del(sensor->description);
    	eina_stringshare_del(sensor->datasheeturl);
    	eina_stringshare_del(sensor->data);
    	eina_stringshare_del(sensor->meter);
    	eina_stringshare_del(sensor->creation);
    	eina_stringshare_del(sensor->revision);
    	FREE(sensor);
    }
}


inline unsigned int
sensor_id_get(const Sensor *sensor)
{
    return sensor->id;
}


inline void
sensor_id_set(Sensor *sensor, unsigned int id)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);
    sensor->id = id;
}

inline const char *
sensor_name_get(const Sensor *sensor)
{
    return sensor->name;
}

inline void
sensor_name_set(Sensor *sensor, const char *name)
{
	char s[PATH_MAX];

    EINA_SAFETY_ON_NULL_RETURN(sensor);

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%s.eet" , edams_sensors_data_path_get(), name);
    eina_stringshare_replace(&(sensor->__eet_filename), s);
	eina_stringshare_replace(&(sensor->name), name);
}

inline const char *
sensor_type_get(const Sensor *sensor)
{
    return sensor->type;
}

inline void
sensor_type_set(Sensor *sensor, const char *type)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);
    eina_stringshare_replace(&(sensor->type), type);
}

inline void
sensor_meter_set(Sensor *sensor, const char *meter)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);
    eina_stringshare_replace(&(sensor->meter), meter);
}


inline const char *
sensor_meter_get(const Sensor *sensor)
{
    return sensor->meter;
}



inline const char *
sensor_description_get(const Sensor *sensor)
{
    return sensor->description;
}

inline void
sensor_description_set(Sensor *sensor, const char *description)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);
    eina_stringshare_replace(&(sensor->description), description);
}

inline const char *
sensor_datasheeturl_get(const Sensor *sensor)
{
    return sensor->datasheeturl;
}

inline void
sensor_datasheeturl_set(Sensor *sensor, const char *datasheeturl)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);
    eina_stringshare_replace(&(sensor->datasheeturl), datasheeturl);
}

void
sensor_image_set(Sensor *sensor, Evas_Object *image)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);

    Eet_File *ef = eet_open(sensor->__eet_filename, EET_FILE_MODE_WRITE);
    if (!ef)
      {
        fprintf(stderr, "ERROR: could not open '%s' for writing!\n", sensor->__eet_filename);
        return;
      }

    int image_w, image_h;
    int image_alpha;
    void *image_data;
    evas_object_image_size_get(image, &image_w, &image_h);
    image_alpha = evas_object_image_alpha_get(image);
    image_data = evas_object_image_data_get(image, EINA_FALSE);
  	eet_data_image_write(ef, "/image/0", image_data, image_w, image_h, image_alpha, 1, 95, 0);
   	eet_close(ef);
}


inline const char *
sensor_creation_get(const Sensor *sensor)
{
    return sensor->creation;
}

inline void
sensor_creation_set(Sensor *sensor, const char *creation)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);
    eina_stringshare_replace(&(sensor->creation), creation);
}


inline const char *
sensor_data_get(const Sensor *sensor)
{
    return sensor->data;
}

inline void
sensor_data_set(Sensor *sensor, char *data)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);

eina_stringshare_replace(&(sensor->data), data);
}


static inline void
_room_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_room_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Room);
    _room_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_room_descriptor, Room, "id", id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_room_descriptor, Room, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_room_descriptor, Room, "description", description, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_LIST(_room_descriptor, Room, "sensors", sensors, _sensor_descriptor);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_room_descriptor, Room, "creation", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_room_descriptor, Room, "revision", revision, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_room_descriptor, Room, "version", version, EET_T_UINT);
}

static inline void
_room_shutdown(void)
{
    if (!_room_descriptor) return;
    eet_data_descriptor_free(_room_descriptor);
    _room_descriptor = NULL;
}

Room *
room_new(unsigned int id, const char * name, const char * description, Eina_List * sensors)
{
	char s[PATH_MAX];

    Room *room = calloc(1, sizeof(Room));

    if (!room)
       {
          fprintf(stderr, "ERROR: could not calloc Room!\n");
          return NULL;
       }

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%s", edams_rooms_data_path_get(), _filename_create());
    room->__eet_filename = eina_stringshare_add(s);
    room->id = id;
    room->name = eina_stringshare_add(name ? name : "undefined");
    room->description = eina_stringshare_add(description ? description : "undefined");
    room->sensors = sensors;

    //Add creation date informations.
	time_t timestamp = time(NULL);
	struct tm *t = localtime(&timestamp);
	snprintf(s, sizeof(s), "%02d-%02d-%d",
				(int)t->tm_mday,
  				(int)t->tm_mon,
  				1900+(int)t->tm_year);

    room->creation = eina_stringshare_add(s);
	room->revision = NULL;
	room->version = 0x0001;

    return room;
}


const char *
room_filename_get(Room *room)
{
     return room->__eet_filename;
}


void
room_free(Room *room)
{
	if(room)
	{
	    eina_stringshare_del(room->__eet_filename);
	    eina_stringshare_del(room->name);
    	eina_stringshare_del(room->description);
    	if (room->sensors)
       	{
          Sensor *sensors_elem;
          EINA_LIST_FREE(room->sensors, sensors_elem)
             sensor_free(sensors_elem);
       	}
	    eina_stringshare_del(room->creation);
    	eina_stringshare_del(room->revision);

	    FREE(room);
	}
}


inline unsigned int
room_id_get(const Room *room)
{
    return room->id;
}

inline void
room_id_set(Room *room, unsigned int id)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    room->id = id;
}

inline const char *
room_name_get(const Room *room)
{
    return room->name;
}


inline void
room_name_set(Room *room, const char *name)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    eina_stringshare_replace(&(room->name), name);
}

inline const char *
room_description_get(const Room *room)
{
    return room->description;
}

inline void
room_description_set(Room *room, const char *description)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    eina_stringshare_replace(&(room->description), description);
}

inline void
room_sensors_add(Room *room, Sensor *sensor)
{
    EINA_SAFETY_ON_NULL_RETURN(room);

    Eina_List *l, *sensors;

	sensors = room_sensors_list_get(room);

	Sensor *data;
	EINA_LIST_FOREACH(sensors, l, data)
	{
		if(sensor_id_get(data) == sensor_id_get(sensor))
		{
			return;
		}
	}

    room->sensors = eina_list_append(room->sensors, sensor);
	room->sensors = eina_list_sort(room->sensors, eina_list_count(room->sensors), EINA_COMPARE_CB(sensors_list_sort_cb));
}

inline void
room_sensors_del(Room *room, Sensor *sensor)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    room->sensors = eina_list_remove(room->sensors, sensor);
	room->sensors = eina_list_sort(room->sensors, eina_list_count(room->sensors), EINA_COMPARE_CB(sensors_list_sort_cb));
}

inline Sensor *
room_sensors_get(const Room *room, unsigned int nth)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(room, NULL);
    return eina_list_nth(room->sensors, nth);
}

inline unsigned int
room_sensors_count(const Room *room)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(room, 0);
    return eina_list_count(room->sensors);
}

void
room_sensors_list_clear(Room *room)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    Sensor *data;
    EINA_LIST_FREE(room->sensors, data) sensor_free(data);
}

inline Eina_List *
room_sensors_list_get(const Room *room)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(room, NULL);

    return eina_list_sort(room->sensors, eina_list_count(room->sensors), EINA_COMPARE_CB(sensors_list_sort_cb));
}

inline void
room_sensors_list_set(Room *room, Eina_List *list)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    room->sensors = list;
}

void
room_image_set(Room *room, Evas_Object *image)
{
    EINA_SAFETY_ON_NULL_RETURN(room);

    Eet_File *ef = eet_open(room->__eet_filename, EET_FILE_MODE_WRITE);
    if (!ef)
      {
        fprintf(stderr, "ERROR: could not open '%s' for writing!\n", room->__eet_filename);
        return;
      }

	int image_w, image_h;
	int image_alpha;
	void *image_data;
	evas_object_image_size_get(image, &image_w, &image_h);
	image_alpha = evas_object_image_alpha_get(image);
	image_data = evas_object_image_data_get(image, EINA_FALSE);
	eet_data_image_write(ef, "/image/0", image_data, image_w, image_h, image_alpha, 1, 95, 0);
	eet_close(ef);
}


inline const char *
room_creation_get(Room *room)
{
    return room->creation;
}

void
room_creation_set(Room *room, const char *creation)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    eina_stringshare_replace(&(room->creation), creation);
}

inline const char *
patient_profil_revision_get(const Room *room)
{
    return room->revision;
}

void
room_revision_set(Room *room, const char *revision)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    eina_stringshare_replace(&(room->revision), revision);
}


Room *
room_load(const char *filename)
{
    Room *room = NULL;

    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
      {
        fprintf(stderr, "ERROR: could not open '%s' for read!\n", filename);
        return NULL;
      }

    room = eet_data_read(ef, _room_descriptor, ROOM_ENTRY);
    if (!room) goto end;
    room->__eet_filename = eina_stringshare_add(filename);

   	if (room->version < 0x0001)
     	{

        	fprintf(stderr,_("Eet file '%s' %#x was too old, upgrading it to %#x!\n"),
        			room->__eet_filename,
                	room->version, 0x0001);

        	room->version = 0x0001;
     	}

end:
    eet_close(ef);
    return room;
}



Eina_Bool
room_save(Room *room)
{
    Eet_File *ef;
    Eina_Bool ret;

    ef = eet_open(room->__eet_filename, EET_FILE_MODE_READ_WRITE);
    if (!ef)
       {
          fprintf(stderr, "ERROR: could not open '%s' for write!\n", room->__eet_filename);
          return EINA_FALSE;
       }

    ret = !!eet_data_write(ef, _room_descriptor, ROOM_ENTRY, room, EINA_TRUE);
    eet_close(ef);

    return ret;
}



//
//Remove room informations file.
//
Eina_Bool
room_remove(Room *room)
{
    if(!room)
        return EINA_FALSE;

    	//INF(_("Removing:%s"), item->filename);
	if(ecore_file_remove(room->__eet_filename) == EINA_FALSE)
	{
	    fprintf(stderr, _("Can't remove Eet file:'%s'!"), room->__eet_filename);
	    return EINA_FALSE;
	}

	return EINA_TRUE;
}


void
rooms_init(void)
{
    _sensor_init();
    _room_init();
}

void
rooms_shutdown(void)
{
    _sensor_shutdown();
    _room_shutdown();
}



//
//Free rooms list.
//
Eina_List *rooms_list_free(Eina_List *rooms)
{
	if(rooms)
	{
		unsigned int n = 0;
		Room *data;

    	EINA_LIST_FREE(rooms, data)
    	{
    		n++;
	    	room_free(data);
	    }
        eina_list_free(rooms);

        fprintf(stdout, _("INFO:%d rooms list freed.\n"), n);
        return NULL;
    }

    return NULL;
}


//
//Remove 'room' from 'rooms' list.
//
Eina_List *
rooms_list_room_remove(Eina_List *rooms, Room *room)
{
	Eina_List *l;
	Room *data;

 	EINA_LIST_FOREACH(rooms, l, data)
   {
		if(data == room)
		{
         	rooms = eina_list_remove_list(rooms, l);
			break;
		}
   }
   return rooms;
}




//
//Read all rooms infos files.
//
Eina_List *rooms_list_get()
{
	int id = 0;
	const Eina_File_Direct_Info *f_info;
	Eina_Iterator *it;
	Eina_List *rooms = NULL;
	Room *room = NULL;
	char s[PATH_MAX];

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S, edams_rooms_data_path_get());
	fprintf(stdout, "OPENING:%s\n", s);
	it = eina_file_stat_ls(s);

   	if(it)
   	{
	   EINA_ITERATOR_FOREACH(it, f_info)
	   {
            //INF("Found %s new file.", ecore_file_file_get(f_info->path));
			if(eina_str_has_extension(f_info->path, ".eet") == EINA_TRUE)
			{
				room = room_load(f_info->path);

				if(room)
				{
					room->id = id++;
					rooms = eina_list_append(rooms, room);
		            //fprintf(stdout, _("INFO:Found new '%s' Eet room file.\n"), ecore_file_file_get(f_info->path));
					if (eina_error_get())
					{
						fprintf(stderr, _("Can't allocate list node!"));
						exit(-1);
					}
				}
			}
		}

	eina_iterator_free(it);
	}

	//fprintf(stdout, _("INFO:%d rooms found in database.\n"), eina_list_count(rooms));
	rooms = eina_list_sort(rooms, eina_list_count(rooms), EINA_COMPARE_CB(_rooms_list_sort_cb));
	return rooms;
}


const char *
sensor_filename_get(Sensor *sensor)
{
     return sensor->__eet_filename;
}


Eina_Bool
sensor_save(Sensor *sensor)
{
    Eet_File *ef;
    Eina_Bool ret;

    ef = eet_open(sensor->__eet_filename, EET_FILE_MODE_READ_WRITE);
    if (!ef)
       {
          fprintf(stderr, "ERROR: could not open '%s' for write!\n", sensor->__eet_filename);
          return EINA_FALSE;
       }

    ret = !!eet_data_write(ef, _sensor_descriptor, SENSOR_ENTRY, sensor, EINA_TRUE);
    eet_close(ef);

    return ret;
}


Sensor *
sensor_load(const char *filename)
{
    Sensor *sensor = NULL;

    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
      {
        fprintf(stderr, "ERROR: could not open '%s' for read!\n", filename);
        return NULL;
      }

    sensor = eet_data_read(ef, _sensor_descriptor, SENSOR_ENTRY);
    if (!sensor) goto end;
    sensor->__eet_filename = eina_stringshare_add(filename);

   	if (sensor->version < 0x0001)
     	{

        	fprintf(stderr,_("Eet file '%s' %#x was too old, upgrading it to %#x!\n"),
        			sensor->__eet_filename,
                	sensor->version, 0x0001);

        	sensor->version = 0x0001;
     	}

end:
    eet_close(ef);
    return sensor;
}


//
//Read all sensors infos files.
//
Eina_List *
sensors_list_get()
{
	const Eina_File_Direct_Info *f_info;
	Eina_Iterator *it;
	Eina_List *sensors = NULL;
	Sensor *sensor = NULL;

	it = eina_file_stat_ls(edams_sensors_data_path_get());

   	if(it)
   	{
	   EINA_ITERATOR_FOREACH(it, f_info)
	   {
			if(eina_str_has_extension(f_info->path, ".eet") == EINA_TRUE)
			{
				sensor = sensor_load(f_info->path);

				if(sensor)
				{
		            //fprintf(stdout, _("INFO:Found new '%s' Eet database sensor file.\n"), ecore_file_file_get(f_info->path));

					sensors = eina_list_append(sensors, sensor);

					if (eina_error_get())
					{
						fprintf(stderr, _("Can't allocate list node!"));
						exit(-1);
					}
				}
			}
		}

	eina_iterator_free(it);
	}

	//fprintf(stdout, _("INFO:%d sensors found in database.\n"), eina_list_count(sensors));
	sensors = eina_list_sort(sensors, eina_list_count(sensors), EINA_COMPARE_CB(sensors_list_sort_cb));
	return sensors;
}


//
//Free sensors list.
//
Eina_List *
sensors_list_free(Eina_List *sensors)
{

	if(sensors)
	{
		unsigned int n = 0;
		Sensor *data;

    	EINA_LIST_FREE(sensors, data)
    	{
			n++;
	    	sensor_free(data);
		}
        eina_list_free(sensors);

        fprintf(stdout, _("INFO:%d sensors list freed.\n"),n);
        return NULL;
    }

    return NULL;
}



Sensor *
sensor_clone(const Sensor *src)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(src, NULL);

    Sensor *dst = calloc(1, sizeof(Sensor));
    if (!dst)
       {
          fprintf(stderr, "ERROR: could not calloc Sensor\n");
          return NULL;
       }

    dst->__eet_filename = eina_stringshare_add(src->__eet_filename);
    dst->id = src->id;
    dst->name = eina_stringshare_add(src->name);
    dst->type = eina_stringshare_add(src->type);
    dst->description = eina_stringshare_add(src->description);
    dst->datasheeturl = eina_stringshare_add(src->datasheeturl);
    dst->data = eina_stringshare_add(src->data);
    dst->meter = eina_stringshare_add(src->meter);
    dst->creation = eina_stringshare_add(src->creation);
    dst->revision = eina_stringshare_add(src->revision);
    dst->version = src->version;

	return dst;
}
