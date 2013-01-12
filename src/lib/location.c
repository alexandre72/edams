/*
 * locations.c
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


#include <Ecore_File.h>

#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "location.h"
#include "path.h"
#include "utils.h"

struct _Widget
{
    const char * name;				//Widget name associated(edc widget group name) e.g. 'meter/counter'.
    unsigned int device_id;			//Device id associated e.g '12'.
    const char *device_filename;	//Device filename associated e.g 'temperature1.eet'.
    unsigned int position;			//Device widget position in Eina_List.
};


struct _Location
{
    const char *__eet_filename;		//Filename name of location, generated and based on location's name.
    unsigned int id;				//Id of location e.g. '10'.
    const char *name;				//Name of location e.g. 'Child Location'.
    const char *description;		//Description of location e.g.'Child's location'.
	Exposure_Flags exposure;	 	//Whether the location is indoors or outdoors
	double latitude; 				//The latitude of the location.
	double longitude; 				//The longitude of the location.
	double elevation; 				//The elevation of the location.
    const char *creation;			//Creation date of location Eet file.
    const char *revision;			//Revision date of location Eet file.
	unsigned int cosm_feedid;		//The cosm url feed location e.g.g 'http://api.cosm.com/v2/feeds/0001'.
    unsigned int version;			//Version of location Eet file.
    Eina_List * widgets;
};

static const char WIDGET_ENTRY[] = "widget";
static const char LOCATION_ENTRY[] = "location";
static Eet_Data_Descriptor *_location_descriptor = NULL;
static Eet_Data_Descriptor *_widget_descriptor = NULL;



static inline void
_widget_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_widget_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Widget);
    _widget_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "device_id", device_id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "device_filename", device_filename, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "position", position, EET_T_UINT);
}


static inline void
_widget_shutdown(void)
{
    if (!_widget_descriptor) return;
    eet_data_descriptor_free(_widget_descriptor);
    _widget_descriptor = NULL;
}


Widget *
widget_new(const char * name, Device *device)
{
    Widget *widget = calloc(1, sizeof(Widget));

    if (!widget)
       {
          debug(stderr, _("Couldn't calloc Widget struct"));
          return NULL;
       }

    widget->name = eina_stringshare_add(name ? name : "meter/counter");
    widget->device_id = device_id_get(device);
    widget->device_filename = device_filename_get(device);

    return widget;
}



void
widget_free(Widget *widget)
{
    eina_stringshare_del(widget->name);
    eina_stringshare_del(widget->device_filename);
    free(widget);
}

inline const char *
widget_name_get(const Widget *widget)
{
    return widget->name;
}

inline void
widget_name_set(Widget *widget, const char *name)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    eina_stringshare_replace(&(widget->name), name);
}



inline const char *
widget_device_filename_get(const Widget *widget)
{
    return widget->device_filename;
}

inline void
widget_device_filename_set(Widget *widget, const char *filename)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    eina_stringshare_replace(&(widget->device_filename), filename);
}


inline unsigned int
widget_device_id_get(const Widget *widget)
{
    return widget->device_id;
}

inline void
widget_device_id_set(Widget *widget, unsigned int device_id)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    widget->device_id = device_id;
}

inline unsigned int
widget_position_get(const Widget *widget)
{
    return widget->position;
}

inline void
widget_position_set(Widget *widget, unsigned int position)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    widget->position = position;
}

static int
_locations_list_sort_cb(const void *d1, const void *d2)
{
    const char *txt = location_name_get((Location*)d1);
    const char *txt2 = location_name_get((Location*)d2);

    if(!txt) return(1);
    if(!txt2) return(-1);

    return(strcoll(txt, txt2));
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
_location_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_location_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Location);
    _location_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "id", id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "description", description, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_LIST(_location_descriptor, Location, "widgets", widgets, _widget_descriptor);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "exposure", exposure, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "latitude", latitude, EET_T_DOUBLE);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "longitude", longitude, EET_T_DOUBLE);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "creation", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "revision", revision, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "version", version, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "cosm_feedid", cosm_feedid, EET_T_UINT);
}

static inline void
_location_shutdown(void)
{
    if (!_location_descriptor) return;
    eet_data_descriptor_free(_location_descriptor);
    _location_descriptor = NULL;
}

Location *
location_new(unsigned int id, const char * name, const char * description)
{
	char s[PATH_MAX];

    Location *location = calloc(1, sizeof(Location));

    if (!location)
       {
          debug(stderr, _("Couldn't calloc Location struct"));
          return NULL;
       }

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%s", edams_locations_data_path_get(), _filename_create());
    location->__eet_filename = eina_stringshare_add(s);
    location->id = id;
    location->name = eina_stringshare_add(name ? name : "undefined");
    location->description = eina_stringshare_add(description ? description : "undefined");
    location->widgets = NULL;

    //Add creation date informations.
	time_t timestamp = time(NULL);
	struct tm *t = localtime(&timestamp);
	snprintf(s, sizeof(s), "%02d-%02d-%d",
				(int)t->tm_mday,
  				(int)t->tm_mon,
  				1900+(int)t->tm_year);

    location->creation = eina_stringshare_add(s);
	location->revision = NULL;
	location->version = 0x0002;
    location->cosm_feedid = 0;

    return location;
}


const char *
location_filename_get(Location *location)
{
     return location->__eet_filename;
}


void
location_free(Location *location)
{
	if(location)
	{
	    eina_stringshare_del(location->__eet_filename);
	    eina_stringshare_del(location->name);
    	eina_stringshare_del(location->description);
    	if (location->widgets)
       	{
          Widget *widgets_elem;
          EINA_LIST_FREE(location->widgets, widgets_elem)
             widget_free(widgets_elem);
       	}
	    eina_stringshare_del(location->creation);
    	eina_stringshare_del(location->revision);
	    FREE(location);
	}
}


inline unsigned int
location_id_get(const Location *location)
{
    return location->id;
}

inline void
location_id_set(Location *location, unsigned int id)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->id = id;
}



inline const char *
location_name_get(const Location *location)
{
    return  elm_entry_markup_to_utf8(location->name);
}


inline void
location_name_set(Location *location, const char *name)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    eina_stringshare_replace(&(location->name), name);
}

inline const char *
location_description_get(const Location *location)
{
    return location->description;
}

inline void
location_description_set(Location *location, const char *description)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    eina_stringshare_replace(&(location->description), description);
}


inline Exposure_Flags
location_exposure_get(const Location *location)
{
    return location->exposure;
}

inline void
location_exposure_set(Location *location, Exposure_Flags exposure)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->exposure = exposure;
}

inline double
location_latitude_get(const Location *location)
{
    return location->latitude;
}

inline void
location_latitude_set(Location *location, double latitude)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->latitude = latitude;
}


inline double
location_longitude_get(const Location *location)
{
    return location->longitude;
}

inline void
location_longitude_set(Location *location, double longitude)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->longitude = longitude;
}


inline unsigned int
location_cosm_feedid_get(const Location *location)
{
    return location->cosm_feedid;
}

inline void
location_cosm_feedid_set(Location *location, unsigned cosm_feedid)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
	location->cosm_feedid =  cosm_feedid;
}



void
location_image_set(Location *location, Evas_Object *image)
{
    EINA_SAFETY_ON_NULL_RETURN(location);

    Eet_File *ef = eet_open(location->__eet_filename, EET_FILE_MODE_WRITE);
    if (!ef)
      {
        debug(stderr, _("Couldn't open '%s' for writing"), location->__eet_filename);
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
location_creation_get(Location *location)
{
    return location->creation;
}

void
location_creation_set(Location *location, const char *creation)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    eina_stringshare_replace(&(location->creation), creation);
}

inline const char *
location_profil_revision_get(const Location *location)
{
    return location->revision;
}

void
location_revision_set(Location *location, const char *revision)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    eina_stringshare_replace(&(location->revision), revision);
}


inline void
location_widgets_add(Location *location, Widget *widget)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    widget_position_set(widget, eina_list_count(location->widgets)+1);
    location->widgets = eina_list_append(location->widgets, widget);
}

inline void
location_widgets_del(Location *location, Widget *widget)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->widgets = eina_list_remove(location->widgets, widget);
}

inline Widget *
location_widgets_get(const Location *location, unsigned int nth)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(location, NULL);
    return eina_list_nth(location->widgets, nth);
}

inline unsigned int
location_widgets_count(const Location *location)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(location, 0);
    return eina_list_count(location->widgets);
}

void
location_widgets_list_clear(Location *location)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    Widget *data;
    EINA_LIST_FREE(location->widgets, data) widget_free(data);
}

inline Eina_List *
location_widgets_list_get(const Location *location)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(location, NULL);
    return location->widgets;
}

inline void
location_widgets_list_set(Location *location, Eina_List *list)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->widgets = list;
}



Location *
location_load(const char *filename)
{
    Location *location = NULL;

    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
      {
        debug(stderr, _("Couldn't open '%s' for read"), filename);
        return NULL;
      }

    location = eet_data_read(ef, _location_descriptor, LOCATION_ENTRY);
    if (!location) goto end;
    location->__eet_filename = eina_stringshare_add(filename);

	/*FIXME:In future release handle this code's parts by adding newly field(extensions)*/
   	if (location->version < LOCATION_FILE_VERSION)
     	{
        	debug(stderr, _("Eet file '%s' %#x was too old, upgrading it to %#x"),
        			location->__eet_filename,
                	location->version,
                	LOCATION_FILE_VERSION);

        	location->version = LOCATION_FILE_VERSION;
     	}

