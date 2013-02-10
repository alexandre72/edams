/*
 * devices.c
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

#include <Ecore.h>
#include <Elementary.h>

#include "device.h"
#include "edams.h"
#include "path.h"
#include "utils.h"


struct _Action
{
	Condition ifcondition;			//Type of condition(=,>,<,<=,>=).
	const char *ifvalue;			//Required condition value, data are converted ton int(atoi like).
	Action_Type type;				//Type of action(exec, mail, cmnd...).
	const char *data;				//Action passed to action separated by comma, depends on action type.
};


struct _Device
{
    const char *__eet_filename;		//Filename name of device, generated and based on device's name.
    const char *name;				//Name of xpl device 'temperature1'. Should be unique.
    Device_Class class;				//Class of xpl device e.g. 'SENSOR_BASIC_CLASS'.
    Device_Type type;				//Type of xpl device e.g. 'TEMP_SENSOR_BASIC_TYPE'.
    const char *description;		//Description of device e.g.'I2C sensor'.
    const char *current;			//Current data of xpl device.
    const char *data1;				//Additional data. Used for control.basic cmnd structure message.
	const char *units;  			//The units of the device e.g. 'Celsius'.
	const char *unit_symbol;		//The symbol of the unit e.g. 'C'.
    const char *creation;			//Creation date of device Eet file.
	const char *revision;			//Revision date of device Eet file.
    unsigned int version;			//Version of device Eet file.
	Eina_List *actions;
};

static const char ACTION_ENTRY[] = "action";
static const char DEVICE_ENTRY[] = "device";
static Eet_Data_Descriptor *_device_descriptor = NULL;
static Eet_Data_Descriptor *_action_descriptor = NULL;


static int
devices_list_sort_cb(const void *d1, const void *d2)
{
    const char *txt = device_name_get((Device *)d1);
    const char *txt2 = device_name_get((Device *)d2);

    if(!txt) return(1);
    if(!txt2) return(-1);

    return(strcoll(txt, txt2));
}


static inline void
_action_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_action_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Action);
    _action_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_action_descriptor, Action, "ifcondition", ifcondition, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_action_descriptor, Action, "ifvalue", ifvalue, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_action_descriptor, Action, "type", type, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_action_descriptor, Action, "data", data, EET_T_STRING);
}


static inline void
_action_shutdown(void)
{
    if (!_action_descriptor) return;
    eet_data_descriptor_free(_action_descriptor);
    _action_descriptor = NULL;
}


Action *
action_new(Condition ifcondition, const char *ifvalue, Action_Type type, const char *data)
{
    Action *action = calloc(1, sizeof(Action));

    if (!action)
       {
          debug(stderr, _("Can't calloc Action struct"));
          return NULL;
       }

    action->ifcondition = ifcondition;
    action->ifvalue = eina_stringshare_add(ifvalue);
    action->type = type;
    action->data = eina_stringshare_add(data);

    return action;
}


void
action_free(Action *action)
{
    eina_stringshare_del(action->ifvalue);
    eina_stringshare_del(action->data);
    free(action);
}


inline void
action_ifcondition_set(Action *action, Condition ifcondition)
{
    EINA_SAFETY_ON_NULL_RETURN(action);
	action->ifcondition = ifcondition;
}


inline Condition
action_ifcondition_get(const Action *action)
{
    return action->ifcondition;
}



void
action_ifvalue_set(const Action *action, const char *ifvalue)
{
    EINA_SAFETY_ON_NULL_RETURN(action);
    eina_stringshare_replace(&(action->ifvalue), ifvalue);
}


inline const char *
action_ifvalue_get(const Action *action)
{
    return action->ifvalue;
}



inline void
action_type_set(Action *action, Action_Type type)
{
    EINA_SAFETY_ON_NULL_RETURN(action);
	action->type = type;
}


inline Action_Type
action_type_get(const Action *action)
{
    return action->type;
}



void
action_data_set(const Action *action, const char *data)
{
    EINA_SAFETY_ON_NULL_RETURN(action);
    eina_stringshare_replace(&(action->data), data);
}


inline const char *
action_data_get(const Action *action)
{
    return action->data;
}



static inline void
_device_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_device_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Device);
    _device_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "class", class, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "type", type, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "current", current, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "data1", data1, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "description", description, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "creation", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "revision", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "version", version, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "units", units, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "unit_symbol", unit_symbol, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_LIST(_device_descriptor, Device, "actions", actions, _action_descriptor);
}

static inline void
_device_shutdown(void)
{
    if (!_device_descriptor) return;
    eet_data_descriptor_free(_device_descriptor);
    _device_descriptor = NULL;
}


void
devices_init(void)
{
	_action_init();
    _device_init();
}

void
devices_shutdown(void)
{
	_action_shutdown();
    _device_shutdown();
}

Device *
device_new(const char * name)
{
	char *s;

	asprintf(&s, "%s"DIR_SEPARATOR_S"%s.eet" , edams_devices_data_path_get(), name);
	//TODO:handle case when device name already exist, can be an, updated device type?
	if(ecore_file_exists(s))
	{
		debug(stderr, _("Eet file '%s' already exist. Device name should be unique into your xpl network or maybe have you  replaced your xPL device with new one(with another type, class...)?"), s);
		return NULL;
	}

    Device *device = calloc(1, sizeof(Device));

    if (!device)
	{
		debug(stderr, _("Can't calloc Device struct"));
		return NULL;
	}

    device->name = eina_stringshare_add(name ? name : "undefined");
    device->__eet_filename = eina_stringshare_add(s);
    FREE(s);
    device->class = UNKNOWN_DEVICE_CLASS;
    device->type = UNKNOWN_DEVICE_TYPE;
    device->description = NULL;
    device->current = NULL;
    device->data1 = NULL;
    device->units =NULL;
    device->unit_symbol = NULL;

    //Add creation date informations.
	time_t timestamp = time(NULL);
	struct tm *t = localtime(&timestamp);
	asprintf(&s, "%02d-%02d-%d",
				(int)t->tm_mday,
  				(int)t->tm_mon,
  				1900+(int)t->tm_year);

    device->creation = eina_stringshare_add(s);
    FREE(s);
	device->revision = NULL;
	device->version = 0x0002;
	device->actions = NULL;

    return device;
}

void
device_free(Device *device)
{
	if(device)
	{
	    eina_stringshare_del(device->__eet_filename);
    	eina_stringshare_del(device->name);
    	eina_stringshare_del(device->description);
    	eina_stringshare_del(device->current);
    	eina_stringshare_del(device->data1);
    	eina_stringshare_del(device->units);
    	eina_stringshare_del(device->unit_symbol);
    	eina_stringshare_del(device->creation);
    	eina_stringshare_del(device->revision);

    	if (device->actions)
       	{
          Action *action_elem;
          EINA_LIST_FREE(device->actions, action_elem)
             action_free(action_elem);
       	}
    	FREE(device);
    }
}


inline const char *
device_name_get(const Device *device)
{
    return elm_entry_markup_to_utf8(device->name);
}

inline void
device_name_set(Device *device, const char *name)
{
	int i = 0;
	char *s;

    EINA_SAFETY_ON_NULL_RETURN(device);
	asprintf(&s, "%s"DIR_SEPARATOR_S"%s.eet", edams_devices_data_path_get(), name);

	while(ecore_file_exists(s) == EINA_TRUE)
	{
		asprintf(&s, "%s"DIR_SEPARATOR_S"%s-%04d.eet" , edams_devices_data_path_get(), name, i);
		i++;
	}

    eina_stringshare_replace(&(device->__eet_filename), s);
    FREE(s);
	eina_stringshare_replace(&(device->name), name);
}

inline Device_Class
device_class_get(const Device *device)
{
    return device->class;
}


inline void
device_class_set(Device *device, const Device_Class class)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    device->class = class;
}


inline Device_Type
device_type_get(const Device *device)
{
    return device->type;
}


inline void
device_units_set(Device *device, const char *units)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    eina_stringshare_replace(&(device->units), units);
}


inline const char *
device_units_get(const Device *device)
{
    return device->units;
}


inline void
device_unit_symbol_set(Device *device, const char *unit_symbol)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    eina_stringshare_replace(&(device->unit_symbol), unit_symbol);
}


inline const char *
device_unit_symbol_get(const Device *device)
{
    return device->unit_symbol;
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


void
device_image_set(Device *device, Evas_Object *image)
{
    EINA_SAFETY_ON_NULL_RETURN(device);

    Eet_File *ef = eet_open(device->__eet_filename, EET_FILE_MODE_WRITE);
    if (!ef)
      {
        debug(stderr, _("Can't open Eet file '%s' in write mode"), device->__eet_filename);
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
device_current_get(const Device *device)
{
    return device->current;
}

inline void
device_current_set(Device *device, const char *data)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
	eina_stringshare_replace(&(device->current), data);
}


int
device_current_to_int(Device *device)
{
    if(!device || !device->current) return -1;

	int ret = atoi(device->current);

	switch(device->type)
	{
		default:
		case UNKNOWN_DEVICE_TYPE:
		case BATTERY_SENSOR_BASIC_TYPE:
		case COUNT_SENSOR_BASIC_TYPE:
		case CURRENT_SENSOR_BASIC_TYPE:
		case DISTANCE_SENSOR_BASIC_TYPE:
		case ENERGY_SENSOR_BASIC_TYPE:
		case FAN_SENSOR_BASIC_TYPE:
		case GENERIC_SENSOR_BASIC_TYPE:
		case POWER_SENSOR_BASIC_TYPE:
		case PRESSURE_SENSOR_BASIC_TYPE:
		case SETPOINT_SENSOR_BASIC_TYPE:
		case SPEED_SENSOR_BASIC_TYPE:
		case TEMP_SENSOR_BASIC_TYPE:
		case UV_SENSOR_BASIC_TYPE:
		case VOLTAGE_SENSOR_BASIC_TYPE:
		case VOLUME_SENSOR_BASIC_TYPE:
		case WEIGHT_SENSOR_BASIC_TYPE:
						return ret;
						break;

		case DIRECTION_SENSOR_BASIC_TYPE:

					if(ret > 360 || ret < 0)
					{
						debug(stderr, _("Device '%s' received data but it seems to be misformatted."), device->name);
						return -1;
					}
					else
						return ret;
					break;

		case HUMIDITY_SENSOR_BASIC_TYPE:
					if(ret > 100 || ret < 0)
					{
						debug(stderr, _("Device '%s' received data but it seems to be misformatted."), device->name);
						return -1;
					}
					else
						return ret;
					break;

		case INPUT_SENSOR_BASIC_TYPE:
		case OUTPUT_SENSOR_BASIC_TYPE:
					if((strcmp(device->current, "HIGH") == 0) || (strcmp(device->current, "PULSE") == 0))
					{
						return 1;
					}
					else if((strcmp(device->current, "LOW") == 0))
					{
						return 0;
					}
					else
					{
						debug(stderr, _("Device '%s' received data but it seems to be misformatted."), device->name);
						return -1;
					}
					break;
	}

	return ret;
}



inline const char *
device_data1_get(const Device *device)
{
    return device->data1;
}

inline void
device_data1_set(Device *device, const char *data1)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
	eina_stringshare_replace(&(device->data1), data1);
}


const char *
device_filename_get(Device *device)
{
     return device->__eet_filename;
}


inline void
device_action_add(Device *device, Action *action)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    device->actions = eina_list_append(device->actions, action);
}

inline void
device_action_del(Device *device, Action *action)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    device->actions = eina_list_remove(device->actions, action);
}


inline Eina_List *
device_actions_list_get(const Device *device)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(device, NULL);
    return device->actions;
}


inline void
device_actions_list_set(Device *device, Eina_List *list)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    device->actions = list;
}



/*
 *Return EINA_TRUE if 'device' arg is correctly saved.
 */
