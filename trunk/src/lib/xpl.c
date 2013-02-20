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

#include "cJSON.h"
#include "cosm.h"
#include "edams.h"
#include "gnuplot.h"
#include "global_view.h"
#include "xpl.h"


/*xPL callbacks*/
static void _xpl_sensor_basic_handler(xPL_ServicePtr service, xPL_MessagePtr msg, xPL_ObjectPtr data);
static void _xpl_handler(void *data __UNUSED__, void *buf, unsigned int len);

/*Others funcs*/
static void _xpl_emulate_messages(Ecore_Pipe *pipe);

/*Local vars*/
//static	xPL_MessagePtr 	xpl_edams_message_stat;
//static	xPL_MessagePtr 	xpl_edams_message_trig;
static xPL_ServicePtr   xpl_edams_service;
static xPL_MessagePtr   xpl_edams_message_cmnd;
static pid_t            child_pid;
static Eina_List*       xpl_devices;
static Eina_Bool        XPL_STARTED;

/*
 *Callback called in xPL Message 'cstatic void
handler(void *data __UNUSED__, void *buf, unsigned int len)ontrol.basic' is triggered.
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
    cJSON_AddItemToObject(root, "DEVICE", cJSON_CreateString(xPL_getNamedValue(values_names, "device")));
    cJSON_AddItemToObject(root, "TYPE", cJSON_CreateString(xPL_getNamedValue(values_names, "type")));
    cJSON_AddItemToObject(root, "CURRENT", cJSON_CreateString(xPL_getNamedValue(values_names, "current")));
    str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

	ecore_pipe_write(pipe, str, strlen(str));
    FREE(str);
}/*_xpl_sensor_basic_handler*/




/*
 *
 */
static void
_xpl_handler(void *data __UNUSED__, void *buf, unsigned int len)
{
    char *device;
    char *type;
    char  *current;

    char *str = malloc(sizeof(char) * len + 1);
    memcpy(str, buf, len);
    str[len] = '\0';

    cJSON *root = cJSON_Parse(str);
	if(!root) return;
    FREE(str);
	cJSON *jdevice = cJSON_GetObjectItem(root, "DEVICE");
	cJSON *jtype = cJSON_GetObjectItem(root, "TYPE");
	cJSON *jcurrent = cJSON_GetObjectItem(root, "CURRENT");
    device = cJSON_PrintUnformatted(jdevice);
    type = cJSON_PrintUnformatted(jtype);
    current = cJSON_PrintUnformatted(jcurrent);
    str = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);

    strdelstr(device, "\"");
    strdelstr(current, "\"");
    strdelstr(type, "\"");

    /*Check if xPL device is already registered in devices Eina_List*/
    Eina_List *l;
    char *elem;

    EINA_LIST_FOREACH(xpl_devices, l, elem)
    {
        cJSON *root = cJSON_Parse(elem);
        char *device_elem;
	    if(!root) continue;

	    cJSON *jdevice = cJSON_GetObjectItem(root, "DEVICE");
        device_elem = cJSON_PrintUnformatted(jdevice);
        strdelstr(device_elem, "\"");

        /*If found sync with widget in global map and cosm if needed*/
        if(strcmp(device, device_elem) == 0)
        {
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
                   	/*Ift xPL widget class, then check xpl device*/
                    if((widget_class_get(widget) == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
                       (widget_class_get(widget) == WIDGET_CLASS_XPL_SENSOR_BASIC))
                    {
                        /*Compare xpl device with arg 'xpl_device', if found return widget*/
	        	        if(strcmp(device, widget_xpl_device_get(widget)) == 0)
	        	        {
	        	            widget_xpl_current_set(widget, current);

                            if(!widget_xpl_highest_get(widget))
                                widget_xpl_highest_set(widget, current);
                            else
                            {
                                if(strcmp(widget_xpl_highest_get(widget), current) < 0)
                                {
                                    debug(stdout, ("New highest value for '%s' set to '%s'(old was '%s')"), widget_name_get(widget), current, widget_xpl_highest_get(widget));
                                widget_xpl_highest_set(widget, current);
                                }
                            }

                            if(!widget_xpl_lowest_get(widget))
                                widget_xpl_lowest_set(widget, current);
                            {
                                if(strcmp(widget_xpl_lowest_get(widget), current) > 0)
                                {
                                    debug(stdout, ("New lowest value for '%s' set to '%s'(old was '%s')"), widget_name_get(widget), current, widget_xpl_lowest_get(widget));
                                    widget_xpl_lowest_set(widget, current);
                                }
                            }

                            location_save(location);

                            if(widget_cosm_get(widget))
                                cosm_device_datastream_update(location, widget);

                            if(widget_gnuplot_get(widget))
                                gnuplot_device_data_write(widget);

                            global_view_widget_data_update(location, widget);
                        }
                    }
                }
            }
            cJSON_Delete(root);
            FREE(device_elem);
            FREE(device);
            FREE(type);
            FREE(current);
            FREE(str);
            return;
        }
        else
        {
          	FREE(device_elem);
            cJSON_Delete(root);
        }
    }

    xpl_devices = eina_list_append(xpl_devices, eina_stringshare_add(str));
    FREE(str);

    asprintf(&str, _("New xPL sensor.basic '%s 'of type '%s' has been discovered"), device, type);
    statusbar_text_set(str, "elm/icon/xpl/default");
    FREE(str);

    FREE(device);
    FREE(type);
    FREE(current);
}/*_xpl_handler*/


