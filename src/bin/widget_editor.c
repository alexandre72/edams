/*
 * devicespicker.c
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
#include "widget_wizzard.h"

static Evas_Object *win = NULL;
static Widget *widget = NULL;


/*
 *
 */
static void
_button_close_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    App_Info *app = data;

    if(!app->widget)
        widget_free(widget);

    evas_object_del(win);
}/*_button_close_clicked_cb*/


/*
 *
 */
static void
_button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    App_Info *app = data;
    const char *s;

    if(!app->widget)
        location_widgets_add(app->location, widget);

    location_save(app->location);
    update_naviframe_content(app->location);

    asprintf(&s, _("Widget '%s' has been added to '%s'"), widget_name_get(widget), location_name_get(app->location));
    statusbar_text_set(s, "dialog-information");
    FREE(s);

    evas_object_del(win);
}/*_button_apply_clicked_cb*/



/*
 * Return a random number between 1 & max.
 */
int prandom(int max)
{
    int partSize   = 1 + (max == RAND_MAX ? 0 : (RAND_MAX - max) / (max + 1));
    int maxUsefull = partSize * max + (partSize - 1);
    int draw;

    do {
        draw = rand();
    } while (draw > maxUsefull);

    return draw / partSize;

}/*prandom*/



/*
 * Send random value to current selected widget to let user see effects on it.
 */
static void
_layout_samples_test(Evas_Object *layout, Widget *widget)
{
	char s[10];
    srand((unsigned int)time((time_t *)NULL));

	Widget *sample = widget_new("sample", widget_class_get(widget));
	snprintf(s, sizeof(s), "%d", prandom(24));
	widget_xpl_current_set(sample, s);
	widget_xpl_type_set(sample, XPL_TYPE_GENERIC_SENSOR_BASIC);

    const char *t;
    /*Special widget with drag part, so need to convert device current value to float.*/
    if ((t = elm_layout_data_get(layout, "drag")))
    {
    	double level;
    	Edje_Message_Float msg;
           level =	(double)atoi(widget_xpl_current_get(sample)) / 100;
    	msg.val = level;
    	edje_object_message_send(elm_layout_edje_get(layout), EDJE_MESSAGE_FLOAT, 1, &msg);
    }

    if (atoi(widget_xpl_current_get(sample)) == 0)
    	elm_object_signal_emit(layout, "false", "over");
    else
    	elm_object_signal_emit(layout, "true", "over");

    if ((t = elm_layout_data_get(layout, "title")))
    {
    	elm_object_part_text_set(layout, "title.text", widget_xpl_device_get(sample));
    }

	if ((t = elm_layout_data_get(layout, "value")))
    {
	   	snprintf(s, sizeof(s), "%s%s", widget_xpl_current_get(sample), xpl_type_to_unit_symbol(widget_xpl_type_get(sample)) ? xpl_type_to_unit_symbol(widget_xpl_type_get(sample)) : "");
	    elm_object_part_text_set(layout, "value.text", s);
	}
    elm_object_signal_emit(layout, "updated", "over");

   	Evas_Object *edje;
	Evas_Coord w, h;
	elm_layout_sizing_eval(layout);
   	edje = elm_layout_edje_get(layout);
	edje_object_size_max_get(edje, &w, &h);
	evas_object_resize(elm_layout_edje_get(layout), w, h);
	elm_layout_sizing_eval(layout);
    widget_free(sample);
}/*_layout_samples_test*/




/*
 *
 */
 /*
static void
_update_cmnd_preview(Widget *widget)
{
	//Evas_Object *entry = elm_object_name_find(win, "widget description entry", -1);
   	//elm_object_text_set(entry, xpl_control_basic_cmnd_to_elm_str(widget));
}_update_cmnd_preview*/



/*
 *
 */
