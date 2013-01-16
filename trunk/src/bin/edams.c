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

#include <Elementary.h>
#include <Ecore_File.h>

#include "action.h"
#include "actions_editor.h"
#include "about_dlg.h"
#include "cosm.h"
#include "device.h"
#include "devices_picker.h"
#include "edams.h"
#include "gnuplot.h"
#include "init.h"
#include "location.h"
#include "locations_creator.h"
#include "map.h"
#include "path.h"
#include "preferences_dlg.h"
#include "shutdown.h"
#include "utils.h"
#include "widgets_picker.h"

static const int TEMP_MIN = -30;
static const int TEMP_MAX = 50;

App_Info *app = NULL;

EAPI_MAIN int elm_main(int argc, char *argv[]);


/*Callbacks*/
static void _button_remove_location_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__,void *event_info __UNUSED__);
static void _list_locations_selected_cb(void *data, Evas_Object * obj, void *event_info);
static void _button_quit_clicked_cb(void *data __UNUSED__,  Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static Eina_Bool _statusbar_timer_cb(void *data);
static int _eina_list_devices_sort_cb(const void *d1, const void *d2);

/*xPL sensor.basic listener*/
static void xpl_process_messages();
static void _xpl_sensor_basic_handler(xPL_ServicePtr service, xPL_MessagePtr msg, xPL_ObjectPtr data);
static void handler(void *data __UNUSED__, void *buf, unsigned int len);

/*Functions*/
Evas_Object *_location_naviframe_content_set(Location * location);



/*
 *Callback
 */
static void
_list_locations_selected_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Eina_List *its, *l;
	Elm_Object_Item *it;

	app->location = elm_object_item_data_get(event_info);

	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	its = elm_naviframe_items_get(naviframe);

	EINA_LIST_FOREACH(its, l, it)
	{
		if (app->location == elm_object_item_data_get(it))
		{
			elm_naviframe_item_promote(it);
			break;
		}
	}
}/*_list_locations_selected_cb*/


/*
 *
 */
static void
_button_quit_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	elm_exit();
}


/*
 *
 */
static Eina_Bool
_statusbar_timer_cb(void *data __UNUSED__)
{
	char s[64];
	time_t timestamp;
	struct tm *t;

	Evas_Object *label = elm_object_name_find(app->win, "status text", -1);
	timestamp = time(NULL);
	t = localtime(&timestamp);
  	snprintf(s, sizeof(s), _("EDAMS v%s - %02d/%02d/%d"),PACKAGE_VERSION, (int)t->tm_mday, (int)t->tm_mon + 1, 1900 + (int)t->tm_year);
	elm_object_text_set(label, s);

	Evas_Object *icon = elm_object_name_find(app->win, "status icon", -1);
	elm_icon_standard_set(icon, "help-about");

	return EINA_FALSE;
}/*_notify_timeout_cb*/



/*
 *Set elm label and elm icon of bottom status bar.
 */
void
statusbar_text_set(const char *msg, const char *ic)
{
	Evas_Object *label, *icon;

	label = elm_object_name_find(app->win, "status text", -1);
	elm_object_text_set(label, msg);

	icon = elm_object_name_find(app->win, "status icon", -1);

	if(!elm_icon_standard_set(icon, ic))
		elm_image_file_set(icon, edams_edje_theme_file_get(), ic);

	ecore_timer_add(5.0, _statusbar_timer_cb, NULL);
}/*statusbar_text_set*/


/*
 * Remove currently selected location.
 */
static void
_button_remove_location_clicked_cb(void *data __UNUSED__,Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	char s[256];
	Evas_Object *list;
	Elm_Object_Item *it;

	list = (Evas_Object *) elm_object_name_find(app->win, "locations list", -1);
	it = (Elm_Object_Item *) elm_list_selected_item_get(list);

	if (!it)
	{
		statusbar_text_set(_("Can't remove:no location selected!"), "dialog-error");
		return;
	}
	else
	{
		Location *location = elm_object_item_data_get(it);

		if(location)
		{
			cosm_location_feed_delete(app, location);
			location_remove(location);
			snprintf(s, sizeof(s), _("Location '%s' have been removed."), location_name_get(location));
			elm_object_item_del(it);

			app->locations = locations_list_location_remove(app->locations, location);
			location_free(location);

			Evas_Object *naviframe = (Evas_Object *) elm_object_name_find(app->win, "naviframe", -1);
			elm_naviframe_item_pop(naviframe);

			statusbar_text_set(s, "dialog-information");
		}
	}
}/*_button_remove_location_clicked_cb*/


