/*
 * osd_editor.c
 * This file is part of EINA_TRUE
 *
 * Copyright (C) 2013 - Alexandre Dussart
 *
 * EINA_TRUE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EINA_TRUE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EINA_TRUE. If not, see <http://www.gnu.org/licenses/>.
 */



#include <Elementary.h>
#include "cJSON.h"
#include "edams.h"

/*Global window elm object*/
static Evas_Object *win = NULL;


/*
 *
 */
const char*
osd_editor_values_get()
{
    const char *ret, *command, *text;
    double delay;

	Evas_Object *entry = elm_object_name_find(win, "command entry", -1);
    command = elm_object_text_get(entry);

	entry = elm_object_name_find(win, "text entry", -1);
    text = elm_object_text_get(entry);

	Evas_Object *slider = elm_object_name_find(win, "delay slider", -1);
    delay = round(elm_slider_value_get(slider));

	ret = action_osd_data_format(command, text, delay);

	return ret;
}/*osd_editor_hbox_get*/


/*
 *
 */
Evas_Object *
osd_editor_hbox_get()
{
	Evas_Object *hbox = elm_object_name_find(win, "hbox", -1);
	return hbox;
}/*osd_editor_hbox_get*/


/*
 *
 */
Evas_Object *
osd_editor_add()
{
	Evas_Object *grid, *hbox, *frame;
	Evas_Object *entry, *slider;

	win = elm_win_util_standard_add("osd_basic_editor", NULL);
	elm_win_title_set(win, _("Edit xPL CMND osd.basic"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Command:"));
	elm_grid_pack(grid, frame, 1, 1, 99, 15);
	evas_object_show(frame);

    entry = elm_entry_add(win);
	evas_object_name_set(entry, "command entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_text_set(entry, "write");
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Text:"));
	elm_grid_pack(grid, frame, 1, 16, 99, 15);
	evas_object_show(frame);

    entry = elm_entry_add(win);
	evas_object_name_set(entry, "text entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Delay:"));
	elm_grid_pack(grid, frame, 1, 32, 99, 15);
	evas_object_show(frame);

    slider = elm_slider_add(win);
   	evas_object_name_set(slider, "delay slider");
    elm_slider_unit_format_set(slider, _("%1.0f seconds"));
    elm_slider_min_max_set(slider, 0, 20);
    evas_object_show(slider);
	elm_object_content_set(frame, slider);

	hbox = elm_box_add(win);
	evas_object_name_set(hbox, "hbox");
	elm_box_horizontal_set(hbox, EINA_TRUE);
	elm_box_homogeneous_set(hbox, EINA_TRUE);
	elm_grid_pack(grid, hbox, 1, 89, 99, 10);
	evas_object_show(hbox);

	evas_object_resize(win, 400, 400);
	evas_object_show(win);

	return win;
}/*debug_editor_add*/
