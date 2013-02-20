/*
 * locations.c
 * This file is part of EDAMS
 *
 * Copyright (C) 2012 - Alexandre Dussart
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


#include <Ecore_File.h>
#include <Elementary.h>

#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "location.h"
#include "path.h"
#include "utils.h"

#define LOCATION_FILE_VERSION 0x91



struct _Action
{
	Condition ifcondition;			/*Type of condition(=,>,<,<=,>=). e.g 'EGAL_TO'*/
	const char *ifvalue;			/*Required condition value. e.g '5'*/
	Action_Type type;				/*Type of action to peform. e.g 'CMND_ACTION'*/
	const char *data;				/*Data passed to action separated in json style, depends on action type*/
};


/*TODO:Add in_cosm to enable/disable cosm datasteam feed*/
struct _Widget
{
    unsigned int id;				    /*GID of widget e.g '12'*/
    const char * name;				    /*Name e.g 'temperature'*/
    const char * group;				    /*Name of edc widget group e.g. 'widget/sensor.basic/counter'*/
    Widget_Class class;                 /*Widget class. Eg 'WIDGET_CLASS_CONTROL_BASIC'*/

    /*Widget xPL specifics fields*/
    const char *xpl_device;				/*Name of xpl device 'temperature1'. Should be unique*/
    Xpl_Type xpl_type;			    	/*Type of xpl device e.g. 'TEMP_SENSOR_BASIC_TYPE'*/
    const char *xpl_current;			/*Current data of xpl device*/
    const char *xpl_data1;				/*Additional data. Used for control.basic cmnd structure message*/
    const char *xpl_highest;			/*Highest recorded value*/
    const char *xpl_lowest;				/*Lowest recorded value*/

    /*Cosm specific fields*/
    Eina_Bool cosm;                     /*Enable data sending to cosm*/

    /*Gnuplot*/
    Eina_Bool gnuplot;                  /*Enable gnuplot data generation*/

    /*Actions to perfoms when widget reach certains condition*/
	Eina_List *actions;
};


struct _Location
{
    const char *__eet_filename;		/*Filename of location, generated and based on location's name*/
    unsigned int id;				/*GID of location e.g. '10'*/
    const char *name;				/*Name of location e.g. 'Child Location'*/
	Exposure exposure;	 	        /*Whether the location is indoors or outdoors. e.g. 'EXPOSURE_INDOOR'*/
	double latitude; 				/*The latitude of the location*/
	double longitude; 				/*The longitude of the location*/
	double elevation; 				/*The elevation of the location*/
	unsigned int cosm_feedid;		/*The cosm url feedid location e.g 'http://api.cosm.com/v2/feeds/0001'*/
    unsigned int version;			/*Version of location Eet file*/
    Eina_List * widgets;            /*List of Widget struct*/
    const char *creation;			/*Creation date of location Eet file*/
    const char *revision;			/*Revision date of location Eet file*/
};

static const char ACTION_ENTRY[] = "action";
static const char WIDGET_ENTRY[] = "widget";
static const char LOCATION_ENTRY[] = "location";
static Eet_Data_Descriptor *_location_descriptor = NULL;
static Eet_Data_Descriptor *_widget_descriptor = NULL;
static Eet_Data_Descriptor *_action_descriptor = NULL;

/*******************************************************************************
 *
 *                                  ACTION FUNCS
 *
 ******************************************************************************/
/*
 *
 */
static inline void
_action_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_action_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Action);
    _action_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_action_descriptor, Action, "ifcondition", ifcondition, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_action_descriptor, Action, "ifvalue", ifvalue, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_action_descriptor, Action, "type", type, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_action_descriptor, Action, "data", data, EET_T_STRING);
}/*_action_init*/

/*
 *
 */
static inline void
_action_shutdown(void)
{
    if (!_action_descriptor) return;
    eet_data_descriptor_free(_action_descriptor);
    _action_descriptor = NULL;
}/*_action_shutdown*/

/*
 *
 */
Action *
action_new(Condition ifcondition, const char *ifvalue, Action_Type type, const char *data)
{
    Action *action = calloc(1, sizeof(Action));

    if (!action)
       {
          debug(stderr, _("Can't calloc Action struct"));
          return NULL;
       }

    action->ifcondition = ifcondition;
    action->ifvalue = eina_stringshare_add(ifvalue);
    action->type = type;
    action->data = eina_stringshare_add(data);

    return action;
}/*action_new*/

