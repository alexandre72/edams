/*
 * device.c
 * This file is part of EDAMS
 *
 * Copyright (C) 2013 - Alexandre Dussart
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "cosm.h"
#include "edams.h"
#include "global_view.h"
#include "location.h"
#include "serial.h"
#include "widget.h"



/*Globals vars*/
static pid_t            child_pid;

static void _devices_handler(void *data __UNUSED__, void *buf, unsigned int len);
static void _serial_devices_process_messages();
static void _emulated_devices_process_messages(Ecore_Pipe *pipe);

/*
 *
 */
static void
_devices_handler(void *data __UNUSED__, void *buf, unsigned int len)
{
    char *str = malloc(sizeof(char) * len + 1);
    memcpy(str, buf, len);
    str[len] = '\0';

    cJSON *root = cJSON_Parse(str);
	if(!root) return;
    FREE(str);

	cJSON *jschema = cJSON_GetObjectItem(root, "SCHEMA");

    if(!jschema) return;

    char *schema = cJSON_PrintUnformatted(jschema);
    strdelstr(schema, "\"");

    /*If schema to parse is osd*/
    if(strcmp(schema, "osd") == 0)
    {
        char *s;
        asprintf(&s, _("CMND CLASS=osd"));
        debug(MSG_DEVICE_OSD, s);
        FREE(s);

        osd_action_parse(str);
    }
    /*If schema to parse is sensor*/
    else if(strcmp(schema, "sensor") == 0)
    {
	    cJSON *jdevice = cJSON_GetObjectItem(root, "DEVICE");
	    cJSON *jtype = cJSON_GetObjectItem(root, "TYPE");
	    cJSON *jcurrent = cJSON_GetObjectItem(root, "CURRENT");
        char *device = cJSON_PrintUnformatted(jdevice);
        char *type = cJSON_PrintUnformatted(jtype);
        char * current = cJSON_PrintUnformatted(jcurrent);
        str = cJSON_PrintUnformatted(root);

        strdelstr(device, "\"");
        strdelstr(current, "\"");
        strdelstr(type, "\"");

        char *s;
        asprintf(&s, _("TRIG CLASS=sensor DEVICE=%s TYPE=%s CURRENT=%s"), device, type, current);
        debug(MSG_DEVICE_SENSOR, s);
        FREE(s);

        /*Parse all locations and sync with global and cosm*/
        Eina_List *locations = NULL, *l;
        Location *location;
        locations = edams_locations_list_get();

        EINA_LIST_FOREACH(locations, l, location)
        {
            Eina_List *widgets = NULL, *l2;
            Widget *widget;

            widgets = location_widgets_list_get(location);

            EINA_LIST_FOREACH(widgets, l2, widget)
            {
                /*If widget class is set to device, then check device*/
                if((widget_class_get(widget) != WIDGET_CLASS_SENSOR)) continue;

                /*Compare device with arg 'device', if found return widget*/
                if(strcmp(device, widget_device_id_get(widget)) == 0)
                {
                    /*Parse all widget action's and to execute them(if condition is full)*/
                    Eina_List *l3, *actions;
                    Action *action;
                    actions = widget_actions_list_get(widget);
                    EINA_LIST_FOREACH(actions, l3, action)
                    {
                        switch(action_ifcondition_get(action))
                        {
                            case CONDITION_EGAL_TO:
                                if(atoi(current) == atoi(action_ifvalue_get(action)))
                                action_parse(action);
                                break;
                            case CONDITION_LESS_THAN:
                                if(atoi(current) < atoi(action_ifvalue_get(action)))
                                action_parse(action);
                                break;
                            case CONDITION_MORE_THAN:
                                if(atoi(current) > atoi(action_ifvalue_get(action)))
                                action_parse(action);
                                break;
                            case CONDITION_LESS_OR_EGAL_TO:
                                if(atoi(current) <= atoi(action_ifvalue_get(action)))
                                action_parse(action);
                                break;
                            case CONDITION_MORE_OR_EGAL_TO:
                                if(atoi(current) >= atoi(action_ifvalue_get(action)))
                                action_parse(action);
                                break;
                            case CONDITION_DIFFERENT_TO:
                                if(atoi(current) != atoi(action_ifvalue_get(action)))
                                action_parse(action);
                                break;
                            case CONDITION_UNKNOWN:
                            case CONDITION_LAST:
                                break;
                        }/*switch action_ifcondition_get*/
                    }/*EINA_LIST_FOREACH actions*/

                    widget_device_current_set(widget, current);

                    if(!widget_device_highest_get(widget))
                           widget_device_highest_set(widget, current);
                    {
                        if(atoi(current) > atoi(widget_device_highest_get(widget)))
                        {
                            debug(MSG_DEVICE_SENSOR, ("New highest value for '%s' set to '%s'(old was '%s')"),
                                                    widget_name_get(widget),
                                                    current,
                                                    widget_device_highest_get(widget));
                            widget_device_highest_set(widget, current);
                        }
                    }

                    if(!widget_device_lowest_get(widget))
                        widget_device_lowest_set(widget, current);
                    {
                        if(atoi(current) < atoi(widget_device_lowest_get(widget)))
                        {
                            debug(MSG_DEVICE_SENSOR, ("New lowest value for '%s' set to '%s'(old was '%s')"),
                                                    widget_name_get(widget),
                                                    current,
                                                    widget_device_lowest_get(widget));
                            widget_device_lowest_set(widget, current);
                        }
                    }


                    global_view_widget_data_update(location, widget);
                    location_save(location);
					
                    if(widget_cosm_get(widget))
                        cosm_device_datastream_update(location, widget);
                }/*If device is registered in EDAMS*/
            }/*EINA_LIST_FOREACH widgets*/
        }/*EINA_LIST_FOREACH locations*/
        FREE(device);
        FREE(type);
        FREE(current);
        FREE(str);
    }/*End if schema to parse is sensor*/

    FREE(schema);
	cJSON_Delete(root);
}/*_devices_handler*/



