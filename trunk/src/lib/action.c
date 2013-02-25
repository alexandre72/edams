/*
 * action.c
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

#include "action.h"
#include "cJSON.h"
#include "edams.h"
#include "global_view.h"
#include "location.h"
#include "utils.h"
#include "xpl.h"

/*
 *
 */
const char *
action_exec_data_format(const char *exec, const char *terminal)
{
    cJSON *root;
    const char *s;

	root = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "EXEC", cJSON_CreateString(exec));
    cJSON_AddItemToObject(root, "TERMINAL", cJSON_CreateString(terminal));
    s = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

    return s;
}/*exec_action_data_format*/


/*
 *
 */
static Eina_Bool
exec_action_parse(const char *data)
{
   	pid_t child_pid;
   	Ecore_Exe *child_handle;

	cJSON *root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

	cJSON *jexec = cJSON_GetObjectItem(root,"EXEC");
	cJSON *jterminal = cJSON_GetObjectItem(root,"TERMINAL");

    char *exec = cJSON_Print(jexec);
    char *terminal = cJSON_Print(jterminal);

    strdelstr(exec, "\"");
    strdelstr(terminal, "\"");

	child_handle = ecore_exe_pipe_run(exec,
                                    ECORE_EXE_PIPE_WRITE |
                                    ECORE_EXE_PIPE_READ_LINE_BUFFERED |
                                    ECORE_EXE_PIPE_READ, NULL);

   	if (child_handle == NULL)
	{
        debug(stderr, _("Can't create an Ecore_Exec_Pipe process"));
        FREE(exec);
        FREE(terminal);
		cJSON_Delete(root);
		return EINA_FALSE;
	}

   child_pid = ecore_exe_pid_get(child_handle);

	if (child_pid == -1)
   	{
        debug(stderr, _("Can't create get PID of Ecore_Exec_Pipe process"));
		cJSON_Delete(root);
        FREE(exec);
        FREE(terminal);
		return EINA_FALSE;
	}
	else
	{
		char *s;
		asprintf(&s, _("Exec '%s' with PID '%d'"), exec, child_pid);
        debug(stdout, s);
		statusbar_text_set(s, "dialog-informations");
        FREE(s);
		cJSON_Delete(root);
        FREE(exec);
        FREE(terminal);
		return EINA_TRUE;
	}
}/*exec_action*/



/*
 *
 */
const char *
action_mail_data_format(const char *from, const char *to, const char *subject, const char *body)
{
    cJSON *root;
    const char *s;

	root = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "FROM", cJSON_CreateString(from));
    cJSON_AddItemToObject(root, "TO", cJSON_CreateString(to));
 	cJSON_AddItemToObject(root, "SUBJECT", cJSON_CreateString(subject));
    cJSON_AddItemToObject(root, "BODY", cJSON_CreateString(body));
    s = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

    return s;
}/*mail_action_data_format*/


/*
 *
 */
static Eina_Bool
mail_action_parse(const char *data)
{
	cJSON *root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

	cJSON *jfrom = cJSON_GetObjectItem(root, "FROM");
	cJSON *jto = cJSON_GetObjectItem(root, "TO");
	cJSON *jsubject = cJSON_GetObjectItem(root, "SUBJECT");
	cJSON *jbody = cJSON_GetObjectItem(root, "BODY");

    char *from = cJSON_PrintUnformatted(jfrom);
    char *to = cJSON_PrintUnformatted(jto);
    char *subject = cJSON_PrintUnformatted(jsubject);
    char *body = cJSON_PrintUnformatted(jbody);

    strdelstr(from, "\"");
    strdelstr(to, "\"");
    strdelstr(subject, "\"");
    strdelstr(body, "\"");

	if(from && to && subject && body)
	{
	    FILE *sendmail_pipe;
        char *str;
        asprintf(&str, "sendmail -a default %s", from);
	    sendmail_pipe = popen(str, "w");
	    if (sendmail_pipe)
	    {
    	    fprintf(sendmail_pipe, "To: %s\n", to);
	        fprintf(sendmail_pipe, "From: %s\n", from);
	        fprintf(sendmail_pipe, "Subject: %s\n", subject);
	        fprintf(sendmail_pipe, "%s\n", body);
	    	pclose(sendmail_pipe);
	    }
	    else
	    {
	    	debug(stderr, _("Can't found sendmail"));
	    }
	    FREE(str);

		//ecore_con_url_url_set(url_con, "smtp://smtp.gmail.com:587");
		//ecore_con_url_httpauth_set(url_con, "username", "userpwd", EINA_TRUE);
		cJSON_Delete(root);
		FREE(from);
		FREE(to);
		FREE(subject);
		FREE(body);
		return EINA_TRUE;
	}
	else
	{
		cJSON_Delete(root);
		return EINA_FALSE;
	}
}/*mail_action*/


