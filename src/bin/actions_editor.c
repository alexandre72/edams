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
#include <stdio.h>

#include "action.h"
#include "cJSON.h"
#include "device.h"
#include "edams.h"
#include "myfileselector.h"
#include "path.h"
#include "utils.h"


/*Global window elm object*/
static Evas_Object *win = NULL;


/*Callbacks*/
static void _button_add_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_remove_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _list_item_selected_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _hoversel_selected_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info);
static void _button_arg_edit_clicked_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info);
/*Others funcs*/
static void _list_action_add(Device *device, Action *action);


/*
 *
 */
static void
_myfileselector_button_action_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	const char *sel;
	MyFileSelector *myfs = (MyFileSelector *) data;

	sel = elm_fileselector_selected_get(myfs->fs);

	if (sel)
	{
		Evas_Object *entry;
		entry = evas_object_data_get(myfs->win, "entry");
		elm_object_text_set(entry, sel);
	}
	myfileselector_close(myfs);
}/*_myfileselector_button_action_clicked_cb*/


/*
 *
 */
static void
_button_open_file_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *entry = data;
	MyFileSelector *myfs;

	myfs = myfileselector_add();
	myfileselector_set_title(myfs, _("Select a program"));
	evas_object_data_set(myfs->win, "entry", entry);
	evas_object_smart_callback_add(myfs->action_bt, "clicked", _myfileselector_button_action_clicked_cb, myfs);
}/*_button_open_file_clicked_cb*/


/*
 *
 */
static void
_button_edit_arg_apply_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *cwin = (Evas_Object *)data;
	const char *title = elm_win_title_get(cwin);
	cJSON *root;

	root=cJSON_CreateObject();

	if(strcmp(title, _("Edit cmnd action")) == 0)
	{
		//asprintf(s, sizeof(s), ""DEVICE=<sensor name>TYPE=<sensor type>CURRENT=<value to which device should be set>[DATA1=<additional data>]");
	}
	else if(strcmp(title, _("Edit exec")) == 0)
	{
		Evas_Object *entry = elm_object_name_find(cwin, "exec entry", -1);
		Evas_Object *check = elm_object_name_find(cwin, "terminal check", -1);

		cJSON_AddItemToObject(root, "EXEC", cJSON_CreateString(elm_object_text_get(entry)));
		cJSON_AddItemToObject(root, "TERMINAL", cJSON_CreateString(elm_check_state_get(check) ? "true" : "false"));
	}
	else if(strcmp(title, _("Edit debug")) == 0)
	{
		Evas_Object *entry = elm_object_name_find(cwin, "debug entry", -1);

		cJSON_AddItemToObject(root, "PRINT", cJSON_CreateString(elm_object_text_get(entry)));
	}
	else if(strcmp(title, _("Edit mail")) == 0)
	{
		Evas_Object *from_entry = elm_object_name_find(cwin, "from entry", -1);
		Evas_Object *to_entry = elm_object_name_find(cwin, "to entry", -1);
		Evas_Object *subject_entry = elm_object_name_find(cwin, "subject entry", -1);
		Evas_Object *body_entry = elm_object_name_find(cwin, "body entry", -1);

		cJSON_AddItemToObject(root, "FROM", cJSON_CreateString(elm_object_text_get(from_entry)));
		cJSON_AddItemToObject(root, "TO", cJSON_CreateString(elm_object_text_get(to_entry)));
		cJSON_AddItemToObject(root, "SUBJECT", cJSON_CreateString(elm_object_text_get(subject_entry)));
		cJSON_AddItemToObject(root, "BODY", cJSON_CreateString(elm_object_text_get(body_entry)));
	}
	else
	{
		debug(stderr, _("Internal error in file %s at %d"), __FILE__, __LINE__);
	}

	evas_object_data_set(win, "data arg", strdup(cJSON_PrintUnformatted(root)));
	cJSON_Delete(root);
	evas_object_del(cwin);
}/*_button_edit_arg_apply_clicked_cb*/


/*
 *
 */
