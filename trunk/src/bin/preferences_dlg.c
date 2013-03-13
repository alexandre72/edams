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

#include <Elementary.h>
#include <Efreet.h>

#include "edams.h"
#include "path.h"
#include "settings.h"
#include "utils.h"
#include "myfileselector.h"

/*Macros*/
#define WEIGHT evas_object_size_hint_weight_set
#define ALIGN_ evas_object_size_hint_align_set
#define EXPAND(X) WEIGHT((X), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND)
#define FILL(X) ALIGN_((X), EVAS_HINT_FILL, EVAS_HINT_FILL)

/*Global objects*/
Evas_Object *win = NULL;

/*Callbacks*/
static void _button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _myfileselector_button_ok_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_open_file_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_gv_reset_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);

/*Others funcs*/
static Evas_Object* _globalview_settings_content();
static Evas_Object* _services_settings_content();
static Evas_Object* _advanced_settings_content();



/*
 *
 */
static void
_button_apply_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{

	Evas_Object *entry;
	Evas_Object *check;

	entry = elm_object_name_find(win, "cosm api key entry", -1);
	edams_settings_cosm_apikey_set(elm_object_text_get(entry));

	entry = elm_object_name_find(win, "voicerss api key entry", -1);
	edams_settings_voicerss_apikey_set(elm_object_text_get(entry));

	entry = elm_object_name_find(win, "global_view background entry", -1);
	edams_settings_global_view_background_set(elm_object_text_get(entry));

	entry = elm_object_name_find(win, "mbox path entry", -1);
	edams_settings_mbox_path_set(elm_object_text_get(entry));

/*
	entry = elm_object_name_find(win, "user name entry", -1);
	edams_settings_user_name_set(elm_object_text_get(entry));

	entry = elm_object_name_find(win, "user mail entry", -1);
	edams_settings_user_mail_set( elm_object_text_get(entry));
*/

	check = elm_object_name_find(win, "emulation check", -1);
	edams_settings_softemu_set(elm_check_state_get(check));

	check = elm_object_name_find(win, "debug check", -1);
	edams_settings_debug_set(elm_check_state_get(check));

	evas_object_del(win);
}/*_button_apply_clicked_cb*/


/*
 *
 */
static Evas_Object*
_general_settings_content()
{
	Evas_Object *grid, *frame;
    Evas_Object *entry;

    grid = elm_grid_add(win);
	EXPAND(grid);
    elm_grid_size_set(grid, 100, 100);
    evas_object_show(grid);

    frame = elm_frame_add(win);
    elm_object_text_set(frame, _("User name:"));
    elm_grid_pack(grid, frame, 1, 1, 99, 15);
    evas_object_show(frame);

    entry = elm_entry_add(win);
    evas_object_name_set(entry, "user name entry");
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_editable_set(entry, EINA_TRUE);
    elm_entry_single_line_set(entry, EINA_TRUE);
    evas_object_show(entry);
    elm_object_content_set(frame, entry);
    elm_object_text_set(entry, edams_settings_user_name_get());

    frame = elm_frame_add(win);
    elm_object_text_set(frame, _("User mail:"));
    elm_grid_pack(grid, frame, 1, 16, 99, 15);
    evas_object_show(frame);

    entry = elm_entry_add(win);
    evas_object_name_set(entry, "user mail entry");
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_editable_set(entry, EINA_TRUE);
    elm_entry_single_line_set(entry, EINA_TRUE);
    evas_object_show(entry);
    elm_object_content_set(frame, entry);
    elm_object_text_set(entry, edams_settings_user_mail_get());

    return grid;
}/*_advanced_settings_content*/


/*
 *
 */
static void
_myfileselector_button_ok_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	MyFileSelector *myfs = data;
	const char *selected;

	selected = elm_fileselector_selected_get(myfs->fs);

	if (selected)
	{
		if ((eina_str_has_extension(selected, ".png") == EINA_TRUE) ||
			(eina_str_has_extension(selected, "jpg") == EINA_TRUE) ||
			(eina_str_has_extension(selected, ".jpeg") == EINA_TRUE) ||
			(eina_str_has_extension(selected, ".gif") == EINA_TRUE) ||
			(eina_str_has_extension(selected, ".bmp") == EINA_TRUE))
		{
			Evas_Object *entry;
	        entry = elm_object_name_find(win, "global_view background entry", -1);
			elm_object_text_set(entry, selected);
		}
	}
	myfileselector_close(myfs);
}/*_myfileselector_button_action_clicked_cb*/


/*
 *
 */
