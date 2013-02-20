/*
 * cmnd_editor.c
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

#include <Elementary.h>

#include "cJSON.h"
#include "edams.h"
#include "path.h"

/*Global objects*/
static Evas_Object *win = NULL;

/*
 *
 */
const char*
cmnd_editor_values_get()
{
	cJSON *root;
    const char *s;

	Widget *widget = evas_object_data_get(win, "widget");
    if((!widget) || (!widget_xpl_current_get(widget)))
        return NULL;

	root=cJSON_CreateObject();
	cJSON_AddItemToObject(root, "DEVICE", cJSON_CreateString(widget_xpl_device_get(widget)));
	cJSON_AddItemToObject(root, "CURRENT", cJSON_CreateString(widget_xpl_current_get(widget)));
	//cJSON_AddItemToObject(root, "DATA1", cJSON_CreateString(widget_xpl_data1_get(widget)));

    //s = action_exec_data_format(elm_object_text_get(entry), );
	return s;
}/*cmnd_editor_values_get*/


/*
 *
 */
Evas_Object *
cmnd_editor_hbox_get()
{
	Evas_Object *hbox = elm_object_name_find(win, "hbox", -1);
	return hbox;
}/*cmnd_editor_hbox_get*/


/*
 *
 */
static void
_slider_control_basic_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
    Widget *widget = data;
    char *s;

    double val = elm_slider_value_get(obj);
    asprintf(&s, "%1.0f", val);

    widget_xpl_current_set(widget, s);

    FREE(s);

}/*_slider_control_basic_changed_cb*/

/*
 *
 */
static void


_radio_control_basic_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
    Widget *widget = data;

    widget_xpl_current_set(widget, elm_object_text_get(obj));
}/*_radio_control_basic_changed_cb*/


/*
 *Callback called in 'control.basic' list when clicked signal is emitted.
 */
