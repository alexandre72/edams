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
#include "osd_editor.h"

/*
 *
 */
static void
_button_cancel_clicked_cb(void *data , Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    OsdEditor *osdeditor = data;
    osdeditor_close(osdeditor);
}/*_cancel_button_clicked_cb*/

/*
 *
 */
void
osdeditor_close(OsdEditor *osdeditor)
{
    evas_object_del(osdeditor->win);
    FREE(osdeditor);
}/*osdeditor_close*/

/*
 *
 */
OsdEditor *
osdeditor_add()
{
    OsdEditor *osdeditor = calloc(1, sizeof(OsdEditor));

	osdeditor->win = elm_win_util_standard_add("osd_editor", NULL);
	elm_win_title_set(osdeditor->win, _("Edit xPL osd.basic CMND"));
	elm_win_autodel_set(osdeditor->win, EINA_TRUE);
	elm_win_center(osdeditor->win, EINA_TRUE, EINA_TRUE);

	osdeditor->grid = elm_grid_add(osdeditor->win);
	elm_grid_size_set(osdeditor->grid, 100, 100);
	elm_win_resize_object_add(osdeditor->win, osdeditor->grid);
	evas_object_size_hint_weight_set(osdeditor->grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(osdeditor->grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(osdeditor->grid);

	osdeditor->frame = elm_frame_add(osdeditor->win);
	elm_object_text_set(osdeditor->frame, _("Command:"));
	elm_grid_pack(osdeditor->grid, osdeditor->frame, 1, 1, 99, 15);
	evas_object_show(osdeditor->frame);

    osdeditor->command_entry = elm_entry_add(osdeditor->win);
	elm_entry_scrollable_set(osdeditor->command_entry, EINA_TRUE);
	elm_entry_editable_set(osdeditor->command_entry, EINA_TRUE);
	elm_entry_single_line_set(osdeditor->command_entry, EINA_TRUE);
	evas_object_show(osdeditor->command_entry);
	elm_object_text_set(osdeditor->command_entry, "write");
	elm_object_content_set(osdeditor->frame, osdeditor->command_entry);

	osdeditor->frame = elm_frame_add(osdeditor->win);
	elm_object_text_set(osdeditor->frame, _("Text:"));
	elm_grid_pack(osdeditor->grid, osdeditor->frame, 1, 16, 99, 15);
	evas_object_show(osdeditor->frame);

    osdeditor->text_entry = elm_entry_add(osdeditor->win);
	elm_entry_scrollable_set(osdeditor->text_entry, EINA_TRUE);
	elm_entry_editable_set(osdeditor->text_entry, EINA_TRUE);
	elm_entry_single_line_set(osdeditor->text_entry, EINA_TRUE);
	evas_object_show(osdeditor->text_entry);
	elm_object_content_set(osdeditor->frame, osdeditor->text_entry);

	osdeditor->frame = elm_frame_add(osdeditor->win);
	elm_object_text_set(osdeditor->frame, _("Delay:"));
	elm_grid_pack(osdeditor->grid, osdeditor->frame, 1, 32, 99, 15);
	evas_object_show(osdeditor->frame);

    osdeditor->delay_slider = elm_slider_add(osdeditor->win);
    elm_slider_unit_format_set(osdeditor->delay_slider, _("%1.0f seconds"));
    elm_slider_min_max_set(osdeditor->delay_slider, 0, 20);
    evas_object_show(osdeditor->delay_slider);
	elm_object_content_set(osdeditor->frame, osdeditor->delay_slider);

	osdeditor->hbox = elm_box_add(osdeditor->win);
	elm_box_horizontal_set(osdeditor->hbox, EINA_TRUE);
	elm_box_homogeneous_set(osdeditor->hbox, EINA_TRUE);
	elm_grid_pack(osdeditor->grid, osdeditor->hbox, 1, 90, 99, 10);
	evas_object_show(osdeditor->hbox);

	osdeditor->ok_button = elm_button_add(osdeditor->win);
	osdeditor->icon = elm_icon_add(osdeditor->win);
	elm_icon_order_lookup_set(osdeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(osdeditor->icon, "apply-window");
	elm_object_part_content_set(osdeditor->ok_button, "icon", osdeditor->icon);
	elm_object_text_set(osdeditor->ok_button, _("Ok"));
	elm_box_pack_end(osdeditor->hbox, osdeditor->ok_button);
	evas_object_show(osdeditor->ok_button);
	evas_object_size_hint_align_set(osdeditor->ok_button, EVAS_HINT_FILL, 0);

	osdeditor->cancel_button = elm_button_add(osdeditor->win);
	osdeditor->icon = elm_icon_add(osdeditor->win);
	elm_icon_order_lookup_set(osdeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(osdeditor->icon, "window-close");
	elm_object_part_content_set(osdeditor->cancel_button, "icon", osdeditor->icon);
	elm_object_text_set(osdeditor->cancel_button, _("Close"));
	elm_box_pack_end(osdeditor->hbox, osdeditor->cancel_button);
	evas_object_show(osdeditor->cancel_button);
	evas_object_size_hint_align_set(osdeditor->cancel_button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(osdeditor->cancel_button, "clicked", _button_cancel_clicked_cb , osdeditor);

	evas_object_resize(osdeditor->win, 400, 400);
	evas_object_show(osdeditor->win);

	return osdeditor;
}/*osdeditor_editor_add*/