/*
 *
 */
void
action_free(Action *action)
{
    eina_stringshare_del(action->ifvalue);
    eina_stringshare_del(action->data);
    free(action);
}/*action_free*/


/*
 *
 */
Action *
action_clone(Action *src)
{
    Action *ret = NULL;

    ret->ifvalue = eina_stringshare_add(src->ifvalue);
    ret->ifcondition = src->ifcondition;
    ret->type = src->type;
    ret->data = eina_stringshare_add(src->data);

    return ret;
}/*action_clone*/

/*
 *
 */
inline void
action_ifcondition_set(Action *action, Condition ifcondition)
{
    EINA_SAFETY_ON_NULL_RETURN(action);
	action->ifcondition = ifcondition;
}/*action_ifcondition_set*/

/*
 *
 */
inline Condition
action_ifcondition_get(const Action *action)
{
    return action->ifcondition;
}/*action_ifcondition_get*/

/*
 *
 */
void
action_ifvalue_set(const Action *action, const char *ifvalue)
{
    EINA_SAFETY_ON_NULL_RETURN(action);
    eina_stringshare_replace(&(action->ifvalue), ifvalue);
}/*action_ifvalue_set*/

/*
 *
 */
inline const char *
action_ifvalue_get(const Action *action)
{
    return action->ifvalue;
}/*action_ifvalue_get*/


/*
 *
 */
inline void
action_type_set(Action *action, Action_Type type)
{
    EINA_SAFETY_ON_NULL_RETURN(action);
	action->type = type;
}/*action_type_set*/

/*
 *
 */
inline Action_Type
action_type_get(const Action *action)
{
    return action->type;
}/*action_type_get*/


/*
 *
 */
void
action_data_set(const Action *action, const char *data)
{
    EINA_SAFETY_ON_NULL_RETURN(action);
    eina_stringshare_replace(&(action->data), data);
}/*action_data_set*/

/*
 *
 */
inline const char *
action_data_get(const Action *action)
{
    return action->data;
}/*action_data_get*/


/*******************************************************************************
 *
 *                                  WIDGET FUNCS
 *
 ******************************************************************************/

/*
 *
 */
static inline void
_widget_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_widget_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Widget);
    _widget_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "id", id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "group", group, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "class", class, EET_T_UINT);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "xpl_device", xpl_device, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "xpl_type", xpl_type, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "xpl_current", xpl_current, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "xpl_data1", xpl_data1, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "xpl_highest", xpl_highest, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "xpl_lowest", xpl_lowest, EET_T_STRING);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "cosm", cosm, EET_T_UINT);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_widget_descriptor, Widget, "gnuplot", gnuplot, EET_T_UINT);

    EET_DATA_DESCRIPTOR_ADD_LIST(_widget_descriptor, Widget, "actions", actions, _action_descriptor);
}/*_widget_init*/

/*
 *
 */
static inline void
_widget_shutdown(void)
{
    if (!_widget_descriptor) return;
    eet_data_descriptor_free(_widget_descriptor);
    _widget_descriptor = NULL;
}/*_widget_shutdown*/


/*
 *
 */
Widget *
widget_new(const char * name, Widget_Class class)
{
    Widget *widget = calloc(1, sizeof(Widget));

    if (!widget)
	{
		debug(stderr, _("Can't calloc Widget struct"));
    	return NULL;
	}

    /*Initialize and set widget generic's fields*/
    widget->name = eina_stringshare_add(name ? name : _("undefined"));
    widget->class = class;

    /*Set default group, according to class*/
	if(class == WIDGET_CLASS_XPL_CONTROL_BASIC)
		widget->group = eina_stringshare_add("widget/control.basic/switch");
	else if(class ==  WIDGET_CLASS_VIRTUAL)
   	 	widget->group = eina_stringshare_add("widget/virtual/clock/main");
	else if(class == WIDGET_CLASS_XPL_SENSOR_BASIC)
   	 	widget->group = eina_stringshare_add("widget/sensor.basic/counter");
    else
        debug(stderr, _("You shouldn't create a new widget with unknown class"));

    /*Initialize xPL specifics field*/
    widget->xpl_device = NULL;
    widget->xpl_type = XPL_TYPE_UNKNOWN;
    widget->xpl_current = NULL;
    widget->xpl_data1 = NULL;
    widget->xpl_highest = NULL;
    widget->xpl_lowest = NULL;

    /*Initialize actions Eina_List*/
    widget->actions = NULL;

    return widget;
}/*widget_new*/


