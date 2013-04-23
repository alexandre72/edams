/*
 * widget_editor.c
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

#include <stdio.h>
#include <Elementary.h>

#include "cJSON.h"
#include "edams.h"
#include "location.h"
#include "path.h"
#include "utils.h"
#include "widget_editor.h"


/*Global objects*/
static Evas_Object *win = NULL;
static Widget *widget = NULL;
static App_Info *app = NULL;

/*Evas_Object Callbacks*/
static void _button_close_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _layout_signals_cb(void *data, Evas_Object *obj, const char  *emission, const char  *source);
static void _list_item_group_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _list_item_class_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _entry_widget_name_changed_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _entry_device_id_changed_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _check_cosm_changed_cb(void *data, Evas_Object *obj, void *event_infxo);

/*Others funcs*/
static void _layout_samples_test(Evas_Object *layout);
static void _list_widgets_groups_fill();
static void _hoversel_device_type_fill(Widget_Class class);
static void _entry_device_preview_update();

/*
 *
 */
static void
_button_close_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    widget_free(widget);

    evas_object_del(win);
}/*_button_close_clicked_cb*/


/*
 *
 */
static void
_button_apply_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    char *s;

    if(!app->widget)
    {
        asprintf(&s, _("Widget '%s' has been added to '%s'"), widget_name_get(widget), location_name_get(app->location));
        location_widgets_add(app->location, widget);
    }
    else
    {
        widget_name_set(app->widget, widget_name_get(widget));
        widget_class_set(app->widget, widget_class_get(widget));
        widget_id_set(app->widget, widget_id_get(widget));
        widget_group_set(app->widget, widget_group_get(widget));
        widget_cosm_set(app->widget, widget_cosm_get(widget));
        widget_device_id_set(app->widget, widget_device_id_get(widget));
        widget_device_type_set(app->widget, widget_device_type_get(widget));

        asprintf(&s, _("Widget '%s' from '%s' has been updated"), widget_name_get(app->widget), location_name_get(app->location));
    }

    location_save(app->location);
    update_naviframe_content(app->location);
    console_text_add(MSG_INFO, s);
    FREE(s);

    evas_object_del(win);
}/*_button_apply_clicked_cb*/





/*
 * Send random value to current selected widget to let user see effects on it.
 */
static void
_layout_samples_test(Evas_Object *layout)
{
   	Evas_Object *edje;
	Evas_Coord w, h;
    Widget *sample;
	char *str;
    const char *type = widget_device_type_get(widget);

    if (!type) return;

    sample = widget_new("sample", WIDGET_CLASS_SENSOR);
	RANDOMIZE();
	asprintf(&str, "%d", RANDOM(device_type_current_max_get(type)));
	widget_device_current_set(sample, str);
    FREE(str);


   	edje = elm_layout_edje_get(layout);
	edje_object_size_min_get(edje, &w, &h);
	evas_object_resize(edje, w, h);

	Evas_Object *entry = elm_object_name_find(win, "preview entry", -1);
	elm_object_text_set(entry, device_control_basic_cmnd_to_elm_str(widget));

    const char *t;
    /*Special widget with drag part, so need to convert device current value to float.*/
    if ((t = elm_layout_data_get(layout, "drag")))
    {
    	double level;
    	Edje_Message_Float msg;
           level =	(double)atoi(widget_device_current_get(sample)) / 100;
    	msg.val = level;
    	edje_object_message_send(edje, EDJE_MESSAGE_FLOAT, 1, &msg);
    }
    edje_object_signal_emit(edje, widget_device_current_get(widget), "whole");

    elm_object_part_text_set(layout, "title.text", widget_name_get(sample));

	asprintf(&str, "%s%s", widget_device_current_get(sample), device_type_to_unit_symbol(type) ? device_type_to_unit_symbol(type) : "");
    elm_object_part_text_set(layout, "value.text", str);
    FREE(str);

    elm_object_signal_emit(layout, "updated", "whole");

    widget_free(sample);
}/*_layout_samples_test*/



/*
 *
 */
