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
    const char *data;				//Current data of xpl device.
    const char *data1;				//Additional data. Used for control.basic cmnd structure message.
	const char *units;  			//The units of the device e.g. 'Celsius'.
	const char *unit_symbol;		//The symbol of the unit e.g. 'C'.
	const char *unit_format;		//The unit format in c printf style e.g.g '%f'.
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
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "data", data, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "data1", data1, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "description", description, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "creation", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "revision", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "version", version, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "units", units, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "unit_symbol", unit_symbol, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "unit_format", unit_format, EET_T_STRING);
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
    device->data = NULL;
    device->data1 = NULL;
    device->units =NULL;
    device->unit_symbol = NULL;
    device->unit_format = NULL;

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
    	eina_stringshare_del(device->data);
    	eina_stringshare_del(device->data1);
    	eina_stringshare_del(device->units);
    	eina_stringshare_del(device->unit_symbol);
    	eina_stringshare_del(device->unit_format);
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
device_type_set(Device *device, const Device_Type type)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    device->type = type;

	switch(type)
	{
		default:
		case UNKNOWN_DEVICE_TYPE:
						debug(stderr, _("Can't set an unknown Type_Flags to device '%s'"), device_name_get(device));
						device_class_set(device, SENSOR_BASIC_CLASS);
						break;
		case GENERIC_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Generic"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case BATTERY_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Percent"));
					device_unit_symbol_set(device, "%");
					device_unit_format_set(device, "%s%%");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case COUNT_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Counter"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case CURRENT_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Amps"));
					device_unit_symbol_set(device, "A");
					device_unit_format_set(device, "%sA");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case DIRECTION_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Degrees"));
					device_unit_symbol_set(device, "°");
					device_unit_format_set(device, "%s°");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case DISTANCE_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Meters"));
					device_unit_symbol_set(device, "m");
					device_unit_format_set(device, "%s m");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case ENERGY_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Kilowatt hours"));
					device_unit_symbol_set(device, "kWh");
					device_unit_format_set(device, "%skWh");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case FAN_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Rotation/min"));
					device_unit_symbol_set(device, "RPM");
					device_unit_format_set(device, "%sRPM");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case HUMIDITY_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Humidity ratio"));
					device_unit_symbol_set(device, "%");
					device_unit_format_set(device, "%s%%");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case INPUT_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Input"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case OUTPUT_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Output"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case POWER_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Kilowatt"));
					device_unit_symbol_set(device, "kW");
					device_unit_format_set(device, "%skW");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case PRESSURE_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Pascals"));
					device_unit_symbol_set(device, "N/m2");
					device_unit_format_set(device, "%sN/m2");
					break;
					device_class_set(device, SENSOR_BASIC_CLASS);
		case SETPOINT_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Celsius"));
					device_unit_symbol_set(device, "°C");
					device_unit_format_set(device, "%s°C");
					break;
		case SPEED_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Miles per Hour"));
					device_unit_symbol_set(device, "Mph");
					device_unit_format_set(device, "%s Mph");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case TEMP_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Celsius"));
					device_unit_symbol_set(device, "°C");
					device_unit_format_set(device, "%s°C");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case UV_SENSOR_BASIC_TYPE:
					device_units_set(device, _("UV"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case VOLTAGE_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Volts"));
					device_unit_symbol_set(device, "V");
					device_unit_format_set(device, "%sV");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case VOLUME_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Cubic meter"));
					device_unit_symbol_set(device, "m3");
					device_unit_format_set(device, "%sm3");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;
		case WEIGHT_SENSOR_BASIC_TYPE:
					device_units_set(device, _("Kilograms"));
					device_unit_symbol_set(device, "kg");
					device_unit_format_set(device, "%skg");
					device_class_set(device, SENSOR_BASIC_CLASS);
					break;

		case BALANCE_CONTROL_BASIC_TYPE:
		case FLAG_CONTROL_BASIC_TYPE:
		case INFRARED_CONTROL_BASIC_TYPE:
		case INPUT_CONTROL_BASIC_TYPE:
		case MACRO_CONTROL_BASIC_TYPE:
		case MUTE_CONTROL_BASIC_TYPE:
		case OUTPUT_CONTROL_BASIC_TYPE:
		case VARIABLE_CONTROL_BASIC_TYPE:
		case PERIODIC_CONTROL_BASIC_TYPE:
		case SCHEDULED_CONTROL_BASIC_TYPE:
		case SLIDER_CONTROL_BASIC_TYPE:
		case TIMER_CONTROL_BASIC_TYPE:
					device_class_set(device, CONTROL_BASIC_CLASS);
					break;
	}
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
device_unit_format_set(Device *device, const char *unit_format)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    eina_stringshare_replace(&(device->unit_format), unit_format);
}


