/*
 * widget.h
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

#ifndef __WIDGET_H__
#define __WIDGET_H__

typedef struct _Widget Widget;

typedef enum _Widget_Class_
{
	WIDGET_CLASS_UNKNOWN    = (0),
	WIDGET_CLASS_SENSOR   	= (1),
	WIDGET_CLASS_CONTROL  	= (2),
	WIDGET_CLASS_VIRTUAL	= (3),
	WIDGET_CLASS_LAST
}Widget_Class;

Widget_Class widget_str_to_class(const char *str);
const char *widget_class_to_str(Widget_Class class);

#endif /* __WIDGET_H__ */
