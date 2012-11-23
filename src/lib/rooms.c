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
#include "mem.h"


struct _Sensor {
    unsigned int id;
    const char * name;
    const char * type;
    const char * description;
    const char * datasheeturl;
    Evas_Object * picture;
    unsigned int picture__id;
    const char * soundfile;
    const char * group;
    const char * creation;    
};

struct _Room {
    unsigned int id;
    const char * name;
    const char * description;
    Eina_List * sensors;
    Evas_Object * photo;
    unsigned int photo__id;
    const char *__eet_filename;
    const char * creation;
    const char * revision;
    int version;    
};


static const char SENSOR_ENTRY[] = "sensor";
static const char ROOM_ENTRY[] = "room";

static Eet_Data_Descriptor *_sensor_descriptor = NULL;
static Eet_Data_Descriptor *_room_descriptor = NULL;


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
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%02d%02d%02d-%03d.eet", edams_data_path_get(), (int)t->tm_hour, (int)t->tm_min, (int) t->tm_sec, i);
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
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "picture__id", picture__id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "soundfile", soundfile, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "group", group, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_sensor_descriptor, Sensor, "creation", creation, EET_T_STRING);
}

static inline void
_sensor_shutdown(void)
{
    if (!_sensor_descriptor) return;
    eet_data_descriptor_free(_sensor_descriptor);
    _sensor_descriptor = NULL;
}

Sensor *
sensor_new(unsigned int id, const char * name, const char * type, const char * description, const char * datasheeturl, Evas_Object * picture, const char * soundfile, const char * group, const char * creation)
{
    Sensor *sensor = calloc(1, sizeof(Sensor));

    if (!sensor)
       {
          fprintf(stderr, "ERROR: could not calloc Sensor\n");
          return NULL;
       }

    sensor->id = id;
    sensor->name = eina_stringshare_add(name ? name : "undefined");
    sensor->type = eina_stringshare_add(type ? type : "undefined");
    sensor->description = eina_stringshare_add(description ? description : "undefined");
    sensor->datasheeturl = eina_stringshare_add(datasheeturl ? datasheeturl : "undefined");
    sensor->picture = picture;
    sensor->picture__id = 0;
    sensor->soundfile = eina_stringshare_add(soundfile);
    sensor->group = eina_stringshare_add(group ? group : "undefined");
    sensor->creation = eina_stringshare_add(creation);

    return sensor;
}

void
sensor_free(Sensor *sensor)
{
    eina_stringshare_del(sensor->name);
    eina_stringshare_del(sensor->type);
    eina_stringshare_del(sensor->description);
    eina_stringshare_del(sensor->datasheeturl);
    if (sensor->picture) evas_object_del(sensor->picture);
    eina_stringshare_del(sensor->soundfile);
    eina_stringshare_del(sensor->group);
    eina_stringshare_del(sensor->creation);
    free(sensor);
}

static void
_load_sensor_images(Sensor *sensor, Evas *evas, const char *filename)
{
    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
       {
          fprintf(stderr, "ERROR: could not open '%s' for read!\n", filename);
          return;
       }

    if (sensor->picture__id)
       {
          char picture_buf[256];
          unsigned int picture_w, picture_h;
          int picture_alpha, picture_compress, picture_quality, picture_lossy;
          void *picture_data;
          sprintf(picture_buf, "/image/%d", sensor->picture__id);
          picture_data = eet_data_image_read(ef, picture_buf, &picture_w, &picture_h, &picture_alpha, &picture_compress, &picture_quality, &picture_lossy);
          if (picture_data)
             {
                sensor->picture = evas_object_image_add(evas);
                evas_object_image_size_set(sensor->picture, picture_w, picture_h);
                evas_object_image_alpha_set(sensor->picture, picture_alpha);
                evas_object_image_data_set(sensor->picture, picture_data);
             }
       }

    eet_close(ef);
}