/*
 *
 */
void
widget_free(Widget *widget)
{
    eina_stringshare_del(widget->name);
    eina_stringshare_del(widget->group);

    /*If class is set on xPL specific, so free xPL specifics field*/
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC ))
    {
        eina_stringshare_del(widget->xpl_device);
        eina_stringshare_del(widget->xpl_current);
        eina_stringshare_del(widget->xpl_data1);
        eina_stringshare_del(widget->xpl_highest);
        eina_stringshare_del(widget->xpl_lowest);
    }

    /*Free actions Eina_List*/
    if (widget->actions)
    {
        Action *action_elem;
        EINA_LIST_FREE(widget->actions, action_elem)
            action_free(action_elem);
    }

    FREE(widget);
}/*widget_free*/


Widget *
widget_clone(Widget *src)
{
    Widget *ret = NULL;

    ret->id = src->id;
    ret->name = eina_stringshare_add(src->name);
    ret->group =  eina_stringshare_add(src->group);
    ret->class =  src->class;

    if((src->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (src->class == WIDGET_CLASS_XPL_SENSOR_BASIC ))
    {
        ret->xpl_device = eina_stringshare_add(src->xpl_device);
        ret->xpl_type = src->xpl_type;
        ret->xpl_current = eina_stringshare_add(src->xpl_current);
        ret->xpl_data1 = eina_stringshare_add(src->xpl_data1);
        ret->xpl_highest = eina_stringshare_add(src->xpl_highest);
        ret->xpl_lowest = eina_stringshare_add(src->xpl_lowest);
    }

    ret->cosm = src->cosm;

    /*Free actions Eina_List*/
    if (src->actions)
    {
        Action *action_elem;
        Eina_List *l;
     	EINA_LIST_FOREACH(src->actions, l, action_elem)
   	    {
            widget_action_add(ret, action_clone(action_elem));
        }
    }

    return ret;
}



/*
 *
 */
inline unsigned int
widget_id_get(const Widget *widget)
{
    return widget->id;
}/*widget_id_get*/


/*
 *
 */
inline void
widget_id_set(Widget *widget, unsigned int id)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    widget->id = id;
}/*widget_id_set*/


/*
 *
 */
inline const char *
widget_name_get(const Widget *widget)
{
    return widget->name;
}/*widget_name_get*/


/*
 *
 */
inline void
widget_name_set(Widget *widget, const char *name)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    eina_stringshare_replace(&(widget->name), name);
}/*widget_name_set*/

/*
 *
 */
inline const char *
widget_group_get(const Widget *widget)
{
    return widget->group;
}/*widget_group_get*/


/*
 *
 */
inline void
widget_group_set(Widget *widget, const char *group)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    eina_stringshare_replace(&(widget->group), group);
}/*widget_group_set*/


/*
 *
 */
inline void
widget_class_set(Widget *widget, const Widget_Class class)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    widget->class = class;
}/*widget_class_set*/


/*
 *
 */
inline Widget_Class
widget_class_get(const Widget *widget)
{
    return widget->class;
}/*widget_class_get*/

/*
 *
 */
inline void
widget_xpl_device_set(Widget *widget, const char *xpl_device)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
    	eina_stringshare_replace(&(widget->xpl_device), xpl_device);
    }
    else
    {
        debug(stderr, ("Can't set xpl name to a not xpl widget"));
        return;
    }
}/*widget_xpl_device_set*/


/*
 *
 */
inline const char *
widget_xpl_device_get(const Widget *widget)
{
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
        return elm_entry_markup_to_utf8(widget->xpl_device);
    }
    else
    {
        debug(stderr, ("Can't get xpl name from a not xpl widget"));
        return NULL;
    }
}/*widget_xpl_device_get*/



/*
 *
 */
inline void
widget_xpl_type_set(Widget *widget, const Xpl_Type xpl_type)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);

    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
        widget->xpl_type = xpl_type;
    }
    else
    {
        debug(stderr, ("Can't set xpl type to a not xpl widget class"));
        widget->xpl_type = XPL_TYPE_UNKNOWN;
    }
}/*widget_xpl_type_set*/

