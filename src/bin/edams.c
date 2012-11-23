/*
 * edams.c
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


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <Ecore_File.h>

#include "edams.h"
#include "utils.h"
#include "myfileselector.h"
#include "rooms.h"


EAPI_MAIN int elm_main(int argc, char *argv[]);

int _log_dom = -1;


//Widgets callbacks.
static void quit_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _rooms_list_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _fill_rooms_list();

static void _notify_timeout(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _notify_close_bt_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _notify_set(const char *msg, const char *icon);

App_Info *app = NULL;

//
//Update current selected patient informations.
//
static void
_add_apply_bt_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    char s[256];
    const char *f, *g;
	Evas_Object *win;
    Evas_Object *img;
    Evas_Object *eo;
	Ecore_Evas *ee;
	Evas *evas;     
	Room *room;

  	win = (Evas_Object *)data;
    
	room = room_new(0,  NULL, NULL, NULL, NULL);
    
    room_name_set(room, elm_object_text_get(elm_object_name_find(win, "room name entry", -1)));

   	eo = NULL;
	ee = ecore_evas_new(NULL, 10, 10, 50, 50, NULL);
	evas = ecore_evas_get(ee); 
	
	img = elm_object_name_find(win, "photo", -1);
    elm_image_file_get(img, &f, &g);
    
    //Don't try to update if isn't a new item image!
    if(f &&  (eina_str_has_extension(f, ".eet") == EINA_FALSE))
    {
		eo = evas_object_image_filled_add(evas);
		evas_object_image_file_set(eo, f, NULL);
    	evas_object_image_alpha_set(eo, EINA_TRUE);
		evas_object_image_scale(eo, 50, 50);
		room_photo_set(room, eo);
    }
    
    room_save(room);

	if(eo)
	{		
		evas_object_del(eo);
    	elm_image_file_set(img, room_filename_get(room), "/image/1");
	}
	
	_fill_rooms_list();

    snprintf(s, sizeof(s), _("Room %s has been created."), 
    						room_name_get(room));
	_notify_set(s, "elm/icon/info/default");
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
//Display ctxpopup when double-click on an patient's gengrid.
//
static void 
_add_room_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win, *gd, *fr;
	Evas_Object *label, *ic, *img;
	Evas_Object *bt;
	Evas_Object *entry;
 
 
	win = elm_win_util_standard_add("rooms_description_dlg", _("Room Description"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
	evas_object_show(win);

	gd = elm_grid_add(win);
	elm_grid_size_set(gd, 100, 120);
	elm_win_resize_object_add(win, gd);
	evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(gd);
	
	fr = elm_frame_add(win);
	elm_grid_pack(gd, fr, 1, 1, 30, 40);
	evas_object_show(fr);
		
	img = elm_image_add(win);
	evas_object_name_set(img, "photo");
	elm_image_smooth_set(img, EINA_TRUE);
	elm_image_aspect_fixed_set(img, EINA_TRUE);
	elm_image_resizable_set(img, EINA_TRUE, EINA_TRUE);

   //if(!elm_image_file_set(img, room_filename_get(room), "/image/1"))
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
	elm_grid_pack(gd, bt, 1, 31, 30, 9);
	evas_object_show(bt);

    label = elm_label_add(win);
	elm_object_text_set(label, _("Name:"));
	elm_grid_pack(gd, label, 32, 2, 30, 7);    
	evas_object_show(label);
		
	entry = elm_entry_add(win);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_grid_pack(gd, entry, 51, 2, 40, 9);    
	evas_object_name_set(entry, "room name entry");
	evas_object_show(entry);

	label = elm_label_add(win);
	elm_object_text_set(label, _("Description:"));
	elm_grid_pack(gd, label, 32, 15, 30, 7);    
	evas_object_show(label);

	entry = elm_entry_add(win);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_grid_pack(gd, entry, 51, 15, 40, 9);    
	evas_object_name_set(entry, "room description entry");
	evas_object_show(entry);
    
	bt = elm_button_add(win);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "dialog-ok-apply");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Ok"));
	elm_grid_pack(gd, bt, 20, 50, 30, 10);    
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", _add_apply_bt_clicked_cb, win);

	bt = elm_button_add(win);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "dialog-cancel");
	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Cancel"));
    elm_grid_pack(gd, bt, 60, 50, 30, 10);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, win);
	
	evas_object_resize(win, 450, 350);
}



static void 
_rooms_list_selected_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	Module_Info *module = NULL;
    Eina_List *its, *l;
    Elm_Object_Item *it;
    
    its = elm_naviframe_items_get(app->naviframe);
    EINA_LIST_FOREACH(its, l, it)
    {
        char *s = elm_object_item_data_get(it);
        if(strcmp(s, module->id) == 0)
        {
            elm_naviframe_item_promote(it);
            break;
        }
    }
}



static void 
quit_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_exit();
}



void
_fill_rooms_list()
{
    Eina_List *rooms = NULL, *l;
	Module_Info *module;
	
	//rooms = rooms_list_get(app->win);

    EINA_LIST_FOREACH(rooms, l, module)
	{
		Evas_Object *ic;
		ic=elm_icon_add(app->win);
		elm_image_file_set(ic, module->icon, "icon");
		elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
		evas_object_show(ic);
		elm_list_item_append(app->rooms_list, module->name, ic, ic, NULL, module);
		
        Elm_Object_Item *it = elm_naviframe_item_push(app->naviframe, _("Options"), NULL, NULL, module->settings, NULL);
        elm_naviframe_item_title_visible_set(it, EINA_FALSE);            
        elm_object_item_data_set(it, module->id);
	}
		
	//Push a new empty item to naviframe to set when no module is selected(at edams startup).
	Elm_Object_Item *it = elm_naviframe_item_push(app->naviframe, _("Options"), NULL, NULL, NULL, NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);            
    elm_object_item_data_set(it, "");	
	elm_list_go(app->rooms_list);
	evas_object_data_set(app->win, "rooms", rooms);    
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
 
	bt = elm_object_name_find(app->win, "notify bt1", -1);
 	evas_object_hide(bt);
 	
	bt = elm_object_name_find(app->win, "notify bt2", -1);
  	evas_object_hide(bt);
 
    notify = elm_object_name_find(app->win, "notify", -1);
	elm_notify_allow_events_set(notify, EINA_FALSE);    
	elm_notify_timeout_set(notify, 5.0);
	evas_object_show(notify);
        
    label = elm_object_name_find(app->win, "notify label", -1);
    elm_object_text_set(label, msg);
    
    ic = elm_object_name_find(app->win, "notify icon", -1);
    elm_image_file_set(ic, edams_edje_theme_file_get(), icon);
}



//
//Main.
//
EAPI_MAIN int 
elm_main(int argc, char **argv)
{
	char s[512];
	time_t timestamp;
	struct tm *t;
	Evas_Object *vbx, *vbx2, *bg, *frame;
	Evas_Object *sep;
	Evas_Object *tb, *bt, *ic, *label, *notify, *bx;
            
            
	// Initialize important stuff like eina debug system, ecore_evas, eet, elementary...
	#if ENABLE_NLS
    const char *locale = setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE_NAME, edams_locale_path_get());
	bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
	textdomain(PACKAGE_NAME);
	#endif


   	_log_dom = eina_log_domain_register("edams",  EINA_COLOR_CYAN);

   	if (_log_dom < 0)
     	{
        	CRITICAL("Can't register log domain 'edams'!");
        	return EXIT_FAILURE;
     	}

	if (!eina_init())
	{
		fprintf(stderr, _("CRI:Can't init Eina!\n"));
               	return EXIT_FAILURE;
	}

	eet_init();
	ecore_init();
	ecore_evas_init();
	edje_init();

	if (!elm_init(argc, argv))
	{
		CRITICAL(_("Can't init Elementary!"));
        return EXIT_FAILURE;
	}

   	elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   	elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
	
	//Set elm locale based on setlocale returns.
	#if ENABLE_NLS
    	elm_language_set(locale);
    	#endif 
    
	INF(_("Initialize Application..."));
	app = MALLOC(sizeof(*app));
	app->module_selection = NULL;

	//Init edams.
	edams_init();


	//Setup main window.
	timestamp = time(NULL);
	t = localtime(&timestamp);

	snprintf(s, sizeof(s), _("Enlightened Domotics Alarm Monitoring System %s - %02d/%02d/%d"),
	   				PACKAGE_VERSION,
	   				(int)t->tm_mday,
	  				(int)t->tm_mon+1,
	  				1900+(int)t->tm_year);
	
	app->win = elm_win_add(NULL, "edams", ELM_WIN_BASIC);
	elm_win_title_set(app->win, s);
	elm_win_autodel_set(app->win, EINA_TRUE);
	evas_object_show(app->win);
    elm_win_center(app->win, EINA_TRUE, EINA_TRUE);

	//Setup notify for user informations.
	notify = elm_notify_add(app->win);
	elm_notify_allow_events_set(notify, EINA_TRUE);	
   	evas_object_name_set(notify, "notify");
  	elm_notify_align_set(notify, ELM_NOTIFY_ALIGN_FILL, 0.0);   	
   	evas_object_size_hint_weight_set(notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_notify_timeout_set(notify, 5.0);
    evas_object_smart_callback_add(notify, "timeout", _notify_timeout, notify); 

   	bx = elm_box_add(app->win);
   	elm_object_content_set(notify, bx);
   	elm_box_horizontal_set(bx, EINA_TRUE);
   	evas_object_show(bx);
   	
	ic = elm_image_add(app->win);
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

	label = elm_label_add(app->win);
   	evas_object_name_set(label, "notify label");
	elm_box_pack_end(bx, label);
	evas_object_show(label);

	bt = elm_button_add(app->win);
   	evas_object_name_set(bt, "notify bt1");
	elm_box_pack_end(bx, bt);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    
    bt = elm_button_add(app->win);
   	evas_object_name_set(bt, "notify bt2");
	elm_box_pack_end(bx, bt);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(bt, "clicked", _notify_close_bt_cb, notify);
	    
   	bx = elm_box_add(app->win);
   	evas_object_show(bx);

  	bg = elm_bg_add(app->win);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(app->win, bg );
	evas_object_show(bg );
  	evas_object_size_hint_min_set(bg, 850, 720);
	
	vbx = elm_box_add(app->win);
	evas_object_size_hint_weight_set(vbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(app->win, vbx);
	evas_object_show(vbx);

	//Setup toolbar.
	app->toolbar = elm_toolbar_add(app->win);
	elm_toolbar_icon_order_lookup_set(app->toolbar, ELM_ICON_LOOKUP_FDO_THEME);
	evas_object_size_hint_align_set(app->toolbar, -1.0, 0.0);
	evas_object_size_hint_weight_set(app->toolbar, 1.0, 0.0);

	elm_toolbar_item_append(app->toolbar,"sensors-browser", _("Sensors"), sensors_browser_new, app);
	elm_toolbar_item_append(app->toolbar,"preferences-browser", _("Preferences"), preferences_dlg_new, app);
	elm_toolbar_item_append(app->toolbar,"about-dlg", _("About"), about_dialog_new, app);
	elm_toolbar_item_append(app->toolbar, "close-window", _("Quit"), quit_bt_clicked_cb, app);
	elm_box_pack_end(vbx, app->toolbar);
	evas_object_show(app->toolbar);

	sep = elm_separator_add(app->win);
	elm_separator_horizontal_set(sep, EINA_TRUE);
	elm_box_pack_end(vbx, sep);
	evas_object_show(sep);

	//Create module list panel selector.
    Evas_Object *panes = elm_panes_add(app->win);
    elm_win_resize_object_add(app->win, panes);
    evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(vbx, panes);
    evas_object_show(panes);
   
   	vbx2 = elm_box_add(app->win);
	evas_object_size_hint_weight_set(vbx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(vbx2, -1.0, -1.0);
	evas_object_show(vbx2);
   
   	frame = elm_frame_add(app->win);
	evas_object_size_hint_align_set(frame, -1.0, -1.0);
	elm_object_text_set(frame, _("Rooms"));
	evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(vbx2, frame);
	evas_object_show(frame);
   
	app->rooms_list = elm_list_add(app->win);
	evas_object_size_hint_weight_set(app->rooms_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(app->rooms_list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(app->rooms_list, "selected", _rooms_list_selected_cb, app);
	evas_object_show(app->rooms_list);
	elm_object_content_set(frame, app->rooms_list);	
    elm_panes_content_left_size_set(panes, 0.15);
	elm_object_part_content_set(panes, "left", vbx2);
	  
	//Table widget, contains group/subgroup genlist navigation and actions buttons like add.
   	tb = elm_table_add(app->win);
   	elm_win_resize_object_add(app->win, tb);
   	evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_box_pack_end(vbx2, tb);
   	evas_object_show(tb);
	  
	bt = elm_button_add(app->win);
    elm_object_text_set(bt, _("Add"));
    ic = elm_icon_add(app->win);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(ic, "add-edit");
    elm_object_part_content_set(bt, "icon", ic);
   	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   	elm_table_pack(tb, bt, 0, 1, 1, 1);
    evas_object_show(bt);
    evas_object_smart_callback_add(bt, "clicked", _add_room_bt_clicked_cb, NULL);  
	  

	vbx2 = elm_box_add(app->win);
	evas_object_size_hint_weight_set(vbx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(vbx2, -1.0, -1.0);
	evas_object_show(vbx2);

	frame = elm_frame_add(app->win);
	evas_object_size_hint_align_set(frame, -1.0, -1.0);
	elm_object_text_set(frame, _("Indications"));
	evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(vbx2, frame);
	evas_object_show(frame);

  	app->description_text =  elm_entry_add(app->win);
	evas_object_size_hint_weight_set(app->description_text, EVAS_HINT_EXPAND, 0.0);
   	evas_object_size_hint_align_set(app->description_text, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_entry_line_wrap_set(app->description_text, EINA_TRUE);
	elm_entry_editable_set(app->description_text, EINA_FALSE);
   	elm_entry_entry_set(app->description_text, _("<b>edams</>Welcome to edams!<br><br>"
   	                                              "You should select or create a patient to start a session with him."));
	evas_object_size_hint_weight_set(app->description_text, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(frame, app->description_text);
	evas_object_show(app->description_text);

	app->naviframe = elm_naviframe_add(app->win);
	evas_object_size_hint_weight_set(app->naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(app->naviframe, -1.0, -1.0);
	elm_object_text_set(app->naviframe, _("Options"));
	elm_box_pack_end(vbx2, app->naviframe);	
	evas_object_show(app->naviframe);
	elm_object_part_content_set(panes, "right", vbx2);

	_fill_rooms_list(app);

	//char *ret;
	//settings_write("wanttoolbar", "no");
	//ret = settings_read("wanttoolbar");
    //INF("Test wanttoolbar setting:%s", ret);

	elm_run();
	edams_shutdown(app);
	
   	eina_log_domain_unregister(_log_dom);
  	 _log_dom = -1;    
    
	eina_shutdown();
	ecore_evas_shutdown();
	ecore_shutdown();
	eet_shutdown();
	edje_shutdown();
	elm_shutdown();
	exit(0);

	return EXIT_SUCCESS;
}
ELM_MAIN()