/*
 *
 */
Eina_List *
xpl_sensor_basic_list_get()
{
    return xpl_devices;
}/*xpl_process_messages*/



/*
 *Child process that listen xPL messages received from xPL hub(hub is an external prog and need to be run).
 */
void
xpl_process_messages()
{
	for (;;)
	{
		xPL_processMessages(-1);
		sleep(1);
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
            char s[255];

            cJSON *root;
        	root = cJSON_CreateObject();
        	cJSON_AddItemToObject(root, "DEVICE", cJSON_CreateString(samples[i][0]));
        	cJSON_AddItemToObject(root, "TYPE", cJSON_CreateString(samples[i][1]));
	        RANDOMIZE();

	        if(xpl_str_to_type(samples[i][1]) == XPL_TYPE_INPUT_SENSOR_BASIC)
	        {
	           unsigned int a = RANDOM(100);
                if(a < 60)
    	            snprintf(s, sizeof(s), "0");
                else
    	            snprintf(s, sizeof(s), "1");
	        }
	        {
    	        snprintf(s, sizeof(s), "%d", RANDOM(100));
            }
        	cJSON_AddItemToObject(root, "CURRENT", cJSON_CreateString(s));
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

    /*Add xPL broadcast messaging*/
    if ((xpl_edams_message_cmnd = xPL_createBroadcastMessage(xpl_edams_service, xPL_MESSAGE_COMMAND)) == NULL)
    {
        debug(stderr, _("Can't create broadcast message"));
    }

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
        debug(stdout, _("Closing xPL Ecore_Pipe"));
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
    debug(stdout, _("Initialize xPL service"));

	/*Initialize xPL*/
	if (!xPL_initialize(xPL_getParsedConnectionType()))
	{
	    debug(stderr, _("Can't init xPL service"));
        XPL_STARTED	= EINA_FALSE;
	    return EINA_FALSE;
	}

	/*Create an xpl service*/
	xpl_edams_service = xPL_createService("edams", "xpl", "edams.xpl");
	xPL_setServiceVersion(xpl_edams_service, VERSION);

	/*Enable the service*/
	xPL_setServiceEnabled(xpl_edams_service, EINA_TRUE);

    xpl_devices = NULL;
    XPL_STARTED	= EINA_TRUE;

	return EINA_TRUE;
}/*xpl_init*/


/*
 *
 */
Eina_Bool
xpl_shutdown()
{
	debug(stdout, _("Shutdown xPL..."));

	if(XPL_STARTED == EINA_TRUE)
	{
        kill(child_pid, SIGKILL);
	    xPL_setServiceEnabled(xpl_edams_service, EINA_FALSE);
	    xPL_releaseService(xpl_edams_service);
	    xPL_shutdown();

        if (xpl_devices)
        {
            char *it;
            EINA_LIST_FREE(xpl_devices, it)
                 eina_stringshare_del(it);
        }
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

    if(!widget_xpl_data1_get(widget))
    {
	    asprintf(&s,"<ps><ps><em>control.basic<br>\
					{<br>\
					<tab>device=%s<br>\
 					<tab>type=%s<br>\
					<tab>current=%s<br>\
					}<br></em>",
					widget_xpl_device_get(widget),
					xpl_type_to_str(widget_xpl_type_get(widget)),
					widget_xpl_current_get(widget));
    }
    else
    {
	    asprintf(&s,"<ps><ps><em>control.basic<br>\
					{<br>\
					<tab>device=%s<br>\
 					<tab>type=%s<br>\
					<tab>current=%s<br>\
					<tab>data11=%s<br>\
					}<br></em>",
					widget_xpl_device_get(widget),
					xpl_type_to_str(widget_xpl_type_get(widget)),
					widget_xpl_current_get(widget),
					widget_xpl_data1_get(widget));
    }

    return s;
}/*xpl_control_basic_cmnd_to_elm_str*/



/*
 *
 */
void
xpl_control_basic_cmnd_to_dbg(Widget *widget)
{
    if(!widget_xpl_data1_get(widget))
    {
	    fprintf(stdout, "\ncontrol.basic\n\
					{\n\
					\tdevice=%s\n\
 					\ttype=%s\n\
					\tcurrent=%s\n\
					}\n",
					widget_xpl_device_get(widget),
					xpl_type_to_str(widget_xpl_type_get(widget)),
					widget_xpl_current_get(widget));
    }
    else
    {
	    fprintf(stdout, "\ncontrol.basic\n\
					{\n\
					\tdevice=%s\n\
 					\ttype=%s\n\
					\tcurrent=%s\n\
					\tdata1=%s\n\
					}\n",
					widget_xpl_device_get(widget),
					xpl_type_to_str(widget_xpl_type_get(widget)),
					widget_xpl_current_get(widget),
					widget_xpl_data1_get(widget));
    }
}/*xpl_control_basic_cmnd_debug*/


/*
 *Return string representation of Device_Type 'type' arg.
 */
const char *
xpl_type_to_str(Xpl_Type type)
{
	if     (type == XPL_TYPE_BATTERY_SENSOR_BASIC)		return "battery";
	else if(type == XPL_TYPE_COUNT_SENSOR_BASIC)		return "count";
	else if(type == XPL_TYPE_CURRENT_SENSOR_BASIC)		return "current";
	else if(type == XPL_TYPE_DIRECTION_SENSOR_BASIC)	return "direction";
	else if(type == XPL_TYPE_DISTANCE_SENSOR_BASIC)		return "distance";
	else if(type == XPL_TYPE_ENERGY_SENSOR_BASIC)		return "energy";
	else if(type == XPL_TYPE_FAN_SENSOR_BASIC)			return "fan";
	else if(type == XPL_TYPE_GENERIC_SENSOR_BASIC)		return "generic";
	else if(type == XPL_TYPE_HUMIDITY_SENSOR_BASIC)		return "humidity";
	else if(type == XPL_TYPE_INPUT_SENSOR_BASIC)		return "input";
	else if(type == XPL_TYPE_OUTPUT_SENSOR_BASIC)		return "output";
	else if(type == XPL_TYPE_POWER_SENSOR_BASIC)		return "power";
	else if(type == XPL_TYPE_PRESSURE_SENSOR_BASIC)		return "pressure";
	else if(type == XPL_TYPE_SETPOINT_SENSOR_BASIC)		return "setpoint";
	else if(type == XPL_TYPE_SPEED_SENSOR_BASIC)		return "speed";
	else if(type == XPL_TYPE_TEMP_SENSOR_BASIC)			return "temp";
	else if(type == XPL_TYPE_UV_SENSOR_BASIC)			return "uv";
	else if(type == XPL_TYPE_VOLTAGE_SENSOR_BASIC)		return "voltage";
	else if(type == XPL_TYPE_VOLUME_SENSOR_BASIC)		return "volume";
	else if(type == XPL_TYPE_WEIGHT_SENSOR_BASIC)		return "weight";
	else if(type == XPL_TYPE_BALANCE_CONTROL_BASIC)		return "balance";
	else if(type == XPL_TYPE_FLAG_CONTROL_BASIC)		return "flag";
	else if(type == XPL_TYPE_INFRARED_CONTROL_BASIC)	return "infrared";
	else if(type == XPL_TYPE_INPUT_CONTROL_BASIC)		return "input";
	else if(type == XPL_TYPE_MACRO_CONTROL_BASIC)		return "macro";
	else if(type == XPL_TYPE_MUTE_CONTROL_BASIC)		return "mute";
	else if(type == XPL_TYPE_OUTPUT_CONTROL_BASIC)		return "output";
	else if(type == XPL_TYPE_VARIABLE_CONTROL_BASIC)	return "variable";
	else if(type == XPL_TYPE_PERIODIC_CONTROL_BASIC)	return "periodic";
	else if(type == XPL_TYPE_SCHEDULED_CONTROL_BASIC)	return "scheduled";
	else if(type == XPL_TYPE_SLIDER_CONTROL_BASIC)		return "slider";
	else if(type == XPL_TYPE_TIMER_CONTROL_BASIC)		return "timer";
	else 											    return NULL;
}/*xpl_type_to_str*/



/*
 *Return Xpl_Type representation of 'xpl_type' arg.
 */
Xpl_Type
xpl_str_to_type(const char *xpl_type)
{
	if(!xpl_type) return XPL_TYPE_UNKNOWN;

	if(strcmp(xpl_type, "battery") == 0) 				return XPL_TYPE_BATTERY_SENSOR_BASIC;
	else if(strcmp(xpl_type, "count") == 0)			    return XPL_TYPE_COUNT_SENSOR_BASIC;
	else if(strcmp(xpl_type ,"current") == 0)			return XPL_TYPE_CURRENT_SENSOR_BASIC;
	else if(strcmp(xpl_type, "direction") == 0)		    return XPL_TYPE_DIRECTION_SENSOR_BASIC;
	else if(strcmp(xpl_type, "distance") == 0)			return XPL_TYPE_DISTANCE_SENSOR_BASIC;
	else if(strcmp(xpl_type, "energy") == 0)			return XPL_TYPE_ENERGY_SENSOR_BASIC;
	else if(strcmp(xpl_type, "fan") == 0)				return XPL_TYPE_FAN_SENSOR_BASIC;
	else if(strcmp(xpl_type, "generic") == 0)			return XPL_TYPE_GENERIC_SENSOR_BASIC;
	else if(strcmp(xpl_type, "humidity") == 0)			return XPL_TYPE_HUMIDITY_SENSOR_BASIC;
	else if(strcmp(xpl_type, "input") == 0)			    return XPL_TYPE_INPUT_SENSOR_BASIC;
	else if(strcmp(xpl_type, "output") == 0)			return XPL_TYPE_OUTPUT_SENSOR_BASIC;
	else if(strcmp(xpl_type, "power") == 0)			    return XPL_TYPE_POWER_SENSOR_BASIC;
	else if(strcmp(xpl_type, "pressure") == 0)			return XPL_TYPE_PRESSURE_SENSOR_BASIC;
	else if(strcmp(xpl_type, "setpoint") == 0)			return XPL_TYPE_SETPOINT_SENSOR_BASIC;
	else if(strcmp(xpl_type, "speed") == 0)			    return XPL_TYPE_SPEED_SENSOR_BASIC;
	else if(strcmp(xpl_type, "temp") == 0)				return XPL_TYPE_TEMP_SENSOR_BASIC;
	else if(strcmp(xpl_type, "uv") == 0)				return XPL_TYPE_UV_SENSOR_BASIC;
	else if(strcmp(xpl_type, "voltage") == 0)			return XPL_TYPE_VOLTAGE_SENSOR_BASIC;
	else if(strcmp(xpl_type, "volume") == 0)			return XPL_TYPE_VOLUME_SENSOR_BASIC;
	else if(strcmp(xpl_type, "weight") == 0)			return XPL_TYPE_WEIGHT_SENSOR_BASIC;
	else if(strcmp(xpl_type, "balance") == 0)			return XPL_TYPE_BALANCE_CONTROL_BASIC;
	else if(strcmp(xpl_type, "flag") == 0)				return XPL_TYPE_FLAG_CONTROL_BASIC;
	else if(strcmp(xpl_type, "infrared") == 0)			return XPL_TYPE_INFRARED_CONTROL_BASIC;
	else if(strcmp(xpl_type, "input") == 0)			    return XPL_TYPE_INPUT_CONTROL_BASIC;
	else if(strcmp(xpl_type, "macro") == 0)			    return XPL_TYPE_MACRO_CONTROL_BASIC;
	else if(strcmp(xpl_type, "mute") == 0)				return XPL_TYPE_MUTE_CONTROL_BASIC;
	else if(strcmp(xpl_type, "output") == 0)			return XPL_TYPE_OUTPUT_CONTROL_BASIC;
	else if(strcmp(xpl_type, "variable") == 0)			return XPL_TYPE_VARIABLE_CONTROL_BASIC;
	else if(strcmp(xpl_type, "periodic") == 0)			return XPL_TYPE_PERIODIC_CONTROL_BASIC;
	else if(strcmp(xpl_type, "scheduled") == 0)		    return XPL_TYPE_SCHEDULED_CONTROL_BASIC;
	else if(strcmp(xpl_type, "slider") == 0)			return XPL_TYPE_SLIDER_CONTROL_BASIC;
	else if(strcmp(xpl_type, "timer") == 0)			    return XPL_TYPE_TIMER_CONTROL_BASIC;
	else										        return XPL_TYPE_UNKNOWN;
}/*xpl_str_to_type*/


/*
 *
 */
int
xpl_type_current_max_get(Xpl_Type type)
{
	if     (type == XPL_TYPE_BATTERY_SENSOR_BASIC)		return 100;
	else if(type == XPL_TYPE_COUNT_SENSOR_BASIC)		return 10000;
	else if(type == XPL_TYPE_CURRENT_SENSOR_BASIC)		return 100;
	else if(type == XPL_TYPE_DIRECTION_SENSOR_BASIC)	return 360;
	else if(type == XPL_TYPE_DISTANCE_SENSOR_BASIC)		return 100;
	else if(type == XPL_TYPE_ENERGY_SENSOR_BASIC)		return 1000;
	else if(type == XPL_TYPE_FAN_SENSOR_BASIC)			return 10000;
	else if(type == XPL_TYPE_GENERIC_SENSOR_BASIC)		return 10000;
	else if(type == XPL_TYPE_HUMIDITY_SENSOR_BASIC)		return 100;
	else if(type == XPL_TYPE_INPUT_SENSOR_BASIC)		return 2;       /*3 states, LOW/HIGH/PULSE*/
	else if(type == XPL_TYPE_OUTPUT_SENSOR_BASIC)		return 1;
	else if(type == XPL_TYPE_POWER_SENSOR_BASIC)		return 1000;
	else if(type == XPL_TYPE_PRESSURE_SENSOR_BASIC)		return 1000;
	else if(type == XPL_TYPE_SETPOINT_SENSOR_BASIC)		return 300;
	else if(type == XPL_TYPE_SPEED_SENSOR_BASIC)		return 1000;
	else if(type == XPL_TYPE_TEMP_SENSOR_BASIC)			return 100;
	else if(type == XPL_TYPE_UV_SENSOR_BASIC)			return 15;
	else if(type == XPL_TYPE_VOLTAGE_SENSOR_BASIC)		return 1000;
	else if(type == XPL_TYPE_VOLUME_SENSOR_BASIC)		return 1000;
	else if(type == XPL_TYPE_WEIGHT_SENSOR_BASIC)		return 1000;
	else 											    return 0;

}/*xpl_type_max_get*/


/*
 *
 */
int
xpl_type_current_min_get(Xpl_Type type)
{
	if     (type == XPL_TYPE_BATTERY_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_COUNT_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_CURRENT_SENSOR_BASIC)		return -100;
	else if(type == XPL_TYPE_DIRECTION_SENSOR_BASIC)	return 0;
	else if(type == XPL_TYPE_DISTANCE_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_ENERGY_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_FAN_SENSOR_BASIC)			return 0;
	else if(type == XPL_TYPE_GENERIC_SENSOR_BASIC)		return -10000;
	else if(type == XPL_TYPE_HUMIDITY_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_INPUT_SENSOR_BASIC)		return 0;       /*3 states, LOW/HIGH/PULSE*/
	else if(type == XPL_TYPE_OUTPUT_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_POWER_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_PRESSURE_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_SETPOINT_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_SPEED_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_TEMP_SENSOR_BASIC)			return -100;
	else if(type == XPL_TYPE_UV_SENSOR_BASIC)			return 1;
	else if(type == XPL_TYPE_VOLTAGE_SENSOR_BASIC)		return -1000;
	else if(type == XPL_TYPE_VOLUME_SENSOR_BASIC)		return 0;
	else if(type == XPL_TYPE_WEIGHT_SENSOR_BASIC)		return 0;
	else 											    return 0;

}/*xpl_type_min_get*/


/*
 *
 */
const char *
xpl_type_to_units(Xpl_Type type)
{
	if     (type == XPL_TYPE_BATTERY_SENSOR_BASIC)		return _("Percent");
	else if(type == XPL_TYPE_COUNT_SENSOR_BASIC)		return _("Counter");
	else if(type == XPL_TYPE_CURRENT_SENSOR_BASIC)		return _("Amps");
	else if(type == XPL_TYPE_DIRECTION_SENSOR_BASIC)	return _("Degrees");
	else if(type == XPL_TYPE_DISTANCE_SENSOR_BASIC)		return _("Meters");
	else if(type == XPL_TYPE_ENERGY_SENSOR_BASIC)		return _("Kilowatt hours");
	else if(type == XPL_TYPE_FAN_SENSOR_BASIC)			return _("Rotation/min");
	else if(type == XPL_TYPE_GENERIC_SENSOR_BASIC)		return _("Generic");
	else if(type == XPL_TYPE_HUMIDITY_SENSOR_BASIC)		return _("Humidity ratio");
	else if(type == XPL_TYPE_INPUT_SENSOR_BASIC)		return _("Input");
	else if(type == XPL_TYPE_OUTPUT_SENSOR_BASIC)		return _("Output");
	else if(type == XPL_TYPE_POWER_SENSOR_BASIC)		return _("Kilowatt");
	else if(type == XPL_TYPE_PRESSURE_SENSOR_BASIC)		return _("Pascals");
	else if(type == XPL_TYPE_SETPOINT_SENSOR_BASIC)		return _("Degrees") ;
	else if(type == XPL_TYPE_SPEED_SENSOR_BASIC)		return _("Miles per Hour");
	else if(type == XPL_TYPE_TEMP_SENSOR_BASIC)			return _("Celsius");
	else if(type == XPL_TYPE_UV_SENSOR_BASIC)			return _("UV");
	else if(type == XPL_TYPE_VOLTAGE_SENSOR_BASIC)		return _("Volts");
	else if(type == XPL_TYPE_VOLUME_SENSOR_BASIC)		return _("Cubic meter");
	else if(type == XPL_TYPE_WEIGHT_SENSOR_BASIC)		return _("Kilograms");
	else 											    return NULL;
}/*xpl_type_to_units*/

/*
 *
 */
const char *
xpl_type_to_unit_symbol(Xpl_Type type)
{
	if     (type == XPL_TYPE_BATTERY_SENSOR_BASIC)		return _("%");
	else if(type == XPL_TYPE_COUNT_SENSOR_BASIC)		return NULL;
	else if(type == XPL_TYPE_CURRENT_SENSOR_BASIC)		return _("A");
	else if(type == XPL_TYPE_DIRECTION_SENSOR_BASIC)	return _("o");
	else if(type == XPL_TYPE_DISTANCE_SENSOR_BASIC)		return _("m");
	else if(type == XPL_TYPE_ENERGY_SENSOR_BASIC)		return _("kWh");
	else if(type == XPL_TYPE_FAN_SENSOR_BASIC)			return _("RPM");
	else if(type == XPL_TYPE_GENERIC_SENSOR_BASIC)		return NULL;
	else if(type == XPL_TYPE_HUMIDITY_SENSOR_BASIC)		return _("%");
	else if(type == XPL_TYPE_INPUT_SENSOR_BASIC)		return NULL;
	else if(type == XPL_TYPE_OUTPUT_SENSOR_BASIC)		return NULL;
	else if(type == XPL_TYPE_POWER_SENSOR_BASIC)		return _("kW");
	else if(type == XPL_TYPE_PRESSURE_SENSOR_BASIC)		return _("N/m2");
	else if(type == XPL_TYPE_SETPOINT_SENSOR_BASIC)		return _("C") ;
	else if(type == XPL_TYPE_SPEED_SENSOR_BASIC)		return _("MpH");
	else if(type == XPL_TYPE_TEMP_SENSOR_BASIC)			return _("C");
	else if(type == XPL_TYPE_UV_SENSOR_BASIC)			return NULL;
	else if(type == XPL_TYPE_VOLTAGE_SENSOR_BASIC)		return _("V");
	else if(type == XPL_TYPE_VOLUME_SENSOR_BASIC)		return _("m3");
	else if(type == XPL_TYPE_WEIGHT_SENSOR_BASIC)		return _("kg");
	else 											    return NULL;
}/*xpl_type_to_unit_symbol*/


/*
 *
 */
Eina_Bool
xpl_control_basic_cmnd_send(Widget *widget)
{

    xpl_control_basic_cmnd_to_dbg(widget);

  	xPL_setSchema(xpl_edams_message_cmnd, "control", "basic");

    /*Install the value(s) and send the message*/
  	xPL_setMessageNamedValue(xpl_edams_message_cmnd, "device", widget_xpl_device_get(widget));
  	xPL_setMessageNamedValue(xpl_edams_message_cmnd, "type", xpl_type_to_str(widget_xpl_type_get(widget)));
  	xPL_setMessageNamedValue(xpl_edams_message_cmnd, "current", widget_xpl_current_get(widget));

	if(widget_xpl_data1_get(widget))
	  	xPL_setMessageNamedValue(xpl_edams_message_cmnd, "data1", widget_xpl_data1_get(widget));

	/*Broadcast the message*/
	if (!xPL_sendMessage(xpl_edams_message_cmnd))
	{
		debug(stderr, _("Can't send xPL message"));
		return EINA_FALSE;
	}

    return EINA_TRUE;
}/*xpl_control_basic_cmnd_send*/