/*
 *
 */
inline Xpl_Type
widget_xpl_type_get(const Widget *widget)
{
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
        return widget->xpl_type;
    }
    else
    {
        debug(stderr, ("Can't get xpl type from a not xpl widget"));
        return XPL_TYPE_UNKNOWN;
    }
}/*widget_xpl_type_get*/



/*
 *
 */
inline void
widget_xpl_current_set(Widget *widget, const char *xpl_current)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
    	eina_stringshare_replace(&(widget->xpl_current), xpl_current);
    }
    else
    {
        debug(stderr, ("Can't set xpl current to a not xpl widget"));
        return;
    }
}/*widget_xpl_current_set*/


/*
 *
 */
inline const char *
widget_xpl_current_get(const Widget *widget)
{
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
        return widget->xpl_current;
    }
    else
    {
        debug(stderr, ("Can't get xpl current from a not xpl widget"));
        return NULL;
    }
}/*widget_xpl_current_get*/


/*
 *
 */
inline void
widget_xpl_data1_set(Widget *widget, const char *xpl_data1)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);

    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
    	eina_stringshare_replace(&(widget->xpl_data1), xpl_data1);
    }
    else
    {
        debug(stderr, ("Can't set xpl data1 to a not xpl widget"));
        return;
    }
}/*widget_xpl_current_set*/


/*
 *
 */
inline const char *
widget_xpl_data1_get(const Widget *widget)
{
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
        return widget->xpl_data1;
    }
    else
    {
        debug(stderr, ("Can't get xpl data1 from a not xpl widget"));
        return NULL;
    }
}/*widget_xpl_current_get*/


/*
 *
 */
inline void
widget_xpl_highest_set(Widget *widget, const char *xpl_highest)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
    	eina_stringshare_replace(&(widget->xpl_highest), xpl_highest);
    }
    else
    {
        debug(stderr, ("Can't set xpl highest to a not xpl widget"));
        return;
    }
}/*widget_xpl_highest_set*/


/*
 *
 */
inline const char *
widget_xpl_highest_get(const Widget *widget)
{
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
        return widget->xpl_highest;
    }
    else
    {
        debug(stderr, ("Can't get xpl highest from a not xpl widget"));
        return NULL;
    }
}/*widget_xpl_highest_get*/


/*
 *
 */
inline void
widget_xpl_lowest_set(Widget *widget, const char *xpl_lowest)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
    	eina_stringshare_replace(&(widget->xpl_lowest), xpl_lowest);
    }
    else
    {
        debug(stderr, ("Can't set xpl lowest to a not xpl widget"));
        return;
    }
}/*widget_xpl_lowest_set*/


/*
 *
 */
inline const char *
widget_xpl_lowest_get(const Widget *widget)
{
    if((widget->class == WIDGET_CLASS_XPL_CONTROL_BASIC) ||
       (widget->class == WIDGET_CLASS_XPL_SENSOR_BASIC))
    {
        return widget->xpl_lowest;
    }
    else
    {
        debug(stderr, ("Can't get xpl lowest from a not xpl widget"));
        return NULL;
    }
}/*widget_xpl_lowest_get*/


/*
 *
 */
inline void
widget_cosm_set(Widget *widget, Eina_Bool cosm)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    widget->cosm = cosm;
}/*widget_cosm_set*/

/*
 *
 */
inline Eina_Bool
widget_cosm_get(const Widget *widget)
{
    return widget->cosm;
}/*widget_cosm_get*/


/*
 *
 */
inline void
widget_gnuplot_set(Widget *widget, Eina_Bool gnuplot)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    widget->gnuplot = gnuplot;
}/*widget_cosm_set*/

/*
 *
 */
inline Eina_Bool
widget_gnuplot_get(const Widget *widget)
{
    return widget->gnuplot;
}/*widget_cosm_get*/



/*
 *
 */
inline void
widget_action_add(Widget *widget, Action *action)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    widget->actions = eina_list_append(widget->actions, action);
}/*widget_action_add*/

/*
 *
 */
inline void
widget_action_del(Widget *widget, Action *action)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    widget->actions = eina_list_remove(widget->actions, action);
}/*widget_action_del*/

/*
 *
 */
