/*
 * action.h
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


#ifndef __ACTION_H
#define __ACTION_H

#include <Eina.h>

typedef struct _Action Action;

typedef enum _Condition_
{
	CONDITION_UNKNOWN		    = (0),
	CONDITION_EGAL_TO		    = (1),
	CONDITION_LESS_THAN			= (2),
	CONDITION_MORE_THAN			= (3),
	CONDITION_LESS_OR_EGAL_TO	= (4),
	CONDITION_MORE_OR_EGAL_TO	= (5),
	CONDITION_LAST
}Condition;


typedef enum _Action_Type
{
	ACTION_TYPE_UNKNOWN     = (0),
	ACTION_TYPE_CMND		= (1),
	ACTION_TYPE_MAIL		= (2),
	ACTION_TYPE_EXEC		= (3),
	ACTION_TYPE_DEBUG		= (4),
	ACTION_TYPE_LAST
}Action_Type;


Eina_Bool action_parse(Action *action);
const char *action_type_to_desc(Action_Type type);
const char *action_type_to_str(Action_Type type);
Action_Type action_str_to_type(const char *s);
Condition action_str_to_condition(const char *s);
const char *action_condition_to_str(Condition condition);

const char *action_debug_data_format(const char *print);
const char *action_exec_data_format(const char *exec, const char *terminal);
const char *action_mail_data_format(const char *from, const char *to, const char *subject, const char *body);
const char *action_cmnd_data_format(const char *device, const char *type, const char *current, const char *data1);

#endif /*__ACTION_H*/