/*
 *Child process that all devices messages received from wireless nodes.
 */
static void
_serial_devices_process_messages(Ecore_Pipe *pipe)
{
	int fd = 0;
    char buf[256];

     int baudrate = 115200;  /*default*/
     fd = serialport_init("/dev/ttyACM0", baudrate); /*Arduino serial*/

     for(;;)
     {                
			serialport_read_until(fd, buf, '\n');
			ecore_pipe_write(pipe, buf, strlen(buf));
			sleep(2);
   }
}/*devices_process_messages*/


/*
 *
 */
static void
_emulated_devices_process_messages(Ecore_Pipe *pipe)
{	
    char *samples[100][100]={
                                {"DHT11", "humidity"},
                                {"DS18B20", "temp"},
                                {"wc_count", "counter"},
                                {"pir_sensor1", "input"},
                                {"pir_sensor2", "input"},
                                {"bulb", "input"},
                                {"led", "input"},
                                {"energy_counter", "energy"},
                                {NULL, NULL}
                            };

    for(;;)
    {
        unsigned int i;
        for(i = 0; samples[i][0] != NULL; i++)
        {
            char *str ;

            cJSON *root;
        	root = cJSON_CreateObject();
            cJSON_AddItemToObject(root, "SCHEMA", cJSON_CreateString("sensor"));
        	cJSON_AddItemToObject(root, "DEVICE", cJSON_CreateString(samples[i][0]));
        	cJSON_AddItemToObject(root, "TYPE", cJSON_CreateString(samples[i][1]));
	        RANDOMIZE();

   	        asprintf(&str, "%d", RANDOM(device_type_current_max_get(samples[i][1])));

        	cJSON_AddItemToObject(root, "CURRENT", cJSON_CreateString(str));
        	FREE(str);
            str = cJSON_PrintUnformatted(root);
            cJSON_Delete(root);
            
			printf("COSM JSON:%s\n", str);
     
            ecore_pipe_write(pipe, str, strlen(str));

            FREE(str);
            sleep(1);
        }
     }
}/*_devices_emulate_messages*/




/*
 *
 */
Ecore_Pipe *
devices_start()
{
    Ecore_Pipe *pipe = NULL;

    pipe = ecore_pipe_add(_devices_handler, NULL);

    child_pid = fork();

    if (!child_pid)
    {
        ecore_pipe_read_close(pipe);

        if(edams_settings_softemu_get() == EINA_TRUE)
            _emulated_devices_process_messages(pipe);
        else
			_serial_devices_process_messages(pipe);
    }
    else
    {
        ecore_pipe_write_close(pipe);
    }

    return pipe;
}/*devices_start*/



/*
 *
 */
Eina_Bool
devices_shutdown()
{
    kill(child_pid, SIGKILL);

	return EINA_FALSE;
}/*devices_shutdown*/



/*
 *Return string description of device 'type' arg.
 */
