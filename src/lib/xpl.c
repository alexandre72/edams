/*
 * xpl.c
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
#include "xpl.h"


/*xPL callbacks*/
static void _xpl_sensor_basic_handler(xPL_ServicePtr service, xPL_MessagePtr msg, xPL_ObjectPtr data);
static void _xpl_osd_basic_handler(xPL_ServicePtr service, xPL_MessagePtr msg, xPL_ObjectPtr data);
static void _xpl_handler(void *data __UNUSED__, void *buf, unsigned int len);

/*Others funcs*/
static void _xpl_emulate_messages(Ecore_Pipe *pipe);
const char *xpl_message_type_to_str(xPL_MessagePtr msg);

/*Globals vars*/
static Eina_Bool        XPL_STARTED;
static xPL_ServicePtr   xpl_edams_service;
static pid_t            child_pid;


/*
 *
 */
const char *
xpl_message_type_to_str(xPL_MessagePtr msg)
{
    switch(xPL_getMessageType(msg))
    {
        case xPL_MESSAGE_COMMAND:
            return "xpl-cmnd";
            break;
        case xPL_MESSAGE_STATUS:
            return "xpl-stat";
            break;
        case xPL_MESSAGE_TRIGGER:
            return "xpl-trig";
            break;
        default:
            return "unknown";
            break;
  }
  return NULL;
}/*xpl_message_type_to_str*/


/*
 *Callback called in xPL Message when 'sensor.basic' is triggered.
 */
static void
_xpl_sensor_basic_handler(xPL_ServicePtr service __UNUSED__, xPL_MessagePtr msg, xPL_ObjectPtr data)
{
    char *str ;
    xPL_NameValueListPtr values_names;

	Ecore_Pipe *pipe = (Ecore_Pipe *) data;

	values_names = xPL_getMessageBody(msg);


    /*Add to xpl_devices in json format to easing parsing*/
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SCHEMA", cJSON_CreateString("sensor.basic"));
    cJSON_AddItemToObject(root, "DEVICE", cJSON_CreateString(xPL_getNamedValue(values_names, "device")));
    cJSON_AddItemToObject(root, "TYPE", cJSON_CreateString(xPL_getNamedValue(values_names, "type")));
    cJSON_AddItemToObject(root, "CURRENT", cJSON_CreateString(xPL_getNamedValue(values_names, "current")));
    str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

	ecore_pipe_write(pipe, str, strlen(str));
    FREE(str);
}/*_xpl_sensor_basic_handler*/



/*
 *Callback called in xPL Message when 'osd.basic' is triggered.
 */
static void
_xpl_osd_basic_handler(xPL_ServicePtr service __UNUSED__, xPL_MessagePtr msg, xPL_ObjectPtr data)
{
    char *str ;
    xPL_NameValueListPtr values_names;

	Ecore_Pipe *pipe = (Ecore_Pipe *) data;

	values_names = xPL_getMessageBody(msg);

    /*Add to xpl_devices in json format to easing parsing*/
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SCHEMA", cJSON_CreateString("osd.basic"));
    cJSON_AddItemToObject(root, "COMMAND", cJSON_CreateString(xPL_getNamedValue(values_names, "command")));

        if(xPL_getNamedValue(values_names, "text"))
            cJSON_AddItemToObject(root, "TEXT", cJSON_CreateString(xPL_getNamedValue(values_names, "text")));

        if(xPL_getNamedValue(values_names, "delay"))
            cJSON_AddItemToObject(root, "DELAY", cJSON_CreateString(xPL_getNamedValue(values_names, "delay")));
    str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

	ecore_pipe_write(pipe, str, strlen(str));
    FREE(str);
}/*_xpl_osd_basic_handler*/



/*
 *
 */
