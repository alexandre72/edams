#include "widget.h"

#include <stdio.h>

Widget_Class
widget_str_to_class(const char *str)
{
	if(!str) return WIDGET_CLASS_UNKNOWN;

	if(strcmp(str, "sensor.basic") == 0) 			return WIDGET_CLASS_XPL_SENSOR_BASIC;
	else if(strcmp(str, "control.basic") == 0)		return WIDGET_CLASS_XPL_CONTROL_BASIC;
	else if(strcmp(str, "virtual") == 0)			return WIDGET_CLASS_VIRTUAL;

}/*widget_str_to_class*/



/*
 *Return string representation of Widget_Class 'class' arg.
 */
const char *
widget_class_to_str(Widget_Class class)
{
	if(class == WIDGET_CLASS_XPL_SENSOR_BASIC)				return "sensor.basic";
	else if(class == WIDGET_CLASS_XPL_CONTROL_BASIC)		return "control.basic";
	else if(class == WIDGET_CLASS_VIRTUAL)				return "virtual";
	else 									            return NULL;
}/*widget_class_to_str*/
