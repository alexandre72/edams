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

/*
 *
 */
Eina_Bool
exec_action(const char *data)
{
   pid_t child_pid;
   Ecore_Exe *child_handle;

	cJSON *root = cJSON_Parse(data);
	cJSON *exec = cJSON_GetObjectItem(root,"EXEC");
	cJSON *terminal = cJSON_GetObjectItem(root,"TERMINAL");
	fprintf(stdout, "Execute %s with '%s' arg\n", cJSON_Print(exec), cJSON_Print(terminal));

	child_handle = ecore_exe_pipe_run(cJSON_Print(exec),
                                    ECORE_EXE_PIPE_WRITE |
                                    ECORE_EXE_PIPE_READ_LINE_BUFFERED |
                                    ECORE_EXE_PIPE_READ, NULL);

   	if (child_handle == NULL)
	{
        debug(stderr, _("Could not create a child process!"));
		return EINA_FALSE;
	}

   child_pid = ecore_exe_pid_get(child_handle);

	if (child_pid == -1)
   	{
		debug(stderr, _("Could not retrieve the PID"));
		return EINA_FALSE;
	}
	else
	{
		debug(stdout, _("The child process has PID:%d\n"), child_pid);
		//statusbar_text_set(_("Launching '%s' with PID:%d"), "dialog-informations");
		return EINA_TRUE;
	}
	cJSON_Delete(root);
}



/*
 *
 */
Eina_Bool
debug_action(const char *data)
{
	cJSON *root = cJSON_Parse(data);
	cJSON *item = cJSON_GetObjectItem(root,"PRINT");

	if(item)
		fprintf(stdout, "%s", cJSON_Print(item));
	else
		return EINA_FALSE;

	cJSON_Delete(root);
	return EINA_TRUE;
}


/*
 *Parse and perfom Action 'action' arg.
 */
Eina_Bool
action_parse(Action *action)
{
	if(!action_data_get(action))
	{
		debug(stderr, _("Coulnd't execute action with no data passed in arg"));
		return EINA_FALSE;
	}

	switch(action_type_get(action))
	{
		case CMND_ACTION:
				break;
		case MAIL_ACTION:
				break;
		case EXEC_ACTION:
				return exec_action(action_data_get(action));
				break;
		case DEBUG_ACTION:
				 return debug_action(action_data_get(action));
				 break;

		case UNKNOWN_ACTION:
		case ACTION_TYPE_LAST:
				debug(stderr, _("Coulnd't execute an unknown action"));
				return EINA_FALSE;
				break;
	}

}