const char *
device_type_to_desc(const char *type)
{
	if(!type) return NULL;

	if(strcmp(type, DEVICE_TYPE_BATTERY_SENSOR) == 0)    return _("A battery level in percent");
	if(strcmp(type, DEVICE_TYPE_COUNT_SENSOR) == 0)    return _("A counter value (door openings, rain fall, etc)");
	if(strcmp(type, DEVICE_TYPE_CURRENT_SENSOR) == 0)    return _("A current value in Amps");
	if(strcmp(type, DEVICE_TYPE_DIRECTION_SENSOR) == 0)    return _("Direction, represented as degrees from north (0-360, 0=north, 180=south, etc)");
	if(strcmp(type, DEVICE_TYPE_DISTANCE_SENSOR) == 0)    return _("Distance measurments. Default unit of measure is meters");
	if(strcmp(type, DEVICE_TYPE_ENERGY_SENSOR) == 0)    return _("Consumption of energy over a preiod of time in kWh (kilowatt hours)");
	if(strcmp(type, DEVICE_TYPE_FAN_SENSOR) == 0)    return _("A fan speed in RPM");
	if(strcmp(type, DEVICE_TYPE_GENERIC_SENSOR) == 0)    return _("A generic analogue value who's units of measurement are application specific");
	if(strcmp(type, DEVICE_TYPE_HUMIDITY_SENSOR) == 0)    return _("A relative humidity percentage (0 to 100, no percent sign)");
	if(strcmp(type, DEVICE_TYPE_INPUT_SENSOR) == 0)    return _("A switch that can either be current=HIGH (on), current=LOW (off) or current=PULSE (representing a button press)");
	if(strcmp(type, DEVICE_TYPE_OUTPUT_SENSOR) == 0)    return _("A change in an output state with values of LOW and HIGH");
	if(strcmp(type, DEVICE_TYPE_POWER_SENSOR) == 0)    return _("Instantaneous energy consumption level in kW");
	if(strcmp(type, DEVICE_TYPE_PRESSURE_SENSOR) == 0)    return _("A pressure value in Pascals (N/m2)");
	if(strcmp(type, DEVICE_TYPE_SETPOINT_SENSOR) == 0)    return _("A thermostat threshold temperature value in degrees. Default unit of measure is centigrade/celsius");
	if(strcmp(type, DEVICE_TYPE_SPEED_SENSOR) == 0)    return _("A generic speed. Default unit of measure is Miles per Hour");
	if(strcmp(type, DEVICE_TYPE_TEMP_SENSOR) == 0)    return _("A temperature value in degrees. Default unit of measure is centigrade celsius");
	if(strcmp(type, DEVICE_TYPE_UV_SENSOR) == 0)    return _("UV Index (with no units). See http://en.wikipedia.org/wiki/UV_index");
	if(strcmp(type, DEVICE_TYPE_VOLTAGE_SENSOR) == 0)    return _("A voltage value in Volts");
	if(strcmp(type, DEVICE_TYPE_VOLUME_SENSOR) == 0)    return _("A volume in m3. Often used as a measure of gas and water consumption");
	if(strcmp(type, DEVICE_TYPE_WEIGHT_SENSOR) == 0)    return _("The default unit is kilograms (yes, kilograms are a unit of mass, not weight)");
	
	if(strcmp(type, DEVICE_TYPE_DIGITAL_CONTROL) == 0)    return _("digital - binaries - 0-1");
	if(strcmp(type, DEVICE_TYPE_VARIABLE_CONTROL) == 0)    return _("variable - min to max");
	if(strcmp(type, DEVICE_TYPE_PERCENTAGE_CONTROL) == 0)    return _("slider - percentage - 0-100%)");
	else	return _("Unknown xPL type");
}/*str_to_desc*/



/*
 *
 */