inline Eina_List *
widget_actions_list_get(const Widget *widget)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(widget, NULL);
    return widget->actions;
}/*widget_actions_list_get*/


/*
 *
 */
inline void
widget_actions_list_set(Widget *widget, Eina_List *list)
{
    EINA_SAFETY_ON_NULL_RETURN(widget);
    widget->actions = list;
}/*widget_actions_list_set*/




/*
 *
 */
 /*
Eina_List *
widget_class_list_get(Widget_Class class)
{
    Eina_List *ret = NULL, *locations = NULL, *l;
    Location *location_elem;

    locations = locations_list_get();
 	EINA_LIST_FOREACH(locations, l, location_elem)
   	{
   	    Eina_List *l2;
   	    Widget *widget_elem;

     	EINA_LIST_FOREACH(location_elem->widgets, l2, widget_elem)
        {
            if(widget_class_get(widget_elem) == class)
                ret = eina_list_append(ret, widget_clone(widget_elem));
        }
   	}
    locations = locations_list_free(locations);

    return ret;
}*/
/*widget_class_list_get*/


/*******************************************************************************
 *
 *                                  LOCATION FUNCS
 *
 ******************************************************************************/
/*
 *
 */
static inline void
_location_init(void)
{
    Eet_Data_Descriptor_Class eddc;

    if (_location_descriptor) return;

    EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Location);
    _location_descriptor = eet_data_descriptor_stream_new(&eddc);

    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "id", id, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "exposure", exposure, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "latitude", latitude, EET_T_DOUBLE);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "longitude", longitude, EET_T_DOUBLE);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "creation", creation, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "revision", revision, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "version", version, EET_T_UINT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(_location_descriptor, Location, "cosm_feedid", cosm_feedid, EET_T_UINT);

    EET_DATA_DESCRIPTOR_ADD_LIST(_location_descriptor, Location, "widgets", widgets, _widget_descriptor);
}/*_location_init*/


/*
 *
 */
static inline void
_location_shutdown(void)
{
    if (!_location_descriptor) return;
    eet_data_descriptor_free(_location_descriptor);
    _location_descriptor = NULL;
}/*_location_shutdown*/


/*
 *Alloc and initialize new location struct
 */
Location *
location_new(unsigned int id, const char * name)
{
	char s[PATH_MAX];
    const char *f;

    Location *location = calloc(1, sizeof(Location));

    if (!location)
	{
		debug(stderr, _("Can't calloc Location struct"));
		return NULL;
	}
    location->name = eina_stringshare_add(name ? name : _("undefined"));

    /*Create an unique filename to avoid conflicts*/
	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%s.eet", edams_locations_data_path_get(), location->name);
    f = filename_create(s);
	location->__eet_filename = eina_stringshare_add(f);
    FREE(f);
    location->id = id;

    /*Initialize widgets Eina_List*/
    location->widgets = NULL;

    /*Add creation date informations*/
	time_t timestamp = time(NULL);
	struct tm *t = localtime(&timestamp);
	snprintf(s, sizeof(s), "%02d-%02d-%d",
				(int)t->tm_mday,
  				(int)t->tm_mon,
  				1900+(int)t->tm_year);
    location->creation = eina_stringshare_add(s);
	location->revision = NULL;
	location->version = 0x0002;
	location->latitude = -1;
	location->longitude = -1;
    location->cosm_feedid = 0;

    return location;
}/*location_new*/


/*
 *Return Eet filename of location struct
 */
const char *
location_filename_get(Location *location)
{
     return location->__eet_filename;
}/*location_filename_get*/

/*
 *
 */
void
location_free(Location *location)
{
	if(location)
	{
	    eina_stringshare_del(location->__eet_filename);
	    eina_stringshare_del(location->name);
    	if (location->widgets)
       	{
          Widget *widgets_elem;
          EINA_LIST_FREE(location->widgets, widgets_elem)
             widget_free(widgets_elem);
       	}
	    eina_stringshare_del(location->creation);
    	eina_stringshare_del(location->revision);
	    FREE(location);
	}
}/*location_free*/

/*
 *
 */
inline unsigned int
location_id_get(const Location *location)
{
    return location->id;
}

inline void
location_id_set(Location *location, unsigned int id)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->id = id;
}/*location_id_set*/


/*
 *
 */
inline const char *
location_name_get(const Location *location)
{
    return  elm_entry_markup_to_utf8(location->name);
}/*location_name_get*/

