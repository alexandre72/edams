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
#include "exec_editor.h"
#include "myfileselector.h"


static void _myfileselector_button_ok_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_open_file_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_cancel_clicked_cb(void *data , Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);

/*
 *
 */
static void
_button_cancel_clicked_cb(void *data , Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    ExecEditor *execeditor = data;
    execeditor_close(execeditor);
}/*_cancel_button_clicked_cb*/

/*
 *
 */
void
execeditor_close(ExecEditor *execeditor)
{
    evas_object_del(execeditor->win);
    FREE(execeditor);
}/*execeditor_close*/

/*
 *
 */
static void
_myfileselector_button_ok_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	MyFileSelector *myfs = data;
	const char *selected;

	selected = elm_fileselector_selected_get(myfs->fs);

	if (selected)
	{
	    Evas_Object *entry = evas_object_data_get(myfs->win, "exec entry");
		elm_object_text_set(entry, selected);
	}
	myfileselector_close(myfs);
}/*_myfileselector_button_action_clicked_cb*/


/*
 *
 */
static void
_button_open_file_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	MyFileSelector *myfs;
    Evas_Object *entry = data;

    myfs = myfileselector_add();
	elm_win_title_set(myfs->win,  _("Select a program"));
	evas_object_data_set(myfs->win, "exec entry", entry);
	evas_object_smart_callback_add(myfs->ok_button, "clicked", _myfileselector_button_ok_clicked_cb, myfs);
}/*_button_open_file_clicked_cb*/


/*
 *
 */
ExecEditor *
execeditor_add()
{
    ExecEditor *execeditor = calloc(1, sizeof(ExecEditor));

	execeditor->win = elm_win_util_standard_add("mail_editor", NULL);
	elm_win_title_set(execeditor->win, _("Edit mail"));
	elm_win_autodel_set(execeditor->win, EINA_TRUE);
	elm_win_center(execeditor->win, EINA_TRUE, EINA_TRUE);

	execeditor->grid = elm_grid_add(execeditor->win);
	elm_grid_size_set(execeditor->grid, 100, 100);
	elm_win_resize_object_add(execeditor->win, execeditor->grid);
	evas_object_size_hint_weight_set(execeditor->grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(execeditor->grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(execeditor->grid);

	execeditor->frame = elm_frame_add(execeditor->win);
	elm_object_text_set(execeditor->frame, _("Exec:"));
	elm_grid_pack(execeditor->grid, execeditor->frame, 1, 1, 99, 15);
	evas_object_show(execeditor->frame);

	execeditor->hbox = elm_box_add(execeditor->win);
    elm_box_horizontal_set(execeditor->hbox, EINA_TRUE);
	evas_object_show(execeditor->hbox);

	execeditor->exec_entry = elm_entry_add(execeditor->win);
	elm_entry_scrollable_set(execeditor->exec_entry, EINA_TRUE);
	elm_entry_editable_set(execeditor->exec_entry, EINA_TRUE);
	elm_entry_single_line_set(execeditor->exec_entry, EINA_TRUE);
	evas_object_size_hint_weight_set(execeditor->exec_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(execeditor->exec_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(execeditor->hbox, execeditor->exec_entry);
	evas_object_show(execeditor->exec_entry);

	Evas_Object *button = elm_button_add(execeditor->win);
	elm_object_text_set(button, _("Open..."));
	execeditor->icon = elm_icon_add(execeditor->win);
	elm_icon_order_lookup_set(execeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(execeditor->icon, "document-open");
	elm_object_part_content_set(button, "icon", execeditor->icon);
	evas_object_smart_callback_add(button, "clicked", _button_open_file_clicked_cb, execeditor->exec_entry);
	elm_box_pack_end(execeditor->hbox, button);
	evas_object_show(button);

	elm_object_content_set(execeditor->frame, execeditor->hbox);

	execeditor->terminal_check = elm_check_add(execeditor->win);
	evas_object_name_set(execeditor->terminal_check , "terminal check");
	elm_object_text_set(execeditor->terminal_check , _("Exec in terminal"));
	elm_grid_pack(execeditor->grid, execeditor->terminal_check , 0, 20, 30, 10);
	evas_object_show(execeditor->terminal_check );

	execeditor->hbox = elm_box_add(execeditor->win);
	elm_box_horizontal_set(execeditor->hbox, EINA_TRUE);
	elm_box_homogeneous_set(execeditor->hbox, EINA_TRUE);
	elm_grid_pack(execeditor->grid, execeditor->hbox, 1, 90, 99, 10);
	evas_object_show(execeditor->hbox);

	execeditor->ok_button = elm_button_add(execeditor->win);
	execeditor->icon = elm_icon_add(execeditor->win);
	elm_icon_order_lookup_set(execeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(execeditor->icon, "apply-window");
	elm_object_part_content_set(execeditor->ok_button, "icon", execeditor->icon);
	elm_object_text_set(execeditor->ok_button, _("Ok"));
	elm_box_pack_end(execeditor->hbox, execeditor->ok_button);
	evas_object_show(execeditor->ok_button);
	evas_object_size_hint_align_set(execeditor->ok_button, EVAS_HINT_FILL, 0);

	execeditor->cancel_button = elm_button_add(execeditor->win);
	execeditor->icon = elm_icon_add(execeditor->win);
	elm_icon_order_lookup_set(execeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(execeditor->icon, "window-close");
	elm_object_part_content_set(execeditor->cancel_button, "icon", execeditor->icon);
	elm_object_text_set(execeditor->cancel_button, _("Close"));
	elm_box_pack_end(execeditor->hbox, execeditor->cancel_button);
	evas_object_show(execeditor->cancel_button);
	evas_object_size_hint_align_set(execeditor->cancel_button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(execeditor->cancel_button, "clicked", _button_cancel_clicked_cb , execeditor);

	evas_object_resize(execeditor->win, 400, 400);
	evas_object_show(execeditor->win);

	return execeditor;
}/*execeditor_add*/
