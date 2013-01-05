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


#include "edams.h"
#include "path.h"
#include "settings.h"
#include "utils.h"
#include "myfileselector.h"

static void _apply_bt_clicked_cb(void *data, Evas_Object * obj __UNUSED__,
								 void *event_info __UNUSED__);




static void
_apply_bt_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win;

	win = (Evas_Object *) data;
	App_Info *app = (App_Info *) evas_object_data_get(win, "app");

	eina_stringshare_replace(&(app->settings->cosm_apikey),
							 elm_object_text_get(elm_object_name_find
												 (win, "cosm api key entry", -1)));
	eina_stringshare_replace(&(app->settings->map_background),
							 elm_object_text_get(elm_object_name_find
												 (win, "map background entry", -1)));

	eina_stringshare_replace(&(app->settings->gnuplot_path),
							 elm_object_text_get(elm_object_name_find
												 (win, "gnuplot path entry", -1)));

	app->settings->softemu = elm_check_state_get(elm_object_name_find(win, "emulation checkb", -1));
	app->settings->debug = elm_check_state_get(elm_object_name_find(win, "debug checkb", -1));

	edams_settings_write(app->settings);
	app->settings = edams_settings_get();

	evas_object_del(win);
}


//
//
//
static void
_action_bt_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
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
}



//
//
//
static void
_open_file_bt_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__,
						 void *event_info __UNUSED__)
{
	Evas_Object *entry = data;
	MyFileSelector *myfs;

	myfs = myfileselector_add();
	myfileselector_set_title(myfs, _("Select a picture file"));
	evas_object_data_set(myfs->win, "entry", entry);
	evas_object_smart_callback_add(myfs->action_bt, "clicked", _action_bt_clicked_cb, myfs);
}



//
//
//
void
preferences_dlg_new(void *data __UNUSED__, Evas_Object * obj __UNUSED__,
					void *event_info __UNUSED__)
{
	Evas_Object *win, *gd;
	Evas_Object *ic, *bx, *frame;
	Evas_Object *bt, *ck;
	Evas_Object *entry;

	App_Info *app = (App_Info *) data;

	win = elm_win_util_standard_add("settings", _("Settings"));
	evas_object_data_set(win, "app", app);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	gd = elm_grid_add(win);
	elm_grid_size_set(gd, 100, 100);
	elm_win_resize_object_add(win, gd);
	evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(gd);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Cosm API key:"));
	elm_grid_pack(gd, frame, 1, 1, 99, 15);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "cosm api key entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);
	elm_object_text_set(entry, app->settings->cosm_apikey);

	bx = elm_box_add(win);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_box_horizontal_set(bx, EINA_TRUE);
	evas_object_show(bx);

	ck = elm_check_add(win);
	evas_object_name_set(ck, "emulation checkb");
	elm_object_text_set(ck, _("Emulation"));
	elm_grid_pack(gd, ck, 0, 20, 30, 10);
	elm_check_state_set(ck, app->settings->softemu);
	evas_object_show(ck);

	ck = elm_check_add(win);
	evas_object_name_set(ck, "debug checkb");
	elm_object_text_set(ck, _("Debug with printf"));
	elm_grid_pack(gd, ck, 30, 20, 50, 10);
	elm_check_state_set(ck, app->settings->debug);
	evas_object_show(ck);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Map background image:"));
	elm_grid_pack(gd, frame, 1, 30, 99, 15);
	evas_object_show(frame);

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	evas_object_show(bx);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "map background entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_box_pack_end(bx, entry);
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(entry, app->settings->map_background);

	bt = elm_button_add(win);
	elm_object_text_set(bt, _("Open..."));
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "document-open");
	elm_object_part_content_set(bt, "icon", ic);
	evas_object_smart_callback_add(bt, "clicked", _open_file_bt_clicked_cb, entry);
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);

	elm_object_content_set(frame, bx);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Gnuplot path:"));
	elm_grid_pack(gd, frame, 1, 49, 99, 15);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "gnuplot path entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(entry, app->settings->gnuplot_path);
	elm_object_content_set(frame, entry);

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_box_homogeneous_set(bx, EINA_TRUE);
	elm_grid_pack(gd, bx, 1, 90, 99, 10);
	evas_object_show(bx);

	bt = elm_button_add(win);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "apply-window");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Ok"));
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(bt, "clicked", _apply_bt_clicked_cb, win);

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

	evas_object_resize(win, 400, 400);
	evas_object_show(win);
}