/*
 *Callback called in Eina list "app->devices" when sorted list_sort func is called.
 */
static int
_eina_list_devices_sort_cb(const void *d1, const void *d2)
{
    const char *txt = device_name_get((Device *)d1);
    const char *txt2 = device_name_get((Device *)d2);

    if(!txt) return(1);
    if(!txt2) return(-1);

    return(strcoll(txt, txt2));
}/*_eina_list_devices_sort_cb*/



/*
 *Callback called in xPL Message 'cstatic void
handler(void *data __UNUSED__, void *buf, unsigned int len)ontrol.basic' is triggered.
 */
static void
_xpl_sensor_basic_handler(xPL_ServicePtr service __UNUSED__, xPL_MessagePtr msg, xPL_ObjectPtr data __UNUSED__)
{
    char buf[256] = "0";
    xPL_NameValueListPtr values_names;

	Ecore_Pipe *pipe = (Ecore_Pipe *) data;

	values_names = xPL_getMessageBody(msg);

	snprintf(buf, sizeof(buf), "%s!%s!%s",
                         xPL_getNamedValue(values_names, "device"),
                         xPL_getNamedValue(values_names, "type"),
                         xPL_getNamedValue(values_names, "current"));

	ecore_pipe_write(pipe, buf, strlen(buf));

}/*_xpl_sensor_basic_handler*/


/*
 *Child process that listen xPL messages received from xPL hub(hub is an external prog and need to be run).
 */
static void
xpl_process_messages()
{
	for (;;)
	{
		xPL_processMessages(100);
		sleep(1);
	}
}/*xpl_process_messages*/


/*
 *
 */
static void
handler(void *data __UNUSED__, void *buf, unsigned int len)
{
        char s[PATH_MAX] = "0";
        char name[255];
        char type[255];
        char sval[4] = "0";
        Device *device;

        char *str = malloc(sizeof(char) * len + 1);
        memcpy(str, buf, len);
        str[len] = '\0';
        sscanf(str, "%[^'!']!%[^'!']!%s", name, type, sval);
        free(str);


        /* TODO:handle case of device has been changed(type, name) and inform user about it*/
        /*Register new devices to EDAMS*/
        if ((device = device_new(name)))
        {
                device_type_set(device, device_str_to_type(type));
                device_class_set(device, SENSOR_BASIC);
                device_save(device);
				statusbar_text_set(_("New xPL sensor.basic has been found"), "elm/icon/xpl/default");

                /*Add new device to edams devices Eina_List*/
                app->devices = eina_list_append(app->devices, device);
                app->devices = eina_list_sort(app->devices, eina_list_count(app->devices), EINA_COMPARE_CB(_eina_list_devices_sort_cb));
        }
        /*Or try to load it instead...*/
        else
        {
                snprintf(s, sizeof(s), "%s"DIR_SEPARATOR_S"%s.eet" , edams_devices_data_path_get(), name);
                device = device_load(s);
        }

        if(!device)
        {
                debug(stderr, _("Couldn't registered new device '%s'"), name);
                return;
        }
        device_data_set(device, sval);

		/*Parse all device action's and to execute them(if condition is full)*/
		Eina_List *l, *actions;
		Action *action;
		actions = device_actions_list_get(device);
		EINA_LIST_FOREACH(actions, l, action)
		{
				switch(action_ifcondition_get(action))
				{
					case EGAL_TO:
							if(atoi(device_data_get(device)) == atoi(action_ifvalue_get(action))) action_parse(action);
							break;
					case LESS_THAN:
							if(atoi(device_data_get(device)) < atoi(action_ifvalue_get(action))) action_parse(action);
							break;
					case MORE_THAN:
							if(atoi(device_data_get(device)) > atoi(action_ifvalue_get(action))) action_parse(action);
							break;
					case LESS_OR_EGAL_TO:
							if(atoi(device_data_get(device)) <= atoi(action_ifvalue_get(action))) action_parse(action);
							break;
					case MORE_OR_EGAL_TO:
							if(atoi(device_data_get(device)) >= atoi(action_ifvalue_get(action))) action_parse(action);
							break;
				}
		}

        /*Write gnuplot file with updated device's data*/
        gnuplot_device_data_write(app, device);

        /*Parse all locations and sync with global and cosm*/
        Location *location;
        EINA_LIST_FOREACH(app->locations, l, location)
        {
                /*Sync device's data with gnuplot, global map and cosm*/
                cosm_device_datastream_update(app, location, device);
                map_widget_data_update(app, location, device);
        }

        debug(stdout, _("Sensors registered:%d"), eina_list_count(app->devices));
}/*handler*/