const char *
device_type_to_units(const char *type)
{
	if(!type) return NULL;

	if(strcmp(type, DEVICE_TYPE_BATTERY_SENSOR) == 0) return _("Percent");
	if(strcmp(type, DEVICE_TYPE_COUNT_SENSOR) == 0)    return _("Counter");
	if(strcmp(type, DEVICE_TYPE_CURRENT_SENSOR) == 0)    return _("Amps");
	if(strcmp(type, DEVICE_TYPE_DIRECTION_SENSOR) == 0)    return _("Degrees");
	if(strcmp(type, DEVICE_TYPE_DISTANCE_SENSOR) == 0)    return _("Meters");
	if(strcmp(type, DEVICE_TYPE_ENERGY_SENSOR) == 0)    return _("Kilowatt hours");
	if(strcmp(type, DEVICE_TYPE_FAN_SENSOR) == 0)    return _("Rotation/min");
	if(strcmp(type, DEVICE_TYPE_GENERIC_SENSOR) == 0)    return _("Generic");
	if(strcmp(type, DEVICE_TYPE_HUMIDITY_SENSOR) == 0)    return _("Humidity ratio");
	if(strcmp(type, DEVICE_TYPE_INPUT_SENSOR) == 0)    return _("Input");
	if(strcmp(type, DEVICE_TYPE_OUTPUT_SENSOR) == 0)    return _("Output");
	if(strcmp(type, DEVICE_TYPE_POWER_SENSOR) == 0)    return _("Kilowatt");
	if(strcmp(type, DEVICE_TYPE_PRESSURE_SENSOR) == 0)    return _("Pascals");
	if(strcmp(type, DEVICE_TYPE_SETPOINT_SENSOR) == 0)    return _("Degrees");
	if(strcmp(type, DEVICE_TYPE_SPEED_SENSOR) == 0)    return _("Miles per Hour");
	if(strcmp(type, DEVICE_TYPE_TEMP_SENSOR) == 0)    return _("Celsius");
	if(strcmp(type, DEVICE_TYPE_UV_SENSOR) == 0)    return _("UV");
	if(strcmp(type, DEVICE_TYPE_VOLTAGE_SENSOR) == 0)    return _("Volts");
	if(strcmp(type, DEVICE_TYPE_VOLUME_SENSOR) == 0)    return _("Cubic meter");
	if(strcmp(type, DEVICE_TYPE_WEIGHT_SENSOR) == 0)    return _("Kilograms");
	else return "";
}/*device_type_to_units*/

/*
 *
 */
const char *
device_type_to_unit_symbol(const char *type)
{
	if(!type) return NULL;

	if(strcmp(type, DEVICE_TYPE_BATTERY_SENSOR) == 0) return _("%");
	if(strcmp(type, DEVICE_TYPE_COUNT_SENSOR) == 0)    return NULL;
	if(strcmp(type, DEVICE_TYPE_CURRENT_SENSOR) == 0)    return _("A");
	if(strcmp(type, DEVICE_TYPE_DIRECTION_SENSOR) == 0)    return _("o");
	if(strcmp(type, DEVICE_TYPE_DISTANCE_SENSOR) == 0)    return _("m");
	if(strcmp(type, DEVICE_TYPE_ENERGY_SENSOR) == 0)    return _("kWh");
	if(strcmp(type, DEVICE_TYPE_FAN_SENSOR) == 0)    return _("RPM");
	if(strcmp(type, DEVICE_TYPE_GENERIC_SENSOR) == 0)    return NULL;
	if(strcmp(type, DEVICE_TYPE_HUMIDITY_SENSOR) == 0)    return _("%");
	if(strcmp(type, DEVICE_TYPE_INPUT_SENSOR) == 0)    return NULL;
	if(strcmp(type, DEVICE_TYPE_OUTPUT_SENSOR) == 0)    return NULL;
	if(strcmp(type, DEVICE_TYPE_POWER_SENSOR) == 0)    return _("kW");
	if(strcmp(type, DEVICE_TYPE_PRESSURE_SENSOR) == 0)    return _("N/m2");
	if(strcmp(type, DEVICE_TYPE_SETPOINT_SENSOR) == 0)    return _("C");
	if(strcmp(type, DEVICE_TYPE_SPEED_SENSOR) == 0)    return _("MpH");
	if(strcmp(type, DEVICE_TYPE_TEMP_SENSOR) == 0)    return _("C");
	if(strcmp(type, DEVICE_TYPE_UV_SENSOR) == 0)    return NULL;
	if(strcmp(type, DEVICE_TYPE_VOLTAGE_SENSOR) == 0)    return _("V");
	if(strcmp(type, DEVICE_TYPE_VOLUME_SENSOR) == 0)    return _("m3");
	if(strcmp(type, DEVICE_TYPE_WEIGHT_SENSOR) == 0)    return _("kg");
	else return "";
}/*device_type_to_unit_symbol*/



/*
 *
 */
