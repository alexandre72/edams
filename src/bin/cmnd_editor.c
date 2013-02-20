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
    const char *s;

	Widget *widget = evas_object_data_get(win, "widget");
    if((!widget) || (!widget_xpl_current_get(widget)))
        return NULL;

    s = action_cmnd_data_format(widget_xpl_device_get(widget),
                            xpl_type_to_str(widget_xpl_type_get(widget)),
                            widget_xpl_current_get(widget),
                            widget_xpl_data1_get(widget));

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
_layout_signals_cb(void *data, Evas_Object *obj, const char  *emission, const char  *source)
{
	Evas_Object *entry = elm_object_name_find(win, "cmnd preview entry", -1);
	Widget *widget = data;
    Xpl_Type type = widget_xpl_type_get(widget);
    char *s;

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

	asprintf(&s, "%s%s", widget_xpl_current_get(widget), xpl_type_to_unit_symbol(widget_xpl_type_get(widget)) ? xpl_type_to_unit_symbol(widget_xpl_type_get(widget)) : "");
	elm_object_part_text_set(obj, "value.text", s);
	FREE(s);

   	elm_object_text_set(entry, xpl_control_basic_cmnd_to_elm_str(widget));

    evas_object_data_set(win, "widget", widget);
}/*_layout_signals_cb*/




/*
 *Callback called in 'control.basic' list when clicked signal is emitted.
 */
static void
_list_control_basic_item_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    Widget *widget = (Widget*) data;

	Evas_Object *layout = elm_object_name_find(win, "widget layout", -1);
    elm_layout_file_set(layout, edams_edje_theme_file_get(), widget_group_get(widget));

	elm_layout_signal_callback_add(layout, "*", "*", _layout_signals_cb, widget);
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
cmnd_editor_add()
{
	Evas_Object *grid, *hbox, *frame, *layout, *entry;
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

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Widget set to"));
	elm_grid_pack(grid, frame, 1, 46, 49, 40);
	evas_object_show(frame);

	layout = elm_layout_add(win);
	evas_object_name_set(layout, "widget layout");
   	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(frame, layout);
	evas_object_show(layout);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("xPL CMND Preview"));
	elm_grid_pack(grid, frame, 50, 46, 49, 40);
	evas_object_show(frame);

   	entry = elm_entry_add(win);
    evas_object_name_set(entry, "cmnd preview entry");
   	elm_entry_scrollable_set(entry, EINA_FALSE);
    elm_entry_editable_set(entry, EINA_FALSE);
   	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(frame, entry);
	evas_object_show(entry);

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
