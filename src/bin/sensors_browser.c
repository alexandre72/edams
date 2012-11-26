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


static void _title_update();
static void _notify_timeout(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _notify_close_bt_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _notify_set(const char *msg, const char *icon);
static char *grid_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__);
static Evas_Object *grid_content_get(void *data, Evas_Object *obj, const char *part);
static void grid_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__);
static void _ggrid_clickeddouble_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info);

//
//Update browser window title.
//
static void 
_title_update()
{
    char s[128];
	Evas_Object *gg;

	gg = elm_object_name_find(browser, "sensors gengrid", -1);
	
    if(elm_gengrid_items_count(gg)> 0)
	    snprintf(s, sizeof(s), _("Sensors Browser - %d sensors"), elm_gengrid_items_count(gg));
    else
	    snprintf(s, sizeof(s), _("sensors Browser - 0 sensor"));
	
	elm_win_title_set(browser, s);
}


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
//Hide notification when clicking on notify close button.
//
static void
_notify_close_bt_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *notify = data;
   evas_object_hide(notify);
}



//
//Show notification.
//
static void 
_notify_set(const char *msg, const char *icon)
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
//
//
static char *
grid_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   	const GenGridItem *ti = data;
   	char buf[256];   
   
	snprintf(buf, sizeof(buf), "%d - %s",
								sensor_id_get(ti->sensor), 
								sensor_name_get(ti->sensor));
   
   	return strdup(buf);
}


//
//
//
static Evas_Object *
grid_content_get(void *data, Evas_Object *obj, const char *part)
{
   const GenGridItem *ti = data;
   
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *icon = elm_bg_add(obj);
        
        	if(!elm_bg_file_set(icon, sensor_filename_get(ti->sensor), "/image/1"))
				elm_bg_file_set(icon, edams_edje_theme_file_get(), "user/male");
        evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
        evas_object_show(icon);
        return icon;
     }
 
   return NULL;
}



//
//
//
static Eina_Bool
grid_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}



//
//
//
static void
grid_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
	GenGridItem *ggi = data;
	FREE(ggi);
}


//
//Display ctxpopup when double-click on an patient's gengrid.
//
static void 
_ggrid_clickeddouble_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win, *gd, *cal, *fr;
	Evas_Object *label, *ic, *img;
	Evas_Object *bt, *sl;
	Evas_Object *entry;
    Elm_Object_Item *gg_it;
    Evas_Object *gg;
    Eina_List *l, *l2, *l3;
   	char s[256];

   gg = (Evas_Object*) elm_object_name_find(browser, "sensors gengrid", -1);
   l = (Eina_List*)elm_gengrid_selected_items_get(gg);
   
   if (!l) return;
   EINA_LIST_FOREACH_SAFE(l, l2, l3, gg_it)
   {
       GenGridItem *ggi = elm_object_item_data_get(gg_it);
   
		win = elm_win_util_standard_add("sensor_description_dlg", _("Sensor"));
		evas_object_data_set(win, "Gengrid Object Item", gg_it);
		elm_win_autodel_set(win, EINA_TRUE);
		elm_win_center(win, EINA_TRUE, EINA_TRUE);

		gd = elm_grid_add(win);
		elm_grid_size_set(gd, 100, 100);
		elm_win_resize_object_add(win, gd);
		evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(gd);

		//
		//Image selector.
		//
		fr = elm_frame_add(win);
		//elm_object_text_set(fr, );
		elm_grid_pack(gd, fr, 1, 1, 30, 40);
		evas_object_show(fr);
		
		img = elm_image_add(win);
		evas_object_name_set(img, "photo");
		elm_image_smooth_set(img, EINA_TRUE);
		elm_image_aspect_fixed_set(img, EINA_TRUE);
		elm_image_resizable_set(img, EINA_TRUE, EINA_TRUE);
       	if(!elm_image_file_set(img, sensor_filename_get(ggi->sensor), "/image/1"))
		    elm_image_file_set(img, edams_edje_theme_file_get(), "default/nopicture");
		elm_grid_pack(gd, img, 5, 5, 25, 25);
		evas_object_show(img);
		//End of image selector.
    
    	label = elm_label_add(win);
		elm_object_text_set(label, _("Name:"));
		elm_grid_pack(gd, label, 32, 2, 30, 7);    
		evas_object_show(label);
		
		entry = elm_entry_add(win);
		elm_entry_scrollable_set(entry, EINA_TRUE);
		elm_entry_editable_set(entry, EINA_FALSE);
		snprintf(s, sizeof(s), "%d - %s", sensor_id_get(ggi->sensor), sensor_name_get(ggi->sensor));
		elm_object_text_set(entry, s);
		elm_entry_single_line_set(entry, EINA_TRUE);
		elm_grid_pack(gd, entry, 51, 2, 40, 9);
		evas_object_show(entry);

		label = elm_label_add(win);
		elm_object_text_set(label, _("Description:"));
		elm_grid_pack(gd, label, 32, 15, 30, 7);    
		evas_object_show(label);

		entry = elm_entry_add(win);
		elm_entry_scrollable_set(entry, EINA_TRUE);
		elm_entry_editable_set(entry, EINA_FALSE);
		elm_object_text_set(entry, sensor_description_get(ggi->sensor));
		elm_entry_single_line_set(entry, EINA_TRUE);
		elm_grid_pack(gd, entry, 51, 15, 40, 9);    
		evas_object_show(entry);
      
      	if(strstr(sensor_datatype_get(ggi->sensor), "INT"))
      	{
		sl = elm_slider_add(win);
   		elm_slider_horizontal_set(sl, EINA_FALSE);
		elm_slider_span_size_set(sl, 120);
   		elm_slider_inverted_set(sl, EINA_TRUE);
  		evas_object_size_hint_align_set(sl, 0.5, EVAS_HINT_FILL);
   		evas_object_size_hint_weight_set(sl, 0, EVAS_HINT_EXPAND);
   		int x, y;
   		sscanf(sensor_data_get(ggi->sensor), "%d.%02d", &x, &y);
   		elm_slider_unit_format_set(sl, "%1.2f °");
		elm_slider_min_max_set(sl, -5, 55);
		elm_slider_value_set(sl, x+y*0.01);
		elm_grid_pack(gd, sl, 10, 61, 90, 15);    
		evas_object_show(sl);
		}    
      
		bt = elm_button_add(win);
		evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		ic = elm_icon_add(win);
	   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	   	elm_icon_standard_set(ic, "close-window");
	   	elm_object_part_content_set(bt, "icon", ic);
		elm_object_text_set(bt, _("Ok"));
		elm_grid_pack(gd, bt, 40, 80, 30, 8);    
		evas_object_show(bt);
		evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, win);
			
		evas_object_resize(win, 450, 450);
		evas_object_show(win);
	}
}