static void
_layout_signals_cb(void *data __UNUSED__, Evas_Object *obj, const char  *emission, const char  *source)
{
    double val = 0;
    const char *type = widget_device_type_get(widget);
    char *s = NULL;

    if (!type) return;

    /*Skip basic's edje signal emission*/
	if(strstr(source, "edje")) return;
	if(strstr(emission, "mouse")) return;
	if(strstr(emission, "show")) return;
	if(strstr(emission, "hide")) return;
	if(strstr(emission, "resize")) return;
	if(strstr(emission, "load")) return;
	if(strstr(emission, "updated")) return;
	if(strstr(emission, "drag,stop")) return;

    if(strstr(emission, "drag"))
    {
        Evas_Object *edje = elm_layout_edje_get(obj);
        Edje_Drag_Dir dir = edje_object_part_drag_dir_get(edje, source);


            if(dir == EDJE_DRAG_DIR_X)
                edje_object_part_drag_value_get(edje, source, &val, NULL);
            else if(dir == EDJE_DRAG_DIR_Y)
                edje_object_part_drag_value_get(edje, source, NULL, &val);
            //else if(dir == EDJE_DRAG_DIR_XY)
            // edje_object_part_drag_value_get(o, source, &hval, &vval);
    }
    else
    {
        val = atoi(emission);
    }

    /*Scale to device type format*/
    if(strcmp(type, DEVICE_TYPE_SLIDER_CONTROL) == 0)
    {
        val = (val * 100);
        asprintf(&s, "%d%%", (int)val);
    }
    else
    {
        asprintf(&s, "%d", (int)val);
    }

    widget_device_current_set(widget, s);
	elm_object_part_text_set(obj, "value.text", s);
	FREE(s);

    _entry_device_preview_update();
}/*_layout_signals_cb*/



/*
 *Callback called in list "groups" object when an item is selected
 */
static void
_list_item_group_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    Widget_Class class = widget_class_get(widget);
	const char *group = data;

	Evas_Object *layout = elm_object_name_find(win, "widget layout", -1);
	if(group)
	{
    	elm_layout_file_set(layout, edams_edje_theme_file_get(), group);
    	widget_group_set(widget, group);
    }

    /*If widget device is set to sensor then load random sample data*/
    if(class == WIDGET_CLASS_SENSOR)
    {
	    _layout_samples_test(layout);
    }
	else if(class == WIDGET_CLASS_CONTROL)
	{
		elm_layout_signal_callback_add(layout, "*", "*", _layout_signals_cb, NULL);
	}
}/*_list_item_group_selected_cb*/


/*
 *
 */
static void
_entry_widget_name_changed_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    widget_name_set(widget, elm_object_text_get(obj));
}/*_entry_widget_name_changed_cb*/


/*
 *
 */
static void
_entry_device_id_changed_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    widget_device_id_set(widget, elm_object_text_get(obj));
    _entry_device_preview_update();
}/*_entry_device_id_changed_cb*/


/*
 *
 */
static void
_list_item_class_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info)
{
    Widget_Class class = (Widget_Class)data;
    Evas_Object *widget_entry = elm_object_name_find(win, "widget name entry", -1);
    Evas_Object *device_frame = elm_object_name_find(win, "device id frame", -1);
    Evas_Object *device_entry = elm_object_name_find(win, "device id entry", -1);
    Evas_Object *hoversel = elm_object_name_find(win, "device type hoversel", -1);
	Evas_Object *layout = elm_object_name_find(win, "widget layout", -1);
	Evas_Object *cosm_check = elm_object_name_find(win, "cosm check", -1);

    if(app->widget)
     {
        elm_list_item_bring_in(event_info);
        elm_layout_file_set(layout, edams_edje_theme_file_get(), widget_group_get(widget));
    }

    evas_object_hide(cosm_check);
    evas_object_hide(hoversel);
    evas_object_hide(preview_frame);
    evas_object_hide(device_frame);

    widget_class_set(widget, class);
    elm_object_text_set(widget_entry, widget_name_get(widget));

    if((class == WIDGET_CLASS_SENSOR) ||
        (class == WIDGET_CLASS_CONTROL))
    {
        evas_object_show(cosm_check);
        evas_object_show(preview_frame);
        evas_object_show(device_frame);
        evas_object_show(hoversel);

        _hoversel_device_type_fill(class);

        if(app->widget)
        {
            elm_object_text_set(device_entry, widget_device_id_get(widget));
            elm_object_text_set(hoversel, widget_device_type_get(widget));
            elm_check_state_set(cosm_check, widget_cosm_get(widget));
        }
        else
        {
            if(edams_settings_cosm_apikey_get())
            elm_check_state_set(cosm_check, EINA_TRUE);
        }
    }

    _list_widgets_groups_fill();
}/*_list_item_class_selected_cb*/


