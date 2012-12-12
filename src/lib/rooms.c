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

struct _Device {
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
    Eina_List * devices;
    const char * creation;
    const char * revision;
    unsigned int version;
};


static const char SENSOR_ENTRY[] = "device";
static const char ROOM_ENTRY[] = "room";

static Eet_Data_Descriptor *_device_descriptor = NULL;
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
devices_list_sort_cb(const void *d1, const void *d2)
{
    unsigned int id1 = device_id_get((Device *)d1);
    unsigned int id2 = device_id_get((Device *)d2);

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
_device_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_device_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Device);
    _device_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "id", id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "type", type, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "description", description, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "datasheeturl", datasheeturl, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "creation", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "revision", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "version", version, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "meter", meter, EET_T_STRING);
}

static inline void
_device_shutdown(void)
{
    if (!_device_descriptor) return;
    eet_data_descriptor_free(_device_descriptor);
    _device_descriptor = NULL;
}

Device *
device_new(unsigned int id, const char * name, const char * type, const char * description, const char * datasheeturl)
{
	char s[PATH_MAX];

    Device *device = calloc(1, sizeof(Device));

    if (!device)
       {
          fprintf(stderr, "ERROR: could not calloc Device\n");
          return NULL;
       }

    device->id = id;
    device->name = eina_stringshare_add(name ? name : "undefined");
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%s.eet" , edams_devices_data_path_get(), device->name);
    device->__eet_filename = eina_stringshare_add(s);
    device->type = eina_stringshare_add(type ? type : "undefined");
    device->description = eina_stringshare_add(description ? description : "undefined");
    device->datasheeturl = eina_stringshare_add(datasheeturl ? datasheeturl : "undefined");
    device->meter = eina_stringshare_add("default");
    device->data = NULL;

    //Add creation date informations.
	time_t timestamp = time(NULL);
	struct tm *t = localtime(&timestamp);
	snprintf(s, sizeof(s), "%02d-%02d-%d",
				(int)t->tm_mday,
  				(int)t->tm_mon,
  				1900+(int)t->tm_year);

    device->creation = eina_stringshare_add(s);
	device->revision = NULL;
	device->version = 0x0001;


    return device;
}

void
device_free(Device *device)
{
	if(device)
	{
	    eina_stringshare_del(device->__eet_filename);
    	eina_stringshare_del(device->name);
    	eina_stringshare_del(device->type);
    	eina_stringshare_del(device->description);
    	eina_stringshare_del(device->datasheeturl);
    	eina_stringshare_del(device->data);
    	eina_stringshare_del(device->meter);
    	eina_stringshare_del(device->creation);
    	eina_stringshare_del(device->revision);
    	FREE(device);
    }
}


inline unsigned int
device_id_get(const Device *device)
{
    return device->id;
}


inline void
device_id_set(Device *device, unsigned int id)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    device->id = id;
}

inline const char *
device_name_get(const Device *device)
{
    return elm_entry_markup_to_utf8(device->name);
}

inline void
device_name_set(Device *device, const char *name)
{
	char s[PATH_MAX];

    EINA_SAFETY_ON_NULL_RETURN(device);

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%s.eet" , edams_devices_data_path_get(), name);
    eina_stringshare_replace(&(device->__eet_filename), s);
	eina_stringshare_replace(&(device->name), name);
}

inline const char *
device_type_get(const Device *device)
{
    return device->type;
}

inline void
device_type_set(Device *device, const char *type)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    eina_stringshare_replace(&(device->type), type);
}

inline void
device_meter_set(Device *device, const char *meter)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    eina_stringshare_replace(&(device->meter), meter);
}


inline const char *
device_meter_get(const Device *device)
{
    return device->meter;
}



inline const char *
device_description_get(const Device *device)
{
    return device->description;
}

inline void
device_description_set(Device *device, const char *description)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    eina_stringshare_replace(&(device->description), description);
}

inline const char *
device_datasheeturl_get(const Device *device)
{
    return device->datasheeturl;
}

inline void
device_datasheeturl_set(Device *device, const char *datasheeturl)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    eina_stringshare_replace(&(device->datasheeturl), datasheeturl);
}