static void
_xpl_handler(void *data __UNUSED__, void *buf, unsigned int len)
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

    /*If schema to parse is osd.basic*/
    if(strcmp(schema, "osd.basic") == 0)
    {
        char *s;
        asprintf(&s, _("MSG=xpl-cmnd CLASS=osd.basic"));
        debug(MSG_XPL, s);
        FREE(s);

        osd_action_parse(str);
    }
    /*If schema to parse is sensor.basic*/
    else if(strcmp(schema, "sensor.basic") == 0)
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
        asprintf(&s, _("MSG=xpl-trig CLASS=sensor.basic DEVICE=%s TYPE=%s CURRENT=%s"), device, type, current);
        debug(MSG_XPL, s);
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
                /*If xPL widget class, then check xpl device*/
                if((widget_class_get(widget) != WIDGET_CLASS_XPL_SENSOR_BASIC)) continue;

                /*Compare xpl device with arg 'xpl_device', if found return widget*/
                if(strcmp(device, widget_xpl_device_get(widget)) == 0)
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

                    widget_xpl_current_set(widget, current);

                    if(!widget_xpl_highest_get(widget))
                           widget_xpl_highest_set(widget, current);
                    {
                        if(atoi(current) > atoi(widget_xpl_highest_get(widget)))
                        {
                            debug(MSG_XPL, ("New highest value for '%s' set to '%s'(old was '%s')"),
                                                    widget_name_get(widget),
                                                    current,
                                                    widget_xpl_highest_get(widget));
                            widget_xpl_highest_set(widget, current);
                        }
                    }

                    if(!widget_xpl_lowest_get(widget))
                        widget_xpl_lowest_set(widget, current);
                    {
                        if(atoi(current) < atoi(widget_xpl_lowest_get(widget)))
                        {
                            debug(MSG_XPL, ("New lowest value for '%s' set to '%s'(old was '%s')"),
                                                    widget_name_get(widget),
                                                    current,
                                                    widget_xpl_lowest_get(widget));
                            widget_xpl_lowest_set(widget, current);
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
    }/*End if schema to parse is sensor.basic*/

    FREE(schema);
	cJSON_Delete(root);
}/*_xpl_handler*/



/*
 *Child process that listen xPL messages received from xPL hub(hub is an external prog and need to be run).
 */
void
xpl_process_messages()
{
	for (;;)
	{
		xPL_processMessages(-1);
	}
}/*xpl_process_messages*/


/*
 *
 */
static void
_xpl_emulate_messages(Ecore_Pipe *pipe)
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
            cJSON_AddItemToObject(root, "SCHEMA", cJSON_CreateString("sensor.basic"));
        	cJSON_AddItemToObject(root, "DEVICE", cJSON_CreateString(samples[i][0]));
        	cJSON_AddItemToObject(root, "TYPE", cJSON_CreateString(samples[i][1]));
	        RANDOMIZE();

   	        asprintf(&str, "%d", RANDOM(xpl_type_current_max_get(samples[i][1])));

        	cJSON_AddItemToObject(root, "CURRENT", cJSON_CreateString(str));
        	FREE(str);
            str = cJSON_PrintUnformatted(root);
            cJSON_Delete(root);
            ecore_pipe_write(pipe, str, strlen(str));
            FREE(str);
            sleep(1);
        }
     }
}/*_xpl_emulate_messages*/




/*
 *
 */
Ecore_Pipe *
xpl_start()
{
    Ecore_Pipe *pipe = NULL;

    if(XPL_STARTED == EINA_FALSE)
        return pipe;

    pipe = ecore_pipe_add(_xpl_handler, NULL);

    /*Add xPL sensor.basic listener*/
    xPL_addServiceListener(xpl_edams_service, _xpl_sensor_basic_handler, xPL_MESSAGE_TRIGGER, "sensor", "basic",(xPL_ObjectPtr)pipe);
    xPL_addServiceListener(xpl_edams_service, _xpl_sensor_basic_handler, xPL_MESSAGE_STATUS, "sensor", "basic",(xPL_ObjectPtr)pipe);

    /*Add xPL osd.basic listener*/
    xPL_addServiceListener(xpl_edams_service, _xpl_osd_basic_handler, xPL_MESSAGE_COMMAND, "osd", "basic",(xPL_ObjectPtr)pipe);

    child_pid = fork();

    if (!child_pid)
    {
        ecore_pipe_read_close(pipe);

        if(edams_settings_softemu_get() == EINA_TRUE)
            _xpl_emulate_messages(pipe);
        else

            xpl_process_messages();
    }
    else
    {
        ecore_pipe_write_close(pipe);
    }

    return pipe;
}/*xpl_start*/



