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

#include <Eina.h>
#include <Evas.h>

#define DEVICE_FILE_VERSION 0x0005

typedef struct _Device Device;
typedef struct _Action Action;


typedef enum _Type_Flags
{
	UNKNOWN_TYPE	= (0),
	BATTERY			= (1),      //battery - a battery level in percent.
	COUNT			= (2),     	//count - a counter value (door openings, rain fall, etc)
	CURRENT			= (3),	   	//current - a current value in Amps.
	DIRECTION		= (4),    	//direction - direction, represented as degrees from north (0-360, 0=north, 180=south, etc)
	DISTANCE		= (5),    	//distance - distance measurments. Default unit of measure is meters.
	ENERGY			= (6),    	//energy - consumption of energy over a preiod of time in kWh (kilowatt hours)
	FAN				= (7),    	//fan - a fan speed in RPM
	GENERIC			= (8),      //generic - a generic analogue value who's units of measurement are application specific
	HUMIDITY		= (9),    	//humidity - a relative humidity percentage (0 to 100, no percent sign)
	INPUT			= (10),    	//input - a switch that can either be current=HIGH (on), current=LOW (off) or current=PULSE (representing a button press)
	OUPUT			= (11),    	//output - a change in an output state with values of LOW and HIGH
	POWER			= (12),    	//power - instantaneous energy consumption level in kW
	PRESSURE		= (13),    	//pressure - a pressure value in Pascals (N/m2)
	SETPOINT		= (14),    	//setpoint - a thermostat threshold temperature value in degrees. Default unit of measure is centigrade/celsius.
	SPEED			= (15),   	//speed - a generic speed. Default unit of measure is Miles per Hour.
	TEMP			= (16),		//temp - a temperature value in degrees. Default unit of measure is centigrade/celsius.
	UV				= (17),    	//uv - UV Index (with no units). See http://en.wikipedia.org/wiki/UV_index
	VOLTAGE			= (18), 	//voltage - a voltage value in Volts.
	VOLUME			= (19),    	//volume - a volume in m3. Often used as a measure of gas and water consumption.
	WEIGHT			= (20),    	//weight - the default unit is kilograms (yes, kilograms are a unit of mass, not weight)
	TYPE_LAST
}Type_Flags;


typedef enum _Class_Flags
{
	UNKNOWN_CLASS			= (0),
	SENSOR_BASIC			= (1),
	CONTROL_BASIC			= (2),
	CLASS_LAST
}Class_Flags;


typedef enum _Action_Type
{
	UNKNOWN_ACTION			= (0),
	CMND_ACTION				= (1),
	MAIL_ACTION				= (2),
	EXEC_ACTION				= (3),
	DEBUG_ACTION			= (4),
	ACTION_TYPE_LAST
}Action_Type;


typedef enum _Condition_
{
	UNKNOWN_CONDITION		= (0),
	EGAL_TO					= (1),
	LESS_THAN				= (2),
	MORE_THAN				= (3),
	LESS_OR_EGAL_TO			= (4),
	MORE_OR_EGAL_TO			= (5),
	CONDITION_LAST
}Condition;


/* Actions */
Action *action_new(Condition condition, const char *value, Action_Type type, const char *data);
void action_free(Action *action);

Condition action_ifcondition_get(const Action *action);
const char * action_ifvalue_get(const Action *action);
Action_Type  action_type_get(const Action *action);
const char * action_data_get(const Action *action);

void device_action_add(Device *device, Action *action);
void device_action_del(Device *device,Action *action);
Action *device_action_get(const Device *device, unsigned int nth);
unsigned int device_actions_count(const Device *device);
Eina_List *device_actions_list_get(const Device *device);
void device_actions_list_clear(Device *device);
void device_actions_list_set(Device *device, Eina_List *list);

/*Convert funcs*/
Type_Flags device_str_to_type(const char *s);
const char *device_type_to_str(Type_Flags type);
Class_Flags device_str_to_class(const char *s);
const char *device_class_to_str(Class_Flags type);
const char *action_condition_to_str(Condition condition);
Condition action_str_to_condition(const char *s);

const char *action_type_to_str(Action_Type type);


/*Device funcs*/
Device *device_new(const char * name);
void device_free(Device *device);
Device *device_clone(const Device *src);
Device *devices_detect(char *s);
Eina_Bool device_save(Device *device);
Device *device_load(const char *filename);

void device_id_set(Device *device, unsigned int id);
void device_name_set(Device *device, const char * name);
void device_class_set(Device *device, Class_Flags class);
void device_type_set(Device *device, Type_Flags type);
void device_description_set(Device *device, const char * description);
void device_datasheeturl_set(Device *device, const char * datasheeturl);
void device_image_set(Device *device, Evas_Object *image);
void device_creation_set(Device *device, const char * creation);
void device_data_set(Device *device, const char * data);
void device_units_set(Device *device, const char *units);
void device_unit_format_set(Device *device, const char *unit_format);
void device_unit_symbol_set(Device *device, const char *unit_symbol);

const char *device_filename_get(Device *device);
unsigned int device_id_get(const Device *device);
const char * device_name_get(const Device *device);
Class_Flags device_class_get(const Device *device);
Type_Flags device_type_get(const Device *device);
const char * device_description_get(const Device *device);
const char * device_datasheeturl_get(const Device *device);
const char * device_creation_get(const Device *device);
const char * device_data_get(const Device *device);
const char * device_units_get(const Device *device);
const char * device_unit_format_get(const Device *device);
const char * device_unit_symbol_get(const Device *device);

/*Devices list funcs.*/
Eina_List *devices_list_get();
Eina_List *devices_list_free(Eina_List *devices);
Device* devices_list_device_with_id_get(Eina_List *devices, unsigned int id);

/* Global initializer / shutdown functions */
void devices_init(void);
void devices_shutdown(void);
#endif /*__DEVICE_H*/