end:
    eet_close(ef);
    return location;
}



Eina_Bool
location_save(Location *location)
{
    Eet_File *ef;
    Eina_Bool ret;

    ef = eet_open(location->__eet_filename, EET_FILE_MODE_READ_WRITE);
    if (!ef)
	{
		debug(stderr, _("Couldn't open '%s' for writing"), location->__eet_filename);
		return EINA_FALSE;
	}

    ret = !!eet_data_write(ef, _location_descriptor, LOCATION_ENTRY, location, EINA_TRUE);
    eet_close(ef);

	if (!ret)
	{
		debug(stderr, _("Couldn't write any data to Eet file '%s'"), location->__eet_filename);
		return EINA_FALSE;
	}

    return EINA_TRUE;
}



void
locations_init(void)
{
    _widget_init();
    _location_init();
}

void
locations_shutdown(void)
{
    _widget_shutdown();
    _location_shutdown();
}

//
//Remove location informations file.
//
Eina_Bool
location_remove(Location *location)
{
    if(!location)
        return EINA_FALSE;

    //INF(_("Removing:%s"), location->__eet_filename);
	if(ecore_file_remove(location->__eet_filename) == EINA_FALSE)
	{
	    debug(stderr, _("Couldn't remove Eet file '%s'"), location->__eet_filename);
	    return EINA_FALSE;
	}

	return EINA_TRUE;
}



