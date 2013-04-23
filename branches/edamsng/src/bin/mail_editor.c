/*
 * mail_editor.c
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
#include "mail_editor.h"


/*
 *
 */
static void
_button_cancel_clicked_cb(void *data , Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    MailEditor *mailedit = data;
    maileditor_close(mailedit);
}/*_cancel_button_clicked_cb*/


/*
 *
 */
void
maileditor_close(MailEditor *mailedit)
{
    evas_object_del(mailedit->win);
    FREE(mailedit);
}/*myfileselector_close*/

/*
 *
 */
MailEditor *
maileditor_add()
{
    MailEditor *mailedit = calloc(1, sizeof(MailEditor));

	mailedit->win = elm_win_util_standard_add("mail_editor", NULL);
	elm_win_title_set(mailedit->win, _("Edit mail"));
	elm_win_autodel_set(mailedit->win, EINA_TRUE);
	elm_win_center(mailedit->win, EINA_TRUE, EINA_TRUE);

	mailedit->grid = elm_grid_add(mailedit->win);
	elm_grid_size_set(mailedit->grid, 100, 100);
	elm_win_resize_object_add(mailedit->win, mailedit->grid);
	evas_object_size_hint_weight_set(mailedit->grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(mailedit->grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(mailedit->grid);

	mailedit->frame = elm_frame_add(mailedit->win);
	elm_object_text_set(mailedit->frame, _("From:"));
	elm_grid_pack(mailedit->grid, mailedit->frame, 1, 1, 99, 15);
	evas_object_show(mailedit->frame);

	mailedit->from_entry = elm_entry_add(mailedit->win);
	elm_entry_scrollable_set(mailedit->from_entry, EINA_TRUE);
	elm_entry_editable_set(mailedit->from_entry, EINA_TRUE);
	elm_entry_single_line_set(mailedit->from_entry, EINA_TRUE);
	elm_object_text_set(mailedit->from_entry, edams_settings_user_mail_get());
	evas_object_show(mailedit->from_entry);
	elm_object_content_set(mailedit->frame, mailedit->from_entry);

	mailedit->frame = elm_frame_add(mailedit->win);
	elm_object_text_set(mailedit->frame, _("To:"));
	elm_grid_pack(mailedit->grid, mailedit->frame, 1, 17, 99, 15);
	evas_object_show(mailedit->frame);

	mailedit->to_entry = elm_entry_add(mailedit->win);
	elm_entry_scrollable_set(mailedit->to_entry, EINA_TRUE);
	elm_entry_editable_set(mailedit->to_entry, EINA_TRUE);
	elm_entry_single_line_set(mailedit->to_entry, EINA_TRUE);
	evas_object_show(mailedit->to_entry);
	elm_object_content_set(mailedit->frame, mailedit->to_entry);

	mailedit->frame = elm_frame_add(mailedit->win);
	elm_object_text_set(mailedit->frame, _("Subject:"));
	elm_grid_pack(mailedit->grid, mailedit->frame, 1, 32, 99, 15);
	evas_object_show(mailedit->frame);

	mailedit->subject_entry = elm_entry_add(mailedit->win);
	elm_entry_scrollable_set(mailedit->subject_entry, EINA_TRUE);
	elm_entry_editable_set(mailedit->subject_entry, EINA_TRUE);
	elm_entry_single_line_set(mailedit->subject_entry, EINA_TRUE);
	evas_object_show(mailedit->subject_entry);
	elm_object_content_set(mailedit->frame, mailedit->subject_entry);

	mailedit->frame = elm_frame_add(mailedit->win);
	elm_object_text_set(mailedit->frame, _("Body:"));
	elm_grid_pack(mailedit->grid, mailedit->frame, 1, 47, 99, 40);
	evas_object_show(mailedit->frame);

	mailedit->body_entry = elm_entry_add(mailedit->win);
	elm_entry_scrollable_set(mailedit->body_entry, EINA_TRUE);
	elm_entry_editable_set(mailedit->body_entry, EINA_TRUE);
	elm_entry_single_line_set(mailedit->body_entry, EINA_FALSE);
	evas_object_show(mailedit->body_entry);
	elm_object_content_set(mailedit->frame, mailedit->body_entry);

	mailedit->hbox = elm_box_add(mailedit->win);
	elm_box_horizontal_set(mailedit->hbox, EINA_TRUE);
	elm_box_homogeneous_set(mailedit->hbox, EINA_TRUE);
	elm_grid_pack(mailedit->grid, mailedit->hbox, 1, 90, 99, 10);
	evas_object_show(mailedit->hbox);

	mailedit->ok_button = elm_button_add(mailedit->win);
	mailedit->icon = elm_icon_add(mailedit->win);
	elm_icon_order_lookup_set(mailedit->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(mailedit->icon, "apply-window");
	elm_object_part_content_set(mailedit->ok_button, "icon", mailedit->icon);
	elm_object_text_set(mailedit->ok_button, _("Ok"));
	elm_box_pack_end(mailedit->hbox, mailedit->ok_button);
	evas_object_show(mailedit->ok_button);
	evas_object_size_hint_align_set(mailedit->ok_button, EVAS_HINT_FILL, 0);

	mailedit->cancel_button = elm_button_add(mailedit->win);
	mailedit->icon = elm_icon_add(mailedit->win);
	elm_icon_order_lookup_set(mailedit->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(mailedit->icon, "window-close");
	elm_object_part_content_set(mailedit->cancel_button, "icon", mailedit->icon);
	elm_object_text_set(mailedit->cancel_button, _("Close"));
	elm_box_pack_end(mailedit->hbox, mailedit->cancel_button);
	evas_object_show(mailedit->cancel_button);
	evas_object_size_hint_align_set(mailedit->cancel_button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(mailedit->cancel_button, "clicked", _button_cancel_clicked_cb , mailedit);

	evas_object_resize(mailedit->win, 400, 400);
	evas_object_show(mailedit->win);

	return mailedit;
}/*mail_editor_add*/