/*
 *
 */
Eina_Bool
xpl_init()
{
	/*Initialize xPL*/
	if (!xPL_initialize(xPL_getParsedConnectionType()))
	{
	    debug(MSG_XPL, _("Can't init xPL service"));
        XPL_STARTED	= EINA_FALSE;
	    return EINA_FALSE;
	}

	/*Create an xpl service*/
	xpl_edams_service = xPL_createService("edams", "xpl", "edams.xpl");
	xPL_setServiceVersion(xpl_edams_service, VERSION);

	/*Enable the service*/
	xPL_setServiceEnabled(xpl_edams_service, EINA_TRUE);
    debug(MSG_XPL, _("xPL service initialized"));

    XPL_STARTED	= EINA_TRUE;

	return EINA_TRUE;
}/*xpl_init*/


/*
 *
 */
Eina_Bool
xpl_shutdown()
{
	debug(MSG_XPL, _("Shutdown xPL..."));

	if(XPL_STARTED == EINA_TRUE)
	{
        kill(child_pid, SIGKILL);
	    xPL_setServiceEnabled(xpl_edams_service, EINA_FALSE);
	    xPL_releaseService(xpl_edams_service);
	    xPL_shutdown();
    }

	return 0;
}/*xpl_shutdown*/



/*
 *
 */
const char*
xpl_control_basic_cmnd_to_elm_str(Widget *widget)
{
    const char *s;

    if( !widget_xpl_device_get(widget) ||
        !widget_xpl_type_get(widget) ||
        !widget_xpl_current_get(widget))
            return NULL;

    if(!widget_xpl_data1_get(widget))
    {
	    asprintf(&s,"<em>control.basic<br>\
					{<br>\
					<tab>device=%s<br>\
 					<tab>type=%s<br>\
					<tab>current=%s<br>\
					}</em>",
					widget_xpl_device_get(widget),
					widget_xpl_type_get(widget),
					widget_xpl_current_get(widget));
    }
    else
    {
	    asprintf(&s,"<em><em>control.basic<br>\
					{<br>\
					<tab>device=%s<br>\
 					<tab>type=%s<br>\
					<tab>current=%s<br>\
					<tab>data11=%s<br>\
					}</em>",
					widget_xpl_device_get(widget),
					widget_xpl_type_get(widget),
					widget_xpl_current_get(widget),
					widget_xpl_data1_get(widget));
    }

    return s;
}/*xpl_control_basic_cmnd_to_elm_str*/



/*
 *Return string description of 'xpl_type' arg.
 */