Eina_Bool
device_save(Device *device)
{
    Eet_File *ef;
    Eina_Bool ret;

    ef = eet_open(device->__eet_filename, EET_FILE_MODE_WRITE);
    if (!ef)
	{
		debug(stderr, _("Can't open Eet file '%s' in write mode"), device->__eet_filename);
		return EINA_FALSE;
	}

    ret = !!eet_data_write(ef, _device_descriptor, DEVICE_ENTRY, device, EINA_TRUE);
    eet_close(ef);

	if (!ret)
	{
		debug(stderr, _("Can't write any data to Eet file '%s'"), device->__eet_filename);
		return EINA_FALSE;
	}

    return EINA_TRUE;
}


/*
 *Return a Device struct allocated read from a Eet file 'filename' arg
 */
Device *
device_load(const char *filename)
{
    Device *device = NULL;

    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
	{
		debug(stderr, _("Can't open Eet file '%s' in read mode"), filename);
        return NULL;
	}

    device = eet_data_read(ef, _device_descriptor, DEVICE_ENTRY);
    if (!device) goto end;
    device->__eet_filename = eina_stringshare_add(filename);

   	if (device->version < 0x0001)
     	{
        	debug(stderr, _("Eet file '%s' %#x was too old, upgrading it to %#x"),
        			device->__eet_filename,
                	device->version,
                	DEVICE_FILE_VERSION);

        	device->version = DEVICE_FILE_VERSION;
     	}

end:
    eet_close(ef);
    return device;
}


