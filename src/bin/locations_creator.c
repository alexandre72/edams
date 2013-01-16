/*
 * locations_editor.c
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

#include "cosm.h"
#include "edams.h"
#include "location.h"
#include "locations_creator.h"
#include "myfileselector.h"
#include "path.h"

/*Global window elm object*/
Evas_Object *win;

/*Callbacks*/
static void _button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _myfileselector_button_action_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_open_picture_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _map_name_loaded_cb(void *data, Evas_Object * obj __UNUSED__, void *ev __UNUSED__);
static void _button_goto_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *ev __UNUSED__);


/*
 *
 */
static void
_button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	char s[256];
	const char *f, *g;
	Evas_Object *win;
	Evas_Object *img;
	Evas_Object *eo;
	Ecore_Evas *ee;
	Evas *evas;
	Location *location;

	win = (Evas_Object *) data;

	location = location_new(0, NULL, NULL);

	location_name_set(location,
					  elm_object_text_get(elm_object_name_find(win, "location name entry", -1)));

	double lon;
	double lat;
	Evas_Object *map = elm_object_name_find(win, "location map", -1);;
	Elm_Map_Name *name = evas_object_data_get(map, "name Elm_Map_Name");
	elm_map_name_region_get(name, &lon, &lat);
	location_longitude_set(location, lon);
	location_latitude_set(location, lat);

	eo = NULL;
	ee = ecore_evas_new(NULL, 10, 10, 50, 50, NULL);
	evas = ecore_evas_get(ee);

	img = elm_object_name_find(win, "location image", -1);
	elm_image_file_get(img, &f, &g);

	// Don't try to update if isn't a new item image!
	if (f && (eina_str_has_extension(f, ".eet") == EINA_FALSE))
	{
		eo = evas_object_image_filled_add(evas);
		evas_object_image_file_set(eo, f, NULL);
		evas_object_image_alpha_set(eo, EINA_TRUE);
		evas_object_image_scale(eo, 50, 50);
		location_image_set(location, eo);
	}

	App_Info *app= 	evas_object_data_get(win, "app");
	cosm_location_feed_add(app, location);
	location_save(location);

	// Append location to locations list.
	Evas_Object *list = elm_object_name_find(app->win, "locations list", -1);
	Elm_Object_Item *it = elm_list_item_append(list, location_name_get(location), NULL, NULL, NULL, location);
	//elm_object_item_del_cb_set(it, _list_item_location_delete_cb);
	elm_list_go(list);
	elm_list_item_bring_in(it);

	// Append location to naviframe and set it contents.
	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	it = elm_naviframe_item_push(naviframe, location_name_get(location), NULL, NULL, _location_naviframe_content_set(location), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	elm_object_item_data_set(it, location);
	elm_naviframe_item_promote(it);

	if (eo)
	{
		evas_object_del(eo);
		elm_image_file_set(img, location_filename_get(location), "/image/0");
	}

	snprintf(s, sizeof(s), _("Location %s has been created."), location_name_get(location));
	statusbar_text_set(s, "dialog-information");
	evas_object_del(win);
}/*_button_apply_clicked_cb*/


/*
 *
 */
static void
_myfileselector_button_action_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	const char *sel;
	MyFileSelector *myfs = (MyFileSelector *) data;

	sel = elm_fileselector_selected_get(myfs->fs);

	if (sel)
	{
		if ((eina_str_has_extension(sel, ".png") == EINA_TRUE) ||
			(eina_str_has_extension(sel, "jpg") == EINA_TRUE) ||
			(eina_str_has_extension(sel, ".jpeg") == EINA_TRUE) ||
			(eina_str_has_extension(sel, ".gif") == EINA_TRUE) ||
			(eina_str_has_extension(sel, ".bmp") == EINA_TRUE))
		{
			Evas_Object *img;
			img = evas_object_data_get(myfs->win, "image");
			elm_image_file_set(img, sel, NULL);
		}
	}
	myfileselector_close(myfs);
}/*_myfileselector_button_action_clicked_cb*/



/*
 *
 */
static void
_button_open_picture_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *img = data;
	MyFileSelector *myfs;

	myfs = myfileselector_add();
	myfileselector_set_title(myfs, _("Select a picture file"));
	evas_object_data_set(myfs->win, "image", img);
	evas_object_smart_callback_add(myfs->action_bt, "clicked", _myfileselector_button_action_clicked_cb, myfs);
}/*_button_open_picture_clicked_cb*/



/*
 *
 */
