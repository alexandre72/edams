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


#include "device.h"
#include "edams.h"
#include "libedams.h"
#include "path.h"
#include "utils.h"

struct _Device
{
    const char *__eet_filename;		//Filename name of device, generated and based on device's name.
    unsigned int id;				//Id of device e.g. '104'.
    const char *name;				//Name of device 'DS18B20'.
    Class_Flags class;				//Class of device e.g. 'SENSOR'.
    Type_Flags type;				//Type of device e.g. 'HUMIDITY'.
    const char *description;		//Description of device e.g.'I2C sensor'.
    const char *datasheeturl;		//URL Datasheet of device e.g. 'http///alldatasheet.com/ds18b20.html'.
    const char *data;				//Current data of device.
	const char *units;  			//The units of the device e.g. 'Celsius'.
	const char *unit_symbol;		//The symbol of the unit e.g. 'C'.
	const char *unit_format;		//The unit format in c printf style e.g.g '%f'.
    const char *creation;			//Creation date of device Eet file.
	const char *revision;			//Revision date of device Eet file.
    unsigned int version;			//Version of device Eet file.
};


static const char DEVICE_ENTRY[] = "device";
static Eet_Data_Descriptor *_device_descriptor = NULL;





static int
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




static inline void
_device_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_device_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Device);
    _device_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "id", id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "class", class, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "type", type, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "description", description, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "datasheeturl", datasheeturl, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "creation", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "revision", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "version", version, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "units", units, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "unit_symbol", unit_symbol, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_device_descriptor, Device, "unit_format", unit_format, EET_T_STRING);
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
    _device_init();
}

void
devices_shutdown(void)
{
    _device_shutdown();
}

Device *
device_new(unsigned int id, const char * name)
{
	char s[PATH_MAX];

    Device *device = calloc(1, sizeof(Device));

    if (!device)
	{
		debug(stderr, _("Couldn't calloc Device struct"));
		return NULL;
	}

   	device->id = id;
    device->name = eina_stringshare_add(name ? name : "undefined");
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%d-%s.eet" , edams_locations_data_path_get(), device->id, device->name);
    device->__eet_filename = eina_stringshare_add(s);
    device->class = UNKNOWN_CLASS;
    device->type = UNKNOWN_TYPE;
    device->description = eina_stringshare_add("undefined");
    device->datasheeturl = eina_stringshare_add("undefined");
    device->data = NULL;
    device->units =NULL;
    device->unit_symbol = NULL;
    device->unit_format = NULL;

    //Add creation date informations.
	time_t timestamp = time(NULL);
	struct tm *t = localtime(&timestamp);
	snprintf(s, sizeof(s), "%02d-%02d-%d",
				(int)t->tm_mday,
  				(int)t->tm_mon,
  				1900+(int)t->tm_year);

    device->creation = eina_stringshare_add(s);
	device->revision = NULL;
	device->version = 0x0002;

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
    	eina_stringshare_del(device->datasheeturl);
    	eina_stringshare_del(device->data);
    	eina_stringshare_del(device->units);
    	eina_stringshare_del(device->unit_symbol);
    	eina_stringshare_del(device->unit_format);
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

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%s.eet" , edams_db_path_get(), name);
    eina_stringshare_replace(&(device->__eet_filename), s);
	eina_stringshare_replace(&(device->name), name);
}

inline Class_Flags
device_class_get(const Device *device)
{
    return device->class;
}


inline void
device_class_set(Device *device, const Class_Flags class)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    device->class = class;
}


inline Type_Flags
device_type_get(const Device *device)
{
    return device->type;
}