static void
_button_open_file_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	MyFileSelector *myfs;

	myfs = myfileselector_add();

	if(edams_settings_global_view_background_get())
        elm_fileselector_selected_set(myfs->fs, edams_settings_global_view_background_get());
	else
        elm_fileselector_path_set(myfs->fs, xdg_pictures_dir_get());

	elm_win_title_set(myfs->win,  _("Select a picture file"));
	evas_object_smart_callback_add(myfs->ok_button, "clicked", _myfileselector_button_ok_clicked_cb, myfs);
}/*_button_open_file_clicked_cb*/


/*
 *
 */
static void
_button_gv_reset_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    Eet_File *ef = NULL;
    char **list;
    int num, i;

    if(!(ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_READ_WRITE)))
    {
        debug(MSG_ERROR, _("Can't open Eet file '%s' in rw mode"));
        return;
    }

    list = eet_list(ef, "*", &num);
    if (list)
    {
        for (i = 0; i < num; i++)
        {
            if(strncmp(list[i], "global_view/", strlen("global_view/")) == 0)
                eet_delete(ef, list[i]);
        }
        free(list);
    }
    eet_close(ef);
}/*_button_gv_reset_clicked_cb*/


/*
 *
 */
static Evas_Object*
_globalview_settings_content()
{
	Evas_Object *grid, *bx, *frame;
	Evas_Object *icon, *entry, *button;

    grid = elm_grid_add(win);
	EXPAND(grid);
    elm_grid_size_set(grid, 100, 100);
    evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Global view background image:"));
	elm_grid_pack(grid, frame, 1, 1, 99, 17);
	evas_object_show(frame);

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	evas_object_show(bx);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "global_view background entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_box_pack_end(bx, entry);
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(entry, edams_settings_global_view_background_get());

	button = elm_button_add(win);
	elm_object_text_set(button, _("Open..."));
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "document-open");
	elm_object_part_content_set(button, "icon", icon);
	evas_object_smart_callback_add(button, "clicked", _button_open_file_clicked_cb, entry);
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	elm_object_content_set(frame, bx);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "view-refresh");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Reset widgets geometry"));
	evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(button);
	elm_grid_pack(grid, button, 1, 18, 50, 8);
	evas_object_smart_callback_add(button, "clicked", _button_gv_reset_clicked_cb, win);

    frame = elm_frame_add(win);
    elm_object_text_set(frame, _("Mbox file path:"));
    elm_grid_pack(grid, frame, 1, 27, 99, 15);
    evas_object_show(frame);

    entry = elm_entry_add(win);
    evas_object_name_set(entry, "mbox path entry");
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_editable_set(entry, EINA_TRUE);
    elm_entry_single_line_set(entry, EINA_TRUE);
    evas_object_show(entry);
    elm_object_content_set(frame, entry);
    elm_object_text_set(entry, edams_settings_mbox_path_get());

    return grid;
}/*_globalview_settings_content*/


/*
 *
 */
static Evas_Object*
_services_settings_content()
{
	Evas_Object *grid, *frame;
	Evas_Object *entry;

    grid = elm_grid_add(win);
	EXPAND(grid);
    elm_grid_size_set(grid, 100, 100);
    evas_object_show(grid);

    frame = elm_frame_add(win);
    elm_object_text_set(frame, _("Cosm API key:"));
    elm_grid_pack(grid, frame, 1, 1, 99, 15);
    evas_object_show(frame);

    entry = elm_entry_add(win);
    evas_object_name_set(entry, "cosm api key entry");
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_editable_set(entry, EINA_TRUE);
    elm_entry_single_line_set(entry, EINA_TRUE);
    evas_object_show(entry);
    elm_object_content_set(frame, entry);
    elm_object_text_set(entry, edams_settings_cosm_apikey_get());

    frame = elm_frame_add(win);
    elm_object_text_set(frame, _("Voicerss API key:"));
    elm_grid_pack(grid, frame, 1, 16, 99, 15);
    evas_object_show(frame);

    entry = elm_entry_add(win);
    evas_object_name_set(entry, "voicerss api key entry");
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_editable_set(entry, EINA_TRUE);
    elm_entry_single_line_set(entry, EINA_TRUE);
    evas_object_show(entry);
    elm_object_content_set(frame, entry);
    elm_object_text_set(entry, edams_settings_voicerss_apikey_get());

    return grid;
}/*_services_settings_content*/

/*
 *
 */