static void
_map_name_loaded_cb(void *data, Evas_Object * obj __UNUSED__, void *ev __UNUSED__)
{
	double lon, lat;
	Evas_Object *win = (Evas_Object *) data;

	Evas_Object *map = elm_object_name_find(win, "location map", -1);;
	Elm_Map_Name *name = evas_object_data_get(map, "name Elm_Map_Name");

	elm_map_name_region_get(name, &lon, &lat);
	elm_map_region_bring_in(map, lon, lat);
}/*_map_name_loaded_cb*/


/*
 *
 */
static void
_button_goto_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *ev __UNUSED__)
{
	Evas_Object *map;
	char *address;
	Evas_Object *win = (Evas_Object *) data;
	Elm_Map_Name *name;

	map = elm_object_name_find(win, "location map", -1);;
	address = (char *)elm_object_text_get(elm_object_name_find(win, "location map entry", -1));

	name = elm_map_name_add(map, address, 0, 0, NULL, NULL);
	elm_map_zoom_set(map, 12);
	evas_object_data_set(map, "name Elm_Map_Name", name);

	evas_object_smart_callback_add(map, "name,loaded", _map_name_loaded_cb, win);
}/*_button_goto_clicked_cb*/



/*
 *
 */
void
locations_creator_add(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *gd, *frame, *bx, *bx2;
	Evas_Object *ic, *img;
	Evas_Object *bt;
	Evas_Object *entry;

	App_Info *app = (App_Info *) data;

	win = elm_win_util_standard_add("locations_description_dlg", _("Add a location"));
	evas_object_data_set(win, "app", app);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
   	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

	gd = elm_grid_add(win);
	elm_grid_size_set(gd, 100, 100);
	elm_win_resize_object_add(win, gd);
	evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(gd);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Photo"));
	elm_grid_pack(gd, frame, 1, 1, 26, 26);
	evas_object_show(frame);

	bx = elm_box_add(win);
	evas_object_show(bx);

	img = elm_image_add(win);
	evas_object_name_set(img, "location image");
	elm_image_smooth_set(img, EINA_TRUE);
	evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_image_aspect_fixed_set(img, EINA_TRUE);
	elm_image_resizable_set(img, EINA_TRUE, EINA_TRUE);
	elm_image_file_set(img, edams_edje_theme_file_get(), "default/nopicture");
	evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(img, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(bx, img);
	evas_object_show(img);

	bt = elm_button_add(win);
	elm_object_text_set(bt, _("Open..."));
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "document-open");
	elm_object_part_content_set(bt, "icon", ic);
	evas_object_smart_callback_add(bt, "clicked", _button_open_picture_clicked_cb, img);
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);

	elm_object_content_set(frame, bx);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Name:"));
	elm_grid_pack(gd, frame, 32, 1, 40, 12);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "location name entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Description:"));
	elm_grid_pack(gd, frame, 32, 15, 40, 12);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "location description entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Geolocalization:"));
	elm_grid_pack(gd, frame, 0, 30, 99, 50);
	evas_object_show(frame);

	bx = elm_box_add(win);
	evas_object_show(bx);

	Evas_Object *map = elm_map_add(win);
	evas_object_name_set(map, "location map");
	elm_map_zoom_set(map, 1);
	elm_win_resize_object_add(win, map);
	evas_object_size_hint_weight_set(map, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(map, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(bx, map);
	evas_object_show(map);

	bx2 = elm_box_add(win);
	elm_box_horizontal_set(bx2, EINA_TRUE);
	elm_box_pack_end(bx, bx2);
	evas_object_show(bx2);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "location map entry");
	elm_entry_scrollable_set(entry, EINA_FALSE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_object_text_set(entry, "Le Mans France");
	elm_box_pack_end(bx2, entry);
	evas_object_show(entry);

	bt = elm_button_add(win);
	elm_object_text_set(bt, _("Go to"));
	evas_object_show(bt);
	elm_box_pack_end(bx2, bt);
	evas_object_smart_callback_add(bt, "clicked", _button_goto_clicked_cb, win);

	elm_object_content_set(frame, bx);

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_box_homogeneous_set(bx, EINA_TRUE);
	elm_grid_pack(gd, bx, 1, 90, 99, 10);
	evas_object_show(bx);

	bt = elm_button_add(win);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "apply-window");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Ok"));
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(bt, "clicked", _button_apply_clicked_cb, win);

	bt = elm_button_add(win);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "window-close");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Close"));
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, win);

	evas_object_resize(win, 500, 500);
	evas_object_show(win);
}
