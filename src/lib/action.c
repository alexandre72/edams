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
#include "xPL.h"


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

	cJSON *exec = cJSON_GetObjectItem(root,"EXEC");
	cJSON *terminal = cJSON_GetObjectItem(root,"TERMINAL");
	debug(stdout, _("Exec '%s'"), cJSON_PrintUnformatted(exec));

	child_handle = ecore_exe_pipe_run(cJSON_Print(exec),
                                    ECORE_EXE_PIPE_WRITE |
                                    ECORE_EXE_PIPE_READ_LINE_BUFFERED |
                                    ECORE_EXE_PIPE_READ, NULL);

   	if (child_handle == NULL)
	{
        debug(stderr, _("Could not create a child process!"));
		cJSON_Delete(root);
		return EINA_FALSE;
	}

   child_pid = ecore_exe_pid_get(child_handle);

	if (child_pid == -1)
   	{
		debug(stderr, _("Could not retrieve the PID"));
		cJSON_Delete(root);
		return EINA_FALSE;
	}
	else
	{
		debug(stdout, _("The child process has PID:%d\n"), child_pid);
		//statusbar_text_set(_("Launching '%s' with PID:%d"), "dialog-informations");
		cJSON_Delete(root);
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

	cJSON *from = cJSON_GetObjectItem(root, "FROM");
	cJSON *to = cJSON_GetObjectItem(root, "TO");
	cJSON *subject = cJSON_GetObjectItem(root, "SUBJECT");
	cJSON *body = cJSON_GetObjectItem(root, "BODY");

	if(from && to && subject && body)
	{
		fprintf(stdout, "From:%s\n", cJSON_PrintUnformatted(from));
		fprintf(stdout, "To:%s\n", cJSON_PrintUnformatted(to));
		fprintf(stdout, "Subject:%s\n", cJSON_PrintUnformatted(subject));
		fprintf(stdout, "Body:%s\n", cJSON_PrintUnformatted(body));

		//ecore_con_url_url_set(url_con, "smtp://smtp.gmail.com:587");
		//ecore_con_url_httpauth_set(url_con, "username", "userpwd", EINA_TRUE);

		cJSON_Delete(root);
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

	cJSON *item = cJSON_GetObjectItem(root,"PRINT");

	if(item)
		debug(stdout, "%s", cJSON_PrintUnformatted(item));
	else
		return EINA_FALSE;

	cJSON_Delete(root);
	return EINA_TRUE;
}/*debug_action*/



/*
 *
 */
Eina_Bool
cmnd_action(const char *data)
{
    Device *device;
	cJSON *root;
    cJSON *device_filename, *current, *data1;

	root = cJSON_Parse(data);

	if(!root) return EINA_FALSE;

	device_filename = cJSON_GetObjectItem(root, "DEVICE_FILENAME");
	current = cJSON_GetObjectItem(root, "CURRENT");

	if(device_filename && current)
	{
        device = device_load(device_filename);

        if(!device) return EINA_FALSE;

        device_current_set(device, current);
	}
	else
	    return EINA_FALSE;

	data1 = cJSON_GetObjectItem(root, "DATA1");
    if(data1)
        device_data1_set(device, data1);

	cJSON_Delete(root);

    return xpl_control_basic_cmnd_send(device);
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
		case CMND_ACTION:
				return cmnd_action(action_data_get(action));
				break;
		case MAIL_ACTION:
				return mail_action(action_data_get(action));
				break;
		case EXEC_ACTION:
				return exec_action(action_data_get(action));
				break;
		case DEBUG_ACTION:
				 return debug_action(action_data_get(action));
				 break;

		case UNKNOWN_ACTION:
		case ACTION_TYPE_LAST:
				debug(stderr, _("Can't execute an unknown action"));
				return EINA_FALSE;
				break;
	}

}/*action_parse*/