inline const char *
device_unit_format_get(const Device *device)
{
    return device->unit_format;
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
device_data_get(const Device *device)
{
    return device->data;
}

inline void
device_data_set(Device *device, const char *data)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
	eina_stringshare_replace(&(device->data), data);
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
    dst->data = eina_stringshare_add(src->data);
    dst->data1 = eina_stringshare_add(src->data1);
    dst->units = eina_stringshare_add(src->units);
    dst->unit_symbol = eina_stringshare_add(src->unit_symbol);
    dst->unit_format = eina_stringshare_add(src->unit_format);
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



/*
 *Remove location informations file.
 */
Eina_Bool
device_remove(Device *device)
{
	if(!device) return EINA_FALSE;

	if(!ecore_file_remove(device->__eet_filename))
	{
	    debug(stderr, _("Can't remove Eet device file '%s'"), device->__eet_filename);
	    return EINA_FALSE;
	}

	debug(stdout, _("Eet device file '%s' has been removed"), device->__eet_filename);

	return EINA_TRUE;
}/*device_remove*/



/*
 *Return string representation of Device_Class 'class' arg.
 */
const char *
device_class_to_str(Device_Class class)
{
	if(class == SENSOR_BASIC_CLASS)				return "sensor.basic";
	else if(class == CONTROL_BASIC_CLASS)		return "control.basic";
	else if(class == VIRTUAL_CLASS)				return "virtual";
	else 									return NULL;
}


/*
 *Return Device_Type representation of 's' arg.
 */
Device_Type
device_str_to_type(const char *s)
{
	if(!s) return UNKNOWN_DEVICE_TYPE;

	if(strcmp(s, "battery") == 0) 				return BATTERY_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "count") == 0)			return COUNT_SENSOR_BASIC_TYPE;
	else if(strcmp(s ,"current") == 0)			return CURRENT_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "direction") == 0)		return DIRECTION_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "distance") == 0)			return DISTANCE_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "energy") == 0)			return ENERGY_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "fan") == 0)				return FAN_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "generic") == 0)			return GENERIC_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "humidity") == 0)			return HUMIDITY_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "input") == 0)			return INPUT_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "output") == 0)			return OUTPUT_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "power") == 0)			return POWER_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "pressure") == 0)			return PRESSURE_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "setpoint") == 0)			return SETPOINT_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "speed") == 0)			return SPEED_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "temp") == 0)				return TEMP_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "uv") == 0)				return UV_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "voltage") == 0)			return VOLTAGE_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "volume") == 0)			return VOLUME_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "weight") == 0)			return WEIGHT_SENSOR_BASIC_TYPE;
	else if(strcmp(s, "balance") == 0)			return BALANCE_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "flag") == 0)				return FLAG_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "infrared") == 0)			return INFRARED_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "input.control") == 0)	return INPUT_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "macro") == 0)			return MACRO_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "mute") == 0)				return MUTE_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "output.control") == 0)	return OUTPUT_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "variable") == 0)			return VARIABLE_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "periodic") == 0)			return PERIODIC_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "scheduled") == 0)		return SCHEDULED_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "slider") == 0)			return SLIDER_CONTROL_BASIC_TYPE;
	else if(strcmp(s, "timer") == 0)			return TIMER_CONTROL_BASIC_TYPE;
	else										return UNKNOWN_DEVICE_TYPE;
}


/*
 *Return string representation of Condition type of 'condition' arg.
 */
