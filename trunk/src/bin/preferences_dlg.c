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






#include "edams.h"
#include "utils.h"


#define WEIGHT evas_object_size_hint_weight_set
#define ALIGN_ evas_object_size_hint_align_set
#define EXPAND(X) WEIGHT((X), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND)
#define FILL(X) ALIGN_((X), EVAS_HINT_FILL, EVAS_HINT_FILL)

#define BUTTON_TEXT_SET(BT, TEXT) \
   elm_object_text_set((BT), (TEXT)); \
   elm_object_tooltip_text_set((BT), (TEXT)); \
   elm_object_tooltip_window_mode_set((BT), EINA_TRUE)
   
   

static Evas_Object *win;

static void
apply_bt_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{

}


static void 
_options_list_selected_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
    Eina_List *its, *l;
    Elm_Object_Item *it;
    
    int id = elm_object_item_data_get(event_info);
    
    Evas_Object *naviframe = elm_object_name_find(win, "NaviFrame", -1);
    	
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

static Evas_Object*
_general_settings_content()
{
	Evas_Object *gd;

    gd = elm_grid_add(win);
	EXPAND(gd);
    elm_grid_size_set(gd, 100, 100);
    evas_object_show(gd);
    
    
    return gd;
}

static Evas_Object*
_modules_settings_content()
{
	Evas_Object *gd;

    gd = elm_grid_add(win);
	EXPAND(gd);
    elm_grid_size_set(gd, 100, 100);
    evas_object_show(gd);
 
    
    return gd;
}

static Evas_Object*
_debug_settings_content()
{
    Evas_Object *gd, *frame, *bx, *table;
    Evas_Object *group;
	Evas_Object *ic;
 	Evas_Object *bt;
   	Evas_Object *checkb;
    Evas_Object *radio;
    Evas_Object *entry;
    Evas_Object *label;
    char s[PATH_MAX];
    

    gd = elm_grid_add(win);
	EXPAND(gd);
    elm_grid_size_set(gd, 100, 100);
    evas_object_show(gd);
    
    checkb = elm_check_add(win);
	evas_object_name_set(checkb, "debug printf checkb");
	elm_object_text_set(checkb, _("Debug printf"));
	elm_grid_pack(gd, checkb , 1, 1, 20, 10);
	evas_object_show(checkb);

	label = elm_label_add(win);
	snprintf(s, sizeof(s), _("Installation path:%s"), edams_install_path_get());
	elm_object_text_set(label, s);
	elm_grid_pack(gd, label, 1, 9, 70, 10);
	evas_object_show(label);

	label = elm_label_add(win);
	snprintf(s, sizeof(s), _("User home path:%s"), user_home_get());
	elm_object_text_set(label, s);
	elm_grid_pack(gd, label, 1, 15, 70, 10);
	evas_object_show(label);

	label = elm_label_add(win);
	snprintf(s, sizeof(s), _("User data path:%s"), edams_data_path_get());
	elm_object_text_set(label, s);
	elm_grid_pack(gd, label, 1, 21, 70, 10);
	evas_object_show(label);
		
	label = elm_label_add(win);
	snprintf(s, sizeof(s), _("Locale path:%s"), edams_locale_path_get());
	elm_object_text_set(label, s);
	elm_grid_pack(gd, label, 1, 27, 70, 10);
	evas_object_show(label);
		
	label = elm_label_add(win);
	snprintf(s, sizeof(s), _("Modules path:%s"), edams_modules_path_get());
	elm_object_text_set(label, s);
	elm_grid_pack(gd, label, 1, 33, 70, 10);
	evas_object_show(label);
		
	label = elm_label_add(win);
	snprintf(s, sizeof(s), _("Edje theme file:%s"), edams_edje_theme_file_get());
	elm_object_text_set(label, s);
	elm_grid_pack(gd, label, 1, 39, 70, 10);
	evas_object_show(label);		
		
	bt = elm_button_add(win);
	BUTTON_TEXT_SET(bt, _("Apply"));
	ic = elm_icon_add(win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   	elm_icon_standard_set(ic, "exec");
   	elm_object_part_content_set(bt, "icon", ic);
	elm_grid_pack(gd, bt , 0, 84, 100, 10);
    evas_object_show(bt);
  
	return gd;
}


//
//
//
void 
preferences_dlg_new(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   	Evas_Object *bt, *hbx;
   	Evas_Object *ic;
	Evas_Object *vbx, *naviframe, *list;
	Elm_Object_Item *it;

  	App_Info *app = (App_Info *)data;

   	win  = elm_win_util_standard_add(_("Preferences"), "");
   	elm_win_autodel_set(win, EINA_TRUE);
   	elm_win_center(win, EINA_TRUE, EINA_TRUE);
   	
   	evas_object_show(win);
   	
 	Evas_Object *bg = elm_bg_add(win);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg );
  	evas_object_size_hint_min_set(bg, 600, 450);
  	
    vbx = elm_box_add(win);
    EXPAND(vbx);
	evas_object_size_hint_weight_set(vbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, vbx);
	evas_object_show(vbx);
    
    Evas_Object *panes = elm_panes_add(win);
    elm_win_resize_object_add(win, panes);
    evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(vbx, panes);
    evas_object_show(panes);
   
	list = elm_list_add(win);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list, -1.0, -1.0);
	evas_object_smart_callback_add(list, "selected", _options_list_selected_cb, NULL);
	evas_object_show(list);
	elm_object_part_content_set(panes, "left", list);
    elm_panes_content_left_size_set(panes, 0.15);
	   
	naviframe = elm_naviframe_add(win);
   	evas_object_name_set(naviframe, "NaviFrame");
	EXPAND(naviframe);
	evas_object_size_hint_align_set(naviframe, -1.0, -1.0);	
	elm_object_text_set(naviframe, _("Options"));
	evas_object_show(naviframe);
	elm_object_part_content_set(panes, "right", naviframe);

	elm_list_item_append(list, _("General"), NULL, NULL, NULL, 0);

	it = elm_naviframe_item_push(naviframe, _("General"), NULL, NULL, _general_settings_content(), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	 elm_object_item_data_set(it, 0);

	elm_list_item_append(list, _("Modules"), NULL, NULL, NULL, 1);
	it = elm_naviframe_item_push(naviframe, _("Modules"), NULL, NULL, _modules_settings_content(), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	 elm_object_item_data_set(it, 1);

	elm_list_item_append(list, _("Debug"), NULL, NULL, NULL, 2);
	it = elm_naviframe_item_push(naviframe, _("Debug"), NULL, NULL, _debug_settings_content(), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	 elm_object_item_data_set(it, 2);

	elm_list_go(list);
}
