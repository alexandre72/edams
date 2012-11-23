/*
 * sensors_browser.c
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
#include "rooms.h"
#include "utils.h"
#include "myfileselector.h"

static Elm_Gengrid_Item_Class *gic;

    
typedef struct _GenGridItem
{
    Elm_Object_Item *ggitem;
    Sensor *sensor;
} GenGridItem;


Evas_Object *browser = NULL;


//
//Update browser window title.
//
static void 
_browser_title_update()
{
    char s[128];
    Eina_List *sensors = NULL;

    if(sensors)
	    snprintf(s, sizeof(s), _("Sensors Browser - %d sensors"), eina_list_count(sensors));
    else
	    snprintf(s, sizeof(s), _("sensors Browser - 0 sensor"));
	
	elm_win_title_set(browser, s);
}
//endof _browser_title_update


//
//Hide notification after timeout.
//
static void
_notify_timeout(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    Evas_Object *notify = (Evas_Object *)data;
    evas_object_hide(notify);
}
//
//endof _bubble_timer_cb
//



//
//Hide notification when clicking on notify close button.
//
static void
_notify_close_bt_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *notify = data;
   evas_object_hide(notify);
}
//
//endof _notify_close_bt_cb
//



//
//Show notification.
//
static void 
_browser_notify_set(const char *msg, const char *icon)
{
    Evas_Object *notify, *label, *ic, *bt;
 
	bt = elm_object_name_find(browser, "notify bt1", -1);
 	evas_object_hide(bt);
 	
	bt = elm_object_name_find(browser, "notify bt2", -1);
  	evas_object_hide(bt);
 
    notify = elm_object_name_find(browser, "notify", -1);
	elm_notify_allow_events_set(notify, EINA_FALSE);    
	elm_notify_timeout_set(notify, 5.0);
	evas_object_show(notify);
        
    label = elm_object_name_find(browser, "notify label", -1);
    elm_object_text_set(label, msg);
    
    ic = elm_object_name_find(browser, "notify icon", -1);
    elm_image_file_set(ic, edams_edje_theme_file_get(), icon);
}
//
//endof _browser_notify_set
//



//
//Create sensors browser.
//
void sensors_browser_new(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    Evas_Object *grid, *bx, *hbx, *bt, *ic, *sp, *label, *notify;

    //Setup a new window browser.
   	browser = elm_win_util_standard_add("sensorsBrowser", "");
   	elm_win_autodel_set(browser, EINA_TRUE);
	_browser_title_update(browser);

	//Setup notify for user informations.
	notify = elm_notify_add(browser);
	elm_notify_allow_events_set(notify, EINA_TRUE);	
   	evas_object_name_set(notify, "notify");
  	elm_notify_align_set(notify, ELM_NOTIFY_ALIGN_FILL, 0.0);   	
   	evas_object_size_hint_weight_set(notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_notify_timeout_set(notify, 5.0);
    evas_object_smart_callback_add(notify, "timeout", _notify_timeout, notify); 

   	bx = elm_box_add(browser);
   	elm_object_content_set(notify, bx);
   	elm_box_horizontal_set(bx, EINA_TRUE);
   	evas_object_show(bx);
   	
	ic = elm_image_add(browser);
  	evas_object_name_set(ic, "notify icon");   
   	evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_image_no_scale_set(ic, EINA_TRUE);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_TRUE);
   elm_image_smooth_set(ic, EINA_FALSE);
   elm_image_orient_set(ic, ELM_IMAGE_FLIP_HORIZONTAL);
   elm_image_aspect_fixed_set(ic, EINA_TRUE);
   elm_image_fill_outside_set(ic, EINA_TRUE);
   elm_image_editable_set(ic, EINA_TRUE);
	elm_box_pack_end(bx, ic);
	evas_object_show(ic);

	label = elm_label_add(browser);
   	evas_object_name_set(label, "notify label");
	elm_box_pack_end(bx, label);
	evas_object_show(label);

	bt = elm_button_add(browser);
   	evas_object_name_set(bt, "notify bt1");
	elm_box_pack_end(bx, bt);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    
    bt = elm_button_add(browser);
   	evas_object_name_set(bt, "notify bt2");
	elm_box_pack_end(bx, bt);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(bt, "clicked", _notify_close_bt_cb, notify);
	    
   	bx = elm_box_add(browser);
   	evas_object_show(bx);
   	 
	//Setup gengrid to display sensors files in a nice and modern way.
   	grid = elm_gengrid_add(browser);
   	evas_object_name_set(grid, "GenGrid");
   	elm_gengrid_item_size_set(grid, 100, 100);
   	elm_gengrid_horizontal_set(grid, EINA_FALSE);
    elm_gengrid_multi_select_set(grid, EINA_TRUE);
    elm_gengrid_select_mode_set(grid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_reorder_mode_set(grid, EINA_FALSE);   
   	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_min_set(grid, 600, 500);
    elm_box_pack_end(bx, grid);
   	evas_object_show(grid);

    //Add a separator bar to show user's actions like add or remove.
    sp = elm_separator_add(browser);
    elm_separator_horizontal_set(sp, EINA_TRUE);
    elm_box_pack_end(bx, sp);
    evas_object_show(sp);

   	hbx = elm_box_add(browser);
   	evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	elm_box_horizontal_set(hbx, EINA_TRUE);
   	elm_box_pack_end(bx, hbx);
   	evas_object_show(hbx);

    bt = elm_button_add(browser);
    elm_object_text_set(bt, _("Add"));
    ic = elm_icon_add(browser);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(ic, "add-edit");
    elm_object_part_content_set(bt, "icon", ic);
    elm_box_pack_end(hbx, bt);
    evas_object_show(bt);

    bt = elm_button_add(browser);
    elm_object_text_set(bt, _("Remove"));
	ic = elm_icon_add(browser);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(ic,  "delete-edit");
    elm_object_part_content_set(bt, "icon", ic);
    elm_box_pack_end(hbx, bt);
    evas_object_show(bt);

	bt = elm_button_add(browser);
    elm_object_text_set(bt, _("Close"));
	ic = elm_icon_add(browser);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "close-window");
   	elm_object_part_content_set(bt, "icon", ic);
    elm_box_pack_end(hbx, bt);
    evas_object_show(bt);
    evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, browser);
    
    sp = elm_separator_add(browser);
    elm_separator_horizontal_set(sp, EINA_TRUE);
    evas_object_show(sp);
    elm_box_pack_end(bx, sp);    
   

   	//Item_class_ref is needed for gic. 
   	//Some items can be added in callbacks.
   	elm_gengrid_item_class_ref(gic);
   	elm_gengrid_item_class_free(gic);

    //Resize browser window.
   	evas_object_resize(browser, 600, 600);
   	evas_object_show(browser);
}