const char *
action_condition_to_str(Condition condition)
{
	if(condition == EGAL_TO)				return "=";
	else if(condition == LESS_THAN)			return "<";
	else if(condition == MORE_THAN)			return ">";
	else if(condition == MORE_OR_EGAL_TO)	return ">=";
	else if(condition == LESS_OR_EGAL_TO)	return "<=";
	else 									return NULL;
}


/*
 *Return Condition representation of 's' arg.
 */
Condition
action_str_to_condition(const char *s)
{
	if(!s) return UNKNOWN_CONDITION;

	if(strcmp(s, "=") == 0) 		return EGAL_TO;
	else if(strcmp(s, "<") == 0)	return LESS_THAN;
	else if(strcmp(s, ">") == 0)	return MORE_THAN;
	else if(strcmp(s, ">=") == 0)	return MORE_OR_EGAL_TO;
	else if(strcmp(s, "<=") == 0)	return LESS_OR_EGAL_TO;
	else							return UNKNOWN_CONDITION;
}


/*
 *Return string representation of Action_Type 'type' arg.
 */
const char *
action_type_to_str(Action_Type type)
{
	if(type == CMND_ACTION)					return _("Send control.basic cmnd");
	else if(type == MAIL_ACTION)			return _("Send mail");
	else if(type == EXEC_ACTION)			return _("Execute external program");
	else if(type == DEBUG_ACTION)			return _("Debug stuff for testing purpose");
	else 									return NULL;
}


/*
 *Return string representation of Device_Type 'type' arg.
 */
const char *
device_type_to_str(Device_Type type)
{
	if(type == BATTERY_SENSOR_BASIC_TYPE)			return "battery";
	else if(type == COUNT_SENSOR_BASIC_TYPE)		return "count";
	else if(type == CURRENT_SENSOR_BASIC_TYPE)		return "current";
	else if(type == DIRECTION_SENSOR_BASIC_TYPE)	return "direction";
	else if(type == DISTANCE_SENSOR_BASIC_TYPE)		return "distance";
	else if(type == ENERGY_SENSOR_BASIC_TYPE)		return "energy";
	else if(type == FAN_SENSOR_BASIC_TYPE)			return "fan";
	else if(type == GENERIC_SENSOR_BASIC_TYPE)		return "generic";
	else if(type == HUMIDITY_SENSOR_BASIC_TYPE)		return "humidity";
	else if(type == INPUT_SENSOR_BASIC_TYPE)		return "input";
	else if(type == OUTPUT_SENSOR_BASIC_TYPE)		return "output";
	else if(type == POWER_SENSOR_BASIC_TYPE)		return "power";
	else if(type == SETPOINT_SENSOR_BASIC_TYPE)		return "setpoint";
	else if(type == SPEED_SENSOR_BASIC_TYPE)		return "speed";
	else if(type == TEMP_SENSOR_BASIC_TYPE)			return "temp";
	else if(type == UV_SENSOR_BASIC_TYPE)			return "uv";
	else if(type == VOLTAGE_SENSOR_BASIC_TYPE)		return "voltage";
	else if(type == VOLUME_SENSOR_BASIC_TYPE)		return "volume";
	else if(type == WEIGHT_SENSOR_BASIC_TYPE)		return "weight";
	else if(type == BALANCE_CONTROL_BASIC_TYPE)		return "balance";
	else if(type == FLAG_CONTROL_BASIC_TYPE)		return "flag";
	else if(type == INFRARED_CONTROL_BASIC_TYPE)	return "infrared";
	else if(type == INPUT_CONTROL_BASIC_TYPE)		return "input.control";
	else if(type == MACRO_CONTROL_BASIC_TYPE)		return "macro";
	else if(type == MUTE_CONTROL_BASIC_TYPE)		return "mute";
	else if(type == OUTPUT_CONTROL_BASIC_TYPE)		return "output.control";
	else if(type == VARIABLE_CONTROL_BASIC_TYPE)	return "variable";
	else if(type == PERIODIC_CONTROL_BASIC_TYPE)	return "periodic";
	else if(type == SCHEDULED_CONTROL_BASIC_TYPE)	return "scheduled";
	else if(type == SLIDER_CONTROL_BASIC_TYPE)		return "slider";
	else if(type == TIMER_CONTROL_BASIC_TYPE)		return "timer";
	else 											return NULL;
}