int
device_type_current_max_get(const char *type)
{
	if(!type) return 32000;

	if(strcmp(type, DEVICE_TYPE_BATTERY_SENSOR) == 0) 	return 100;
	if(strcmp(type, DEVICE_TYPE_COUNT_SENSOR) == 0)    return 10000;
	if(strcmp(type, DEVICE_TYPE_CURRENT_SENSOR) == 0)    return 100;
	if(strcmp(type, DEVICE_TYPE_DIRECTION_SENSOR) == 0)    return 360;
	if(strcmp(type, DEVICE_TYPE_DISTANCE_SENSOR) == 0)    return 100;
	if(strcmp(type, DEVICE_TYPE_ENERGY_SENSOR) == 0)    return 1000;
	if(strcmp(type, DEVICE_TYPE_FAN_SENSOR) == 0)    return 10000;
	if(strcmp(type, DEVICE_TYPE_GENERIC_SENSOR) == 0)    return 10000;
	if(strcmp(type, DEVICE_TYPE_HUMIDITY_SENSOR) == 0)    return 100;
	if(strcmp(type, DEVICE_TYPE_INPUT_SENSOR) == 0)    return 2; /*3 states, LOW/HIGH/PULSE*/
	if(strcmp(type, DEVICE_TYPE_OUTPUT_SENSOR) == 0)    return 1;
	if(strcmp(type, DEVICE_TYPE_POWER_SENSOR) == 0)    return 1000;
	if(strcmp(type, DEVICE_TYPE_PRESSURE_SENSOR) == 0)    return 1000;
	if(strcmp(type, DEVICE_TYPE_SETPOINT_SENSOR) == 0)    return 300;
	if(strcmp(type, DEVICE_TYPE_SPEED_SENSOR) == 0)    return 1000;
	if(strcmp(type, DEVICE_TYPE_TEMP_SENSOR) == 0)    return 100;
	if(strcmp(type, DEVICE_TYPE_UV_SENSOR) == 0)    return 15;
	if(strcmp(type, DEVICE_TYPE_VOLTAGE_SENSOR) == 0)    return 1000;
	if(strcmp(type, DEVICE_TYPE_VOLUME_SENSOR) == 0)    return 1000;
	if(strcmp(type, DEVICE_TYPE_WEIGHT_SENSOR) == 0)    return 1000;
	else return 32000;

}/*device_type_max_get*/


/*
 *
 */
int
device_type_current_min_get(const char *type)
{
	if(!type) return 0;

	if(strcmp(type, DEVICE_TYPE_BATTERY_SENSOR) == 0) 	return 0;
	if(strcmp(type, DEVICE_TYPE_COUNT_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_CURRENT_SENSOR) == 0)    return -100;
	if(strcmp(type, DEVICE_TYPE_DIRECTION_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_DISTANCE_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_ENERGY_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_FAN_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_GENERIC_SENSOR) == 0)    return -10000;
	if(strcmp(type, DEVICE_TYPE_HUMIDITY_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_INPUT_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_OUTPUT_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_POWER_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_PRESSURE_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_SETPOINT_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_SPEED_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_TEMP_SENSOR) == 0)    return -100;
	if(strcmp(type, DEVICE_TYPE_UV_SENSOR) == 0)    return 1;
	if(strcmp(type, DEVICE_TYPE_VOLTAGE_SENSOR) == 0)    return -1000;
	if(strcmp(type, DEVICE_TYPE_VOLUME_SENSOR) == 0)    return 0;
	if(strcmp(type, DEVICE_TYPE_WEIGHT_SENSOR) == 0)    return 0;
	else return 0;
}/*device_type_min_get*/



/*
 *
 */
Eina_Bool
device_osd_cmnd_send(char *command, char *text, char *delay)
{
    char *s;
    asprintf(&s, _("CMND CLASS=osd COMMAND=%s TEXT=%s DELAY=%s"),
                    command,
                    text,
                    delay);
    debug(MSG_DEVICE_OSD, s);
    FREE(s);

    return EINA_TRUE;
}/*device_osd_basic_cmnd_send*/


/*
 *
 */
Eina_Bool
device_control_cmnd_send(Widget *widget)
{
    char *s;
    asprintf(&s, _("CMND CLASS=control DEVICE=%s TYPE=%s CURRENT=%s"),
                    widget_device_id_get(widget),
                    widget_device_type_get(widget),
                    widget_device_current_get(widget));
    debug(MSG_DEVICE_CONTROL, s);

    return EINA_TRUE;
}/*device_control_cmnd_send*/


/*
 *
 */
Eina_Bool
device_sensor_cmnd_send(Widget *widget)
{
    return EINA_TRUE;
}/*device_sensor_cmnd_send*/
