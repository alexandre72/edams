/*
 * locations.h
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


#ifndef __LOCATION_H__
#define __LOCATION_H__

#include <Eina.h>
#include <Eet.h>
#include <Evas.h>

#include "xpl.h"
#include "action.h"
#include "widget.h"


typedef struct _Location Location;


typedef enum _Exposure_
{
	EXPOSURE_INDOOR = 1,
	EXPOSURE_OUTDOOR = 2,
	EXPOSURE_LAST
}Exposure;

/* Actions */
Action *action_new(Condition condition, const char *value, Action_Type type, const char *data);
void action_free(Action *action);

void action_ifcondition_set(Action *action, Condition ifcondition);
Condition action_ifcondition_get(const Action *action);
const char * action_ifvalue_get(const Action *action);
void action_ifvalue_set(const Action *action, const char *ifvalue);
void action_type_set(Action *action, Action_Type type);
Action_Type  action_type_get(const Action *action);
void action_data_set(const Action *action, const char *data);
const char * action_data_get(const Action *action);

void widget_action_add(Widget *widget, Action *action);
void widget_action_del(Widget *widget,Action *action);
unsigned int widget_actions_count(const Widget *widget);
Eina_List *widget_actions_list_get(const Widget *widget);
void widget_actions_list_clear(Widget *widget);
void widget_actions_list_set(Widget *widget, Eina_List *list);

/* Widget */
Widget *widget_new(const char * name, Widget_Class class);
void widget_free(Widget *widget);

void widget_id_set(Widget *widget, unsigned int position);
unsigned int widget_id_get(const Widget *widget);
void widget_name_set(Widget *widget, const char * name);
const char * widget_name_get(const Widget *widget);
void widget_group_set(Widget *widget, const char * group);
const char * widget_group_get(const Widget *widget);
Widget_Class widget_class_get(const Widget *widget);
void widget_class_set(Widget *widget, const Widget_Class class);

const char *widget_xpl_device_get(const Widget *widget);
void widget_xpl_device_set(Widget *widget, const char *xpl_device);
Xpl_Type widget_xpl_type_get(const Widget *widget);
void widget_xpl_type_set(Widget *widget, const Xpl_Type xpl_type);
const char *widget_xpl_current_get(const Widget *widget);
void widget_xpl_current_set(Widget *widget, const char *xpl_current);
const char *widget_xpl_data1_get(const Widget *widget);
void widget_xpl_data1_set(Widget *widget, const char *xpl_data1);


void widget_cosm_set(Widget *widget, Eina_Bool cosm);
Eina_Bool widget_cosm_get(const Widget *widget);

void widget_gnuplot_set(Widget *widget, Eina_Bool gnuplot);
Eina_Bool widget_gnuplot_get(const Widget *widget);

void widget_action_add(Widget *widget, Action *action);
void widget_action_del(Widget *widget,Action *action);
Action *widget_action_get(const Widget *widget, unsigned int nth);
unsigned int widget_actions_count(const Widget *widget);
Eina_List *widget_actions_list_get(const Widget *widget);
void widget_actions_list_clear(Widget *widget);
void widget_actions_list_set(Widget *widget, Eina_List *list);

/* Location */
Location *location_new(unsigned int id, const char * name);
void location_free(Location *location);
Location *location_load(const char *filename);
Eina_Bool location_save(Location *location);
Eina_Bool location_remove(Location *location);

void location_id_set(Location *location, unsigned int id);
void location_name_set(Location *location, const char * name);
void location_image_set(Location *location, Evas_Object *image);
void location_exposure_set(Location *location, Exposure exposure);
void location_latitude_set(Location *location, double latitude);
void location_longitude_set(Location *location, double longitude);
void location_cosm_feedid_set(Location *location, unsigned int cosm_feedid);

const char *location_filename_get(Location *location);
unsigned int location_id_get(const Location *location);
const char * location_name_get(const Location *location);
Exposure location_exposure_get(const Location *location);
double location_latitude_get(const Location *location);
double location_longitude_get(const Location *location);
unsigned int location_cosm_feedid_get(const Location *location);

void location_widgets_add(Location *location, Widget *widget);
void location_widgets_del(Location *location, Widget *widget);
Widget *location_widgets_get(const Location *location, unsigned int nth);
unsigned int location_widgets_count(const Location *location);
Eina_List *location_widgets_list_get(const Location *location);
void location_widgets_list_clear(Location *location);
void location_widgets_list_set(Location *location, Eina_List *list);

Eina_List *locations_list_get();
Eina_List * locations_list_free(Eina_List *locations);
Eina_List *locations_list_location_remove(Eina_List *locations, Location *location);

/* Global initializer / shutdown functions */
void locations_init(void);
void locations_shutdown(void);

#endif /* __LOCATION_H__ */