static Evas_Object*
_advanced_settings_content()
{
	Evas_Object *grid, *check;

    grid = elm_grid_add(win);
	EXPAND(grid);
    elm_grid_size_set(grid, 100, 100);
    evas_object_show(grid);

	check = elm_check_add(win);
	evas_object_name_set(check, "emulation check");
	elm_object_text_set(check, _("xPL software emulation"));
	elm_grid_pack(grid, check, 1, 1, 99, 5);
	elm_check_state_set(check, edams_settings_softemu_get());
	evas_object_show(check);

	check = elm_check_add(win);
	evas_object_name_set(check, "debug check");
	elm_object_text_set(check, _("Debug with printf"));
	elm_grid_pack(grid, check, 1, 10, 99, 5);
	elm_check_state_set(check, edams_settings_debug_get());
	evas_object_show(check);

    return grid;
}/*_advanced_settings_content*/


/*
 *
 */
static void
_options_list_selected_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
    Eina_List *its, *l;
    Elm_Object_Item *it;

    int id = elm_object_item_data_get(event_info);

    Evas_Object *naviframe = elm_object_name_find(win, "naviframe", -1);

    its = elm_naviframe_items_get(naviframe);
    EINA_LIST_FOREACH(its, l, it)
    {
        int i = elm_object_item_data_get(it);

	    if(i == id)
	    {
	        elm_naviframe_item_promote(it);
            break;
        }
   }
}


/*
 *
 */
void
preferences_dlg_new(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
   	Evas_Object *button, *hbox;
   	Evas_Object *icon, *separator;
	Evas_Object *vbox, *naviframe, *list;
	Elm_Object_Item *it;

	win = elm_win_util_standard_add("settings", _("Settings"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
   	evas_object_show(win);

 	Evas_Object *bg = elm_bg_add(win);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg );
  	evas_object_size_hint_min_set(bg, 750, 450);

    vbox = elm_box_add(win);
    EXPAND(vbox);
	evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, vbox);
	evas_object_show(vbox);

    Evas_Object *panes = elm_panes_add(win);
    elm_win_resize_object_add(win, panes);
    evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(vbox, panes);
    evas_object_show(panes);

	list = elm_list_add(win);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list, -1.0, -1.0);
	evas_object_smart_callback_add(list, "selected", _options_list_selected_cb, NULL);
	evas_object_show(list);
	elm_object_part_content_set(panes, "left", list);
    elm_panes_content_left_size_set(panes, 0.25);

	naviframe = elm_naviframe_add(win);
   	evas_object_name_set(naviframe, "naviframe");
	EXPAND(naviframe);
	evas_object_size_hint_align_set(naviframe, -1.0, -1.0);
	elm_object_text_set(naviframe, _("Options"));
	evas_object_show(naviframe);

	elm_list_item_append(list, _("General"), NULL, NULL, NULL, 1);
	it = elm_naviframe_item_push(naviframe, _("General"), NULL, NULL, _general_settings_content(), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	elm_object_item_data_set(it, 1);

	elm_list_item_append(list, _("Services"), NULL, NULL, NULL, 2);
	it = elm_naviframe_item_push(naviframe, _("Services"), NULL, NULL, _services_settings_content(), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
    elm_object_item_data_set(it, 2);

	elm_list_item_append(list, _("Global view"), NULL, NULL, NULL, 3);
	it = elm_naviframe_item_push(naviframe, _("Global view"), NULL, NULL, _globalview_settings_content(), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	elm_object_item_data_set(it, 3);

	elm_list_item_append(list, _("Advanced"), NULL, NULL, NULL, 4);
	it = elm_naviframe_item_push(naviframe, _("Advanced"), NULL, NULL, _advanced_settings_content(), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
    elm_object_item_data_set(it, 4);

    it = elm_naviframe_item_push(naviframe, NULL, NULL, NULL, NULL, 0);
	 elm_object_item_data_set(it, 0);
    elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	elm_list_go(list);
	elm_object_part_content_set(panes, "right", naviframe);

    separator = elm_separator_add(win);
    elm_separator_horizontal_set(separator, EINA_TRUE);
    elm_box_pack_end(vbox, separator);
    evas_object_show(separator);

	hbox = elm_box_add(win);
	elm_box_horizontal_set(hbox, EINA_TRUE);
	elm_box_homogeneous_set(hbox, EINA_TRUE);
    elm_box_pack_end(vbox, hbox);
	evas_object_show(hbox);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "apply-window");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Ok"));
	elm_box_pack_end(hbox, button);
	evas_object_show(button);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
    evas_object_smart_callback_add(button, "clicked", _button_apply_clicked_cb, win);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "window-close");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Close"));
	elm_box_pack_end(hbox, button);
	evas_object_show(button);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
    evas_object_smart_callback_add(button, "clicked", window_clicked_close_cb, win);

    separator = elm_separator_add(win);
    elm_separator_horizontal_set(separator, EINA_TRUE);
    elm_box_pack_end(vbox, separator);
    evas_object_show(separator);
}
