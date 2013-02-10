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

#define DEVICE_FILE_VERSION 0x0007

typedef struct _Device Device;
typedef struct _Action Action;


typedef enum _Device_Type
{
	UNKNOWN_DEVICE_TYPE					= (0),
	BATTERY_SENSOR_BASIC_TYPE			= (1),  /*battery - a battery level in percent.*/
	COUNT_SENSOR_BASIC_TYPE				= (2),  /*count - a counter value (door openings, rain fall, etc).*/
	CURRENT_SENSOR_BASIC_TYPE			= (3),	/*current - a current value in Amps.*/
	DIRECTION_SENSOR_BASIC_TYPE			= (4),  /*direction - direction, represented as degrees from north (0-360, 0=north, 180=south, etc)*/
	DISTANCE_SENSOR_BASIC_TYPE			= (5),  /*distance - distance measurments. Default unit of measure is meters.*/
	ENERGY_SENSOR_BASIC_TYPE			= (6),  /*energy - consumption of energy over a preiod of time in kWh (kilowatt hours).*/
	FAN_SENSOR_BASIC_TYPE				= (7),  /*fan - a fan speed in RPM.*/
	GENERIC_SENSOR_BASIC_TYPE			= (8),  /*generic - a generic analogue value who's units of measurement are application specific*/
	HUMIDITY_SENSOR_BASIC_TYPE			= (9),  /*humidity - a relative humidity percentage (0 to 100, no percent sign).*/
	INPUT_SENSOR_BASIC_TYPE				= (10), /*input - a switch that can either be current=HIGH (on), current=LOW (off) or current=PULSE (representing a button press)*/
	OUTPUT_SENSOR_BASIC_TYPE			= (11), /*output - a change in an output state with values of LOW and HIGH*/
	POWER_SENSOR_BASIC_TYPE				= (12), /*power - instantaneous energy consumption level in kW*/
	PRESSURE_SENSOR_BASIC_TYPE			= (13), /*pressure - a pressure value in Pascals (N/m2)*/
	SETPOINT_SENSOR_BASIC_TYPE			= (14), /*setpoint - a thermostat threshold temperature value in degrees. Default unit of measure is centigrade/celsius.*/
	SPEED_SENSOR_BASIC_TYPE				= (15),	/*speed - a generic speed. Default unit of measure is Miles per Hour.*/
	TEMP_SENSOR_BASIC_TYPE				= (16),	/*temp - a temperature value in degrees. Default unit of measure is centigrade celsius.*/
	UV_SENSOR_BASIC_TYPE				= (17), /*uv - UV Index (with no units). See http://en.wikipedia.org/wiki/UV_index.*/
	VOLTAGE_SENSOR_BASIC_TYPE			= (18), /*voltage - a voltage value in Volts.*/
	VOLUME_SENSOR_BASIC_TYPE			= (19), /*volume - a volume in m3. Often used as a measure of gas and water consumption.*/
	WEIGHT_SENSOR_BASIC_TYPE			= (20), /*weight - the default unit is kilograms (yes, kilograms are a unit of mass, not weight)*/
	BALANCE_CONTROL_BASIC_TYPE			= (21), /*balance - -100 to +100.*/
	FLAG_CONTROL_BASIC_TYPE				= (22), /*flag - set, clear, neutral.*/
	INFRARED_CONTROL_BASIC_TYPE			= (23), /*infrared - send, enable_rx, disable_rx, enable_tx, disable_tx, sendx (send x times).*/
	INPUT_CONTROL_BASIC_TYPE			= (24), /*input - enable, disable.*/
	MACRO_CONTROL_BASIC_TYPE			= (25), /*macro - enable, disable, do.*/
	MUTE_CONTROL_BASIC_TYPE				= (26), /*mute - yes, no.*/
	OUTPUT_CONTROL_BASIC_TYPE			= (27), /*output - enable, disable, high, low, toggle, pulse.*/
	VARIABLE_CONTROL_BASIC_TYPE			= (28), /*variable - inc, dec, 0-255 (for set).*/
	PERIODIC_CONTROL_BASIC_TYPE			= (29), /*periodic - started, enable, disable.*/
	SCHEDULED_CONTROL_BASIC_TYPE		= (30), /*scheduled - started, enable, disable.*/
	SLIDER_CONTROL_BASIC_TYPE			= (31), /*slider -	nn = set to value (0-255),*/
												/* +nn = increment by nn, -nn = decrement by nn,*/
												/*n% = set to nn (where nn is a percentage - 0-100%)*/
	TIMER_CONTROL_BASIC_TYPE			= (32), /*timer - went off, start, stop, halt, resume.*/
	DEVICE_TYPE_LAST
}Device_Type;