static void
_list_control_basic_item_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    Evas_Object *bx;
    Evas_Object *group, *radio, *slider;
    Evas_Object *frame;
    Evas_Object *grid = elm_object_name_find(win, "grid", -1);
    Widget *widget = data;
    Xpl_Type type = widget_xpl_type_get(widget);

	frame = elm_frame_add(win);
    elm_object_text_set(frame, _("Value"));
    elm_grid_pack(grid, frame, 1, 42, 99, 40);
    evas_object_show(frame);

    switch(type)
    {
        case XPL_TYPE_INPUT_CONTROL_BASIC:
   		case XPL_TYPE_MUTE_CONTROL_BASIC:
        case XPL_TYPE_MACRO_CONTROL_BASIC:
        case XPL_TYPE_FLAG_CONTROL_BASIC:
        case XPL_TYPE_OUTPUT_CONTROL_BASIC:
		case XPL_TYPE_INFRARED_CONTROL_BASIC:
        case XPL_TYPE_PERIODIC_CONTROL_BASIC:
        case XPL_TYPE_SCHEDULED_CONTROL_BASIC:
        case XPL_TYPE_TIMER_CONTROL_BASIC:

                bx = elm_box_add(win);
                elm_box_horizontal_set(bx, EINA_FALSE);
                evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
                evas_object_show(bx);

                group = radio = elm_radio_add(win);

                    if(type == XPL_TYPE_MUTE_CONTROL_BASIC)
                        elm_object_text_set(radio, "Yes");
                      else if((type == XPL_TYPE_INPUT_CONTROL_BASIC) ||
                            (type == XPL_TYPE_MACRO_CONTROL_BASIC) ||
                            (type == XPL_TYPE_SCHEDULED_CONTROL_BASIC) ||
                            (type == XPL_TYPE_PERIODIC_CONTROL_BASIC))
                        elm_object_text_set(radio, "Enable");
                    else if(type == XPL_TYPE_FLAG_CONTROL_BASIC)
                        elm_object_text_set(radio, "Set");
                    else if(type == XPL_TYPE_INFRARED_CONTROL_BASIC)
                        elm_object_text_set(radio, "Send");
                    else if(type == XPL_TYPE_TIMER_CONTROL_BASIC)
                        elm_object_text_set(radio, "Off");

                elm_radio_state_value_set(radio, 0);
                evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                elm_box_pack_end(bx, radio);
                evas_object_show(radio);
                evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, widget);

                radio = elm_radio_add(win);
                elm_radio_group_add(radio, group);

                    if(type == XPL_TYPE_MUTE_CONTROL_BASIC)
                        elm_object_text_set(radio, "No");
                    else if((type == XPL_TYPE_INPUT_CONTROL_BASIC) ||
                            (type == XPL_TYPE_MACRO_CONTROL_BASIC) ||
                            (type == XPL_TYPE_SCHEDULED_CONTROL_BASIC) ||
                            (type == XPL_TYPE_PERIODIC_CONTROL_BASIC))
                        elm_object_text_set(radio, "Disable");
                    else if(type == XPL_TYPE_FLAG_CONTROL_BASIC)
                        elm_object_text_set(radio, "Clear");
                    else if(type == XPL_TYPE_INFRARED_CONTROL_BASIC)
                        elm_object_text_set(radio, "Enable_rx");
                    else if(type == XPL_TYPE_TIMER_CONTROL_BASIC)
                            elm_object_text_set(radio, "Start");

                elm_radio_state_value_set(radio, 1);
                evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                elm_box_pack_end(bx, radio);
                evas_object_show(radio);
                evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, widget);

                    if((type == XPL_TYPE_MACRO_CONTROL_BASIC) ||
                        (type == XPL_TYPE_FLAG_CONTROL_BASIC) ||
                        (type == XPL_TYPE_PERIODIC_CONTROL_BASIC) ||
                        (type == XPL_TYPE_SCHEDULED_CONTROL_BASIC) ||
                        (type == XPL_TYPE_TIMER_CONTROL_BASIC) ||
                        (type == XPL_TYPE_OUTPUT_CONTROL_BASIC))
                    {
                        radio = elm_radio_add(win);
                        elm_radio_group_add(radio, group);

                            if((type == XPL_TYPE_MACRO_CONTROL_BASIC))
                                elm_object_text_set(radio, "Do");
                            else if(type == XPL_TYPE_FLAG_CONTROL_BASIC)
                                elm_object_text_set(radio, "Neutral");
                            else if(type == XPL_TYPE_OUTPUT_CONTROL_BASIC)
                                elm_object_text_set(radio, "high");
                            else if(type == XPL_TYPE_INFRARED_CONTROL_BASIC)
                                elm_object_text_set(radio, "Disable_rx");
                            else if((type == XPL_TYPE_PERIODIC_CONTROL_BASIC) ||
                                    (type == XPL_TYPE_SCHEDULED_CONTROL_BASIC))
                                elm_object_text_set(radio, "Started");
                            else if(type == XPL_TYPE_TIMER_CONTROL_BASIC)
                                 elm_object_text_set(radio, "Stop");

                        elm_radio_state_value_set(radio, 2);
                        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                        elm_box_pack_end(bx, radio);
                        evas_object_show(radio);
                        evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, widget);
                    }

                    if((type == XPL_TYPE_INFRARED_CONTROL_BASIC) ||
                        (type == XPL_TYPE_TIMER_CONTROL_BASIC) ||
                        (type == XPL_TYPE_OUTPUT_CONTROL_BASIC))
                    {
                        radio = elm_radio_add(win);
                        elm_radio_group_add(radio, group);

                            if(type == XPL_TYPE_OUTPUT_CONTROL_BASIC)
                                elm_object_text_set(radio, "low");
                            else if(type == XPL_TYPE_INFRARED_CONTROL_BASIC)
                                elm_object_text_set(radio, "Enable_tx");
                            else if(type == XPL_TYPE_TIMER_CONTROL_BASIC)
                                 elm_object_text_set(radio, "Halt");

                        elm_radio_state_value_set(radio, 3);
                        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                        elm_box_pack_end(bx, radio);
                        evas_object_show(radio);
                        evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, widget);
                    }

                   if((type == XPL_TYPE_INFRARED_CONTROL_BASIC) ||
                        (type == XPL_TYPE_TIMER_CONTROL_BASIC) ||
                        (type == XPL_TYPE_OUTPUT_CONTROL_BASIC))
                    {
                        radio = elm_radio_add(win);
                        elm_radio_group_add(radio, group);

                            if(type == XPL_TYPE_OUTPUT_CONTROL_BASIC)
                                elm_object_text_set(radio, "pulse");
                          else if(type == XPL_TYPE_INFRARED_CONTROL_BASIC)
                                elm_object_text_set(radio, "Disable_tx");
                            else if(type == XPL_TYPE_TIMER_CONTROL_BASIC)
                                 elm_object_text_set(radio, "Resume");

                        elm_radio_state_value_set(radio, 4);
                        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                        elm_box_pack_end(bx, radio);
                        evas_object_show(radio);
                        evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, widget);
                    }


                    if(type == XPL_TYPE_OUTPUT_CONTROL_BASIC)
                    {
                        radio = elm_radio_add(win);
                        elm_radio_group_add(radio, group);

                            if(type == XPL_TYPE_OUTPUT_CONTROL_BASIC)
                                elm_object_text_set(radio, "toggle");

                        elm_radio_state_value_set(radio, 5);
                        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                        elm_box_pack_end(bx, radio);
                        evas_object_show(radio);
                        evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, widget);
                    }


                elm_object_content_set(frame, bx);
                break;

	            case XPL_TYPE_BALANCE_CONTROL_BASIC:
        		case XPL_TYPE_SLIDER_CONTROL_BASIC:
        		case XPL_TYPE_VARIABLE_CONTROL_BASIC:
                                slider = elm_slider_add(win);
                                elm_slider_unit_format_set(slider, "%1.0f");

                                if(type == XPL_TYPE_BALANCE_CONTROL_BASIC)
                                    elm_slider_min_max_set(slider, -100, 100);
                                else if((type == XPL_TYPE_SLIDER_CONTROL_BASIC) ||
                                        (type == XPL_TYPE_SLIDER_CONTROL_BASIC))
                                    elm_slider_min_max_set(slider, 0, 255);

                                evas_object_show(slider);
                                evas_object_smart_callback_add(slider, "changed", _slider_control_basic_changed_cb, widget);
                                elm_object_content_set(frame, slider);
	                            break;

                case XPL_TYPE_LAST:
                case XPL_TYPE_UNKNOWN:
                case XPL_TYPE_BATTERY_SENSOR_BASIC:
                case XPL_TYPE_COUNT_SENSOR_BASIC:
                case XPL_TYPE_CURRENT_SENSOR_BASIC:
                case XPL_TYPE_DIRECTION_SENSOR_BASIC:
                case XPL_TYPE_DISTANCE_SENSOR_BASIC:
                case XPL_TYPE_ENERGY_SENSOR_BASIC:
                case XPL_TYPE_FAN_SENSOR_BASIC:
                case XPL_TYPE_GENERIC_SENSOR_BASIC:
                case XPL_TYPE_HUMIDITY_SENSOR_BASIC:
                case XPL_TYPE_INPUT_SENSOR_BASIC:
                case XPL_TYPE_OUTPUT_SENSOR_BASIC:
                case XPL_TYPE_POWER_SENSOR_BASIC:
                case XPL_TYPE_PRESSURE_SENSOR_BASIC:
                case XPL_TYPE_SETPOINT_SENSOR_BASIC:
                case XPL_TYPE_SPEED_SENSOR_BASIC:
                case XPL_TYPE_TEMP_SENSOR_BASIC:
                case XPL_TYPE_UV_SENSOR_BASIC:
                case XPL_TYPE_VOLTAGE_SENSOR_BASIC:
                case XPL_TYPE_VOLUME_SENSOR_BASIC:
                case XPL_TYPE_WEIGHT_SENSOR_BASIC:
                break;
    }

    evas_object_data_set(win, "widget", widget);
}/*_list_control_basic_item_selected_cb*/


