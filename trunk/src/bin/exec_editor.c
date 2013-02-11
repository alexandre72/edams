/*
 * exec_editor.c
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
#include "myfileselector.h"

/*Global objects*/
static Evas_Object *win = NULL;


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
const char*
exec_editor_values_get()
{
	cJSON *root;
    const char *s;

	root=cJSON_CreateObject();

	Evas_Object *entry = elm_object_name_find(win, "exec entry", -1);
	Evas_Object *check = elm_object_name_find(win, "terminal check", -1);

	cJSON_AddItemToObject(root, "EXEC", cJSON_CreateString(elm_object_text_get(entry)));
	cJSON_AddItemToObject(root, "TERMINAL", cJSON_CreateString(elm_check_state_get(check) ? "true" : "false"));
    s = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return s;
}/*exec_editor_hbox_get*/


/*
 *
 */
Evas_Object *
exec_editor_hbox_get()
{
	Evas_Object *hbox = elm_object_name_find(win, "hbox", -1);
	return hbox;
}/*exec_editor_hbox_get*/


/*
 *
 */
Evas_Object *
exec_editor_add()
{
	Evas_Object *grid, *hbox, *frame;
	Evas_Object *entry, *check, *icon, *button;

	win = elm_win_util_standard_add("exec_editor", NULL);
	elm_win_title_set(win, _("Edit program exec"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Path of program:"));
	elm_grid_pack(grid, frame, 1, 1, 99, 15);
	evas_object_show(frame);

	hbox = elm_box_add(win);
    elm_box_horizontal_set(hbox, EINA_TRUE);
	evas_object_show(hbox);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "exec entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(hbox, entry);
	evas_object_show(entry);

	button = elm_button_add(win);
	elm_object_text_set(button, _("Open..."));
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "document-open");
	elm_object_part_content_set(button, "icon", icon);
	evas_object_smart_callback_add(button, "clicked", _button_open_file_clicked_cb, entry);
	elm_box_pack_end(hbox, button);
	evas_object_show(button);

	elm_object_content_set(frame, hbox);

	check = elm_check_add(win);
	evas_object_name_set(check, "terminal check");
	elm_object_text_set(check, _("Exec in terminal"));
	elm_grid_pack(grid, check, 0, 20, 30, 10);
	evas_object_name_set(grid, "grid");
	elm_check_state_set(check, EINA_FALSE);
	evas_object_show(check);

	hbox = elm_box_add(win);
	evas_object_name_set(hbox, "hbox");
	elm_box_horizontal_set(hbox, EINA_TRUE);
	elm_box_homogeneous_set(hbox, EINA_TRUE);
	elm_grid_pack(grid, hbox, 1, 89, 99, 10);
	evas_object_show(hbox);

	evas_object_resize(win, 400, 400);
	evas_object_show(win);

	return win;
}/*exec_editor_add*/
