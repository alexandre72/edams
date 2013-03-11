/*
 * debug_editor.c
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
#include "debug_editor.h"
#include "edams.h"

/*
 *
 */
static void
_button_cancel_clicked_cb(void *data , Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    DebugEditor *debugeditor = data;
    debugeditor_close(debugeditor);
}/*_cancel_button_clicked_cb*/


/*
 *
 */
void
debugeditor_close(DebugEditor *debugeditor)
{
    evas_object_del(debugeditor->win);
    FREE(debugeditor);
}/*myfileselector_close*/



/*
 *
 */
DebugEditor *
debugeditor_add()
{
    DebugEditor *debugeditor = calloc(1, sizeof(DebugEditor));

	debugeditor->win = elm_win_util_standard_add("voice_editor", NULL);
	elm_win_title_set(debugeditor->win, _("Edit debug message"));
	elm_win_autodel_set(debugeditor->win, EINA_TRUE);
	elm_win_center(debugeditor->win, EINA_TRUE, EINA_TRUE);

	debugeditor->grid = elm_grid_add(debugeditor->win);
	elm_grid_size_set(debugeditor->grid, 100, 100);
	elm_win_resize_object_add(debugeditor->win, debugeditor->grid);
	evas_object_size_hint_weight_set(debugeditor->grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(debugeditor->grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(debugeditor->grid);

	debugeditor->frame = elm_frame_add(debugeditor->win);
	elm_object_text_set(debugeditor->frame, _("Message:"));
	elm_grid_pack(debugeditor->grid, debugeditor->frame, 1, 1, 99, 79);
	evas_object_show(debugeditor->frame);

	debugeditor->message_entry = elm_entry_add(debugeditor->win);
	elm_entry_scrollable_set(debugeditor->message_entry, EINA_TRUE);
	elm_entry_editable_set(debugeditor->message_entry, EINA_TRUE);
	elm_entry_single_line_set(debugeditor->message_entry, EINA_TRUE);
	evas_object_show(debugeditor->message_entry);
	elm_object_content_set(debugeditor->frame, debugeditor->message_entry);

	debugeditor->hbox = elm_box_add(debugeditor->win);
	elm_box_horizontal_set(debugeditor->hbox, EINA_TRUE);
	elm_box_homogeneous_set(debugeditor->hbox, EINA_TRUE);
	elm_grid_pack(debugeditor->grid, debugeditor->hbox, 1, 80, 99, 10);
	evas_object_show(debugeditor->hbox);

	debugeditor->ok_button = elm_button_add(debugeditor->win);
	debugeditor->icon = elm_icon_add(debugeditor->win);
	elm_icon_order_lookup_set(debugeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(debugeditor->icon, "apply-window");
	elm_object_part_content_set(debugeditor->ok_button, "icon", debugeditor->icon);
	elm_object_text_set(debugeditor->ok_button, _("Ok"));
	elm_box_pack_end(debugeditor->hbox, debugeditor->ok_button);
	evas_object_show(debugeditor->ok_button);
	evas_object_size_hint_align_set(debugeditor->ok_button, EVAS_HINT_FILL, 0);

	debugeditor->cancel_button = elm_button_add(debugeditor->win);
	debugeditor->icon = elm_icon_add(debugeditor->win);
	elm_icon_order_lookup_set(debugeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(debugeditor->icon, "window-close");
	elm_object_part_content_set(debugeditor->cancel_button, "icon", debugeditor->icon);
	elm_object_text_set(debugeditor->cancel_button, _("Close"));
	elm_box_pack_end(debugeditor->hbox, debugeditor->cancel_button);
	evas_object_show(debugeditor->cancel_button);
	evas_object_size_hint_align_set(debugeditor->cancel_button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(debugeditor->cancel_button, "clicked", _button_cancel_clicked_cb , debugeditor);

	evas_object_resize(debugeditor->win, 400, 180);
	evas_object_show(debugeditor->win);

	return debugeditor;
}/*debug_editor_add*/
