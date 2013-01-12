/*
 * widgets_picker.c
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

#include "widgets_picker.h"
#include "device.h"
#include "edams.h"
#include "path.h"
#include "utils.h"


/*Global window elm object*/
static Evas_Object *win;


/*Others callbacks*/
static void _button_add_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);


/*
 *
 */
static void
_hoversel_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_text_set(obj, elm_object_item_text_get(event_info));
}



/*
 *Callback called in button "add" object when clicked signal is emitted.
 */
static void
_button_add_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Device *device;
	Evas_Object *entry, *hoversel;
	Condition ifcondition;
	char ifvalue[256];
	Class_Flags toclass;
	char tocmnd[256];

	device = evas_object_data_get(win, "device");

	hoversel = elm_object_name_find(win, "ifcondition hoversel", -1);
	ifcondition = device_str_to_condition(elm_object_text_get(hoversel));

	entry = elm_object_name_find(win, "ifvalue entry", -1);
	snprintf(ifvalue, sizeof(ifvalue), "%s", elm_object_text_get(entry));

	hoversel = elm_object_name_find(win, "toclass hoversel", -1);
	toclass = device_str_to_class(elm_object_text_get(hoversel));

	entry = elm_object_name_find(win, "tocmnd entry", -1);
	snprintf(tocmnd, sizeof(tocmnd), "%s", elm_object_text_get(entry));

	Action *action;
	action = action_new(ifcondition, ifvalue , toclass, tocmnd);

	device_action_add(device, action);
	device_save(device);
}


/*
 *
 */
void
actions_editor_add(void *data __UNUSED__, Evas_Object * obj __UNUSED__,
					void *event_info __UNUSED__)
{
	char s[256];

	Widget *widget;
	Device *device;

	Evas_Object *entry, *hoversel;
	Evas_Object *gd;
	Evas_Object *ic, *bx, *frame;
	Evas_Object *bt;
	Evas_Object *list;

	App_Info *app = (App_Info *) data;

	Evas_Object *widgets_list = elm_object_name_find(app->win, "widgets list", -1);
	Elm_Object_Item *selected_item = elm_list_selected_item_get(widgets_list);

	if(!(widget = elm_object_item_data_get(selected_item))) return;

	if(!(device = device_load(widget_device_filename_get(widget)))) return;

	snprintf(s, sizeof(s), _("Edit actions for '%s' xPL device"), device_name_get(device));

	win = elm_win_util_standard_add("actions_editor", s);
	evas_object_data_set(win, "device", device);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
   	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

	gd = elm_grid_add(win);
	elm_grid_size_set(gd, 100, 100);
	elm_win_resize_object_add(win, gd);
	evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gd, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(gd);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Actions list"));
	elm_grid_pack(gd, frame, 1, 1, 99, 60);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_scroller_policy_set(list, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_ON);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_name_set(list, "actions list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Action"));
	elm_grid_pack(gd, frame, 1, 61, 99, 20);
	evas_object_show(frame);

   	bx = elm_box_add(win);
	elm_object_content_set(frame, bx);
   	elm_box_horizontal_set(bx, EINA_TRUE);

   	hoversel = elm_hoversel_add(gd);
   	evas_object_name_set(hoversel, "ifcondition hoversel");
   	elm_object_text_set(hoversel, _("Condition"));
   	elm_box_pack_end(bx, hoversel);
	int x = 0;
	for(x = 0;x != CONDITION_LAST;x++)
	{
		if(x == UNKNOWN_CONDITION) continue;
	   elm_hoversel_item_add(hoversel, device_condition_to_str(x), ELM_ICON_NONE, ELM_ICON_NONE, NULL, (void*)(int)x);
	}
	evas_object_show(hoversel);
	evas_object_smart_callback_add(hoversel, "selected", _hoversel_selected_cb, NULL);

   	frame = elm_frame_add(win);
	elm_box_pack_end(bx, frame);
   	elm_layout_text_set(frame, NULL, _("Value"));
   	evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(frame);

   	entry = elm_entry_add(win);
   	evas_object_name_set(entry, "ifvalue entry");
	elm_object_content_set(frame, entry);

   	hoversel = elm_hoversel_add(gd);
   	evas_object_name_set(hoversel, "toclass hoversel");
   	elm_object_text_set(hoversel, _("Type"));
	elm_box_pack_end(bx, hoversel);
	for(x = 0;x != CLASS_LAST;x++)
	{
		if(x == UNKNOWN_CLASS) continue;
		elm_hoversel_item_add(hoversel, device_class_to_str(x), ELM_ICON_NONE, ELM_ICON_NONE, NULL, (void*)(int)x);
	}
	evas_object_show(hoversel);
	evas_object_smart_callback_add(hoversel, "selected", _hoversel_selected_cb, NULL);

   	frame = elm_frame_add(win);
	elm_box_pack_end(bx, frame);
   	elm_layout_text_set(frame, NULL, _("Data"));
   	evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(frame);

   	entry = elm_entry_add(win);
   	evas_object_name_set(entry, "tocmnd entry");
	elm_object_content_set(frame, entry);

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_box_homogeneous_set(bx, EINA_TRUE);
	elm_grid_pack(gd, bx, 1, 89, 99, 10);
	evas_object_show(bx);

	bt = elm_button_add(win);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "list-add");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Add"));
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(bt, "clicked", _button_add_clicked_cb, NULL);

	bt = elm_button_add(win);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "list-remove");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Remove"));
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
	//evas_object_smart_callback_add(bt, "clicked", _button_remove_clicked_cb, NULL);

	bt = elm_button_add(win);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "window-close");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Close"));
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, win);

	evas_object_resize(win, 600, 450);
	evas_object_show(win);
}/*widgets_picker_add*/