const char *
xpl_type_to_desc(const char *xpl_type)
{
	if(!xpl_type) return NULL;

	if(strcmp(xpl_type, XPL_TYPE_BATTERY_SENSOR_BASIC) == 0)    return _("A battery level in percent");
	if(strcmp(xpl_type, XPL_TYPE_COUNT_SENSOR_BASIC) == 0)    return _("A counter value (door openings, rain fall, etc)");
	if(strcmp(xpl_type, XPL_TYPE_CURRENT_SENSOR_BASIC) == 0)    return _("A current value in Amps");
	if(strcmp(xpl_type, XPL_TYPE_DIRECTION_SENSOR_BASIC) == 0)    return _("Direction, represented as degrees from north (0-360, 0=north, 180=south, etc)");
	if(strcmp(xpl_type, XPL_TYPE_DISTANCE_SENSOR_BASIC) == 0)    return _("Distance measurments. Default unit of measure is meters");
	if(strcmp(xpl_type, XPL_TYPE_ENERGY_SENSOR_BASIC) == 0)    return _("Consumption of energy over a preiod of time in kWh (kilowatt hours)");
	if(strcmp(xpl_type, XPL_TYPE_FAN_SENSOR_BASIC) == 0)    return _("A fan speed in RPM");
	if(strcmp(xpl_type, XPL_TYPE_GENERIC_SENSOR_BASIC) == 0)    return _("A generic analogue value who's units of measurement are application specific");
	if(strcmp(xpl_type, XPL_TYPE_HUMIDITY_SENSOR_BASIC) == 0)    return _("A relative humidity percentage (0 to 100, no percent sign)");
	if(strcmp(xpl_type, XPL_TYPE_INPUT_SENSOR_BASIC) == 0)    return _("A switch that can either be current=HIGH (on), current=LOW (off) or current=PULSE (representing a button press)");
	if(strcmp(xpl_type, XPL_TYPE_OUTPUT_SENSOR_BASIC) == 0)    return _("A change in an output state with values of LOW and HIGH");
	if(strcmp(xpl_type, XPL_TYPE_POWER_SENSOR_BASIC) == 0)    return _("Instantaneous energy consumption level in kW");
	if(strcmp(xpl_type, XPL_TYPE_PRESSURE_SENSOR_BASIC) == 0)    return _("A pressure value in Pascals (N/m2)");
	if(strcmp(xpl_type, XPL_TYPE_SETPOINT_SENSOR_BASIC) == 0)    return _("A thermostat threshold temperature value in degrees. Default unit of measure is centigrade/celsius");
	if(strcmp(xpl_type, XPL_TYPE_SPEED_SENSOR_BASIC) == 0)    return _("A generic speed. Default unit of measure is Miles per Hour");
	if(strcmp(xpl_type, XPL_TYPE_TEMP_SENSOR_BASIC) == 0)    return _("A temperature value in degrees. Default unit of measure is centigrade celsius");
	if(strcmp(xpl_type, XPL_TYPE_UV_SENSOR_BASIC) == 0)    return _("UV Index (with no units). See http://en.wikipedia.org/wiki/UV_index");
	if(strcmp(xpl_type, XPL_TYPE_VOLTAGE_SENSOR_BASIC) == 0)    return _("A voltage value in Volts");
	if(strcmp(xpl_type, XPL_TYPE_VOLUME_SENSOR_BASIC) == 0)    return _("A volume in m3. Often used as a measure of gas and water consumption");
	if(strcmp(xpl_type, XPL_TYPE_WEIGHT_SENSOR_BASIC) == 0)    return _("The default unit is kilograms (yes, kilograms are a unit of mass, not weight)");
	if(strcmp(xpl_type, XPL_TYPE_BALANCE_CONTROL_BASIC) == 0)    return _("balance - -100 to +100");
	if(strcmp(xpl_type, XPL_TYPE_FLAG_CONTROL_BASIC) == 0)    return _("flag - set, clear, neutral");
	if(strcmp(xpl_type, XPL_TYPE_INFRARED_CONTROL_BASIC) == 0)    return _("infrared - send, enable_rx, disable_rx, enable_tx, disable_tx, sendx (send x times)");
	if(strcmp(xpl_type, XPL_TYPE_INPUT_CONTROL_BASIC) == 0)    return _("input - enable, disable");
	if(strcmp(xpl_type, XPL_TYPE_MACRO_CONTROL_BASIC) == 0)    return _("macro - enable, disable, do");
	if(strcmp(xpl_type, XPL_TYPE_MUTE_CONTROL_BASIC) == 0)    return _("mute - yes, no");
	if(strcmp(xpl_type, XPL_TYPE_OUTPUT_CONTROL_BASIC) == 0)    return _("output - enable, disable, high, low, toggle, pulse");
	if(strcmp(xpl_type, XPL_TYPE_VARIABLE_CONTROL_BASIC) == 0)    return _("variable - inc, dec, 0-255 (for set)");
	if(strcmp(xpl_type, XPL_TYPE_PERIODIC_CONTROL_BASIC) == 0)    return _("periodic - started, enable, disable");
	if(strcmp(xpl_type, XPL_TYPE_SCHEDULED_CONTROL_BASIC) == 0)    return _("scheduled - started, enable, disable");
	if(strcmp(xpl_type, XPL_TYPE_SLIDER_CONTROL_BASIC) == 0)    return _("slider - nn = set to value (0-255), +nn = increment by nn, -nn = decrement by nn, n% = set to nn (where nn is a percentage - 0-100%)");
	if(strcmp(xpl_type, XPL_TYPE_TIMER_CONTROL_BASIC) == 0)    return _("timer - went off, start, stop, halt, resume");
	else	return _("Unknown xPL type");
}/*xpl_str_to_desc*/


