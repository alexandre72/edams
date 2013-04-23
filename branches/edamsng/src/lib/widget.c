/*
 * widget.c
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



#include "widget.h"

#include <stdio.h>
#include <string.h>

Widget_Class
widget_str_to_class(const char *str)
{
	if(!str) return WIDGET_CLASS_UNKNOWN;

	if(strcmp(str, "sensor") == 0) 			return WIDGET_CLASS_SENSOR;
	else if(strcmp(str, "control") == 0)		return WIDGET_CLASS_CONTROL;
	else if(strcmp(str, "virtual") == 0)			return WIDGET_CLASS_VIRTUAL;
    else return WIDGET_CLASS_UNKNOWN;
}/*widget_str_to_class*/



/*
 *Return string representation of Widget_Class 'class' arg.
 */
const char *
widget_class_to_str(Widget_Class class)
{
	if(class == WIDGET_CLASS_SENSOR)			return "sensor";
	else if(class == WIDGET_CLASS_CONTROL)		return "control";
	else if(class == WIDGET_CLASS_VIRTUAL)		return "virtual";
	else 									    return NULL;
}/*widget_class_to_str*/
