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
#include "cmnd_editor.h"
#include "edams.h"
#include "path.h"


static void _layout_signals_cb(void *data, Evas_Object *obj, const char  *emission, const char  *source);
static void _list_control_basic_item_add(CmndEditor *cmndeditor, Widget *widget);
static void _list_control_basic_item_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);


/*
 *
 */
static void
_layout_signals_cb(void *data, Evas_Object *obj, const char  *emission, const char  *source)
{
    double val = 0;
    CmndEditor *cmndeditor = data;
    const char *type;
    char *s;

	Elm_Object_Item *selected_item = elm_list_selected_item_get(cmndeditor->list);

	if(!selected_item) return;

    Widget *widget = elm_object_item_data_get(selected_item);
    type = widget_xpl_type_get(widget);

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
    if(strcmp(type, XPL_TYPE_OUTPUT_CONTROL_BASIC) == 0)
    {
            if(val == 0)
                asprintf(&s, "disable");
            else
                asprintf(&s, "enable");
    }
    else if(strcmp(type, XPL_TYPE_INPUT_CONTROL_BASIC) == 0)
    {
            if(val == 0)
                asprintf(&s, "disable");
            else
                asprintf(&s, "enable");
    }
    else if(strcmp(type, XPL_TYPE_SLIDER_CONTROL_BASIC) == 0)
    {
        val = (val * 100);
        asprintf(&s, "%d%%", (int)val);
    }
    else
    {
        asprintf(&s, "%d", (int)val);
    }

    widget_xpl_current_set(widget, s);
	elm_object_part_text_set(obj, "value.text", s);
	FREE(s);

   	elm_object_text_set(cmndeditor->entry, xpl_control_basic_cmnd_to_elm_str(widget));
}/*_layout_signals_cb*/


/*
 *Callback called in 'control.basic' list when clicked signal is emitted.
 */
static void
_list_control_basic_item_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    Widget *widget = data;
    Evas_Object *list = elm_object_item_widget_get(event_info);

	CmndEditor *cmndeditor = evas_object_data_get(list, "cmndeditor");

    elm_layout_file_set(cmndeditor->layout, edams_edje_theme_file_get(), widget_group_get(widget));

	elm_layout_signal_callback_add(cmndeditor->layout, "*", "*", _layout_signals_cb, cmndeditor);
}/*_list_control_basic_item_selected_cb*/


/*
 *
 */
static void
_list_control_basic_item_add(CmndEditor *cmndeditor, Widget *widget)
{
	char *s;

	cmndeditor->icon = elm_icon_add(cmndeditor->win);
	asprintf(&s, "%s/icon", widget_group_get(widget));
   	elm_image_file_set(cmndeditor->icon, edams_edje_theme_file_get(), s);
   	FREE(s);
	elm_image_aspect_fixed_set(cmndeditor->icon, EINA_TRUE);
	elm_image_resizable_set(cmndeditor->icon, 1, 0);

	asprintf(&s, "%s %s", widget_xpl_device_get(widget), widget_xpl_type_get(widget));

	elm_list_item_append(cmndeditor->list, s, cmndeditor->icon, NULL, _list_control_basic_item_selected_cb, widget);
	elm_list_go(cmndeditor->list);
	FREE(s);
}/*_list_control_basic_item_selected_cb*/


/*
 *
 */
static void
_button_cancel_clicked_cb(void *data , Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    CmndEditor *cmndeditor = data;
    cmndeditor_close(cmndeditor);
}/*_cancel_button_clicked_cb*/

/*
 *
 */
void
cmndeditor_close(CmndEditor *cmndeditor)
{
    evas_object_del(cmndeditor->win);
    FREE(cmndeditor);
}/*cmndeditor_close*/


/*
 *
 */