/*
 *
 */
int
xpl_type_current_max_get(const char *xpl_type)
{
	if(!xpl_type) return 32000;

	if(strcmp(xpl_type, XPL_TYPE_BATTERY_SENSOR_BASIC) == 0) 	return 100;
	if(strcmp(xpl_type, XPL_TYPE_COUNT_SENSOR_BASIC) == 0)    return 10000;
	if(strcmp(xpl_type, XPL_TYPE_CURRENT_SENSOR_BASIC) == 0)    return 100;
	if(strcmp(xpl_type, XPL_TYPE_DIRECTION_SENSOR_BASIC) == 0)    return 360;
	if(strcmp(xpl_type, XPL_TYPE_DISTANCE_SENSOR_BASIC) == 0)    return 100;
	if(strcmp(xpl_type, XPL_TYPE_ENERGY_SENSOR_BASIC) == 0)    return 1000;
	if(strcmp(xpl_type, XPL_TYPE_FAN_SENSOR_BASIC) == 0)    return 10000;
	if(strcmp(xpl_type, XPL_TYPE_GENERIC_SENSOR_BASIC) == 0)    return 10000;
	if(strcmp(xpl_type, XPL_TYPE_HUMIDITY_SENSOR_BASIC) == 0)    return 100;
	if(strcmp(xpl_type, XPL_TYPE_INPUT_SENSOR_BASIC) == 0)    return 2; /*3 states, LOW/HIGH/PULSE*/
	if(strcmp(xpl_type, XPL_TYPE_OUTPUT_SENSOR_BASIC) == 0)    return 1;
	if(strcmp(xpl_type, XPL_TYPE_POWER_SENSOR_BASIC) == 0)    return 1000;
	if(strcmp(xpl_type, XPL_TYPE_PRESSURE_SENSOR_BASIC) == 0)    return 1000;
	if(strcmp(xpl_type, XPL_TYPE_SETPOINT_SENSOR_BASIC) == 0)    return 300;
	if(strcmp(xpl_type, XPL_TYPE_SPEED_SENSOR_BASIC) == 0)    return 1000;
	if(strcmp(xpl_type, XPL_TYPE_TEMP_SENSOR_BASIC) == 0)    return 100;
	if(strcmp(xpl_type, XPL_TYPE_UV_SENSOR_BASIC) == 0)    return 15;
	if(strcmp(xpl_type, XPL_TYPE_VOLTAGE_SENSOR_BASIC) == 0)    return 1000;
	if(strcmp(xpl_type, XPL_TYPE_VOLUME_SENSOR_BASIC) == 0)    return 1000;
	if(strcmp(xpl_type, XPL_TYPE_WEIGHT_SENSOR_BASIC) == 0)    return 1000;
	else return 32000;

}/*xpl_type_max_get*/


/*
 *
 */
