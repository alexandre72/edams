/*
 * events_editor.c
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

#include "crontab.h"
#include "edams.h"
#include "path.h"
#include "settings.h"
#include "utils.h"
#include "myfileselector.h"

/*Evas_Object Callbacks*/
static void _button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);


/*
 *
 */
static void
_button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win;

	win = (Evas_Object *) data;

	evas_object_del(win);
}/*_button_apply_clicked_cb*/

/*
 *
 */
void
scheduler_editor_new(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win, *grid;
	Evas_Object *icon, *bx, *frame, *separator;
	Evas_Object *list, *button, *check, *entry;

	win = elm_win_util_standard_add("scheduler_editor", _("Scheduler editor"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Events"));
	elm_grid_pack(grid, frame, 1, 1, 99, 49);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS );
	evas_object_show(list);
	elm_object_content_set(frame, list);

    Eina_List *crons, *l;
    Cron_Entry *cron_elem;

    crons = crons_list_get();
    EINA_LIST_FOREACH(crons, l, cron_elem)
    {
        const char *s;

        asprintf(&s, "On %s %d %s at %d:%d",
                    day_to_str(cron_elem->day), cron_elem->mday, month_to_str(cron_elem->month),
                    cron_elem->hour, cron_elem->minute);

        const char *buf = strdup(cron_elem->cmnd);

        printf("%s\n", buf);
        FREE(buf);

        elm_list_item_append(list, s, NULL, NULL, NULL, cron_elem);
        FREE(s);
    }
    elm_list_go(list);


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

	evas_object_resize(win, 600, 450);
	evas_object_show(win);
}/*scheduler_editor_new*/