/*
 *
 */
inline void
location_name_set(Location *location, const char *name)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    eina_stringshare_replace(&(location->name), name);
}/*location_name_set*/


/*
 *
 */
inline Exposure
location_exposure_get(const Location *location)
{
    return location->exposure;
}/*location_exposure_get*/

/*
 *
 */
inline void
location_exposure_set(Location *location, Exposure exposure)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->exposure = exposure;
}/*location_exposure_set*/

/*
 *
 */
inline double
location_latitude_get(const Location *location)
{
    return location->latitude;
}/*location_latitude_get*/

/*
 *
 */
inline void
location_latitude_set(Location *location, double latitude)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->latitude = latitude;
}/*location_latitude_set*/


inline double
location_longitude_get(const Location *location)
{
    return location->longitude;
}

inline void
location_longitude_set(Location *location, double longitude)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->longitude = longitude;
}


inline unsigned int
location_cosm_feedid_get(const Location *location)
{
    return location->cosm_feedid;
}

inline void
location_cosm_feedid_set(Location *location, unsigned cosm_feedid)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
	location->cosm_feedid =  cosm_feedid;
}



void
location_image_set(Location *location, Evas_Object *image)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    EINA_SAFETY_ON_NULL_RETURN(image);

    Eet_File *ef;
    Eina_Bool ret;

    ef = eet_open(location->__eet_filename, EET_FILE_MODE_WRITE);
    if (!ef)
      {
        debug(stderr, _("Can't open Eet file '%s' in write mode"), location->__eet_filename);
        return;
      }

	int image_w, image_h;
	int image_alpha;
	void *image_data;
	evas_object_image_size_get(image, &image_w, &image_h);
	image_alpha = evas_object_image_alpha_get(image);
	image_data = evas_object_image_data_get(image, EINA_FALSE);

	ret = eet_data_image_write(ef, "/image/0", image_data, image_w, image_h, image_alpha, 1, 95, 0);
    eet_close(ef);

	if (!ret)
	{
		debug(stderr, _("Can't write any data to Eet file '%s'"), location->__eet_filename);
	}
}


inline const char *
location_creation_get(Location *location)
{
    return location->creation;
}

void
location_creation_set(Location *location, const char *creation)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    eina_stringshare_replace(&(location->creation), creation);
}

inline const char *
location_profil_revision_get(const Location *location)
{
    return location->revision;
}

void
location_revision_set(Location *location, const char *revision)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    eina_stringshare_replace(&(location->revision), revision);
}


inline void
location_widgets_add(Location *location, Widget *widget)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->widgets = eina_list_append(location->widgets, widget);

	Eina_List *l;
	Widget *data;
	unsigned int id = 0;

    /*Generate an unique ID to avoid widgets ID conflicts*/
REDO:
 	EINA_LIST_FOREACH(location->widgets, l, data)
   	{
		if(data->id == id)
		{
			id++;
			goto REDO;
		}
	}

	widget_id_set(widget, id);
}/*location_widgets_add*/

inline void
location_widgets_del(Location *location, Widget *widget)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->widgets = eina_list_remove(location->widgets, widget);
}/*location_widgets_del*/


inline Widget *
location_widgets_get(const Location *location, unsigned int nth)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(location, NULL);
    return eina_list_nth(location->widgets, nth);
}/*location_widgets_get*/

inline unsigned int
location_widgets_count(const Location *location)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(location, 0);
    return eina_list_count(location->widgets);
}/*location_widgets_count*/

void
location_widgets_list_clear(Location *location)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    Widget *data;
    EINA_LIST_FREE(location->widgets, data) widget_free(data);
}/*location_widgets_list_clear*/

inline Eina_List *
location_widgets_list_get(const Location *location)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(location, NULL);
    return location->widgets;
}/*location_widgets_list_get*/


inline void
location_widgets_list_set(Location *location, Eina_List *list)
{
    EINA_SAFETY_ON_NULL_RETURN(location);
    location->widgets = list;
}/*location_widgets_list_set*/


/*
 *Return location struct from Eet file 'filename'
 */
