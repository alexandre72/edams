#ifndef __DEVICE_H
#define __DEVICE_H

#include <Eina.h>
#include <Ecore.h>

#include "widget.h"

#define DEVICE_TYPE_BATTERY_SENSOR "battery"
#define DEVICE_TYPE_COUNT_SENSOR "count"
#define DEVICE_TYPE_CURRENT_SENSOR "current"
#define DEVICE_TYPE_DIRECTION_SENSOR "direction"
#define DEVICE_TYPE_DISTANCE_SENSOR "distance"
#define DEVICE_TYPE_ENERGY_SENSOR "energy"
#define DEVICE_TYPE_FAN_SENSOR "fan"
#define DEVICE_TYPE_GENERIC_SENSOR "generic"
#define DEVICE_TYPE_HUMIDITY_SENSOR "humidity"
#define DEVICE_TYPE_INPUT_SENSOR "input"
#define DEVICE_TYPE_OUTPUT_SENSOR "output"
#define DEVICE_TYPE_POWER_SENSOR "power"
#define DEVICE_TYPE_PRESSURE_SENSOR "pressure"
#define DEVICE_TYPE_SETPOINT_SENSOR "setpoint"
#define DEVICE_TYPE_SPEED_SENSOR "speed"
#define DEVICE_TYPE_TEMP_SENSOR "temp"
#define DEVICE_TYPE_UV_SENSOR "uv"
#define DEVICE_TYPE_VOLTAGE_SENSOR "voltage"
#define DEVICE_TYPE_VOLUME_SENSOR "volume"
#define DEVICE_TYPE_WEIGHT_SENSOR "weight"

#define DEVICE_TYPE_DIGITAL_CONTROL "output"	/*Control handle 0/1 values*/
#define DEVICE_TYPE_SLIDER_CONTROL "slider"		/*Control handle values between min and max*/

Eina_Bool device_control_send(Widget *widget);
Eina_Bool device_sensor_basic_cmnd_send(Widget *widget);
Eina_Bool device_osd_send(char *command, char *text, char *delay);

const char *device_control_basic_cmnd_to_elm_str(Widget *widget);
const char *device_type_to_desc(const char *type);
const char *device_type_to_units(const char *type);
const char *device_type_to_unit_symbol(const char *type);
int device_type_current_min_get(const char *type);
int device_type_current_max_get(const char *type);
Eina_List *device_sensors_list_get();

#endif /*__DEVICE_H*/