int
xpl_type_current_min_get(const char *xpl_type)
{
	if(!xpl_type) return 0;

	if(strcmp(xpl_type, XPL_TYPE_BATTERY_SENSOR_BASIC) == 0) 	return 0;
	if(strcmp(xpl_type, XPL_TYPE_COUNT_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_CURRENT_SENSOR_BASIC) == 0)    return -100;
	if(strcmp(xpl_type, XPL_TYPE_DIRECTION_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_DISTANCE_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_ENERGY_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_FAN_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_GENERIC_SENSOR_BASIC) == 0)    return -10000;
	if(strcmp(xpl_type, XPL_TYPE_HUMIDITY_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_INPUT_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_OUTPUT_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_POWER_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_PRESSURE_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_SETPOINT_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_SPEED_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_TEMP_SENSOR_BASIC) == 0)    return -100;
	if(strcmp(xpl_type, XPL_TYPE_UV_SENSOR_BASIC) == 0)    return 1;
	if(strcmp(xpl_type, XPL_TYPE_VOLTAGE_SENSOR_BASIC) == 0)    return -1000;
	if(strcmp(xpl_type, XPL_TYPE_VOLUME_SENSOR_BASIC) == 0)    return 0;
	if(strcmp(xpl_type, XPL_TYPE_WEIGHT_SENSOR_BASIC) == 0)    return 0;
	else return 0;
}/*xpl_type_min_get*/


/*
 *
 */
const char *
xpl_type_to_units(const char *xpl_type)
{
	if(!xpl_type) return NULL;

	if(strcmp(xpl_type, XPL_TYPE_BATTERY_SENSOR_BASIC) == 0) return _("Percent");
	if(strcmp(xpl_type, XPL_TYPE_COUNT_SENSOR_BASIC) == 0)    return _("Counter");
	if(strcmp(xpl_type, XPL_TYPE_CURRENT_SENSOR_BASIC) == 0)    return _("Amps");
	if(strcmp(xpl_type, XPL_TYPE_DIRECTION_SENSOR_BASIC) == 0)    return _("Degrees");
	if(strcmp(xpl_type, XPL_TYPE_DISTANCE_SENSOR_BASIC) == 0)    return _("Meters");
	if(strcmp(xpl_type, XPL_TYPE_ENERGY_SENSOR_BASIC) == 0)    return _("Kilowatt hours");
	if(strcmp(xpl_type, XPL_TYPE_FAN_SENSOR_BASIC) == 0)    return _("Rotation/min");
	if(strcmp(xpl_type, XPL_TYPE_GENERIC_SENSOR_BASIC) == 0)    return _("Generic");
	if(strcmp(xpl_type, XPL_TYPE_HUMIDITY_SENSOR_BASIC) == 0)    return _("Humidity ratio");
	if(strcmp(xpl_type, XPL_TYPE_INPUT_SENSOR_BASIC) == 0)    return _("Input");
	if(strcmp(xpl_type, XPL_TYPE_OUTPUT_SENSOR_BASIC) == 0)    return _("Output");
	if(strcmp(xpl_type, XPL_TYPE_POWER_SENSOR_BASIC) == 0)    return _("Kilowatt");
	if(strcmp(xpl_type, XPL_TYPE_PRESSURE_SENSOR_BASIC) == 0)    return _("Pascals");
	if(strcmp(xpl_type, XPL_TYPE_SETPOINT_SENSOR_BASIC) == 0)    return _("Degrees");
	if(strcmp(xpl_type, XPL_TYPE_SPEED_SENSOR_BASIC) == 0)    return _("Miles per Hour");
	if(strcmp(xpl_type, XPL_TYPE_TEMP_SENSOR_BASIC) == 0)    return _("Celsius");
	if(strcmp(xpl_type, XPL_TYPE_UV_SENSOR_BASIC) == 0)    return _("UV");
	if(strcmp(xpl_type, XPL_TYPE_VOLTAGE_SENSOR_BASIC) == 0)    return _("Volts");
	if(strcmp(xpl_type, XPL_TYPE_VOLUME_SENSOR_BASIC) == 0)    return _("Cubic meter");
	if(strcmp(xpl_type, XPL_TYPE_WEIGHT_SENSOR_BASIC) == 0)    return _("Kilograms");
	else return "";
}/*xpl_type_to_units*/