Location *
location_load(const char *filename)
{
    Location *location = NULL;

    Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
    if (!ef)
      {
        debug(stderr, _("Can't open Eet file '%s' in read mode"), filename);
        return NULL;
      }

    location = eet_data_read(ef, _location_descriptor, LOCATION_ENTRY);
    if (!location) goto end;
    location->__eet_filename = eina_stringshare_add(filename);

	/*FIXME:In future release handle this code's parts by adding newly field(extensions)*/
   	if (location->version < LOCATION_FILE_VERSION)
     	{
        	debug(stderr, _("Eet file '%s' %#x was too old, upgrading it to %#x"),
        			location->__eet_filename,
                	location->version,
                	LOCATION_FILE_VERSION);

        	location->version = LOCATION_FILE_VERSION;
     	}

end:
    eet_close(ef);
    return location;
}/*location_load*/


/*
 *Write location struct to Eet file
 */
Eina_Bool
location_save(Location *location)
{
    Eet_File *ef;
    Eina_Bool ret;

    /*Try to open location Eet filename in writing mode*/
    ef = eet_open(location->__eet_filename, EET_FILE_MODE_WRITE);
    if (!ef)
	{
        debug(stderr, _("Can't open Eet file '%s' in write mode"), location->__eet_filename);
		return EINA_FALSE;
	}

    /*Try to write location data struct to Eet file opened*/
    ret = !!eet_data_write(ef, _location_descriptor, LOCATION_ENTRY, location, EINA_TRUE);
    eet_close(ef);
	if (!ret)
	{
		debug(stderr, _("Can't write any data to Eet file '%s'"), location->__eet_filename);
		return EINA_FALSE;
	}

    return EINA_TRUE;
}/*location_save*/


/*
 *
 */
void
locations_init(void)
{
    _action_init();
    _widget_init();
    _location_init();
}/*locations_init*/

/*
 *
 */
void
locations_shutdown(void)
{
    _action_shutdown();
    _widget_shutdown();
    _location_shutdown();
}/*locations_shutdown*/

/*
 *Remove location Eet file.
*/
Eina_Bool
location_remove(Location *location)
{
	if(!location) return EINA_FALSE;

	if(!ecore_file_remove(location->__eet_filename))
	{
	    debug(stderr, _("Can't remove Eet location file '%s'"), location->__eet_filename);
	    return EINA_FALSE;
	}

	debug(stdout, _("Eet Location file '%s' has been removed"), location->__eet_filename);

	return EINA_TRUE;
}/*location_remove*/



/*
 *Free locations Eina_List
 */
Eina_List *
locations_list_free(Eina_List *locations)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(locations, NULL);

	unsigned int n = 0;
	Location *data;

    /*Free all location node of Eina_List*/
	EINA_LIST_FREE(locations, data)
	{
		n++;
		location_free(data);
	}
	eina_list_free(locations);

	debug(stdout, _("%d Location struct of Eina_list freed"), n);

    return NULL;
}/*locations_list_free*/


/*
 *Remove 'location' node from 'locations' Eina_List
 */
Eina_List *
locations_list_location_remove(Eina_List *locations, Location *location)
{
	Eina_List *l;
	Location *data;

 	EINA_LIST_FOREACH(locations, l, data)
   {
		if(data == location)
		{
         	locations = eina_list_remove_list(locations, l);
			break;
		}
   }
   return locations;
}/*locations_list_location_remove*/




/*
 *Return Eina_List of all locations Eet files read
 */
Eina_List *
locations_list_get()
{
	int id = 0;
	const Eina_File_Direct_Info *f_info;
	Eina_Iterator *it;
	Eina_List *locations = NULL;
	Location *location = NULL;
	char s[PATH_MAX];

	snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S, edams_locations_data_path_get());
	it = eina_file_stat_ls(s);

   	if(it)
   	{
	   EINA_ITERATOR_FOREACH(it, f_info)
	   {
            //INF("Found %s new file.", ecore_file_file_get(f_info->path));
			if(eina_str_has_extension(f_info->path, ".eet") == EINA_TRUE)
			{
				location = location_load(f_info->path);

				if(location)
				{
					location->id = id++;
					locations = eina_list_append(locations, location);
					if (eina_error_get())
					{
						debug(stderr, _("Can't alloc Eina_List node"));
						exit(-1);
					}
				}
			}
		}

	eina_iterator_free(it);
	}

	debug(stdout, _("%d locations registered"), eina_list_count(locations));

	return locations;
}/*locations_list_get*/