/*
 *
 */
static void
_list_widgets_groups_fill()
{
    Evas_Object *list = elm_object_name_find(win, "groups list", -1);
	Eina_List *groups = NULL, *l;
    Widget_Class class = widget_class_get(widget);
    char *s;

    elm_object_disabled_set(list, EINA_FALSE);
    elm_list_clear(list);
	groups = edje_file_collection_list(edams_edje_theme_file_get());
	if (groups)
	{
		char *group;
		EINA_LIST_FOREACH(groups, l, group)
		{
			asprintf(&s, "widget/%s", widget_class_to_str(class));
			if((strncmp(group, s, strlen(s)) == 0) && (!strstr(group, "/icon")))
			{
    			FREE(s);
				Evas_Object *icon;
				icon = elm_icon_add(win);
				asprintf(&s, "%s/icon", group);
			   	elm_image_file_set(icon, edams_edje_theme_file_get(), s);
			   	FREE(s);
				elm_image_aspect_fixed_set(icon, EINA_TRUE);
				elm_image_resizable_set(icon, 1, 0);

				Elm_Object_Item *it = elm_list_item_append(list, group, icon, NULL, _list_item_group_selected_cb, group);

				if((strcmp(group, widget_group_get(widget)) == 0))
				{
                    elm_list_item_selected_set(it, EINA_TRUE);
                    elm_list_item_bring_in(it);
                }
			}
			else
			{
                FREE(s);
            }
		}
		edje_file_collection_list_free(groups);
	}

    elm_list_item_selected_set(elm_list_first_item_get(list), EINA_TRUE);

	elm_list_go(list);
}/*_fill_widget_groups*/



/*
 *Callback called in any hoversel objects when clicked signal is emitted.
 */
static void
_hoversel_selected_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
	elm_object_text_set(obj, elm_object_item_text_get(event_info));
    widget_device_type_set(widget, elm_object_item_text_get(event_info));
}/*_hoversel_selected_cb*/


/*
 *
 */
static void
_check_cosm_changed_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
    widget_cosm_set(widget, elm_check_state_get(obj));
}/*_check_cosm_changed_cb*/


/*
 *
 */
