/*
 * actions_editor.c
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

#include <stdio.h>

#include "action.h"
#include "cJSON.h"
#include "edams.h"
#include "myfileselector.h"
#include "path.h"
#include "utils.h"
#include "xpl.h"

/*Global objects*/
static Evas_Object *win = NULL;

/*Evas_Object Callbacks*/
static void _button_add_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_remove_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _list_item_selected_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _hoversel_selected_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info);
static void _button_arg_edit_clicked_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info);

/*Others funcs*/
static void _list_action_add(Evas_Object *list, Action *action);

/*
 *
 */
static void
_button_edit_arg_apply_clicked_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
	Action_Type type = (Action_Type)data;
	Evas_Object *cwin;
    const char *s = NULL;

    cwin = elm_object_top_widget_get(obj);

    switch(type)
	{
	    case ACTION_TYPE_CMND:
                s = cmnd_editor_values_get();
                break;

		case ACTION_TYPE_MAIL:
                s = mail_editor_values_get();
                break;

   		case ACTION_TYPE_EXEC:
                s = exec_editor_values_get();
                break;

   		case ACTION_TYPE_DEBUG:
                s = debug_editor_values_get();
                break;

		case ACTION_TYPE_UNKNOWN:
		case ACTION_TYPE_LAST:
            	evas_object_del(cwin);
                return;
    }

    if(s)
    {
        evas_object_data_set(win, "data arg", s);
    }

	evas_object_del(cwin);
}/*_button_edit_arg_apply_clicked_cb*/



/*
 *
 */
static void
_button_arg_edit_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	App_Info *app = evas_object_data_get(win, "app");
	Evas_Object *cwin = NULL;
	Evas_Object *hbox = NULL;
	Evas_Object *button, *icon;

	Action_Type type = (Action_Type)data;

	switch(type)
	{
		case ACTION_TYPE_CMND:
	            cwin = cmnd_editor_add(app);
                hbox = cmnd_editor_hbox_get();
                break;

		case ACTION_TYPE_MAIL:
	            cwin = mail_editor_add();
                hbox = mail_editor_hbox_get();
                break;

		case ACTION_TYPE_EXEC:
	            cwin = exec_editor_add();
                hbox = exec_editor_hbox_get();
                break;

		case ACTION_TYPE_DEBUG:
	            cwin = debug_editor_add();
                hbox = debug_editor_hbox_get();
			    break;

		case ACTION_TYPE_UNKNOWN:
		case ACTION_TYPE_LAST:
				break;
	}

	button = elm_button_add(cwin);
	icon = elm_icon_add(cwin);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "apply-window");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Ok"));
	elm_box_pack_end(hbox, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_edit_arg_apply_clicked_cb, (void *)type);
}/*_button_arg_edit_mail_clicked_cb*/

/*
 *Callback called in any hoversel objects when clicked signal is emitted.
 */
static void
_hoversel_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_text_set(obj, elm_object_item_text_get(event_info));
    evas_object_data_set(obj, "selected", elm_object_item_data_get(event_info));

	if(strcmp(evas_object_name_get(obj), "type hoversel") == 0 )
	{
    	Action_Type type = (Action_Type)elm_object_item_data_get(event_info);

    	Evas_Object *button = elm_object_name_find(win, "arg edit button", -1);
    	elm_object_disabled_set(button, EINA_FALSE);
    	evas_object_smart_callback_del(button, "clicked", _button_arg_edit_clicked_cb);
    	evas_object_smart_callback_add(button, "clicked", _button_arg_edit_clicked_cb, (void*)(unsigned int)type);
    }
    else
    {
        return;
    }
}/*_hoversel_selected_cb*/

/*
 *Callback called in button "remove" object when clicked signal is emitted.
 */
static void
_button_remove_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *list = elm_object_name_find(win, "actions list", -1);
    App_Info *app = evas_object_data_get(win, "app");

	Elm_Object_Item *selected_item = elm_list_selected_item_get(list);

	if(!selected_item) return;

	Action *action = elm_object_item_data_get(selected_item);

	widget_action_del(app->widget, action);
   	elm_object_item_del(selected_item);

    location_save(app->location);
}/*_button_remove_clicked_cb*/