static void
_layout_signals_cb(void *data, Evas_Object *obj, const char  *emission, const char  *source)
{
	Widget *widget = data;
    Xpl_Type type = widget_xpl_type_get(widget);
    const char *s;

    /*Skip basic's edje signal emission*/
	if(strstr(source, "edje")) return;
	if(strstr(emission, "mouse")) return;
	if(strstr(emission, "show")) return;
	if(strstr(emission, "hide")) return;
	if(strstr(emission, "resize")) return;
	if(strstr(emission, "load")) return;
	if(strstr(emission, "updated")) return;
	if(strstr(emission, "drag,stop")) return;

	//fprintf(stdout, "emission=%s\n", emission);
	//fprintf(stdout, "source=%s\n", source);

    if(strstr(emission, "drag"))
    {
        Evas_Object *edje_obj = elm_layout_edje_get(obj);
        Edje_Drag_Dir dir = edje_object_part_drag_dir_get(edje_obj, source);
        double val;

        if(dir == EDJE_DRAG_DIR_X)
            edje_object_part_drag_value_get(edje_obj, source, &val, NULL);
        else if(dir == EDJE_DRAG_DIR_Y)
            edje_object_part_drag_value_get(edje_obj, source, NULL, &val);
        //else if(dir == EDJE_DRAG_DIR_XY)
        // edje_object_part_drag_value_get(o, source, &hval, &vval);

        /*Scale to device type format*/
        if(type == XPL_TYPE_SLIDER_CONTROL_BASIC)
        {
            val = (100 * val);
            asprintf(&s, "%d%%", (int)val);
        }
        else
        {
            val = (100 * val);
            asprintf(&s, "%d%%", (int)val);
        }

        widget_xpl_current_set(widget, s);
        FREE(s);
    }
    else
    {
	    widget_xpl_current_set(widget, emission);
    }

	if(elm_layout_data_get(obj, "value"))
	{
		asprintf(&s, "%s%s", widget_xpl_current_get(widget), xpl_type_to_unit_symbol(widget_xpl_type_get(widget)) ? xpl_type_to_unit_symbol(widget_xpl_type_get(widget)) : "");
		elm_object_part_text_set(obj, "value.text", s);
		FREE(s);
	}

    //_update_cmnd_preview(widget);
}/*_layout_signals_cb*/



/*
 *Callback called in list "groups" object when an item is selected
 */
static void
_list_item_group_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	const char *group = data;
	const char *t;

	Evas_Object *layout = elm_object_name_find(win, "widget layout", -1);
	elm_layout_file_set(layout, edams_edje_theme_file_get(), group);

    if(widget_class_get(widget) == WIDGET_CLASS_XPL_SENSOR_BASIC)
	    _layout_samples_test(layout, widget);
/*
	Evas_Object *entry = elm_object_name_find(win, "widget description entry", -1);
	if((t = elm_layout_data_get(layout, "description")))
	{
		const char *s;
		asprintf(&s, _("Description:%s"), t);
   		elm_object_text_set(entry, t);
   		FREE(s);
   	}
   	else
   	{
   		elm_object_text_set(entry, NULL);
   	}
*/
	/*Show cmnd schema specific parameters for control.basic widget according to edc item flags values.*/
	if(widget_class_get(widget) == WIDGET_CLASS_XPL_CONTROL_BASIC)
	{
	    /*xPL type is set by widget Edc file in 'tag' data*/
		if((t = elm_layout_data_get(layout, "tags")))
		{
			//FIXME:Hack to avoid setting input or ouput from sensor.basic class!
			if(strcmp(t, "input") == 0)			widget_xpl_type_set(widget, XPL_TYPE_INPUT_CONTROL_BASIC);
			else if(strcmp(t, "ouput") == 0)	widget_xpl_type_set(widget, XPL_TYPE_OUTPUT_CONTROL_BASIC);
			else								widget_xpl_type_set(widget, xpl_str_to_type(t));
			elm_layout_signal_callback_add(layout, "*", "*", _layout_signals_cb, widget);
		}
	}

	widget_group_set(widget, group);
}/*_list_item_group_selected_cb*/


/*
 *
 */
static void
_entry_name_changed_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    widget_name_set(widget, elm_object_text_get(obj));

    if( (widget_class_get(widget) == WIDGET_CLASS_XPL_SENSOR_BASIC) ||
        (widget_class_get(widget) == WIDGET_CLASS_XPL_CONTROL_BASIC))
    {
        widget_xpl_device_set(widget, elm_object_text_get(obj));
    }
}/*_entry_name_changed_cb*/


/*
 *
 */
static void
_list_item_class_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    Evas_Object *list = elm_object_name_find(win, "sensors.basic list", -1);
    Evas_Object *frame = elm_object_name_find(win, "name frame", -1);
    Widget_Class class = (Widget_Class) data;
	Evas_Object *layout = elm_object_name_find(win, "widget layout", -1);

	elm_layout_file_set(layout, edams_edje_theme_file_get(), NULL);

    widget_class_set(widget, class);

    if(class == WIDGET_CLASS_XPL_SENSOR_BASIC)
    {
        elm_object_disabled_set(list, EINA_FALSE);
        elm_object_text_set(frame, _("xpl device"));
    }
    else if(class== WIDGET_CLASS_XPL_CONTROL_BASIC)
    {
        elm_object_text_set(frame, _("xpl device"));
        elm_object_disabled_set(list, EINA_TRUE);
    }
    else if(class== WIDGET_CLASS_VIRTUAL)
    {
        elm_object_text_set(frame, _("Widget name"));
        elm_object_disabled_set(list, EINA_TRUE);
    }


    list = elm_object_name_find(win, "groups list", -1);
    elm_object_disabled_set(list, EINA_FALSE);
    elm_list_clear(list);
	Eina_List *groups = NULL, *l;
    const char *s;
	groups = edje_file_collection_list(edams_edje_theme_file_get());
	if (groups)
	{
		char *group;
		EINA_LIST_FOREACH(groups, l, group)
		{
			asprintf(&s, "widget/%s", widget_class_to_str(widget_class_get(widget)));
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

				elm_list_item_append(list, group, icon, NULL, _list_item_group_selected_cb, group);
			}
			else
			{
                FREE(s);
            }
		}
		edje_file_collection_list_free(groups);
	}
	elm_list_go(list);

}/*_list_item_class_selected_cb*/



