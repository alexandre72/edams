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

/*Global objects*/
static Evas_Object *win = NULL;


/*Evas_Object Callbacks*/
static void _button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_add_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_remove_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _hoversel_selected_cb(void *data, Evas_Object *obj, void *event_info);
/*Others funcs*/
static void _list_crons_add(Evas_Object *list, Cron_Entry *cron_elem);


/*
 *
 */
static void
_button_edit_arg_apply_clicked_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
	Action_Type type = (Action_Type)data;
	Evas_Object *cwin;
    const char *s = NULL;

    cwin = elm_object_top_widget_get(obj);

    switch(type)
	{
	    case ACTION_TYPE_CMND:
                s = cmnd_editor_values_get();
                break;

		case ACTION_TYPE_MAIL:
                s = mail_editor_values_get();
                break;

   		case ACTION_TYPE_EXEC:
                s = exec_editor_values_get();
                break;

   		case ACTION_TYPE_DEBUG:
                s = debug_editor_values_get();
                break;

		case ACTION_TYPE_UNKNOWN:
		case ACTION_TYPE_LAST:
            	evas_object_del(cwin);
                return;
    }

    if(s)
    {
        evas_object_data_set(win, "action data", s);
    }

	evas_object_del(cwin);
}/*_button_edit_arg_apply_clicked_cb*/



/*
 *Callback called in any hoversel objects when clicked signal is emitted.
 */
static void
_hoversel_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	App_Info *app = evas_object_data_get(win, "app");
	Evas_Object *cwin = NULL;
	Evas_Object *hbox = NULL;
	Evas_Object *button, *icon;
    Action_Type type = (Action_Type)elm_object_item_data_get(event_info);

	elm_object_text_set(obj, elm_object_item_text_get(event_info));
    evas_object_data_set(win, "action type", (void*)type);

	switch(type)
	{
		case ACTION_TYPE_CMND:
	            cwin = cmnd_editor_add(app);
                hbox = cmnd_editor_hbox_get();
                break;

		case ACTION_TYPE_MAIL:
	            cwin = mail_editor_add();
                hbox = mail_editor_hbox_get();
                break;

		case ACTION_TYPE_EXEC:
	            cwin = exec_editor_add();
                hbox = exec_editor_hbox_get();
                break;

		case ACTION_TYPE_DEBUG:
	            cwin = debug_editor_add();
                hbox = debug_editor_hbox_get();
			    break;

		case ACTION_TYPE_UNKNOWN:
		case ACTION_TYPE_LAST:
				break;
	}

    button = elm_button_add(cwin);
	icon = elm_icon_add(cwin);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "apply-window");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Ok"));
	elm_box_pack_end(hbox, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_edit_arg_apply_clicked_cb, (void *)type);
}/*_hoversel_selected_cb*/


/*
 *Callback called in button "remove" object when clicked signal is emitted.
 */
static void
_button_remove_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *list = elm_object_name_find(win, "crons list", -1);

	Elm_Object_Item *selected_item = elm_list_selected_item_get(list);

	if(!selected_item) return;

    Cron_Entry *cron_elem = elm_object_item_data_get(selected_item);

    crons_list_entry_remove(cron_elem);

    elm_object_item_del(selected_item);

}/*_button_remove_clicked_cb*/



/*
 *Callback called in button "add" object when clicked signal is emitted.
 */
static void
_button_add_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *list = elm_object_name_find(win, "crons list", -1);
    Cron_Entry *cron_elem = NULL;
	char *arg = evas_object_data_get(win, "action data");
	Action_Type type = (Action_Type) evas_object_data_get(win, "action type");

	if(!arg) return;

    cron_elem = cron_entry_new(MON, ANY, JAN, 12, 30,  type, arg);
    crons_list_entry_add(cron_elem);

	_list_crons_add(list, cron_elem);
}/*_button_add_clicked_cb*/




/*
 *Add cron_elem to list object
 */
static void
_list_crons_add(Evas_Object *list, Cron_Entry *cron_elem)
{
    Action_Type type = cron_elem->action_type;

    if(type == ACTION_TYPE_UNKNOWN) return;

    char *s;
    asprintf(&s, "On %s %d %s at %d:%d",
                    day_to_str(cron_elem->day), cron_elem->mday, month_to_str(cron_elem->month),
                    cron_elem->hour, cron_elem->minute);

	Evas_Object *icon;
	icon = elm_icon_add(win);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
    evas_object_size_hint_min_set(icon, 48, 48);
    evas_object_size_hint_align_set(icon, 0.5, EVAS_HINT_FILL);

    if(type == ACTION_TYPE_CMND)
       	elm_image_file_set(icon, edams_edje_theme_file_get(), "elm/icon/xpl/default");
    else if(type == ACTION_TYPE_MAIL)
        elm_icon_standard_set(icon, "mail-send");
    else if(type == ACTION_TYPE_EXEC)
        elm_icon_standard_set(icon, "system-run");
    else if(type == ACTION_TYPE_DEBUG)
        elm_icon_standard_set(icon, "debug");
    else
       	elm_image_file_set(icon, edams_edje_theme_file_get(), "");

	elm_list_item_append(list, s, icon, NULL, NULL, cron_elem);
    FREE(s);

	elm_list_go(list);
}/*_list_action_add*/


/*
 *
 */
void
scheduler_editor_new(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *grid;
	Evas_Object *icon, *bx, *frame, *separator;
	Evas_Object *list, *button, *check, *entry, *hoversel;
	unsigned int x = 0;

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
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_name_set(list, "crons list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

    Eina_List *crons, *l;
    Cron_Entry *cron_elem;

    crons = crons_list_get();
    EINA_LIST_FOREACH(crons, l, cron_elem)
        _list_crons_add(list, cron_elem);

   	hoversel = elm_hoversel_add(grid);
   	evas_object_name_set(hoversel, "type hoversel");
   	elm_object_text_set(hoversel, _("Action"));
	elm_grid_pack(grid, hoversel, 52, 68, 40, 8);
	for(x = 0;x != ACTION_TYPE_LAST;x++)
	{
		if(x == ACTION_TYPE_UNKNOWN) continue;
		if((x == ACTION_TYPE_DEBUG) && (!edams_settings_debug_get())) continue;
		elm_hoversel_item_add(hoversel, action_type_to_desc(x), ELM_ICON_NONE, ELM_ICON_NONE, NULL, (void*)(unsigned int)x);
	}
	evas_object_show(hoversel);
	evas_object_smart_callback_add(hoversel, "selected", _hoversel_selected_cb, NULL);

	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_grid_pack(grid, separator, 1, 89, 99, 1);
	evas_object_show(separator);

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_box_homogeneous_set(bx, EINA_TRUE);
	elm_grid_pack(grid, bx, 1, 90, 99, 10);
	evas_object_show(bx);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "list-add");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Add"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_add_clicked_cb, NULL);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "list-remove");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Remove"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_remove_clicked_cb, NULL);


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

	/*
	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_grid_pack(grid, bx, 1, 89, 99, 1);
	evas_object_show(separator);
*/

	evas_object_resize(win, 600, 450);
	evas_object_show(win);
}/*scheduler_editor_new*/