/*
 *
 */
static void
_list_control_basic_item_add(Evas_Object *list, Widget *widget)
{
	char *s;

	Evas_Object *icon;
	icon = elm_icon_add(win);
	asprintf(&s, "%s/icon", widget_group_get(widget));
   	elm_image_file_set(icon, edams_edje_theme_file_get(), s);
   	FREE(s);
	elm_image_aspect_fixed_set(icon, EINA_TRUE);
	elm_image_resizable_set(icon, 1, 0);

	asprintf(&s, "%s %s", widget_xpl_device_get(widget), xpl_type_to_str(widget_xpl_type_get(widget)));

	elm_list_item_append(list, s, icon, NULL, _list_control_basic_item_selected_cb, widget);
	elm_list_go(list);
	FREE(s);
}/*_list_control_basic_item_selected_cb*/


/*
 *
 */
Evas_Object *
cmnd_editor_add(Widget *widget __UNUSED__)
{
	Evas_Object *grid, *hbox, *frame;
	Evas_Object *list;

	win = elm_win_util_standard_add("cmnd_editor", NULL);
	elm_win_title_set(win, _("Edit cmnd control.basic"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	evas_object_name_set(grid, "grid");
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Select control.basic"));
	elm_grid_pack(grid, frame, 1, 1, 99, 40);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_scroller_policy_set(list, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_ON);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_name_set(list, "control.basic list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

    Eina_List *l = NULL;
    Location *location_elem;
	EINA_LIST_FOREACH(edams_locations_list_get(), l, location_elem)
	{
	    Eina_List *l2, *widgets = NULL;
	    Widget *widget_elem;
	    widgets = location_widgets_list_get(location_elem);
        EINA_LIST_FOREACH(widgets, l2, widget_elem)
        {
            if(widget_class_get(widget_elem) == WIDGET_CLASS_XPL_CONTROL_BASIC)
                _list_control_basic_item_add(list, widget_elem);
        }
    }

	hbox = elm_box_add(win);
	evas_object_name_set(hbox, "hbox");
	elm_box_horizontal_set(hbox, EINA_TRUE);
	elm_box_homogeneous_set(hbox, EINA_TRUE);
	elm_grid_pack(grid, hbox, 1, 89, 99, 10);
	evas_object_show(hbox);

	evas_object_resize(win, 400, 400);
	evas_object_show(win);

	return win;
}/*cmnd_editor_add*/