/*
 *
 */
const char *
xpl_type_to_unit_symbol(const char *xpl_type)
{
	if(!xpl_type) return NULL;

	if(strcmp(xpl_type, XPL_TYPE_BATTERY_SENSOR_BASIC) == 0) return _("%");
	if(strcmp(xpl_type, XPL_TYPE_COUNT_SENSOR_BASIC) == 0)    return " ";
	if(strcmp(xpl_type, XPL_TYPE_CURRENT_SENSOR_BASIC) == 0)    return _("A");
	if(strcmp(xpl_type, XPL_TYPE_DIRECTION_SENSOR_BASIC) == 0)    return _("o");
	if(strcmp(xpl_type, XPL_TYPE_DISTANCE_SENSOR_BASIC) == 0)    return _("m");
	if(strcmp(xpl_type, XPL_TYPE_ENERGY_SENSOR_BASIC) == 0)    return _("kWh");
	if(strcmp(xpl_type, XPL_TYPE_FAN_SENSOR_BASIC) == 0)    return _("RPM");
	if(strcmp(xpl_type, XPL_TYPE_GENERIC_SENSOR_BASIC) == 0)    return " ";
	if(strcmp(xpl_type, XPL_TYPE_HUMIDITY_SENSOR_BASIC) == 0)    return _("%");
	if(strcmp(xpl_type, XPL_TYPE_INPUT_SENSOR_BASIC) == 0)    return " ";
	if(strcmp(xpl_type, XPL_TYPE_OUTPUT_SENSOR_BASIC) == 0)    return " ";
	if(strcmp(xpl_type, XPL_TYPE_POWER_SENSOR_BASIC) == 0)    return _("kW");
	if(strcmp(xpl_type, XPL_TYPE_PRESSURE_SENSOR_BASIC) == 0)    return _("N/m2");
	if(strcmp(xpl_type, XPL_TYPE_SETPOINT_SENSOR_BASIC) == 0)    return _("C");
	if(strcmp(xpl_type, XPL_TYPE_SPEED_SENSOR_BASIC) == 0)    return _("MpH");
	if(strcmp(xpl_type, XPL_TYPE_TEMP_SENSOR_BASIC) == 0)    return _("C");
	if(strcmp(xpl_type, XPL_TYPE_UV_SENSOR_BASIC) == 0)    return " ";
	if(strcmp(xpl_type, XPL_TYPE_VOLTAGE_SENSOR_BASIC) == 0)    return _("V");
	if(strcmp(xpl_type, XPL_TYPE_VOLUME_SENSOR_BASIC) == 0)    return _("m3");
	if(strcmp(xpl_type, XPL_TYPE_WEIGHT_SENSOR_BASIC) == 0)    return _("kg");
	else return "";
}/*xpl_type_to_unit_symbol*/

/*
 *
 */
Eina_Bool
xpl_osd_basic_cmnd_send(char *command, char *text, char *delay)
{
    xPL_MessagePtr xpl_message_cmnd = NULL;

    /* Create an appropriate message */
    if ((xpl_message_cmnd = xPL_createBroadcastMessage(xpl_edams_service, xPL_MESSAGE_COMMAND)) == NULL)
    {
        debug(MSG_XPL, _("Can't create broadcast message"));
        return EINA_FALSE;
    }

  	xPL_setSchema(xpl_message_cmnd, "osd", "basic");

    /*Install the value(s) and send the message*/
  	xPL_setMessageNamedValue(xpl_message_cmnd, "command", command);

  	if(text)
      	xPL_setMessageNamedValue(xpl_message_cmnd, "text", text);

  	if(delay)
        xPL_setMessageNamedValue(xpl_message_cmnd, "delay", delay);

	/*Broadcast the message*/
	if (!xPL_sendMessage(xpl_message_cmnd))
	{
		debug(MSG_XPL, _("Can't send xPL message"));
        xPL_releaseMessage(xpl_message_cmnd);
		return EINA_FALSE;
	}

    char *s;
    asprintf(&s, _("MSG=xpl-cmnd CLASS=osd.basic COMMAND=%s TEXT=%s DELAY=%s"),
                    command,
                    text,
                    delay);
    debug(MSG_XPL, s);
    FREE(s);

    xPL_releaseMessage(xpl_message_cmnd);
    return EINA_TRUE;
}/*xpl_osd_basic_cmnd_send*/