/*
 *Return Eina_List of all Eet devices read in user's EDAMS data path.
 */
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
				if((device = device_load(f_info->path)))
				{
					devices = eina_list_append(devices, device);

					if (eina_error_get())
					{
						debug(stderr, _("Can't alloc Eina_List node"));
						exit(-1);
					}
				}
			}
		}
		eina_iterator_free(it);
	}

	debug(stdout, _("%d devices registered"), eina_list_count(devices));
	devices = eina_list_sort(devices, eina_list_count(devices), EINA_COMPARE_CB(devices_list_sort_cb));
	return devices;
}


/*
 *Return freed Eina_List 'devices' arg
 */
Eina_List *
devices_list_free(Eina_List *devices)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(devices, NULL);

	unsigned int n = 0;
	Device *data;

    EINA_LIST_FREE(devices, data)
    {
		n++;
	    device_free(data);
	}
	eina_list_free(devices);

	debug(stdout, _("%d Device struct of Eina_list freed"), n);

    return NULL;
}


/*
 *Return an allocated Device* struct clone of Device 'src' arg.
 */
Device *
device_clone(const Device *src)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(src, NULL);

    Device *dst = calloc(1, sizeof(Device));
    if (!dst)
	{
		debug(stderr, _("Can't calloc Device struct"));
		return NULL;
	}

    dst->__eet_filename = eina_stringshare_add(src->__eet_filename);
    dst->name = eina_stringshare_add(src->name);
    dst->type = src->type;
    dst->class = src->class;
    dst->description = eina_stringshare_add(src->description);
    dst->current = eina_stringshare_add(src->current);
    dst->data1 = eina_stringshare_add(src->data1);
    dst->units = eina_stringshare_add(src->units);
    dst->unit_symbol = eina_stringshare_add(src->unit_symbol);
    dst->creation = eina_stringshare_add(src->creation);
    dst->revision = eina_stringshare_add(src->revision);
    dst->version = src->version;

	Eina_List *l, *actions;
	Action *action;
    actions = device_actions_list_get(src);
   	EINA_LIST_FOREACH(actions, l, action)
   	{
   		device_action_add(dst, action_new(action_ifcondition_get(action), action_ifvalue_get(action), action_type_get(action), action_data_get(action)));
	}

	return dst;
}/*device_clone*/
