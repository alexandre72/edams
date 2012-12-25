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


static void _apply_bt_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);




static void
_apply_bt_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win;

  	win = (Evas_Object *)data;
	App_Info* app = (App_Info*)evas_object_data_get(win, "app");

	eina_stringshare_replace(&(app->settings->cosm_apikey), elm_object_text_get(elm_object_name_find(win, "cosm api key entry", -1)));
	eina_stringshare_replace(&(app->settings->map_background), elm_object_text_get(elm_object_name_find(win, "map background entry", -1)));

	app->settings->softemu = elm_check_state_get(elm_object_name_find(win, "emulation checkb", -1));
	app->settings->debug = elm_check_state_get(elm_object_name_find(win, "debug checkb", -1));

    edams_settings_write(app->settings);
	app->settings = edams_settings_get();

	evas_object_del(win);
}


//
//
//
void
preferences_dlg_new(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win, *gd;
	Evas_Object *label, *ic, *bx, *frame;
	Evas_Object *bt, *ck;
	Evas_Object *entry;

	App_Info *app = (App_Info*)data;

	win = elm_win_util_standard_add("settings", _("Settings"));
	evas_object_data_set(win, "app", app);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
	evas_object_show(win);

	gd = elm_grid_add(win);
	elm_grid_size_set(gd, 100, 100);
	elm_win_resize_object_add(win, gd);
	evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(gd);

    label = elm_label_add(win);
	elm_object_text_set(label, _("Cosm API Key:"));
	elm_grid_pack(gd, label, 0, 1, 100, 8);
	evas_object_show(label);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "cosm api key entry");
	elm_object_text_set(entry, app->settings->cosm_apikey);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_grid_pack(gd, entry, 51, 2, 100, 9);
	evas_object_show(entry);

    bx = elm_box_add(win);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_box_horizontal_set(bx, EINA_TRUE);
    evas_object_show(bx);

    ck = elm_check_add(win);
	evas_object_name_set(ck, "emulation checkb");
    elm_object_text_set(ck, _("Emulation"));
  	elm_grid_pack(gd, ck , 0, 10, 100, 10);
  	elm_check_state_set(ck, app->settings->softemu);
    evas_object_show(ck);

    ck = elm_check_add(win);
	evas_object_name_set(ck, "debug checkb");
    elm_object_text_set(ck, _("Debug with printf"));
  	elm_grid_pack(gd, ck , 0, 20, 100, 10);
  	elm_check_state_set(ck, app->settings->debug);
    evas_object_show(ck);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "map background entry");
	elm_object_text_set(entry, app->settings->map_background);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_grid_pack(gd, entry, 0, 30, 100, 9);
	evas_object_show(entry);


	bt = elm_button_add(win);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "apply-window");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Ok"));
	elm_grid_pack(gd, bt, 20, 85, 20, 12);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", _apply_bt_clicked_cb, win);

	bt = elm_button_add(win);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "window-close");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Close"));
    elm_grid_pack(gd, bt, 60, 85, 20, 12);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, win);

	evas_object_resize(win, 400, 250);
}