void
device_image_set(Device *device, Evas_Object *image)
{
    EINA_SAFETY_ON_NULL_RETURN(device);

    Eet_File *ef = eet_open(device->__eet_filename, EET_FILE_MODE_WRITE);
    if (!ef)
      {
        fprintf(stderr, "ERROR: could not open '%s' for writing!\n", device->__eet_filename);
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
device_creation_get(const Device *device)
{
    return device->creation;
}

inline void
device_creation_set(Device *device, const char *creation)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    eina_stringshare_replace(&(device->creation), creation);
}


inline const char *
device_data_get(const Device *device)
{
    return device->data;
}

inline void
device_data_set(Device *device, char *data)
{
    EINA_SAFETY_ON_NULL_RETURN(device);

eina_stringshare_replace(&(device->data), data);
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
    EET_DATA_DESCRIPTOR_ADD_LIST(_room_descriptor, Room, "devices", devices, _device_descriptor);
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
room_new(unsigned int id, const char * name, const char * description, Eina_List * devices)
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
    room->devices = devices;

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
    	if (room->devices)
       	{
          Device *devices_elem;
          EINA_LIST_FREE(room->devices, devices_elem)
             device_free(devices_elem);
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
    return  elm_entry_markup_to_utf8(room->name);
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
room_devices_add(Room *room, Device *device)
{
    EINA_SAFETY_ON_NULL_RETURN(room);

    Eina_List *l, *devices;

	devices = room_devices_list_get(room);

	Device *data;
	EINA_LIST_FOREACH(devices, l, data)
	{
		if(device_id_get(data) == device_id_get(device))
		{
			return;
		}
	}

    room->devices = eina_list_append(room->devices, device);
	room->devices = eina_list_sort(room->devices, eina_list_count(room->devices), EINA_COMPARE_CB(devices_list_sort_cb));
}

inline void
room_devices_del(Room *room, Device *device)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    room->devices = eina_list_remove(room->devices, device);
	room->devices = eina_list_sort(room->devices, eina_list_count(room->devices), EINA_COMPARE_CB(devices_list_sort_cb));
}

inline Device *
room_devices_get(const Room *room, unsigned int nth)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(room, NULL);
    return eina_list_nth(room->devices, nth);
}

inline unsigned int
room_devices_count(const Room *room)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(room, 0);
    return eina_list_count(room->devices);
}

void
room_devices_list_clear(Room *room)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    Device *data;
    EINA_LIST_FREE(room->devices, data) device_free(data);
}

inline Eina_List *
room_devices_list_get(const Room *room)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(room, NULL);

    return eina_list_sort(room->devices, eina_list_count(room->devices), EINA_COMPARE_CB(devices_list_sort_cb));
}

inline void
room_devices_list_set(Room *room, Eina_List *list)
{
    EINA_SAFETY_ON_NULL_RETURN(room);
    room->devices = list;
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
    _device_init();
    _room_init();
}

void
rooms_shutdown(void)
{
    _device_shutdown();
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
device_filename_get(Device *device)
{
     return device->__eet_filename;
}


Eina_Bool
device_save(Device *device)
{
    Eet_File *ef;
    Eina_Bool ret;

    ef = eet_open(device->__eet_filename, EET_FILE_MODE_READ_WRITE);
    if (!ef)
       {
          fprintf(stderr, "ERROR: could not open '%s' for write!\n", device->__eet_filename);
          return EINA_FALSE;
       }

    ret = !!eet_data_write(ef, _device_descriptor, SENSOR_ENTRY, device, EINA_TRUE);
    eet_close(ef);

    return ret;
}


Device *
device_load(const char *filename)
{
    Device *device = NULL;

    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
      {
        fprintf(stderr, "ERROR: could not open '%s' for read!\n", filename);
        return NULL;
      }

    device = eet_data_read(ef, _device_descriptor, SENSOR_ENTRY);
    if (!device) goto end;
    device->__eet_filename = eina_stringshare_add(filename);

   	if (device->version < 0x0001)
     	{

        	fprintf(stderr,_("Eet file '%s' %#x was too old, upgrading it to %#x!\n"),
        			device->__eet_filename,
                	device->version, 0x0001);

        	device->version = 0x0001;
     	}

end:
    eet_close(ef);
    return device;
}


//
//Read all devices infos files.
//
Eina_List *
devices_list_get()
{
	const Eina_File_Direct_Info *f_info;
	Eina_Iterator *it;
	Eina_List *devices = NULL;
	Device *device = NULL;

	it = eina_file_stat_ls(edams_devices_data_path_get());

   	if(it)
   	{
	   EINA_ITERATOR_FOREACH(it, f_info)
	   {
			if(eina_str_has_extension(f_info->path, ".eet") == EINA_TRUE)
			{
				device = device_load(f_info->path);

				if(device)
				{
		            //fprintf(stdout, _("INFO:Found new '%s' Eet database device file.\n"), ecore_file_file_get(f_info->path));

					devices = eina_list_append(devices, device);

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

	//fprintf(stdout, _("INFO:%d devices found in database.\n"), eina_list_count(devices));
	devices = eina_list_sort(devices, eina_list_count(devices), EINA_COMPARE_CB(devices_list_sort_cb));
	return devices;
}


//
//Free devices list.
//
Eina_List *
devices_list_free(Eina_List *devices)
{

	if(devices)
	{
		unsigned int n = 0;
		Device *data;

    	EINA_LIST_FREE(devices, data)
    	{
			n++;
	    	device_free(data);
		}
        eina_list_free(devices);

        fprintf(stdout, _("INFO:%d devices list freed.\n"),n);
        return NULL;
    }

    return NULL;
}



Device *
device_clone(const Device *src)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(src, NULL);

    Device *dst = calloc(1, sizeof(Device));
    if (!dst)
       {
          fprintf(stderr, "ERROR: could not calloc Device\n");
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