inline void
device_type_set(Device *device, const Type_Flags type)
{
    EINA_SAFETY_ON_NULL_RETURN(device);
    device->type = type;

	switch(type)
	{
		default:
		case UNKNOWN_TYPE:
						debug(stderr, _("Couldn't set an unknown device type to device with id"), device_name_get(device), device_id_get(device));
						break;
		case GENERIC:
					device_units_set(device, _("Generic"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					break;
		case BATTERY:
					device_units_set(device, _("Percent"));
					device_unit_symbol_set(device, "%");
					device_unit_format_set(device, "%s%%");
					break;
		case COUNT:
					device_units_set(device, _("Counter"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					break;
		case CURRENT:
					device_units_set(device, _("Amps"));
					device_unit_symbol_set(device, "A");
					device_unit_format_set(device, "%sA");
					break;
		case DIRECTION:
					device_units_set(device, _("Degrees"));
					device_unit_symbol_set(device, "°");
					device_unit_format_set(device, "%s°");
					break;
		case DISTANCE:
					device_units_set(device, _("Meters"));
					device_unit_symbol_set(device, "m");
					device_unit_format_set(device, "%s m");
					break;
		case ENERGY:
					device_units_set(device, _("Kilowatt hours"));
					device_unit_symbol_set(device, "kWh");
					device_unit_format_set(device, "%skWh");
					break;
		case FAN:
					device_units_set(device, _("Rotation/min"));
					device_unit_symbol_set(device, "RPM");
					device_unit_format_set(device, "%sRPM");
					break;
		case HUMIDITY:
					device_units_set(device, _("Humidity ratio"));
					device_unit_symbol_set(device, "%");
					device_unit_format_set(device, "%s%%");
					break;
		case INPUT:
					device_units_set(device, _("Input"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					break;
		case OUPUT:
					device_units_set(device, _("Output"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					break;
		case POWER:
					device_units_set(device, _("Kilowatt"));
					device_unit_symbol_set(device, "kW");
					device_unit_format_set(device, "%skW");
					break;
		case PRESSURE:
					device_units_set(device, _("Pascals"));
					device_unit_symbol_set(device, "N/m2");
					device_unit_format_set(device, "%sN/m2");
					break;
		case SETPOINT:
					device_units_set(device, _("Celsius"));
					device_unit_symbol_set(device, "°C");
					device_unit_format_set(device, "%s°C");
					break;
		case SPEED:
					device_units_set(device, _("Miles per Hour"));
					device_unit_symbol_set(device, "Mph");
					device_unit_format_set(device, "%s Mph");
					break;
		case TEMP:
					device_units_set(device, _("Celsius"));
					device_unit_symbol_set(device, "°C");
					device_unit_format_set(device, "%s°C");
					break;
		case UV:
					device_units_set(device, _("UV"));
					device_unit_symbol_set(device, "");
					device_unit_format_set(device, "%s");
					break;
		case VOLTAGE:
					device_units_set(device, _("Volts"));
					device_unit_symbol_set(device, "V");
					device_unit_format_set(device, "%sV");
					break;
		case VOLUME:
					device_units_set(device, _("Cubic meter"));
					device_unit_symbol_set(device, "m3");
					device_unit_format_set(device, "%sm3");
					break;
		case WEIGHT:
					device_units_set(device, _("Kilograms"));
					device_unit_symbol_set(device, "kg");
					device_unit_format_set(device, "%skg");
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
        debug(stderr, _("Couldn't open Eet file '%s' in writing mode"), device->__eet_filename);
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
		debug(stderr, _("Couldn't open Eet file '%s' in writing mode"), device->__eet_filename);
		return EINA_FALSE;
	}

    ret = !!eet_data_write(ef, _device_descriptor, DEVICE_ENTRY, device, EINA_TRUE);
    eet_close(ef);

	if (!ret)
	{
		debug(stderr, _("Couldn't write any data to Eet file '%s'"), device->__eet_filename);
		return EINA_FALSE;
	}

    return EINA_TRUE;
}


Device *
device_load(const char *filename)
{
    Device *device = NULL;

    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
	{
		debug(stderr, _("Couldn't open Eet file '%s' in reading mode"), filename);
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


//
//Read all devices infos files.
//
Device *
device_with_id_from_data_get(unsigned int id)
{
	const Eina_File_Direct_Info *f_info;
	Eina_Iterator *it;
	Device *device = NULL;

	it = eina_file_stat_ls(edams_locations_data_path_get());

   	if(it)
   	{
	   EINA_ITERATOR_FOREACH(it, f_info)
	   {
			if(eina_str_has_extension(f_info->path, ".eet") == EINA_TRUE)
			{
				char s[PATH_MAX];
				snprintf(s, sizeof(s), "%d-", id);
				if(strncmp(s, ecore_file_file_get(f_info->path), strlen(s)) == 0)
				{
					device = device_load(f_info->path);
					return device;
				}
			}
		}

		eina_iterator_free(it);
	}

	debug(stderr, _("No device with id '%d' registered"), id);
	return NULL;
}




//
//Read all devices infos files.
//
Eina_List *
devices_database_list_get()
{
	const Eina_File_Direct_Info *f_info;
	Eina_Iterator *it;
	Eina_List *devices = NULL;
	Device *device = NULL;

	it = eina_file_stat_ls(edams_db_path_get());

   	if(it)
   	{
	   EINA_ITERATOR_FOREACH(it, f_info)
	   {
			if(eina_str_has_extension(f_info->path, ".eet") == EINA_TRUE)
			{
				device = device_load(f_info->path);

				if(device)
				{
					devices = eina_list_append(devices, device);

					if (eina_error_get())
					{
						debug(stderr, _("Couldn't allocate Eina list node!"));
						exit(-1);
					}
				}
			}
		}

	eina_iterator_free(it);
	}

	debug(stdout, _("%d devices registered in database"), eina_list_count(devices));
	devices = eina_list_sort(devices, eina_list_count(devices), EINA_COMPARE_CB(devices_list_sort_cb));
	return devices;
}


//
//Free devices list.
//
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



Device *
device_clone(const Device *src)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(src, NULL);

    Device *dst = calloc(1, sizeof(Device));
    if (!dst)
	{
		debug(stderr, _("Couldn't calloc Device struct"));
		return NULL;
	}

    dst->__eet_filename = eina_stringshare_add(src->__eet_filename);
    dst->id = src->id;
    dst->name = eina_stringshare_add(src->name);
    dst->type = src->type;
    dst->description = eina_stringshare_add(src->description);
    dst->datasheeturl = eina_stringshare_add(src->datasheeturl);
    dst->data = eina_stringshare_add(src->data);
    dst->units = eina_stringshare_add(src->units);
    dst->unit_symbol = eina_stringshare_add(src->unit_symbol);
    dst->unit_format = eina_stringshare_add(src->unit_format);
    dst->creation = eina_stringshare_add(src->creation);
    dst->revision = eina_stringshare_add(src->revision);
    dst->version = src->version;

	return dst;
}


//
//
//
Device*
devices_list_device_with_id_get(Eina_List *devices, unsigned int id)
{
	Eina_List *l;
	Device *device;

	EINA_LIST_FOREACH(devices, l, device)
	{
		if(device_id_get(device) == id)
		{
			return device;
		}
	}
	return NULL;
}


Class_Flags
device_str_to_class(const char *s)
{
	if(!s) return UNKNOWN_TYPE;

	if(strcmp(s, "control.basic") == 0) return CONTROL;
	if(strcmp(s, "sensor.basic") == 0) 	return SENSOR;
	else								return UNKNOWN_TYPE;
}


const char *
device_class_to_str(Class_Flags class)
{
	if(class == SENSOR)			return "sensor.basic";
	else if(class == CONTROL)	return "control.basic";
	else 						return NULL;
}



Type_Flags
device_str_to_type(const char *s)
{
	if(!s) return UNKNOWN_TYPE;

	if(strcmp(s, "battery") == 0) 		return BATTERY;
	else if(strcmp(s, "count") == 0)	return COUNT;
	else if(strcmp(s ,"current") == 0)	return CURRENT;
	else if(strcmp(s, "direction") == 0)return DIRECTION;
	else if(strcmp(s, "distance") == 0)	return DISTANCE;
	else if(strcmp(s, "energy") == 0)	return ENERGY;
	else if(strcmp(s, "fan") == 0)		return FAN;
	else if(strcmp(s, "generic") == 0)	return GENERIC;
	else if(strcmp(s, "humidity") == 0)	return HUMIDITY;
	else if(strcmp(s, "input") == 0)	return INPUT;
	else if(strcmp(s, "output") == 0)	return OUPUT;
	else if(strcmp(s, "power") == 0)	return POWER;
	else if(strcmp(s, "pressure") == 0)	return PRESSURE;
	else if(strcmp(s, "setpoint") == 0)	return SETPOINT;
	else if(strcmp(s, "speed") == 0)	return SPEED;
	else if(strcmp(s, "temp") == 0)		return TEMP;
	else if(strcmp(s, "uv") == 0)		return UV;
	else if(strcmp(s, "voltage") == 0)	return VOLTAGE;
	else if(strcmp(s, "volume") == 0)	return VOLUME;
	else if(strcmp(s, "weight") == 0)	return WEIGHT;
	else								return UNKNOWN_TYPE;
}



const char *
device_type_to_str(Type_Flags type)
{
	if(type == BATTERY)			return "battery";
	else if(type == COUNT)		return "count";
	else if(type == CURRENT)	return "current";
	else if(type == DIRECTION)	return "direction";
	else if(type == DISTANCE)	return "distance";
	else if(type == ENERGY)		return "energy";
	else if(type == FAN)		return "fan";
	else if(type == GENERIC)	return "generic";
	else if(type == HUMIDITY)	return "humidity";
	else if(type == INPUT)		return "input";
	else if(type == OUPUT)		return "output";
	else if(type == POWER)		return "power";
	else if(type == SETPOINT)	return "setpoint";
	else if(type == SPEED)		return "speed";
	else if(type == TEMP)		return "temp";
	else if(type == UV)			return "uv";
	else if(type == VOLTAGE)	return "voltage";
	else if(type == VOLUME)		return "volume";
	else if(type == WEIGHT)		return "weight";
	else 						return NULL;
}