/*
 *
 */
const char *
action_debug_data_format(const char *print)
{
    cJSON *root;
    const char *s;

	root = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "PRINT", cJSON_CreateString(print));
    s = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

    return s;
}/*debug_action_data_format*/

/*
 *
 */
static Eina_Bool
debug_action_parse(const char *data)
{
	cJSON *root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

	cJSON *jprint = cJSON_GetObjectItem(root, "PRINT");

    char *print =cJSON_PrintUnformatted(jprint);

    strdelstr(print, "\"");

	if(print)
		debug(stdout, print);
	else
		return EINA_FALSE;

	cJSON_Delete(root);
	FREE(print);
	return EINA_TRUE;
}/*debug_action*/


/*
 *
 */
const char *
action_osd_data_format(const char *command, const char *text, double delay)
{
    cJSON *root;
    const char *s;

	root = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "COMMAND", cJSON_CreateString(command));

	if(text)
    	cJSON_AddItemToObject(root, "TEXT", cJSON_CreateString(text));

    if(delay != 0)
    	cJSON_AddItemToObject(root, "DELAY", cJSON_CreateNumber(delay));

    s = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);

    return s;
}/*osd_action_data_format*/


/*
 *
 */
Eina_Bool
osd_action_parse(const char *data)
{
	cJSON *root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

    cJSON *jcommand = cJSON_GetObjectItem(root, "COMMAND");
    cJSON *jtext = cJSON_GetObjectItem(root, "TEXT");
    cJSON *jdelay = cJSON_GetObjectItem(root, "DELAY");

    if(jcommand)
    {
        char *command = cJSON_PrintUnformatted(jcommand);
        strdelstr(command, "\"");

        if((strcmp(command, "clear") == 0) ||
            (strcmp(command, "write") == 0))
         {
            if(jtext)
            {
                char *text = cJSON_PrintUnformatted(jtext);
                strdelstr(text, "\"");

                if(jdelay)
                {
                    char *delay = cJSON_PrintUnformatted(jdelay);
                    global_view_osd_write(text, atoi(delay));
                    xpl_osd_basic_cmnd_send(command, text, delay);
                    FREE(delay);
                }
                else
                {
                    global_view_osd_write(text, -1);
                    xpl_osd_basic_cmnd_send(command, text, NULL);
                    FREE(text);
                }
            }
            FREE(command);
        }
    }
	cJSON_Delete(root);

	return EINA_TRUE;
}/*debug_action*/



/*
 *
 */
const char *
action_cmnd_data_format(const char *device, const char *type, const char *current, const char *data1)
{
    cJSON *root;
    const char *s;

	root = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "DEVICE", cJSON_CreateString(device));
    cJSON_AddItemToObject(root, "TYPE", cJSON_CreateString(type));
 	cJSON_AddItemToObject(root, "CURRENT", cJSON_CreateString(current));

 	if(data1)
        cJSON_AddItemToObject(root, "DATA1", cJSON_CreateString(data1));

    s = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

    return s;
}/*exec_action_format*/


/*
 *
 */
static Eina_Bool
cmnd_action_parse(const char *data)
{
    Widget *widget = NULL;
	cJSON *root;
    cJSON *jdevice, *jtype, *jcurrent, *jdata1;
    char *device, *current, *type, *data1;

	root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

	jdevice = cJSON_GetObjectItem(root, "DEVICE");
	jtype = cJSON_GetObjectItem(root, "TYPE");
	jcurrent = cJSON_GetObjectItem(root, "CURRENT");
	jdata1 = cJSON_GetObjectItem(root, "DATA1");

    device =cJSON_PrintUnformatted(jdevice);
    type =cJSON_PrintUnformatted(jtype);
    current =cJSON_PrintUnformatted(jcurrent);
    data1 =cJSON_PrintUnformatted(jdata1);

    strdelstr(device, "\"");
    strdelstr(type, "\"");
    strdelstr(current, "\"");

    widget_xpl_device_set(widget, device);
    widget_xpl_type_set(widget, type);
    widget_xpl_current_set(widget, current);

    if(data1)
    {
        strdelstr(data1, "\"");
        widget_xpl_data1_set(widget, data1);
    }

	cJSON_Delete(root);
    FREE(device);
    FREE(type);
    FREE(current);
    FREE(data1);

    return xpl_control_basic_cmnd_send(widget);
}/*cmnd_action*/



