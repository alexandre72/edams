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



#include "xPL.h"
#include "edams.h"




static xPL_ServicePtr 	xpl_edams_service;
//static	xPL_MessagePtr 	xpl_edams_message_stat;
//static	xPL_MessagePtr 	xpl_edams_message_trig;
static	xPL_MessagePtr 	xpl_edams_message_cmnd;

static void _xpl_sensor_basic_handler(xPL_ServicePtr service, xPL_MessagePtr msg, xPL_ObjectPtr data);



/*
 *Callback called in xPL Message 'cstatic void
handler(void *data __UNUSED__, void *buf, unsigned int len)ontrol.basic' is triggered.
 */
static void
_xpl_sensor_basic_handler(xPL_ServicePtr service __UNUSED__, xPL_MessagePtr msg, xPL_ObjectPtr data __UNUSED__)
{
    char buf[351] = "0";
    xPL_NameValueListPtr values_names;

	Ecore_Pipe *pipe = (Ecore_Pipe *) data;

	values_names = xPL_getMessageBody(msg);

	snprintf(buf, sizeof(buf), "%s!%s!%s",
                         xPL_getNamedValue(values_names, "device"),
                         xPL_getNamedValue(values_names, "type"),
                         xPL_getNamedValue(values_names, "current"));

	ecore_pipe_write(pipe, buf, strlen(buf));

}/*_xpl_sensor_basic_handler*/




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
void
xpl_services_install(Ecore_Pipe *pipe)
{
        /*Add xPL sensor.basic listener*/
        xPL_addServiceListener(xpl_edams_service, _xpl_sensor_basic_handler, xPL_MESSAGE_TRIGGER, "sensor", "basic",(xPL_ObjectPtr)pipe);

        /*Add xPL broadcast messaging*/
        if ((xpl_edams_message_cmnd = xPL_createBroadcastMessage(xpl_edams_service, xPL_MESSAGE_COMMAND)) == NULL)
        {
            debug(stderr, _("Can't create broadcast message"));
        }

}/*xpl_services_install*/




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
		return EINA_TRUE;
	    }

	    /*Create an xpl service*/
	    xpl_edams_service = xPL_createService("edams", "xpl", "edams.xpl");
	    xPL_setServiceVersion(xpl_edams_service, VERSION);

	    /*Enable the service*/
	    xPL_setServiceEnabled(xpl_edams_service, EINA_TRUE);


	    return EINA_TRUE;
}/*xpl_init*/


/*
 *
 */
Eina_Bool
xpl_shutdown()
{
	debug(stdout, _("Shutdown xPL..."));
	xPL_setServiceEnabled(xpl_edams_service, EINA_FALSE);
	xPL_releaseService(xpl_edams_service);
	xPL_shutdown();

	return 0;
}/*xpl_shutdown*/



/*
 *
 */
const char*
xpl_control_basic_cmnd_to_elm_str(Device *device)
{
    const char *s;

    if(!device_data1_get(device))
    {
	    asprintf(&s,"<ps><ps><em>control.basic<br>\
					{<br>\
					<tab>device=%s<br>\
 					<tab>type=%s<br>\
					<tab>current=%s<br>\
					}<br></em>",
					device_name_get(device),
					device_type_to_str(device_type_get(device)),
					device_current_get(device));
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
					device_name_get(device),
					device_type_to_str(device_type_get(device)),
					device_current_get(device),
					device_data1_get(device));
    }

    return s;
}/*xpl_control_basic_cmnd_to_elm_str*/




/*
 *
 */
void
xpl_control_basic_cmnd_to_dbg(Device *device)
{
    if(!device_data1_get(device))
    {
	    debug(stdout, "control.basic\n\
					{\n\
					\tdevice=%s\n\
 					\ttype=%s\n\
					\tcurrent=%s\n\
					}",
					device_name_get(device),
					device_type_to_str(device_type_get(device)),
					device_current_get(device));
    }
    else
    {
	    debug(stdout, "control.basic\n\
					{\n\
					\tdevice=%s\n\
 					\ttype=%s\n\
					\tcurrent=%s\n\
					\tdata1=%s\n\
					}",
					device_name_get(device),
					device_type_to_str(device_type_get(device)),
					device_current_get(device),
					device_data1_get(device));
    }
}/*xpl_control_basic_cmnd_debug*/





/*
 *
 */
Eina_Bool
xpl_control_basic_cmnd_send(Device *device)
{

    xpl_control_basic_cmnd_to_dbg(device);

  	xPL_setSchema(xpl_edams_message_cmnd, "control", "basic");

    /*Install the value(s) and send the message*/
  	xPL_setMessageNamedValue(xpl_edams_message_cmnd, "device", device_name_get(device));
  	xPL_setMessageNamedValue(xpl_edams_message_cmnd, "type", device_type_to_str(device_type_get(device)));
  	xPL_setMessageNamedValue(xpl_edams_message_cmnd, "current", device_current_get(device));

	if(device_data1_get(device))
	  	xPL_setMessageNamedValue(xpl_edams_message_cmnd, "data1", device_data1_get(device));

	/*Broadcast the message*/
	if (!xPL_sendMessage(xpl_edams_message_cmnd))
	{
		debug(stderr, _("Can't send xPL message"));
		return EINA_FALSE;
	}

    return EINA_TRUE;
}/*xpl_control_basic_cmnd_send*/