/*
 *
 */
static void
_list_item_xpl_device_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    const char *str = data;
    Evas_Object *entry = elm_object_name_find(win, "name entry", -1);

    cJSON *root = cJSON_Parse(str);
	if(!root) return;
	cJSON *jdevice = cJSON_GetObjectItem(root, "DEVICE");
	cJSON *jtype = cJSON_GetObjectItem(root, "TYPE");
    const char *device = cJSON_PrintUnformatted(jdevice);
    const char *type = cJSON_PrintUnformatted(jtype);
	cJSON_Delete(root);

    strdelstr(device, "\"");
    strdelstr(type, "\"");

    widget_xpl_device_set(widget, device);
    widget_xpl_type_set(widget, xpl_str_to_type(type));
    elm_object_text_set(entry, device);

    FREE(device);
    FREE(type);
}


/*
 *Create widget wizzard to allow user to add a new widget in an easy way.
 */
void
widget_wizzard_add(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *grid, *box;
	Evas_Object *button, *icon, *separator, *frame, *list, *entry, *layout, *label;
	App_Info *app = (App_Info *) data;

	if (!app->location)	return;

    if(!app->widget)
        widget = widget_new(NULL, WIDGET_CLASS_UNKNOWN);
    else
        widget = app->widget;

    char *s;
	asprintf(&s, _("Adding a new widget to '%s' location"), location_name_get(app->location));
    win = elm_win_util_standard_add("widget_wizzard", s);
	FREE(s);
    elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
    elm_win_autodel_set(win, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Class"));
	elm_grid_pack(grid, frame, 1, 1, 42, 44);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS );
	evas_object_show(list);
	elm_object_content_set(frame, list);

    int x;
    for(x = 0; x != WIDGET_CLASS_LAST ; x++)
	{
	    if(x != WIDGET_CLASS_UNKNOWN)
	    {
    		Elm_Object_Item *it =  elm_list_item_append(list, widget_class_to_str(x), NULL, NULL, _list_item_class_selected_cb, (void*)x);

            if((app->widget)  && (x == widget_class_get(widget)))
                elm_list_item_selected_set(it, EINA_TRUE);
    	}
	}
	elm_list_go(list);
	elm_object_content_set(frame, list);

   	frame = elm_frame_add(win);
    elm_object_text_set(frame, _("Available"));
    elm_grid_pack(grid, frame, 43, 1, 56, 30);
    evas_object_show(frame);

    list = elm_list_add(win);
    evas_object_name_set(list, "sensors.basic list");
    elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS );
    evas_object_show(list);
    elm_object_content_set(frame, list);

    Eina_List *devices = NULL, *l;
    char *device_elem;

    devices = xpl_sensor_basic_list_get();

    EINA_LIST_FOREACH(devices, l, device_elem)
    {
        cJSON *root = cJSON_Parse(device_elem);
	    if(!root) continue;
	    cJSON *jdevice = cJSON_GetObjectItem(root, "DEVICE");
	    cJSON *jtype = cJSON_GetObjectItem(root, "TYPE");
        const char *device = cJSON_PrintUnformatted(jdevice);
        const char *type = cJSON_PrintUnformatted(jtype);
	    cJSON_Delete(root);

        strdelstr(device, "\"");
        strdelstr(type, "\"");

	    asprintf(&s, _("'%s' of type %s"), device, type);
        elm_list_item_append(list, s, NULL, NULL, _list_item_xpl_device_selected_cb, device_elem);
        FREE(s);
        FREE(device);
        FREE(type);
    }
    elm_list_go(list);
	elm_object_content_set(frame, list);
    elm_object_disabled_set(list, EINA_TRUE);

	frame = elm_frame_add(win);
	evas_object_name_set(frame, "name frame");
	elm_object_text_set(frame, NULL);
    elm_grid_pack(grid, frame, 43, 32, 56, 13);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "name entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);
	evas_object_smart_callback_add(entry, "changed", _entry_name_changed_cb, NULL);
    elm_object_text_set(entry, widget_name_get(widget));

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Preview"));
	elm_grid_pack(grid, frame, 1, 46, 42, 40);
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

	list = elm_list_add(win);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS );
	evas_object_name_set(list, "groups list");
	evas_object_show(list);
	elm_object_content_set(frame, list);
    elm_object_disabled_set(list, EINA_TRUE);

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

	/*Resize window*/
	evas_object_resize(win, 650, 500);
	evas_object_show(win);
}/*widget_wizzard_add*/