static void
_button_arg_edit_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *grid;
	Evas_Object *icon, *bx, *frame;
	Evas_Object *button, *entry, *check;
	Evas_Object *cwin;

	Action_Type type = (Action_Type)data;

	cwin = elm_win_util_standard_add("actions_editor", NULL);
	elm_win_autodel_set(cwin, EINA_TRUE);
	elm_win_center(cwin, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(cwin);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(cwin, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(grid);

	switch(type)
	{
		case CMND_ACTION:
				elm_win_title_set(cwin, _("Edit cmnd action"));
				break;
		case MAIL_ACTION:
				elm_win_title_set(cwin, _("Edit mail"));

				frame = elm_frame_add(cwin);
				elm_object_text_set(frame, _("From:"));
				elm_grid_pack(grid, frame, 1, 1, 99, 15);
				evas_object_show(frame);

				entry = elm_entry_add(cwin);
				evas_object_name_set(entry, "from entry");
				elm_entry_scrollable_set(entry, EINA_TRUE);
				elm_entry_editable_set(entry, EINA_TRUE);
				elm_entry_single_line_set(entry, EINA_TRUE);
				evas_object_show(entry);
				elm_object_content_set(frame, entry);

				frame = elm_frame_add(cwin);
				elm_object_text_set(frame, _("To:"));
				elm_grid_pack(grid, frame, 1, 17, 99, 15);
				evas_object_show(frame);

				entry = elm_entry_add(cwin);
				evas_object_name_set(entry, "to entry");
				elm_entry_scrollable_set(entry, EINA_TRUE);
				elm_entry_editable_set(entry, EINA_TRUE);
				elm_entry_single_line_set(entry, EINA_TRUE);
				evas_object_show(entry);
				elm_object_content_set(frame, entry);

				frame = elm_frame_add(cwin);
				elm_object_text_set(frame, _("Subject:"));
				elm_grid_pack(grid, frame, 1, 32, 99, 15);
				evas_object_show(frame);

				entry = elm_entry_add(cwin);
				evas_object_name_set(entry, "subject entry");
				elm_entry_scrollable_set(entry, EINA_TRUE);
				elm_entry_editable_set(entry, EINA_TRUE);
				elm_entry_single_line_set(entry, EINA_TRUE);
				evas_object_show(entry);
				elm_object_content_set(frame, entry);

				frame = elm_frame_add(cwin);
				elm_object_text_set(frame, _("Body:"));
				elm_grid_pack(grid, frame, 1, 47, 99, 40);
				evas_object_show(frame);

				entry = elm_entry_add(cwin);
				evas_object_name_set(entry, "body entry");
				elm_entry_scrollable_set(entry, EINA_TRUE);
				elm_entry_editable_set(entry, EINA_TRUE);
				elm_entry_single_line_set(entry, EINA_FALSE);
				evas_object_show(entry);
				elm_object_content_set(frame, entry);

				break;
		case EXEC_ACTION:
				elm_win_title_set(cwin, _("Edit exec"));

				frame = elm_frame_add(cwin);
				elm_object_text_set(frame, _("Path of program:"));
				elm_grid_pack(grid, frame, 1, 1, 99, 15);
				evas_object_show(frame);

				bx = elm_box_add(cwin);
				elm_box_horizontal_set(bx, EINA_TRUE);
				evas_object_show(bx);

				entry = elm_entry_add(cwin);
				evas_object_name_set(entry, "exec entry");
				elm_entry_scrollable_set(entry, EINA_TRUE);
				elm_entry_editable_set(entry, EINA_TRUE);
				elm_entry_single_line_set(entry, EINA_TRUE);
				elm_box_pack_end(bx, entry);
				evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
				evas_object_show(entry);

				button = elm_button_add(cwin);
				elm_object_text_set(button, _("Open..."));
				icon = elm_icon_add(cwin);
				elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
				elm_icon_standard_set(icon, "document-open");
				elm_object_part_content_set(button, "icon", icon);
				evas_object_smart_callback_add(button, "clicked", _button_open_file_clicked_cb, entry);
				elm_box_pack_end(bx, button);
				evas_object_show(button);

				elm_object_content_set(frame, bx);

				check = elm_check_add(cwin);
				evas_object_name_set(check, "terminal check");
				elm_object_text_set(check, _("Exec in terminal"));
				elm_grid_pack(grid, check, 0, 20, 30, 10);
				elm_check_state_set(check, EINA_FALSE);
				evas_object_show(check);
				break;
		case DEBUG_ACTION:
				elm_win_title_set(cwin, _("Edit debug"));

				frame = elm_frame_add(cwin);
				elm_object_text_set(frame, _("Printf message:"));
				elm_grid_pack(grid, frame, 1, 1, 99, 15);
				evas_object_show(frame);

				entry = elm_entry_add(cwin);
				evas_object_name_set(entry, "debug entry");
				elm_entry_scrollable_set(entry, EINA_TRUE);
				elm_entry_editable_set(entry, EINA_TRUE);
				elm_entry_single_line_set(entry, EINA_TRUE);
				evas_object_show(entry);
				elm_object_content_set(frame, entry);
				break;

		case UNKNOWN_ACTION:
		case ACTION_TYPE_LAST:
				break;
	}

	bx = elm_box_add(cwin);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_box_homogeneous_set(bx, EINA_TRUE);
	elm_grid_pack(grid, bx, 1, 89, 99, 10);
	evas_object_show(bx);

	button = elm_button_add(cwin);
	icon = elm_icon_add(cwin);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "window-close");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Close"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", window_clicked_close_cb, cwin);

	button = elm_button_add(cwin);
	icon = elm_icon_add(cwin);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "apply-window");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Ok"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_edit_arg_apply_clicked_cb, cwin);

	evas_object_resize(cwin, 400, 400);
	evas_object_show(cwin);
}/*_button_arg_edit_mail_clicked_cb*/