/*
 *
 */
Eina_Bool
xpl_control_basic_cmnd_send(Widget *widget)
{
    xPL_MessagePtr xpl_message_cmnd = NULL;

    /* Create an appropriate message */
    if ((xpl_message_cmnd = xPL_createBroadcastMessage(xpl_edams_service, xPL_MESSAGE_COMMAND)) == NULL)
    {
        debug(MSG_XPL, _("Can't create broadcast message"));
        return EINA_FALSE;
    }

  	xPL_setSchema(xpl_message_cmnd, "control", "basic");

    /*Install the value(s) and send the message*/
  	xPL_setMessageNamedValue(xpl_message_cmnd, "device", widget_xpl_device_get(widget));
  	xPL_setMessageNamedValue(xpl_message_cmnd, "type", widget_xpl_type_get(widget));
  	xPL_setMessageNamedValue(xpl_message_cmnd, "current", widget_xpl_current_get(widget));

	if(widget_xpl_data1_get(widget))
	  	xPL_setMessageNamedValue(xpl_message_cmnd, "data1", widget_xpl_data1_get(widget));

	/*Broadcast the message*/
	if (!xPL_sendMessage(xpl_message_cmnd))
	{
		debug(MSG_XPL, _("Can't send xPL message"));
		xPL_releaseMessage(xpl_message_cmnd);
		return EINA_FALSE;
	}

    char *s;
    asprintf(&s, _("MSG=xpl-cmnd CLASS=control.basic DEVICE=%s TYPE=%s CURRENT=%s"),
                    widget_xpl_device_get(widget),
                    widget_xpl_type_get(widget),
                    widget_xpl_current_get(widget));
    debug(MSG_XPL, s);
    FREE(s);

    xPL_releaseMessage(xpl_message_cmnd);
    return EINA_TRUE;
}/*xpl_control_basic_cmnd_send*/


/*
 *
 */
Eina_Bool
xpl_sensor_basic_cmnd_send(Widget *widget)
{
    xPL_MessagePtr xpl_message_cmnd = NULL;

    /* Create an appropriate message */
    if ((xpl_message_cmnd = xPL_createBroadcastMessage(xpl_edams_service, xPL_MESSAGE_COMMAND)) == NULL)
    {
        debug(MSG_XPL, _("Can't create broadcast message"));
        return EINA_FALSE;
    }

  	xPL_setSchema(xpl_message_cmnd, "sensor", "request");

    /*Install the value(s) and send the message*/
  	xPL_setMessageNamedValue(xpl_message_cmnd, "request", "current");
  	xPL_setMessageNamedValue(xpl_message_cmnd, "device", widget_xpl_device_get(widget));

	/*Broadcast the message*/
	if (!xPL_sendMessage(xpl_message_cmnd))
	{
		debug(MSG_XPL, _("Can't send xPL message"));
		xPL_releaseMessage(xpl_message_cmnd);
		return EINA_FALSE;
	}

    char *s;
    asprintf(&s, _("MSG=xpl-cmnd CLASS=sensor.request DEVICE=%s"), widget_xpl_device_get(widget));
    debug(MSG_XPL, s);
    FREE(s);

    xPL_releaseMessage(xpl_message_cmnd);
    return EINA_TRUE;
}/*xpl_sensor_basic_stat_send*/