//
//Free locations list.
//
Eina_List *
locations_list_free(Eina_List *locations)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(locations, NULL);

	unsigned int n = 0;
	Location *data;

	EINA_LIST_FREE(locations, data)
	{
		n++;
		location_free(data);
	}
	eina_list_free(locations);

	debug(stdout, _("%d Location struct of Eina_list freed"), n);

    return NULL;
}


//
//Remove 'location' from 'locations' list.
//
Eina_List *
locations_list_location_remove(Eina_List *locations, Location *location)
{
	Eina_List *l;
	Location *data;

 	EINA_LIST_FOREACH(locations, l, data)
   {
		if(data == location)
		{
         	locations = eina_list_remove_list(locations, l);
			break;
		}
   }
   return locations;
}




//
//Read all locations infos files.
//
Eina_List *
locations_list_get()
{
	int id = 0;
	const Eina_File_Direct_Info *f_info;
	Eina_Iterator *it;
	Eina_List *locations = NULL;
	Location *location = NULL;
	char s[PATH_MAX];

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S, edams_locations_data_path_get());
	it = eina_file_stat_ls(s);

   	if(it)
   	{
	   EINA_ITERATOR_FOREACH(it, f_info)
	   {
            //INF("Found %s new file.", ecore_file_file_get(f_info->path));
			if(eina_str_has_extension(f_info->path, ".eet") == EINA_TRUE)
			{
				location = location_load(f_info->path);

				if(location)
				{
					location->id = id++;
					locations = eina_list_append(locations, location);
					if (eina_error_get())
					{
						debug(stderr, _("Couldn't allocate Eina_List node!"));
						exit(-1);
					}
				}
			}
		}

	eina_iterator_free(it);
	}

	debug(stdout, _("%d locations registered"), eina_list_count(locations));
	locations = eina_list_sort(locations, eina_list_count(locations), EINA_COMPARE_CB(_locations_list_sort_cb));
	return locations;
}