/*
 *
 */
static void
_hoversel_selected_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
	elm_object_text_set(obj, elm_object_item_text_get(event_info));
	evas_object_data_set(obj, "selected", elm_object_item_data_get(event_info));

	if(strcmp(evas_object_name_get(obj), "type hoversel") != 0 ) return;

	Action_Type type = (Action_Type)elm_object_item_data_get(event_info);

	Evas_Object *button = elm_object_name_find(win, "arg edit button", -1);
	elm_object_disabled_set(button, EINA_FALSE);
	evas_object_smart_callback_del(button, "clicked", _button_arg_edit_clicked_cb);
	evas_object_smart_callback_add(button, "clicked", _button_arg_edit_clicked_cb, (void*)(unsigned int)type);
}/*_hoversel_selected_cb*/



/*
 *Callback called in button "remove" object when clicked signal is emitted.
 */
static void
_button_remove_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *list = elm_object_name_find(win, "actions list", -1);

	Elm_Object_Item *selected_item = elm_list_selected_item_get(list);

	if(!selected_item) return;

	Action *action = elm_object_item_data_get(selected_item);
	Device *device = evas_object_data_get(win, "device");

	device_action_del(device, action);
   	elm_object_item_del(selected_item);
   	device_save(device);
}/*_button_remove_clicked_cb*/



/*
 *Callback called in button "add" object when clicked signal is emitted.
 */
static void
_button_add_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Device *device;
	Evas_Object *entry, *hoversel;
	Condition ifcondition;
	char *ifvalue;
	Action_Type type;

	device = evas_object_data_get(win, "device");

	hoversel = elm_object_name_find(win, "ifcondition hoversel", -1);
	ifcondition = (Condition) evas_object_data_get(hoversel, "selected");

	entry = elm_object_name_find(win, "ifvalue entry", -1);
	asprintf(&ifvalue, "%s",  elm_object_text_get(entry));

	hoversel = elm_object_name_find(win, "type hoversel", -1);
	type = (Action_Type) evas_object_data_get(hoversel, "selected");

	char *arg = evas_object_data_get(win, "data arg");

	if(!arg || !ifvalue) return;

	Action *action;
	action = action_new(ifcondition, ifvalue , type, arg);
	device_action_add(device, action);
	device_save(device);

	FREE(arg);
	arg = evas_object_data_del(win, "data arg");

	_list_action_add(device, action);
}/*_button_add_clicked_cb*/


/*
 *Callback called in list "actions" when item clicked signal is emitted.
 */
