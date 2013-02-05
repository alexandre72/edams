/*
 * preferences_dlg.c
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

#include <Elementary.h>

#include "edams.h"
#include "path.h"
#include "settings.h"
#include "utils.h"
#include "myfileselector.h"

/*Callbacks*/
static void _button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_open_file_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _myfileselector_button_action_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);

/*
 *
 */
static void
_button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win;
	Evas_Object *entry;
	Evas_Object *check;

	win = (Evas_Object *) data;

	entry = elm_object_name_find(win, "cosm api key entry", -1);
	edams_settings_cosm_apikey_set( elm_object_text_get(entry));

	entry =elm_object_name_find(win, "global_view background entry", -1);
	edams_settings_global_view_background_set(elm_object_text_get(entry));

	entry =elm_object_name_find(win, "gnuplot path entry", -1);
	edams_settings_gnuplot_path_set(elm_object_text_get(entry));

	entry =elm_object_name_find(win, "smtp server entry", -1);
	edams_settings_smtp_server_set(elm_object_text_get(entry));

	entry =elm_object_name_find(win, "smtp username entry", -1);
	edams_settings_smtp_username_set(elm_object_text_get(entry));

	entry =elm_object_name_find(win, "smtp userpwd entry", -1);
	edams_settings_smtp_userpwd_set( elm_object_text_get(entry));

	check = elm_object_name_find(win, "emulation check", -1);
	edams_settings_softemu_set(elm_check_state_get(check));

	check = elm_object_name_find(win, "debug check", -1);
	edams_settings_debug_set(elm_check_state_get(check));

	evas_object_del(win);
}/*_button_apply_clicked_cb*/

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
		if ((eina_str_has_extension(sel, ".png") == EINA_TRUE) ||
			(eina_str_has_extension(sel, "jpg") == EINA_TRUE) ||
			(eina_str_has_extension(sel, ".jpeg") == EINA_TRUE) ||
			(eina_str_has_extension(sel, ".gif") == EINA_TRUE) ||
			(eina_str_has_extension(sel, ".bmp") == EINA_TRUE))
		{
			Evas_Object *entry;
			entry = evas_object_data_get(myfs->win, "entry");
			elm_object_text_set(entry, sel);
		}
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
	myfileselector_set_title(myfs, _("Select a picture file"));
	evas_object_data_set(myfs->win, "entry", entry);
	evas_object_smart_callback_add(myfs->action_bt, "clicked", _myfileselector_button_action_clicked_cb, myfs);
}/*_button_open_file_clicked_cb*/


/*
 *
 */
void
preferences_dlg_new(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win, *grid;
	Evas_Object *icon, *bx, *frame, *separator;
	Evas_Object *button, *check, *entry;

	win = elm_win_util_standard_add("settings", _("Settings"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Cosm API key:"));
	elm_grid_pack(grid, frame, 1, 1, 99, 13);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "cosm api key entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);
	elm_object_text_set(entry, edams_settings_cosm_apikey_get());

	bx = elm_box_add(win);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_box_horizontal_set(bx, EINA_TRUE);
	evas_object_show(bx);

	check = elm_check_add(win);
	evas_object_name_set(check, "emulation check");
	elm_object_text_set(check, _("Emulation"));
	elm_grid_pack(grid, check, 0, 15, 45, 5);
	elm_check_state_set(check, edams_settings_softemu_get());
	evas_object_show(check);

	check = elm_check_add(win);
	evas_object_name_set(check, "debug check");
	elm_object_text_set(check, _("Debug with printf"));
	elm_grid_pack(grid, check, 30, 15, 45, 5);
	elm_check_state_set(check, edams_settings_debug_get());
	evas_object_show(check);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Global view background image:"));
	elm_grid_pack(grid, frame, 1, 21, 99, 13);
	evas_object_show(frame);

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	evas_object_show(bx);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "global_view background entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_box_pack_end(bx, entry);
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(entry, edams_settings_global_view_background_get());

	button = elm_button_add(win);
	elm_object_text_set(button, _("Open..."));
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "document-open");
	elm_object_part_content_set(button, "icon", icon);
	evas_object_smart_callback_add(button, "clicked", _button_open_file_clicked_cb, entry);
	elm_box_pack_end(bx, button);
	evas_object_show(button);

	elm_object_content_set(frame, bx);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Gnuplot path:"));
	elm_grid_pack(grid, frame, 1, 35, 99, 13);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "gnuplot path entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(entry, edams_settings_gnuplot_path_get());
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("SMTP server adress:"));
	elm_grid_pack(grid, frame, 1, 49, 99, 13);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "smtp server entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);
	elm_object_text_set(entry, edams_settings_smtp_server_get());

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("SMTP username:"));
	elm_grid_pack(grid, frame, 1, 63, 99, 13);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "smtp username entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);
	elm_object_text_set(entry, edams_settings_smtp_username_get());

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("SMTP password:"));
	elm_grid_pack(grid, frame, 1, 77, 99, 13);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "smtp userpwd entry");
	elm_entry_password_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);
	elm_object_text_set(entry, edams_settings_smtp_userpwd_get());

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_box_homogeneous_set(bx, EINA_TRUE);
	elm_grid_pack(grid, bx, 1, 90, 99, 10);
	evas_object_show(bx);

	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_box_pack_end(bx, separator);
	evas_object_show(separator);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "apply-window");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Ok"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_apply_clicked_cb, win);

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

	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_box_pack_end(bx, separator);
	evas_object_show(separator);

	evas_object_resize(win, 400, 450);
	evas_object_show(win);
}
