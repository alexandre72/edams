/*
 * devices_creator.c
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


#include <Evas.h>


#include "edams.h"
#include "myfileselector.h"
#include "utils.h"
#include "path.h"
#include "device.h"


static void _apply_bt_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _action_bt_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _photo_bt_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);


/*
 * Callback 'clicked' event apply button
 */
static void
_apply_bt_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	const char *f, *g;
	Evas_Object *win;
	Evas_Object *img;
	Evas_Object *eo;
	Ecore_Evas *ee;
	Evas *evas;
	Device *device;

	win = (Evas_Object *) data;

	device = device_new(-1, NULL);
	device_name_set(device,
					elm_object_text_get(elm_object_name_find(win, "device name entry", -1)));
	device_description_set(device,
						   elm_object_text_get(elm_object_name_find
											   (win, "device description entry", -1)));
	device_datasheeturl_set(device,
							elm_object_text_get(elm_object_name_find
												(win, "device datasheeturl entry", -1)));

	Elm_Object_Item *it;
	if((it = elm_list_selected_item_get(elm_object_name_find(win, "device type list", -1))))
		device_type_set(device,  device_str_to_type((char *)elm_object_item_data_get(it)));

	eo = NULL;
	ee = ecore_evas_new(NULL, 10, 10, 50, 50, NULL);
	evas = ecore_evas_get(ee);

	img = elm_object_name_find(win, "device image", -1);
	elm_image_file_get(img, &f, &g);

	// Don't try to update if isn't a new item image!
	if (f && (eina_str_has_extension(f, ".eet") == EINA_FALSE))
	{
		eo = evas_object_image_filled_add(evas);
		evas_object_image_file_set(eo, f, NULL);
		evas_object_image_alpha_set(eo, EINA_TRUE);
		evas_object_image_scale(eo, 50, 50);
		device_image_set(device, eo);
	}
	device_save(device);

	if (eo)	evas_object_del(eo);

	evas_object_del(win);
}/*_apply_bt_clicked_cb*/



/*
 * Callback 'clicked' event 'action' button
 */
static void
_action_bt_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
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
			//Update picture in evas image object
			Evas_Object *img;
			img = evas_object_data_get(myfs->win, "image");
			elm_image_file_set(img, sel, NULL);
		}
	}
	myfileselector_close(myfs);
}/*_action_bt_clicked_cb*/



/*
 *
 */
static void
_photo_bt_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__,
					 void *event_info __UNUSED__)
{
	Evas_Object *img = data;
	MyFileSelector *myfs;

	myfs = myfileselector_add();
	myfileselector_set_title(myfs, _("Select a picture file"));
	evas_object_data_set(myfs->win, "image", img);
	evas_object_smart_callback_add(myfs->action_bt, "clicked", _action_bt_clicked_cb, myfs);
}/*_photo_bt_clicked_cb*/



/*
 *
 */
void
devices_creator_new(void *data __UNUSED__, Evas_Object * obj __UNUSED__,
					void *event_info __UNUSED__)
{
	Evas_Object *win, *gd, *frame, *bx;
	Evas_Object *label, *ic, *img;
	Evas_Object *bt, *list;
	Evas_Object *entry;

	win = elm_win_util_standard_add("device_creator", _("Device Creator"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

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
	evas_object_name_set(img, "device image");
	elm_image_smooth_set(img, EINA_TRUE);
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
	evas_object_smart_callback_add(bt, "clicked", _photo_bt_clicked_cb, img);
	elm_box_pack_end(bx, bt);
	evas_object_show(bt);
	elm_object_content_set(frame, bx);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Type"));
	elm_grid_pack(gd, frame, 1, 27, 26, 60);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS );
	evas_object_name_set(list, "device type list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

	int x;
	for(x=0;x != TYPE_LAST;x++)
	{
		elm_list_item_append(list, device_type_to_str(x), NULL, NULL, NULL,  device_type_to_str(x));
	}
	elm_list_go(list);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Name"));
	elm_grid_pack(gd, frame, 32, 1, 60, 12);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "device name entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Description"));
	elm_grid_pack(gd, frame, 32, 15, 60, 12);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "device description entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Datasheet URL"));
	elm_grid_pack(gd, frame, 32, 45, 60, 12);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "device datasheeturl entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

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
	evas_object_smart_callback_add(bt, "clicked", _apply_bt_clicked_cb, win);

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

	evas_object_show(win);
	evas_object_resize(win, 500, 450);
}/*devices_creator_new*/