static void
_list_item_selected_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info)
{
	Evas_Object *entry, *hoversel;
	Action *action = (Action *)elm_object_item_data_get(event_info);

	entry = elm_object_name_find(win, "ifvalue entry", -1);
	elm_object_text_set(entry, action_ifvalue_get(action));

	hoversel = elm_object_name_find(win, "ifcondition hoversel", -1);
	elm_object_text_set(hoversel, action_condition_to_str(action_ifcondition_get(action)));
	evas_object_data_set(hoversel, "selected", (void*)(unsigned int)action_ifcondition_get(action));


	hoversel = elm_object_name_find(win, "type hoversel", -1);
	elm_object_text_set(hoversel, action_type_to_str(action_type_get(action)));
	evas_object_data_set(hoversel, "selected", (void*)(unsigned int)action_type_get(action));

	entry = elm_object_name_find(win, "data entry", -1);
	elm_object_text_set(entry, action_data_get(action));
}/*_list_item_selected_cb*/


/*
 *
 */
static void
_list_action_add(Device *device, Action *action)
{
	char *s;
	Evas_Object *list = elm_object_name_find(win, "actions list", -1);

	asprintf(&s, _("If %s value %s %s then %s with arg=%s"),
										device_name_get(device),
										action_condition_to_str(action_ifcondition_get(action)),
										action_ifvalue_get(action),
										action_type_to_str(action_type_get(action)),
									action_data_get(action));

	elm_list_item_append(list, s, NULL, NULL, _list_item_selected_cb, action);
	elm_list_go(list);
}/*_list_action_add*/


/*
 *
 */
void
actions_editor_add(void *data, Evas_Object * obj __UNUSED__,	void *event_info __UNUSED__)
{
	char *s;
	Evas_Object *entry, *hoversel;
	Evas_Object *grid;
	Evas_Object *icon, *bx, *frame;
	Evas_Object *button;
	Evas_Object *list;

	Evas_Object *widgets_list = data;
	Elm_Object_Item *selected_item = elm_list_selected_item_get(widgets_list);

	if(!selected_item) return;

	Widget *widget = elm_object_item_data_get(selected_item);
	Device *device = widget_device_get(widget);

	asprintf(&s, _("Edit actions for '%s' xPL device"), device_name_get(device));
	win = elm_win_util_standard_add("actions_editor", s);
	FREE(s);
	evas_object_data_set(win, "device", device);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
   	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Actions list"));
	elm_grid_pack(grid, frame, 1, 1, 99, 60);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_scroller_policy_set(list, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_ON);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_name_set(list, "actions list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

	Eina_List *l, *actions;
	Action *action;
	actions = device_actions_list_get(device);
	EINA_LIST_FOREACH(actions, l, action)
		_list_action_add(device, action);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Action"));
	elm_grid_pack(grid, frame, 1, 61, 99, 20);
	evas_object_show(frame);

   	bx = elm_box_add(win);
	elm_object_content_set(frame, bx);
   	elm_box_horizontal_set(bx, EINA_TRUE);

   	hoversel = elm_hoversel_add(grid);
   	evas_object_name_set(hoversel, "ifcondition hoversel");
   	elm_object_text_set(hoversel, _("Condition"));
   	elm_box_pack_end(bx, hoversel);
	int x = 0;
	for(x = 0;x != CONDITION_LAST;x++)
	{
		if(x == UNKNOWN_CONDITION) continue;
	   	elm_hoversel_item_add(hoversel, action_condition_to_str(x), ELM_ICON_NONE, ELM_ICON_NONE, NULL, (void*)(unsigned int)x);
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

   	hoversel = elm_hoversel_add(grid);
   	evas_object_name_set(hoversel, "type hoversel");
   	elm_object_text_set(hoversel, _("Action"));
	elm_box_pack_end(bx, hoversel);
	for(x = 0;x != ACTION_TYPE_LAST;x++)
	{
		if(x == UNKNOWN_ACTION) continue;
		elm_hoversel_item_add(hoversel, action_type_to_str(x), ELM_ICON_NONE, ELM_ICON_NONE, NULL, (void*)(unsigned int)x);
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
	elm_box_pack_end(bx, button);
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
