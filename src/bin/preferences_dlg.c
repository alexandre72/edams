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

	if((elm_radio_value_get(elm_object_name_find(win, "emulation radio group", -1))) == 0)
	{
		app->settings->softemu = EINA_FALSE;
		app->settings->hardemu = EINA_FALSE;
	}
	else if((elm_radio_value_get(elm_object_name_find(win, "emulation radio group", -1))) == 1)
	{
		app->settings->softemu = EINA_TRUE;
		app->settings->hardemu = EINA_FALSE;
	}
	else if((elm_radio_value_get(elm_object_name_find(win, "emulation radio group", -1))) == 2)
	{
		 app->settings->hardemu = EINA_TRUE;
		 app->settings->softemu = EINA_FALSE;

	}

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
	Evas_Object *win, *gd, *fr;
	Evas_Object *label, *ic, *bx, *frame;
	Evas_Object *bt, *radio, *group;
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

	//fr = elm_frame_add(win);
	//elm_grid_pack(gd, fr, 1, 1, 30, 40);
	//evas_object_show(fr);

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
	elm_grid_pack(gd, entry, 51, 2, 40, 9);
	evas_object_show(entry);

    bx = elm_box_add(win);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_box_horizontal_set(bx, EINA_TRUE);
    evas_object_show(bx);

    group = radio = elm_radio_add(win);
	evas_object_name_set(radio, "emulation radio group");
    elm_object_text_set(radio, _("Normal"));
    elm_radio_state_value_set(radio, 0);
    elm_radio_group_add(radio, group);
    elm_box_pack_end(bx, radio);
    evas_object_show(radio);

    radio = elm_radio_add(win);
    elm_object_text_set(radio, _("Serial software"));
    elm_radio_state_value_set(radio, 1);
    elm_radio_group_add(radio, group);
    elm_box_pack_end(bx, radio);
    evas_object_show(radio);

    radio = elm_radio_add(win);
    elm_object_text_set(radio, _("Hardware loopback"));
    elm_radio_state_value_set(radio, 2);
    elm_radio_group_add(radio, group);
    elm_box_pack_end(bx, radio);
    evas_object_show(radio);
    evas_object_show(group);

    frame = elm_frame_add(win);
    elm_object_content_set(frame, bx);
    elm_object_text_set(frame, _("Serial Emulation"));
  	elm_grid_pack(gd, frame , 0, 40, 100, 40);
    evas_object_show(frame);

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