static int
_write_sensor_images(Sensor *sensor, Eet_File *ef, int image_id)
{
    if (sensor->picture)
       {
          char picture_buf[256];
          int picture_w, picture_h;
          int picture_alpha;
          void *picture_data;
          sensor->picture__id = image_id;
          sprintf(picture_buf, "/image/%d", image_id++);
          evas_object_image_size_get(sensor->picture, &picture_w, &picture_h);
          picture_alpha = evas_object_image_alpha_get(sensor->picture);
          picture_data = evas_object_image_data_get(sensor->picture, EINA_FALSE);
          eet_data_image_write(ef, picture_buf, picture_data, picture_w, picture_h, picture_alpha, 1, 95, 0);
       }
    return image_id;
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
    EINA_SAFETY_ON_NULL_RETURN(sensor);
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
sensor_picture_set(Sensor *sensor, Evas_Object *picture)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);
    if (sensor->picture) evas_object_del(sensor->picture);
    sensor->picture__id = 0;
    sensor->picture = picture;
}

Evas_Object *
sensor_picture_get(const Sensor *sensor, Evas *evas, const char *eet_file)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(sensor, NULL);
    if (sensor->picture) return sensor->picture;
    _load_sensor_images(sensor, evas, eet_file);
    return sensor->picture;
}

inline const char *
sensor_soundfile_get(const Sensor *sensor)
{
    return sensor->soundfile;
}

inline void
sensor_soundfile_set(Sensor *sensor, const char *soundfile)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);
    eina_stringshare_replace(&(sensor->soundfile), soundfile);
}

inline const char *
sensor_group_get(const Sensor *sensor)
{
    return sensor->group;
}

inline void
sensor_group_set(Sensor *sensor, const char *group)
{
    EINA_SAFETY_ON_NULL_RETURN(sensor);
    eina_stringshare_replace(&(sensor->group), group);
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
    EET_DATA_DESCRIPTOR_ADD_BASIC(_room_descriptor, Room, "photo__id", photo__id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_room_descriptor, Room, "creation", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_room_descriptor, Room, "revision", revision, EET_T_STRING); 
}

static inline void
_room_shutdown(void)
{
    if (!_room_descriptor) return;
    eet_data_descriptor_free(_room_descriptor);
    _room_descriptor = NULL;
}

Room *
room_new(unsigned int id, const char * name, const char * description, Eina_List * sensors, Evas_Object * photo)
{
	char s[16];

    Room *room = calloc(1, sizeof(Room));

    if (!room)
       {
          fprintf(stderr, "ERROR: could not calloc Room!\n");
          return NULL;
       }

    room->__eet_filename = _filename_create();
    room->id = id;
    room->name = eina_stringshare_add(name ? name : "undefined");
    room->description = eina_stringshare_add(description ? description : "undefined");
    room->sensors = sensors;
    room->photo = photo;
    room->photo__id = -1;

    //Add patient profil's creation informations.
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
	    eina_stringshare_del(room->name);
    	eina_stringshare_del(room->description);
    	if (room->sensors)
       	{
          Sensor *sensors_elem;
          EINA_LIST_FREE(room->sensors, sensors_elem)
             sensor_free(sensors_elem);
       	}
    	if (room->photo) evas_object_del(room->photo);
	    eina_stringshare_del(room->creation);
    	eina_stringshare_del(room->revision);

	}
    FREE(room);
}

static void
_load_room_images(Room *room, Evas *evas, const char *filename)
{
    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
       {
          fprintf(stderr, "ERROR: could not open '%s' for read!\n", filename);
          return;
       }

    if (room->photo__id)
       {
          char photo_buf[256];
          unsigned int photo_w, photo_h;
          int photo_alpha, photo_compress, photo_quality, photo_lossy;
          void *photo_data;
          sprintf(photo_buf, "/image/%d", room->photo__id);
          photo_data = eet_data_image_read(ef, photo_buf, &photo_w, &photo_h, &photo_alpha, &photo_compress, &photo_quality, &photo_lossy);
          if (photo_data)
             {
                room->photo = evas_object_image_add(evas);
                evas_object_image_size_set(room->photo, photo_w, photo_h);
                evas_object_image_alpha_set(room->photo, photo_alpha);
                evas_object_image_data_set(room->photo, photo_data);
             }
       }

    eet_close(ef);
}

