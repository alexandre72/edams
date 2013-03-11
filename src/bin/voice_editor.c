/*
 * voice_editor.c
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
#include "voice_editor.h"
#include "edams.h"
#include "voicerss.h"


char *sound_file = NULL;

/*
 *
 */
static void
_button_cancel_clicked_cb(void *data , Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    VoiceEditor *voiceeditor = data;
    voiceeditor_close(voiceeditor);
}/*_cancel_button_clicked_cb*/


/*
 *
 */
void
voiceeditor_close(VoiceEditor *voiceeditor)
{
    evas_object_del(voiceeditor->win);
    FREE(voiceeditor);
}/*myfileselector_close*/


/*
 *
 */
static void
_button_make_clicked_cb(void *data , Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    VoiceEditor *voiceeditor = data;
    if(!elm_entry_is_empty(voiceeditor->message_entry))
    {
        elm_object_disabled_set(voiceeditor->ok_button, EINA_TRUE);
        elm_progressbar_pulse(voiceeditor->progressbar, EINA_TRUE);
        voicerss_play(elm_entry_markup_to_utf8(elm_object_text_get(voiceeditor->message_entry)), voiceeditor);
    }
}/*_button_test_clicked_cb*/



/*
 *
 */
VoiceEditor *
voiceeditor_add()
{
    VoiceEditor *voiceeditor = calloc(1, sizeof(VoiceEditor));

	voiceeditor->win = elm_win_util_standard_add("voice_editor", NULL);
	elm_win_title_set(voiceeditor->win, _("Edit voice message"));
	elm_win_autodel_set(voiceeditor->win, EINA_TRUE);
	elm_win_center(voiceeditor->win, EINA_TRUE, EINA_TRUE);

	voiceeditor->grid = elm_grid_add(voiceeditor->win);
	elm_grid_size_set(voiceeditor->grid, 100, 100);
	elm_win_resize_object_add(voiceeditor->win, voiceeditor->grid);
	evas_object_size_hint_weight_set(voiceeditor->grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(voiceeditor->grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(voiceeditor->grid);

	voiceeditor->frame = elm_frame_add(voiceeditor->win);
	elm_object_text_set(voiceeditor->frame, _("Message:"));
	elm_grid_pack(voiceeditor->grid, voiceeditor->frame, 1, 1, 99, 59);
	evas_object_show(voiceeditor->frame);

	voiceeditor->hbox = elm_box_add(voiceeditor->win);
    elm_box_horizontal_set(voiceeditor->hbox, EINA_TRUE);
	evas_object_show(voiceeditor->hbox);

	voiceeditor->message_entry = elm_entry_add(voiceeditor->win);
	elm_entry_scrollable_set(voiceeditor->message_entry, EINA_TRUE);
	elm_entry_editable_set(voiceeditor->message_entry, EINA_TRUE);
	elm_entry_single_line_set(voiceeditor->message_entry, EINA_FALSE);
	evas_object_size_hint_weight_set(voiceeditor->message_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(voiceeditor->message_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(voiceeditor->message_entry);
	elm_box_pack_end(voiceeditor->hbox, voiceeditor->message_entry);

	Evas_Object *button = elm_button_add(voiceeditor->win);
	elm_object_text_set(button, _("Make and play"));
	voiceeditor->icon = elm_icon_add(voiceeditor->win);
	elm_icon_order_lookup_set(voiceeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(voiceeditor->icon, "media-playback-start");
	elm_object_part_content_set(button, "icon", voiceeditor->icon);
	evas_object_smart_callback_add(button, "clicked", _button_make_clicked_cb, voiceeditor);
	evas_object_show(button);
	elm_box_pack_end(voiceeditor->hbox, button);

	elm_object_content_set(voiceeditor->frame, voiceeditor->hbox);

    voiceeditor->progressbar = elm_progressbar_add(voiceeditor->win);
    elm_progressbar_unit_format_set(voiceeditor->progressbar, _("Making voice"));
    evas_object_size_hint_align_set(voiceeditor->progressbar, EVAS_HINT_FILL, 0.5);
    evas_object_size_hint_weight_set(voiceeditor->progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_progressbar_pulse_set(voiceeditor->progressbar, EINA_TRUE);
	elm_grid_pack(voiceeditor->grid, voiceeditor->progressbar, 1, 60, 99, 19);
    evas_object_show(voiceeditor->progressbar);

	voiceeditor->hbox = elm_box_add(voiceeditor->win);
	elm_box_horizontal_set(voiceeditor->hbox, EINA_TRUE);
	elm_box_homogeneous_set(voiceeditor->hbox, EINA_TRUE);
	elm_grid_pack(voiceeditor->grid, voiceeditor->hbox, 1, 80, 99, 10);
	evas_object_show(voiceeditor->hbox);

	voiceeditor->ok_button = elm_button_add(voiceeditor->win);
	voiceeditor->icon = elm_icon_add(voiceeditor->win);
	elm_icon_order_lookup_set(voiceeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(voiceeditor->icon, "apply-window");
	elm_object_part_content_set(voiceeditor->ok_button, "icon", voiceeditor->icon);
	elm_object_text_set(voiceeditor->ok_button, _("Ok"));
	elm_box_pack_end(voiceeditor->hbox, voiceeditor->ok_button);
	evas_object_show(voiceeditor->ok_button);
	evas_object_size_hint_align_set(voiceeditor->ok_button, EVAS_HINT_FILL, 0);

	voiceeditor->cancel_button = elm_button_add(voiceeditor->win);
	voiceeditor->icon = elm_icon_add(voiceeditor->win);
	elm_icon_order_lookup_set(voiceeditor->icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(voiceeditor->icon, "window-close");
	elm_object_part_content_set(voiceeditor->cancel_button, "icon", voiceeditor->icon);
	elm_object_text_set(voiceeditor->cancel_button, _("Close"));
	elm_box_pack_end(voiceeditor->hbox, voiceeditor->cancel_button);
	evas_object_show(voiceeditor->cancel_button);
	evas_object_size_hint_align_set(voiceeditor->cancel_button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(voiceeditor->cancel_button, "clicked", _button_cancel_clicked_cb , voiceeditor);

	evas_object_resize(voiceeditor->win, 400, 180);
	evas_object_show(voiceeditor->win);

	return voiceeditor;
}/*voice_editor_add*/