typedef enum _Device_Class_
{
	UNKNOWN_DEVICE_CLASS	= (0),
	SENSOR_BASIC_CLASS		= (1),
	CONTROL_BASIC_CLASS		= (2),
	VIRTUAL_CLASS			= (3),
	CLASS_LAST
}Device_Class;





/* Actions */
Action *action_new(Condition condition, const char *value, Action_Type type, const char *data);
void action_free(Action *action);

Condition action_ifcondition_get(const Action *action);
const char * action_ifvalue_get(const Action *action);
Action_Type  action_type_get(const Action *action);
const char * action_data_get(const Action *action);

void action_ifcondition_set(Action *action, Condition ifcondition);
void action_ifvalue_set(const Action *action, const char *ifvalue);
void action_type_set(Action *action, Action_Type type);
void action_data_set(const Action *action, const char *data);

void device_action_add(Device *device, Action *action);
void device_action_del(Device *device,Action *action);
Action *device_action_get(const Device *device, unsigned int nth);
unsigned int device_actions_count(const Device *device);
Eina_List *device_actions_list_get(const Device *device);
void device_actions_list_clear(Device *device);
void device_actions_list_set(Device *device, Eina_List *list);

/*Convert funcs*/
Device_Type device_str_to_type(const char *s);
const char *device_type_to_str(Device_Type type);



const char *action_condition_to_str(Condition condition);
Condition action_str_to_condition(const char *s);

const char *action_type_to_str(Action_Type type);

int device_current_to_int(Device *device);

/*Device funcs*/
Device *device_new(const char * name);
void device_free(Device *device);
Device *device_clone(const Device *src);
Device *devices_detect(char *s);
Eina_Bool device_save(Device *device);
Device *device_load(const char *filename);
Eina_Bool device_remove(Device *device);

void device_name_set(Device *device, const char * name);
void device_class_set(Device *device, Device_Class class);
void device_type_set(Device *device, Device_Type type);
void device_description_set(Device *device, const char * description);
void device_image_set(Device *device, Evas_Object *image);
void device_creation_set(Device *device, const char * creation);
void device_current_set(Device *device, const char * current);
void device_data1_set(Device *device, const char * data1);
void device_units_set(Device *device, const char *units);
void device_unit_symbol_set(Device *device, const char *unit_symbol);

const char *device_filename_get(Device *device);
const char * device_name_get(const Device *device);
Device_Class device_class_get(const Device *device);
Device_Type device_type_get(const Device *device);
const char * device_description_get(const Device *device);
const char * device_creation_get(const Device *device);
const char * device_current_get(const Device *device);
const char * device_data1_get(const Device *device);
const char * device_units_get(const Device *device);
const char * device_unit_symbol_get(const Device *device);

/*Devices list funcs*/
Eina_List *devices_list_get();
Eina_List *devices_list_free(Eina_List *devices);
Device* devices_list_device_with_id_get(Eina_List *devices, unsigned int id);

/*Global initializer / shutdown functions*/
void devices_init(void);
void devices_shutdown(void);

#endif /*__DEVICE_H*/