static int
_write_room_images(Room *room, Eet_File *ef, int image_id)
{
    if (room->photo)
       {
          char photo_buf[256];
          int photo_w, photo_h;
          int photo_alpha;
          void *photo_data;
          room->photo__id = image_id;
          sprintf(photo_buf, "/image/%d", image_id++);
          evas_object_image_size_get(room->photo, &photo_w, &photo_h);
          photo_alpha = evas_object_image_alpha_get(room->photo);
          photo_data = evas_object_image_data_get(room->photo, EINA_FALSE);
          eet_data_image_write(ef, photo_buf, photo_data, photo_w, photo_h, photo_alpha, 1, 95, 0);
       }
    return image_id;
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
    room->sensors = eina_list_append(room->sensors, sensor);
}

inline void
room_sensors_del(Room *room, Sensor *sensor)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    room->sensors = eina_list_remove(room->sensors, sensor);
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
    return room->sensors;
}

inline void
room_sensors_list_set(Room *room, Eina_List *list)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    room->sensors = list;
}

void
room_photo_set(Room *room, Evas_Object *photo)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    if (room->photo) evas_object_del(room->photo);
    room->photo__id = 0;
    room->photo = photo;
}

Evas_Object *
room_photo_get(const Room *room, Evas *evas, const char *eet_file)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(room, NULL);
    if (room->photo) return room->photo;
    _load_room_images(room, evas, eet_file);
    return room->photo;
}

inline Eina_Bool
room_photo_is_available_get(const Room *room)
{
	if(!room->photo__id)
	    return EINA_FALSE;
	else
    	return EINA_TRUE;
}


inline const char *
room_creation_get(Room *room)
{
    return room->creation;
}

inline void
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

inline void
room_revision_set(Room *room, const char *revision)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    eina_stringshare_replace(&(room->revision), revision);
}


Room *
room_load(Evas *evas, const char *filename)
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
	int i;
	
    ef = eet_open(room->__eet_filename, EET_FILE_MODE_READ_WRITE);
    if (!ef)
       {
          fprintf(stderr, "ERROR: could not open '%s' for write!\n", room->__eet_filename);
          return EINA_FALSE;
       }

    i = 1;
    i = _write_room_images(room, ef, i);
    if (room->sensors)
       {
          Sensor *sensor;
          Eina_List *sensor_list;
          EINA_LIST_FOREACH(room->sensors, sensor_list, sensor)
             i = _write_sensor_images(sensor, ef, i);
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
		Room *room;

        //Point to first node of list.
        for(rooms = eina_list_last(rooms); rooms; rooms = eina_list_prev(rooms));

        EINA_LIST_FREE(rooms, room)
        {
			room_free(room);
			n++;
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

	it = eina_file_stat_ls(edams_data_path_get());

   	if(it)
   	{
	   EINA_ITERATOR_FOREACH(it, f_info)
	   {
            //INF("Found %s new file.", ecore_file_file_get(f_info->path));
			if(eina_str_has_extension(f_info->path, ".eet") == EINA_TRUE)
			{
				room = room_load(NULL, f_info->path);

				if(room)
				{
					room->id = id++;
					rooms = eina_list_append(rooms, room);

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

	fprintf(stdout, _("INFO:%d rooms list found and registered.\n"), eina_list_count(rooms));
	rooms = eina_list_sort(rooms, eina_list_count(rooms), EINA_COMPARE_CB(strcoll));
	return rooms;
}