/*
 *Callback called in button "add" object when clicked signal is emitted.
 */
static void
_button_add_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	char *ifvalue;
    App_Info *app = evas_object_data_get(win, "app");
	Evas_Object *hoversel, *slider;
	Evas_Object *list;

	hoversel = elm_object_name_find(win, "ifcondition hoversel", -1);
	Condition ifcondition = (Condition)evas_object_data_get(hoversel, "selected");

	slider = elm_object_name_find(win, "ifvalue slider", -1);
	asprintf(&ifvalue, "%1.0f", elm_slider_value_get(slider));

	hoversel = elm_object_name_find(win, "type hoversel", -1);
	Action_Type type = (Action_Type)evas_object_data_get(hoversel, "selected");

	char *arg = evas_object_data_get(win, "data arg");

	if(!arg || !ifvalue) return;

	Action *action;
	action = action_new(ifcondition, ifvalue , type, arg);
	widget_action_add(app->widget, action);
	location_save(app->location);
    FREE(ifvalue);
	FREE(arg);
	arg = evas_object_data_del(win, "data arg");

	list = elm_object_name_find(win, "actions list", -1);
	_list_action_add(list, action);
}/*_button_add_clicked_cb*/


/*
 *Callback called in list "actions" when item clicked signal is emitted.
 */
static void
_list_item_selected_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info)
{
	Evas_Object *slider, *hoversel;
	Action *action = (Action *)elm_object_item_data_get(event_info);

	slider = elm_object_name_find(win, "ifvalue slider", -1);
    elm_slider_value_set(slider, atoi(action_ifvalue_get(action)));

	hoversel = elm_object_name_find(win, "ifcondition hoversel", -1);
	elm_object_text_set(hoversel, action_condition_to_str(action_ifcondition_get(action)));
    evas_object_data_set(hoversel, "selected", (void*)(unsigned int)action_ifcondition_get(action));

	hoversel = elm_object_name_find(win, "type hoversel", -1);
	elm_object_text_set(hoversel, action_type_to_desc(action_type_get(action)));
    evas_object_data_set(hoversel, "selected", (void*)(unsigned int)action_type_get(action));

}/*_list_item_selected_cb*/


/*
 *Add actions to list object
 */
static void
_list_action_add(Evas_Object *list, Action *action)
{
	char *s;
	asprintf(&s, _("If %s %s then %s"),
										action_condition_to_str(action_ifcondition_get(action)),
										action_ifvalue_get(action),
										action_type_to_desc(action_type_get(action)));

	Evas_Object *icon;
	icon = elm_icon_add(win);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
    evas_object_size_hint_min_set(icon, 48, 48);
    evas_object_size_hint_align_set(icon, 0.5, EVAS_HINT_FILL);

    if(action_type_get(action) == ACTION_TYPE_CMND)
       	elm_image_file_set(icon, edams_edje_theme_file_get(), "elm/icon/xpl/default");
    else if(action_type_get(action) == ACTION_TYPE_MAIL)
        elm_icon_standard_set(icon, "mail-send");
    else if(action_type_get(action) == ACTION_TYPE_EXEC)
        elm_icon_standard_set(icon, "system-run");
    else if(action_type_get(action) == ACTION_TYPE_DEBUG)
        elm_icon_standard_set(icon, "debug");
    else
       	elm_image_file_set(icon, edams_edje_theme_file_get(), "");

	//elm_image_aspect_fixed_set(icon, EINA_TRUE);
	//evas_object_resize(icon, 24, 24);
   // elm_image_resizable_set(icon, EINA_FALSE, EINA_FALSE);

	elm_list_item_append(list, s, icon, NULL, _list_item_selected_cb, action);
	FREE(s);
	elm_list_go(list);
}/*_list_action_add*/


/*
 *
 */