/*
 *Callback called in ctxpopup object when "remove" item is selected
 */
static void
_button_remove_widget_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	if (!app->location)
	{
		statusbar_text_set(_("Couldn't remove widgets:no location selected!"), "dialog-error");
		return;
	}

	Evas_Object *list = elm_object_name_find(app->win, "widgets list", -1);

	Elm_Object_Item *selected_item = elm_list_selected_item_get(list);
	Widget *widget = elm_object_item_data_get(selected_item);
	Device *device = device_load(widget_device_filename_get(widget));

	location_widgets_del(app->location, widget);
   	elm_object_item_del(selected_item);
	location_save(app->location);

	char s[256];
	snprintf(s, sizeof(s), _("Widget for device '%s' have been removed from location '%s'."),
							device_name_get(device),
							location_name_get(app->location));
	statusbar_text_set(s, "elm/icon/device/default");


	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	elm_object_item_part_content_set(elm_naviframe_top_item_get(naviframe), NULL, _location_naviframe_content_set(app->location));
}/*_button_remove_widget_clicked_cb*/



#define ICON_NEW(ic, _icon)									  	\
   if (_icon)                                         			\
     {                                                 			\
        ic = elm_icon_add(app->win);               				\
        elm_icon_standard_set(ic, _icon);                      	\
        elm_image_resizable_set(ic, EINA_FALSE, EINA_TRUE);   	\
		elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME); \
     }                                                         	\
   else                                                        	\
      ic = NULL;												\




/*
 *Callback called in list "widgets" when sorted sorted_insert func is called.
 */
int
_list_widgets_sort_cb(const void *pa, const void *pb)
{
   const Elm_Object_Item *ia = pa, *ib = pb;

   Device *a = (Device *)elm_object_item_data_get(ia);
   Device *b = (Device *)elm_object_item_data_get(ib);

   return strcoll(device_name_get(a), device_name_get(b));
}/*_list_widgets_sort_cb*/


/*
 *
 */