//{"TYPE":"EXEC","DATA":"/usr/bin/gedit"}


/*
 *Parse and perfom Action 'action' arg.
 */
Eina_Bool
action_parse(Action *action)
{
    if(!action) return EINA_FALSE;


	if(!action_data_get(action))
	{
		debug(stderr, _("Can't execute action with no data passed in arg"));
		return EINA_FALSE;
	}

	switch(action_type_get(action))
	{
		case ACTION_TYPE_CMND:
				return cmnd_action_parse(action_data_get(action));
				break;
		case ACTION_TYPE_MAIL:
				return mail_action_parse(action_data_get(action));
				break;
		case ACTION_TYPE_EXEC:
				return exec_action_parse(action_data_get(action));
				break;
		case ACTION_TYPE_DEBUG:
				 return debug_action_parse(action_data_get(action));
				 break;
		case ACTION_TYPE_OSD:
				 return osd_action_parse(action_data_get(action));
				 break;
		case ACTION_TYPE_UNKNOWN:
		case ACTION_TYPE_LAST:
				debug(stderr, _("Can't execute an unknown action"));
				return EINA_FALSE;
				break;
	}

    return EINA_TRUE;
}/*action_parse*/



/*
 *Return string representation of Condition type of 'condition' arg.
 */
const char *
action_condition_to_str(Condition condition)
{
	if(condition == CONDITION_EGAL_TO)				return "=";
	else if(condition == CONDITION_LESS_THAN)		return "<";
	else if(condition == CONDITION_MORE_THAN)		return ">";
	else if(condition == CONDITION_MORE_OR_EGAL_TO)	return ">=";
	else if(condition == CONDITION_LESS_OR_EGAL_TO)	return "<=";
	else if(condition == CONDITION_DIFFERENT_TO)	return "!=";
	else 							           		return NULL;
}/*action_condition_to_str*/


/*
 *Return Condition representation of 's' arg.
 */
Condition
action_str_to_condition(const char *s)
{
	if(!s) return CONDITION_UNKNOWN;

	if(strcmp(s, "=") == 0) 		return CONDITION_EGAL_TO;
	else if(strcmp(s, "<") == 0)	return CONDITION_LESS_THAN;
	else if(strcmp(s, ">") == 0)	return CONDITION_MORE_THAN;
	else if(strcmp(s, ">=") == 0)	return CONDITION_MORE_OR_EGAL_TO;
	else if(strcmp(s, "<=") == 0)	return CONDITION_LESS_OR_EGAL_TO;
	else if(strcmp(s, "!=") == 0)	return CONDITION_DIFFERENT_TO;
	else							return CONDITION_UNKNOWN;
}/*action_str_to_condition*/


/*
 *Return Condition representation of 's' arg.
 */
Action_Type
action_str_to_type(const char *s)
{
	if(!s) return ACTION_TYPE_UNKNOWN;

	if(strcmp(s, "EXEC") == 0) 		return ACTION_TYPE_EXEC;
	else if(strcmp(s, "CMND") == 0)	return ACTION_TYPE_CMND;
	else if(strcmp(s, "DEBUG") == 0)return ACTION_TYPE_DEBUG;
	else if(strcmp(s, "MAIL") == 0)	return ACTION_TYPE_MAIL;
	else if(strcmp(s, "OSD") == 0)	return ACTION_TYPE_OSD;
	else							return ACTION_TYPE_UNKNOWN;
}/*action_str_to_condition*/


/*
 *Return string description of Action_Type 'type' arg.
 */
const char *
action_type_to_desc(Action_Type type)
{
	if(     type == ACTION_TYPE_CMND)		return _("Send xPL CMND to control.basic");
	else if(type == ACTION_TYPE_MAIL)		return _("Send a mail");
	else if(type == ACTION_TYPE_EXEC)		return _("Execute an external program");
	else if(type == ACTION_TYPE_DEBUG)		return _("Debug stuff for testing purpose");
	else if(type == ACTION_TYPE_OSD)		return _("Send xPL CMND to osd.basic");
	else 									return NULL;
}/*action_type_to_str*/


/*
 *Return string representation of Action_Type 'type' arg.
 */
const char *
action_type_to_str(Action_Type type)
{
	if(     type == ACTION_TYPE_CMND)		return "CMND";
	else if(type == ACTION_TYPE_MAIL)		return "MAIL";
	else if(type == ACTION_TYPE_EXEC)		return "EXEC";
	else if(type == ACTION_TYPE_DEBUG)		return "DEBUG";
	else if(type == ACTION_TYPE_OSD)		return "OSD";
	else 									return NULL;
}/*action_type_to_str*/
