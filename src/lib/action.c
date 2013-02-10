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
#include "utils.h"
#include "xpl.h"


/*
 *
 */
Eina_Bool
exec_action(const char *data)
{
   	pid_t child_pid;
   	Ecore_Exe *child_handle;

	cJSON *root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

	cJSON *jexec = cJSON_GetObjectItem(root,"EXEC");
	cJSON *jterminal = cJSON_GetObjectItem(root,"TERMINAL");

    const char *exec = cJSON_Print(jexec);
    const char *terminal = cJSON_Print(jterminal);

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
		const char *s;
		asprintf(&s, _("Exec '%s' with PID '%d'"), exec, child_pid);
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
Eina_Bool
mail_action(const char *data)
{
	cJSON *root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

	cJSON *jfrom = cJSON_GetObjectItem(root, "FROM");
	cJSON *jto = cJSON_GetObjectItem(root, "TO");
	cJSON *jsubject = cJSON_GetObjectItem(root, "SUBJECT");
	cJSON *jbody = cJSON_GetObjectItem(root, "BODY");

    const char *from = cJSON_PrintUnformatted(jfrom);
    const char *to = cJSON_PrintUnformatted(jto);
    const char *subject = cJSON_PrintUnformatted(jsubject);
    const char *body = cJSON_PrintUnformatted(jbody);

    strdelstr(from, "\"");
    strdelstr(to, "\"");
    strdelstr(subject, "\"");
    strdelstr(body, "\"");

	if(from && to && subject && body)
	{
		fprintf(stdout, "From:%s\n", from);
		fprintf(stdout, "To:%s\n", to);
		fprintf(stdout, "Subject:%s\n", subject);
		fprintf(stdout, "Body:%s\n", body);

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
Eina_Bool
debug_action(const char *data)
{
	cJSON *root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

	cJSON *jprint = cJSON_GetObjectItem(root,"PRINT");

    const char *print =cJSON_PrintUnformatted(jprint);

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
Eina_Bool
cmnd_action(const char *data)
{
    Widget *widget;
	cJSON *root;
    cJSON *device, *current, *data1;
/*
	root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

	device = cJSON_GetObjectItem(root, "DEVICE");
	current = cJSON_GetObjectItem(root, "CURRENT");

	if(device && current)
	{
        device = device_load(device_filename);

        if(!device) return EINA_FALSE;

        widget_xpl_current_set(device, current);
	}
	else
	    return EINA_FALSE;

	data1 = cJSON_GetObjectItem(root, "DATA1");
    if(data1)
        widget_xpl_data1_set(widget, data1);

	cJSON_Delete(root);
*/
    return xpl_control_basic_cmnd_send(widget);
}/*cmnd_action*/



/*
 *Parse and perfom Action 'action' arg.
 */
Eina_Bool
action_parse(Action *action)
{
	if(!action_data_get(action))
	{
		debug(stderr, _("Can't execute action with no data passed in arg"));
		return EINA_FALSE;
	}

	switch(action_type_get(action))
	{
		case ACTION_TYPE_CMND:
				return cmnd_action(action_data_get(action));
				break;
		case ACTION_TYPE_MAIL:
				return mail_action(action_data_get(action));
				break;
		case ACTION_TYPE_EXEC:
				return exec_action(action_data_get(action));
				break;
		case ACTION_TYPE_DEBUG:
				 return debug_action(action_data_get(action));
				 break;

		case ACTION_TYPE_UNKNOWN:
		case ACTION_TYPE_LAST:
				debug(stderr, _("Can't execute an unknown action"));
				return EINA_FALSE;
				break;
	}

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
	else							return CONDITION_UNKNOWN;
}/*action_str_to_condition*/


/*
 *Return string description of Action_Type 'type' arg.
 */
const char *
action_type_to_desc(Action_Type type)
{
	if(type == ACTION_TYPE_CMND)			return _("Send xPL CMND to control.basic");
	else if(type == ACTION_TYPE_MAIL)		return _("Send a mail");
	else if(type == ACTION_TYPE_EXEC)		return _("Execute an external program");
	else if(type == ACTION_TYPE_DEBUG)		return _("Debug stuff for testing purpose");
	else 									return NULL;
}/*action_type_to_str*/