static void
_hoversel_device_type_fill(Widget_Class class)
{
    Evas_Object *hoversel = elm_object_name_find(win, "device type hoversel", -1);
    elm_hoversel_clear(hoversel);
    elm_object_text_set(hoversel, _("Device type"));

    if(class == WIDGET_CLASS_CONTROL)
    {
	    elm_hoversel_item_add(hoversel, DEVICE_TYPE_OUTPUT_CONTROL_BASIC, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
	    elm_hoversel_item_add(hoversel, DEVICE_TYPE_SLIDER_CONTROL_BASIC, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
	}
	else if(class == WIDGET_CLASS_SENSOR)
	{
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_BATTERY_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_COUNT_SENSOR_, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_CURRENT_SENSOR_, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_DIRECTION_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_DISTANCE_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_ENERGY_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_FAN_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_GENERIC_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_HUMIDITY_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_INPUT_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_OUTPUT_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_POWER_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_PRESSURE_SENSOR ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_SETPOINT_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_SPEED_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_TEMP_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_UV_SENSOR_BASIC, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_VOLTAGE_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_VOLUME_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
    	elm_hoversel_item_add(hoversel, DEVICE_TYPE_WEIGHT_SENSOR, ELM_ICON_NONE, ELM_ICON_NONE, NULL, NULL);
	}
}/*_hoversel_device_type_fill*/



/*
 *Create widget editor to allow user to add a new widget in an easy way.
 */
void
widget_editor_add(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *grid, *box, *layout, *frame;
	Evas_Object *button, *icon, *separator, *class_list, *groups_list, *entry, *check, *hoversel;
    char *s;

	app = edams_app_info_get();

	if (!app->location)	return;

    if(!app->widget)
    {
    	asprintf(&s, _("Add a new widget to '%s' location"),
    	                location_name_get(app->location));
        widget = widget_new(NULL, WIDGET_CLASS_SENSOR);
    }
    else
    {
    	asprintf(&s, _("Edit widget '%s' from '%s' location"),
    	                widget_name_get(app->widget),
                        location_name_get(app->location));
        widget = widget_new(widget_name_get(app->widget), widget_class_get(app->widget));
        widget_id_set(widget, widget_id_get(app->widget));
        widget_group_set(widget, widget_group_get(app->widget));
        widget_cosm_set(widget, widget_cosm_get(app->widget));
        widget_device_id_set(widget, widget_device_id_get(app->widget));
        widget_device_type_set(widget, widget_device_type_get(app->widget));
    }


    win = elm_win_util_standard_add("widget_editor", s);
	FREE(s);
    elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
    elm_win_autodel_set(win, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Widget class"));
	elm_grid_pack(grid, frame, 1, 1, 42, 44);
	evas_object_show(frame);

	class_list = elm_list_add(win);
	elm_list_select_mode_set(class_list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_show(class_list);
	elm_object_content_set(frame, class_list);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Widget name"));
    elm_grid_pack(grid, frame, 43, 1, 50, 10);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "widget name entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_object_content_set(frame, entry);
	evas_object_smart_callback_add(entry, "changed", _entry_widget_name_changed_cb, NULL);

	frame = elm_frame_add(win);
	evas_object_name_set(frame, "device id frame");
	elm_object_text_set(frame, _("Device id"));
    elm_grid_pack(grid, frame, 43, 12, 50, 10);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "device id entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_object_content_set(frame, entry);
    evas_object_smart_callback_add(entry, "changed", _entry_device_id_changed_cb, NULL);

   	hoversel = elm_hoversel_add(grid);
   	evas_object_name_set(hoversel, "device type hoversel");
   	elm_object_text_set(hoversel, _("Device type"));
    elm_grid_pack(grid, hoversel, 43, 23, 50, 6);
    evas_object_smart_callback_add(hoversel, "selected", _hoversel_selected_cb, NULL);

	check = elm_check_add(win);
	evas_object_name_set(check, "cosm check");
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "cosm-logo");
	elm_object_part_content_set(check, "icon", icon);
	elm_grid_pack(grid, check, 43, 30, 15, 5);
	evas_object_smart_callback_add(check, "changed", _check_cosm_changed_cb, NULL);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Preview"));
	elm_grid_pack(grid, frame, 1, 46, 42, 20);
	evas_object_show(frame);

	layout = elm_layout_add(win);
	evas_object_name_set(layout, "widget layout");
   	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(frame, layout);
	evas_object_show(layout);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Widgets"));
	elm_grid_pack(grid, frame, 43, 46, 58, 40);
	evas_object_show(frame);

	groups_list = elm_list_add(win);
	elm_list_select_mode_set(groups_list ,ELM_OBJECT_SELECT_MODE_ALWAYS );
	evas_object_name_set(groups_list, "groups list");
	evas_object_show(groups_list);
	elm_object_content_set(frame, groups_list);

    unsigned int x;
    for(x = 0; x != WIDGET_CLASS_LAST ; x++)
	{
	    if(x != WIDGET_CLASS_UNKNOWN)
	    {
    		Elm_Object_Item *it =  elm_list_item_append(class_list, widget_class_to_str(x), NULL, NULL, _list_item_class_selected_cb, x);

            if((x == widget_class_get(widget)))
            {
                elm_list_item_selected_set(it, EINA_TRUE);
            }
    	}
	}
	elm_list_go(class_list);

	box = elm_box_add(win);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_box_homogeneous_set(box, EINA_TRUE);
	elm_grid_pack(grid, box, 1, 90, 99, 10);
	evas_object_show(box);

	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_box_pack_end(box, separator);
	evas_object_show(separator);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "apply-window");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Ok"));
	elm_box_pack_end(box, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_apply_clicked_cb, app);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "window-close");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Close"));
	elm_box_pack_end(box, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_close_clicked_cb, app);

	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_box_pack_end(box, separator);
	evas_object_show(separator);

	evas_object_resize(win, 650, 650);
	evas_object_show(win);
}/*widget_editor_add*/