Evas_Object *
_location_naviframe_content_set(Location * location)
{
	Evas_Object *gd;
	Evas_Object *bx, *frame;
	Evas_Object *list;
	Evas_Object *img, *ic;
	Evas_Object *en, *bt;
	char s[256];

	if (!location) return NULL;

	gd = elm_grid_add(app->win);
	evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gd, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_grid_size_set(gd, 100, 100);
	evas_object_show(gd);

	frame = elm_frame_add(app->win);
	elm_object_text_set(frame, _("Informations"));
	elm_grid_pack(gd, frame, 1, 1, 99, 30);
	evas_object_show(frame);

   	bx = elm_box_add(app->win);
   	elm_box_horizontal_set(bx, EINA_TRUE);
   	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	elm_win_resize_object_add(app->win, bx);
   	evas_object_show(bx);

	img = elm_image_add(app->win);
	elm_image_smooth_set(img, EINA_TRUE);
	elm_image_aspect_fixed_set(img, EINA_TRUE);
	elm_image_resizable_set(img, EINA_FALSE, EINA_TRUE);

	if (!elm_image_file_set(img, location_filename_get(location), "/image/0"))
	{
		elm_image_file_set(img, edams_edje_theme_file_get(), "default/nopicture");
	}
   	evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(img, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(bx, img);
	evas_object_show(img);

   	en = elm_entry_add(app->win);
   	elm_entry_line_wrap_set(en, ELM_WRAP_MIXED);
   	snprintf(s, sizeof(s),"%s", location_name_get(location));
   	elm_object_text_set(en, s);
   	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   	elm_box_pack_end(bx, en);
	evas_object_show(en);

	elm_object_content_set(frame, bx);

	frame = elm_frame_add(app->win);
	elm_object_text_set(frame, _("Widgets"));
	elm_grid_pack(gd, frame, 1, 31, 99, 49);
	evas_object_show(frame);

	list = elm_list_add(app->win);
	elm_scroller_policy_set(list, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_ON);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_name_set(list, "widgets list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

	Eina_List *l, *widgets;
	Widget *widget;
	widgets = location_widgets_list_get(location);
	EINA_LIST_FOREACH(widgets, l, widget)
	{
		Evas_Object *ic;
		ic = elm_icon_add(app->win);
		snprintf(s, sizeof(s), "%s/icon", widget_name_get(widget));
   		elm_image_file_set(ic, edams_edje_theme_file_get(), s);
		elm_image_aspect_fixed_set(ic, EINA_TRUE);
		elm_image_resizable_set(ic, 1, 0);

		Device *device = widget_device_get(widget);
		if(!device) continue;

		snprintf(s, sizeof(s), "%d - %s", widget_id_get(widget), device_name_get(device));
		elm_list_item_append(list, strdup(s), ic, NULL, NULL, widget);
		device_free(device);
	}
	elm_list_go(list);

	bx = elm_box_add(app->win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_grid_pack(gd, bx, 1, 81, 90, 10);
	evas_object_show(bx);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Edit"));
    ic = elm_icon_add(app->win);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(ic, "document-properties");
    elm_object_part_content_set(bt, "icon", ic);
	elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", widgets_picker_add, app);
    evas_object_show(bt);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Actions"));
    ic = elm_icon_add(app->win);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(ic, "document-properties");
    elm_object_part_content_set(bt, "icon", ic);
	elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", actions_editor_add, app);
    evas_object_show(bt);

    bt = elm_button_add(app->win);
    elm_object_text_set(bt, _("Add"));
	ic = elm_icon_add(app->win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "device-add");
	elm_object_part_content_set(bt, "icon", ic);
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", devices_picker_add, app);
	evas_object_show(bt);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Remove"));
    ic = elm_icon_add(app->win);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(ic, "device-remove");
    elm_object_part_content_set(bt, "icon", ic);
	elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", _button_remove_widget_clicked_cb, NULL);
    evas_object_show(bt);

	return gd;
}/*_location_naviframe_content_set*/


/*
 * Edams main loop.
 */
EAPI_MAIN int elm_main(int argc, char **argv)
{

	Evas_Object *vbx, *vbx2, *bg, *frame;
	Evas_Object *sep;
	Evas_Object *tb, *bt, *icon, *label, *bx, *list, *naviframe;
	Eina_List *l;

	// Allocate and initialize App_Info struct.
	app = calloc(1, sizeof(App_Info));

	if (!app)
	{
		fprintf(stderr, _("\033[31mERROR:\033[0mCan't allocate App_Info struct!\n"));
		exit(-1);
	}
	app->argc = argc;
	app->argv = argv;
	app->settings = edams_settings_get();

	// Initialize edams.
	edams_init(app);

	//Load registered devices.
	app->devices = devices_list_get();

	// Setup main window.
	app->locations = locations_list_get();

	app->win = elm_win_add(NULL, "edams", ELM_WIN_BASIC);
	elm_win_title_set(app->win, _("Enlightened Domotics Alarm Monitoring System"));
	elm_win_autodel_set(app->win, EINA_TRUE);
	elm_win_center(app->win, EINA_TRUE, EINA_TRUE);

	bg = elm_bg_add(app->win);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(app->win, bg);
	evas_object_show(bg);

	vbx = elm_box_add(app->win);
	evas_object_size_hint_weight_set(vbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(app->win, vbx);
	evas_object_show(vbx);

	/*Setup toolbar.*/
	app->toolbar = elm_toolbar_add(app->win);
	elm_toolbar_icon_order_lookup_set(app->toolbar, ELM_ICON_LOOKUP_FDO_THEME);
	evas_object_size_hint_align_set(app->toolbar, -1.0, 0.0);
	evas_object_size_hint_weight_set(app->toolbar, 1.0, 0.0);
	elm_toolbar_item_append(app->toolbar, "map", _("Global Map"), map_new, app);
	elm_toolbar_item_append(app->toolbar, "applications-utilities", _("Preferences"), preferences_dlg_new, app);
	elm_toolbar_item_append(app->toolbar, "help-about", _("About"), about_dialog_new, app);
	elm_toolbar_item_append(app->toolbar, "application-exit", _("Quit"), _button_quit_clicked_cb, app);
	elm_box_pack_end(vbx, app->toolbar);
	evas_object_show(app->toolbar);

	sep = elm_separator_add(app->win);
	elm_separator_horizontal_set(sep, EINA_TRUE);
	elm_box_pack_end(vbx, sep);
	evas_object_show(sep);

	// Create locations list panel selector.
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
	elm_object_text_set(frame, _("Locations"));
	evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(vbx2, frame);
	evas_object_show(frame);

	list = elm_list_add(app->win);
	elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_list_mode_set(list, ELM_LIST_EXPAND);
	evas_object_name_set(list, "locations list");
	evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(list, "selected", _list_locations_selected_cb, NULL);
	evas_object_show(list);
	elm_object_content_set(frame, list);
	elm_panes_content_left_size_set(panes, 0.20);
	elm_object_part_content_set(panes, "left", vbx2);

	//Table widget, contains group/subgroup genlist navigation and actions buttons like add...
	tb = elm_table_add(app->win);
	elm_win_resize_object_add(app->win, tb);
	evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_box_pack_end(vbx2, tb);
	evas_object_show(tb);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Add"));
	icon = elm_icon_add(app->win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "list-add");
	elm_object_part_content_set(bt, "icon", icon);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_table_pack(tb, bt, 0, 0, 1, 1);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", locations_creator_add, app);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Remove"));
	icon = elm_icon_add(app->win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "list-remove");
	elm_object_part_content_set(bt, "icon", icon);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_table_pack(tb, bt, 0, 1, 1, 1);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", _button_remove_location_clicked_cb, list);

	frame = elm_frame_add(app->win);
	evas_object_name_set(frame, "location frame");
	evas_object_size_hint_align_set(frame, -1.0, -1.0);
	evas_object_size_hint_weight_set(frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(panes, "right", frame);
	evas_object_show(frame);

	naviframe = elm_naviframe_add(app->win);
	elm_naviframe_content_preserve_on_pop_set(naviframe, EINA_FALSE);
	evas_object_name_set(naviframe, "naviframe");
	evas_object_size_hint_weight_set(naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(naviframe);


	Location *location;
	EINA_LIST_FOREACH(app->locations, l, location)
	{
   		icon = elm_icon_add(app->win);
   		elm_image_file_set(icon, location_filename_get(location), "/image/0");
   		evas_object_size_hint_min_set(icon, 48, 48);
   		evas_object_size_hint_align_set(icon, 0.5, EVAS_HINT_FILL);
   		evas_object_show(icon);
		Elm_Object_Item *it = elm_list_item_append(list, location_name_get(location), icon, icon,  NULL, location);
		elm_object_item_data_set(it, location);

		it = elm_naviframe_item_push(naviframe, location_name_get(location), NULL, NULL, _location_naviframe_content_set(location), NULL);
		elm_naviframe_item_title_visible_set(it, EINA_FALSE);
		elm_object_item_data_set(it, location);
	}



	Elm_Object_Item *it = elm_naviframe_item_push(naviframe, NULL, NULL, NULL, NULL, NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	elm_list_go(list);

	elm_object_content_set(frame, naviframe);

	/*Setup status bar to inform user.*/
	sep = elm_separator_add(app->win);
	elm_separator_horizontal_set(sep, EINA_TRUE);
	elm_box_pack_end(vbx, sep);
	evas_object_show(sep);

	bx = elm_box_add(app->win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_box_pack_end(vbx, bx);
	evas_object_show(bx);

   	icon = elm_icon_add(bx);
   	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	evas_object_name_set(icon, "status icon");
   	evas_object_size_hint_min_set(icon, 24, 24);
   	evas_object_size_hint_align_set(icon, 0.5, EVAS_HINT_FILL);
   	evas_object_show(icon);
   	elm_box_pack_end(bx, icon);

   	label = elm_label_add(app->win);
   	elm_label_slide_duration_set(label, 3);
   	elm_label_slide_set(label, EINA_TRUE);
   	elm_object_style_set(label, "slide_bounce");
	evas_object_name_set(label, "status text");
   	evas_object_show(label);
	elm_box_pack_end(bx, label);
   	evas_object_show(label);

	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_image_aspect_fixed_set(icon, EINA_TRUE);
	elm_image_fill_outside_set(icon, EINA_FALSE);

	sep = elm_separator_add(app->win);
	elm_separator_horizontal_set(sep, EINA_TRUE);
	elm_box_pack_end(vbx, sep);
	evas_object_show(sep);

	evas_object_resize(app->win, 700, 500);
	evas_object_show(app->win);

	Ecore_Pipe *pipe;
	pid_t child_pid;

    pipe = ecore_pipe_add(handler, NULL);
	xPL_addServiceListener(app->edamsService, _xpl_sensor_basic_handler, xPL_MESSAGE_TRIGGER, "sensor", "basic",(xPL_ObjectPtr)pipe);
	child_pid = fork();

	if (!child_pid)
	{
		ecore_pipe_read_close(pipe);
		xpl_process_messages(pipe);
	}
	else
	{
		ecore_pipe_write_close(pipe);
		elm_run();
	}

	map_quit();
	ecore_pipe_del(pipe);
	kill(child_pid, SIGKILL);

	edams_shutdown(app);

	return EXIT_SUCCESS;
}/*main*/
ELM_MAIN()