void
actions_editor_add(void *data, Evas_Object * obj __UNUSED__,	void *event_info __UNUSED__)
{
	App_Info *app = (App_Info *) data;
	Evas_Object *hoversel, *slider;
	Evas_Object *grid;
	Evas_Object *icon, *bx, *frame;
	Evas_Object *button;
	Evas_Object *list;
	char *s;

    if((!app->widget) || (widget_class_get(app->widget) != WIDGET_CLASS_XPL_SENSOR_BASIC)) return;

    Xpl_Type type = widget_xpl_type_get(app->widget);

	asprintf(&s, _("Edit actions for '%s' xPL sensor.basic"), widget_xpl_device_get(app->widget));
	win = elm_win_util_standard_add("actions_editor", s);
	FREE(s);
	evas_object_data_set(win, "app", app);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
   	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Actions list"));
	elm_grid_pack(grid, frame, 1, 1, 99, 60);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_scroller_policy_set(list, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_ON);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_name_set(list, "actions list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

	Eina_List *l, *actions;
	Action *action;
	actions = widget_actions_list_get(app->widget);
	EINA_LIST_FOREACH(actions, l, action)
		_list_action_add(list, action);

   	hoversel = elm_hoversel_add(grid);
   	evas_object_name_set(hoversel, "ifcondition hoversel");
	unsigned int x = 0;
	for(x = 0;x != CONDITION_LAST;x++)
	{
		if(x == CONDITION_UNKNOWN) continue;
	   	elm_hoversel_item_add(hoversel, action_condition_to_str(x), ELM_ICON_NONE, ELM_ICON_NONE, NULL, (void*)(unsigned int)x);
	}
	elm_grid_pack(grid, hoversel, 1, 68, 10, 8);
	evas_object_show(hoversel);
	evas_object_smart_callback_add(hoversel, "selected", _hoversel_selected_cb, NULL);

   	frame = elm_frame_add(win);
	elm_grid_pack(grid, frame, 11, 61, 40, 20);
   	elm_object_text_set(frame, xpl_type_to_units(type));
	evas_object_show(frame);

    slider = elm_slider_add(win);
   	evas_object_name_set(slider, "ifvalue slider");
    //FIXME:'%' could be a symbol format used by device unit, but it's a C printf* reserved keyword too.
    //so, try to print it correctly.
    asprintf(&s, "%%1.0f %s", xpl_type_to_unit_symbol(type));
    elm_slider_unit_format_set(slider, s);
    FREE(s);
    elm_slider_min_max_set(slider, xpl_type_current_min_get(type), xpl_type_current_max_get(type));
    evas_object_show(slider);
	elm_object_content_set(frame, slider);

   	hoversel = elm_hoversel_add(grid);
   	evas_object_name_set(hoversel, "type hoversel");
   	elm_object_text_set(hoversel, _("Action"));
	elm_grid_pack(grid, hoversel, 52, 68, 40, 8);
	for(x = 0;x != ACTION_TYPE_LAST;x++)
	{
		if(x == ACTION_TYPE_UNKNOWN) continue;
		if((x == ACTION_TYPE_DEBUG) && (!edams_settings_debug_get())) continue;
		elm_hoversel_item_add(hoversel, action_type_to_desc(x), ELM_ICON_NONE, ELM_ICON_NONE, NULL, (void*)(unsigned int)x);
	}
	evas_object_show(hoversel);
	evas_object_smart_callback_add(hoversel, "selected", _hoversel_selected_cb, NULL);

	button = elm_button_add(win);
	elm_object_disabled_set(button, EINA_TRUE);
	elm_object_text_set(button, _("Edit..."));
   	evas_object_name_set(button, "arg edit button");
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "edit");
	elm_object_part_content_set(button, "icon", icon);
	elm_grid_pack(grid, button, 93, 68, 6, 8);
	evas_object_show(button);

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_box_homogeneous_set(bx, EINA_TRUE);
	elm_grid_pack(grid, bx, 1, 89, 99, 10);
	evas_object_show(bx);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "list-add");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Add"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_add_clicked_cb, NULL);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "list-remove");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Remove"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_remove_clicked_cb, NULL);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "window-close");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Close"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", window_clicked_close_cb, win);

	evas_object_resize(win, 600, 450);
	evas_object_show(win);
}/*actions_editor_add*/