CmndEditor *
cmndeditor_add()
{
    CmndEditor *cmndeditor = calloc(1, sizeof(CmndEditor));

	cmndeditor->win = elm_win_util_standard_add("osd_editor", NULL);
	elm_win_title_set(cmndeditor->win, _("Edit xPL control.basic CMND"));
	elm_win_autodel_set(cmndeditor->win, EINA_TRUE);
	elm_win_center(cmndeditor->win, EINA_TRUE, EINA_TRUE);

	cmndeditor->grid = elm_grid_add(cmndeditor->win);
	elm_grid_size_set(cmndeditor->grid, 100, 100);
	elm_win_resize_object_add(cmndeditor->win, cmndeditor->grid);
	evas_object_size_hint_weight_set(cmndeditor->grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(cmndeditor->grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(cmndeditor->grid);

	cmndeditor->frame = elm_frame_add(cmndeditor->win);
	elm_object_text_set(cmndeditor->frame, _("Select control.basic:"));
	elm_grid_pack(cmndeditor->grid, cmndeditor->frame, 1, 1, 99, 40);
	evas_object_show(cmndeditor->frame);

	cmndeditor->list = elm_list_add(cmndeditor->win);
	elm_scroller_policy_set(cmndeditor->list, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_ON);
	elm_list_select_mode_set(cmndeditor->list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_show(cmndeditor->list);
	elm_object_content_set(cmndeditor->frame, cmndeditor->list);

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
                _list_control_basic_item_add(cmndeditor, widget_elem);
        }
    }

	cmndeditor->frame = elm_frame_add(cmndeditor->win);
	elm_object_text_set(cmndeditor->frame, _("Widget set to"));
	elm_grid_pack(cmndeditor->grid, cmndeditor->frame, 1, 46, 49, 40);
	evas_object_show(cmndeditor->frame);

	cmndeditor->layout = elm_layout_add(cmndeditor->win);
	evas_object_data_set(cmndeditor->list, "cmndeditor", cmndeditor);
   	evas_object_size_hint_weight_set(cmndeditor->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(cmndeditor->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(cmndeditor->frame, cmndeditor->layout);
	evas_object_show(cmndeditor->layout);

	cmndeditor->frame = elm_frame_add(cmndeditor->win);
	elm_object_text_set(cmndeditor->frame, _("xPL CMND Preview"));
	elm_grid_pack(cmndeditor->grid, cmndeditor->frame, 50, 46, 49, 40);
	evas_object_show(cmndeditor->frame);

    cmndeditor->entry = elm_entry_add(cmndeditor->win);
   	elm_entry_scrollable_set( cmndeditor->entry, EINA_FALSE);
    elm_entry_editable_set( cmndeditor->entry, EINA_FALSE);
   	evas_object_size_hint_weight_set( cmndeditor->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set( cmndeditor->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(cmndeditor->frame,  cmndeditor->entry);
	evas_object_show( cmndeditor->entry);

	cmndeditor->hbox = elm_box_add(cmndeditor->win);
	elm_box_horizontal_set(cmndeditor->hbox, EINA_TRUE);
	elm_box_homogeneous_set(cmndeditor->hbox, EINA_TRUE);
	elm_grid_pack(cmndeditor->grid, cmndeditor->hbox, 1, 90, 99, 10);
	evas_object_show(cmndeditor->hbox);

	cmndeditor->ok_button = elm_button_add(cmndeditor->win);
	cmndeditor->icon = elm_icon_add(cmndeditor->win);
	elm_icon_order_lookup_set(cmndeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(cmndeditor->icon, "apply-window");
	elm_object_part_content_set(cmndeditor->ok_button, "icon", cmndeditor->icon);
	elm_object_text_set(cmndeditor->ok_button, _("Ok"));
	elm_box_pack_end(cmndeditor->hbox, cmndeditor->ok_button);
	evas_object_show(cmndeditor->ok_button);
	evas_object_size_hint_align_set(cmndeditor->ok_button, EVAS_HINT_FILL, 0);

	cmndeditor->cancel_button = elm_button_add(cmndeditor->win);
	cmndeditor->icon = elm_icon_add(cmndeditor->win);
	elm_icon_order_lookup_set(cmndeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(cmndeditor->icon, "window-close");
	elm_object_part_content_set(cmndeditor->cancel_button, "icon", cmndeditor->icon);
	elm_object_text_set(cmndeditor->cancel_button, _("Close"));
	elm_box_pack_end(cmndeditor->hbox, cmndeditor->cancel_button);
	evas_object_show(cmndeditor->cancel_button);
	evas_object_size_hint_align_set(cmndeditor->cancel_button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(cmndeditor->cancel_button, "clicked", _button_cancel_clicked_cb , cmndeditor);

	evas_object_resize(cmndeditor->win, 400, 400);
	evas_object_show(cmndeditor->win);

	return cmndeditor;
}/*cmnd_editor_add*/
