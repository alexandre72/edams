/*
 * sensors_creator.c
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



#include "edams.h"
#include "libedams.h"
#include "myfileselector.h"


//
//Apply adding new sensor file.
//
static void
_add_apply_bt_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    const char *f, *g;
	Evas_Object *win;
    Evas_Object *img;
    Evas_Object *eo;
	Ecore_Evas *ee;
	Evas *evas;     
	Sensor *sensor;

  	win = (Evas_Object *)data;
    
	sensor = sensor_new(-1,  NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    sensor_name_set(sensor, elm_object_text_get(elm_object_name_find(win, "sensor name entry", -1)));
    sensor_description_set(sensor, elm_object_text_get(elm_object_name_find(win, "sensor description entry", -1)));
    sensor_type_set(sensor, elm_object_text_get(elm_object_name_find(win, "sensor type entry", -1)));
    sensor_datasheeturl_set(sensor, elm_object_text_get(elm_object_name_find(win, "sensor datasheeturl entry", -1)));

   	eo = NULL;
	ee = ecore_evas_new(NULL, 10, 10, 50, 50, NULL);
	evas = ecore_evas_get(ee); 
	
	img = elm_object_name_find(win, "sensor image", -1);
    elm_image_file_get(img, &f, &g);
    
    //Don't try to update if isn't a new item image!
    if(f &&  (eina_str_has_extension(f, ".eet") == EINA_FALSE))
    {
		eo = evas_object_image_filled_add(evas);
		evas_object_image_file_set(eo, f, NULL);
    	evas_object_image_alpha_set(eo, EINA_TRUE);
		evas_object_image_scale(eo, 50, 50);
		sensor_picture_set(sensor, eo);
    }
    sensor_save(sensor);

	//Evas_Object *list = elm_object_name_find(app->win, "sensor list", -1);
	//Elm_Object_Item *it = elm_list_item_append(list, sensor_name_get(sensor), NULL, NULL, NULL, sensor);
	//elm_object_item_del_cb_set(it, _sensor_item_del_cb);

	if(eo)
	{		
		evas_object_del(eo);
    	elm_image_file_set(img, sensor_filename_get(sensor), "/image/1");
	}
	
	evas_object_del(win);
}



//
//
//
static void
_action_bt_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    const char *sel;
	MyFileSelector *myfs = (MyFileSelector *)data;

    sel = elm_fileselector_selected_get(myfs->fs);
    
    if(sel)
    {
        if((eina_str_has_extension(sel, ".png") == EINA_TRUE) ||
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
}



//
//
//
static void
_photo_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{    
    Evas_Object *img = data;
 	MyFileSelector *myfs;
 	
	myfs = myfileselector_add();
	myfileselector_set_title(myfs, _("Select a picture file"));
	evas_object_data_set(myfs->win, "image", img);
    evas_object_smart_callback_add(myfs->action_bt, "clicked", _action_bt_clicked_cb, myfs);
}



//
//
//
void 
sensors_creator_new(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win, *gd, *fr;
	Evas_Object *label, *ic, *img;
	Evas_Object *bt;
	Evas_Object *entry;
 	App_Info *app;
 
	app = (App_Info*)data;
 
	win = elm_win_util_standard_add("sensor_creator", _("Sensor Creator"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
	evas_object_show(win);

	gd = elm_grid_add(win);
	elm_grid_size_set(gd, 100, 100);
	elm_win_resize_object_add(win, gd);
	evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(gd);
	
	fr = elm_frame_add(win);
	elm_grid_pack(gd, fr, 1, 1, 30, 40);
	evas_object_show(fr);
		
	img = elm_image_add(win);
	evas_object_name_set(img, "sensor image");
	elm_image_smooth_set(img, EINA_TRUE);
	elm_image_aspect_fixed_set(img, EINA_TRUE);
	elm_image_resizable_set(img, EINA_TRUE, EINA_TRUE);
	elm_image_file_set(img, edams_edje_theme_file_get(), "default/nopicture");    
	elm_grid_pack(gd, img, 5, 5, 25, 25);
	evas_object_show(img);

	bt = elm_button_add(win);
	elm_object_text_set(bt, _("Photo..."));
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "document-open");
	elm_object_part_content_set(bt, "icon", ic);
	evas_object_smart_callback_add(bt, "clicked",  _photo_bt_clicked_cb, img);
	elm_grid_pack(gd, bt, 1, 31, 30, 12);
	evas_object_show(bt);

    label = elm_label_add(win);
	elm_object_text_set(label, _("Name:"));
	elm_grid_pack(gd, label, 32, 2, 30, 7);    
	evas_object_show(label);
		
	entry = elm_entry_add(win);
	evas_object_name_set(entry, "sensor name entry");	
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_grid_pack(gd, entry, 51, 2, 40, 9);    
	evas_object_show(entry);

	label = elm_label_add(win);
	elm_object_text_set(label, _("Description:"));
	elm_grid_pack(gd, label, 32, 15, 30, 7);    
	evas_object_show(label);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "sensor description entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_grid_pack(gd, entry, 51, 15, 40, 9);    
	evas_object_show(entry);
    
    label = elm_label_add(win);
	elm_object_text_set(label, _("Type:"));
	elm_grid_pack(gd, label, 32, 30, 30, 7);    
	evas_object_show(label);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "sensor type entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_grid_pack(gd, entry, 51, 30, 40, 9);    
	evas_object_show(entry);    
    
	label = elm_label_add(win);
	elm_object_text_set(label, _("Datasheet URL:"));
	elm_grid_pack(gd, label, 32, 45, 30, 7);    
	evas_object_show(label);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "sensor datasheeturl entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_grid_pack(gd, entry, 51, 45, 40, 9);    
	evas_object_show(entry);    
    
	bt = elm_button_add(win);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "dialog-ok-apply");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Ok"));
	elm_grid_pack(gd, bt, 20, 85, 20, 12);    
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", _add_apply_bt_clicked_cb, win);

	bt = elm_button_add(win);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "dialog-cancel");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Cancel"));
    elm_grid_pack(gd, bt, 60, 85, 20, 12);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, win);
	
	evas_object_resize(win, 400, 250);
}