//
//Create sensors browser.
//
void sensors_browser_new(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    Evas_Object *grid, *bx, *hbx, *bt, *ic, *sp, *label, *notify;

	App_Info *app = (App_Info *)data;

    //Setup a new window browser.
   	browser = elm_win_util_standard_add("sensorsBrowser", "");
   	elm_win_autodel_set(browser, EINA_TRUE);
	_title_update();

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
   	evas_object_name_set(grid, "sensors gengrid");
	elm_gengrid_filled_set(grid, EINA_TRUE);
	elm_gengrid_align_set(grid, 0, 0);
   	elm_gengrid_item_size_set(grid, 150, 150);
   	elm_gengrid_horizontal_set(grid, EINA_FALSE);
    elm_gengrid_multi_select_set(grid, EINA_TRUE);
    elm_gengrid_select_mode_set(grid, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_gengrid_reorder_mode_set(grid, EINA_FALSE);   
   	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_min_set(grid, 400, 400);
    evas_object_smart_callback_add(grid, "clicked,double",_ggrid_clickeddouble_cb, NULL);
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
   
    //Defines gengrid item class.
   	gic = elm_gengrid_item_class_new();
   	gic->item_style = "default";
   	gic->func.text_get = grid_text_get;
   	gic->func.content_get = grid_content_get;
   	gic->func.state_get = grid_state_get;
   	gic->func.del = grid_del;

    //Fill gengrid with patients files list.
    Eina_List *l;
    Sensor *sensor;

    EINA_LIST_FOREACH(app->sensors, l, sensor)
    {
        GenGridItem *ti;
        ti = calloc(1, sizeof(*ti));
        ti->sensor = sensor;
   		ti->ggitem = elm_gengrid_item_append(grid, gic, ti, NULL, NULL);
       	//ti->item = elm_gengrid_item_sorted_insert(grid, gic, ti, compare_cb, grid_sel, NULL);
	}
   
   	//Item_class_ref is needed for gic. 
   	//Some items can be added in callbacks.
   	elm_gengrid_item_class_ref(gic);
   	elm_gengrid_item_class_free(gic);

    //Resize browser window.
   	evas_object_resize(browser, 400, 450);
   	evas_object_show(browser);
}


