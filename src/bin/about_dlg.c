/*
 * about_dlg.c
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
#include "utils.h"



//
//
//
void about_dialog_new(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    Evas_Object *win, *bg;
    Evas_Object *bx, *ic, *bt;
	Evas_Object *layout;

   	win = elm_win_util_standard_add("aboutbox", _("About E.D.A.M.S..."));
   	elm_win_autodel_set(win, EINA_TRUE);

    bg = elm_bg_add(win);
    elm_bg_file_set(bg, edams_edje_theme_file_get(), "bg/edams-bg");
    elm_bg_option_set(bg, ELM_BG_OPTION_TILE);
    elm_win_resize_object_add(win, bg);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(bg);

	bx = elm_box_add(win);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_win_resize_object_add(win, bx);
	evas_object_show(bx);

    layout = elm_layout_add(win);
	elm_layout_file_set(layout, edams_edje_theme_file_get(), "about");
   	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(bx, layout);
   	evas_object_show(layout);

    bt = elm_button_add(win);
	ic = elm_icon_add(win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   	elm_icon_standard_set(ic, "window-close");
   	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Close"));
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, win);
	evas_object_show(bt);

    evas_object_resize(win, 500, 300);
    evas_object_show(win);
}
