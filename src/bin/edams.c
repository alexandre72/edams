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
#include "map.h"
#include "utils.h"
#include "init.h"
#include "myfileselector.h"
#include "path.h"
#include "location.h"
#include "device.h"
#include "serial.h"
#include "devices_picker.h"
#include "devices_creator.h"
#include "shutdown.h"
#include "about_dlg.h"
#include "cosm.h"
#include "preferences_dlg.h"

static const int TEMP_MIN = -30;
static const int TEMP_MAX = 50;


EAPI_MAIN int elm_main(int argc, char *argv[]);


// Widgets callbacks.
static void quit_bt_clicked_cb(void *data __UNUSED__,
							   Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _locations_list_selected_cb(void *data, Evas_Object * obj, void *event_info);
static void _remove_location_bt_clicked_cb(void *data __UNUSED__,
										   Evas_Object * obj __UNUSED__,
										   void *event_info __UNUSED__);
static void _add_location_bt_clicked_cb(void *data __UNUSED__,
										Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);

static void _notify_timeout(void *data __UNUSED__,
							Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _notify_close_bt_cb(void *data, Evas_Object * obj __UNUSED__,
								void *event_info __UNUSED__);
static void _notify_set(const char *msg, const char *icon);
static void _location_item_del_cb(void *data, Evas_Object * obj, void *event_info);
static void _device_widget_data_update(Widget * widget);
static void _device_widget_layout_update(Widget * widget);
App_Info *app = NULL;

// xPL sensor.basic listener.
void edamsMessageSensorBasicHandler(xPL_ServicePtr theService,
									xPL_MessagePtr theMessage, xPL_ObjectPtr userValue);


//
// Update current selected location informations.
//
static void
_add_apply_bt_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
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
	cosm_location_feed_add(app, location);
	location_save(location);

	// Append location to locations list.
	Evas_Object *list = elm_object_name_find(app->win, "locations list", -1);
	Elm_Object_Item *it = elm_list_item_append(list, location_name_get(location), NULL, NULL,
											   NULL, location);
	elm_object_item_del_cb_set(it, _location_item_del_cb);
	elm_list_go(list);
	elm_list_item_bring_in(it);

	// Append location to naviframe and set it contents.
	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	it = elm_naviframe_item_push(naviframe, location_name_get(location), NULL,
								 NULL, _location_naviframe_content(location), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	elm_object_item_data_set(it, location);
	elm_naviframe_item_promote(it);

	if (eo)
	{
		evas_object_del(eo);
		elm_image_file_set(img, location_filename_get(location), "/image/0");
	}

	snprintf(s, sizeof(s), _("Location %s has been created."), location_name_get(location));
	_notify_set(s, "elm/icon/info/default");
	evas_object_del(win);
}



//
//
//
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
_photo_bt_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__,
					 void *event_info __UNUSED__)
{
	Evas_Object *img = data;
	MyFileSelector *myfs;

	myfs = myfileselector_add();
	myfileselector_set_title(myfs, _("Select a picture file"));
	evas_object_data_set(myfs->win, "image", img);
	evas_object_smart_callback_add(myfs->action_bt, "clicked", _action_bt_clicked_cb, myfs);
}


static void _name_loaded(void *data, Evas_Object * obj __UNUSED__, void *ev __UNUSED__)
{
	double lon, lat;
	Evas_Object *win = (Evas_Object *) data;

	Evas_Object *map = elm_object_name_find(win, "location map", -1);;
	Elm_Map_Name *name = evas_object_data_get(map, "name Elm_Map_Name");

	elm_map_name_region_get(name, &lon, &lat);
	elm_map_region_bring_in(map, lon, lat);
}


static void _bt_route(void *data, Evas_Object * obj __UNUSED__, void *ev __UNUSED__)
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

	evas_object_smart_callback_add(map, "name,loaded", _name_loaded, win);
}



/*
 *
 */
static void
_add_location_bt_clicked_cb(void *data __UNUSED__,
							Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *win, *gd, *frame, *bx, *bx2;
	Evas_Object *ic, *img;
	Evas_Object *bt;
	Evas_Object *entry;

	win = elm_win_util_standard_add("locations_description_dlg", _("Add a location"));
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
	evas_object_smart_callback_add(bt, "clicked", _photo_bt_clicked_cb, img);
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
	evas_object_smart_callback_add(bt, "clicked", _bt_route, win);

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
	evas_object_smart_callback_add(bt, "clicked", _add_apply_bt_clicked_cb, win);

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



static void
_locations_list_selected_cb(void *data __UNUSED__,
							Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
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
}



static void
quit_bt_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	elm_exit();
}


static void
_notify_timeout(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *notify = (Evas_Object *) data;
	evas_object_hide(notify);
}


//
// Hide notification when clicking on notify close button.
//
static void
_notify_close_bt_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *notify = data;
	evas_object_hide(notify);
}



//
// Show notification.
//
static void _notify_set(const char *msg, const char *icon)
{
	Evas_Object *notify, *label, *ic, *bt;

	bt = elm_object_name_find(app->win, "notify bt1", -1);
	evas_object_hide(bt);

	bt = elm_object_name_find(app->win, "notify bt2", -1);
	evas_object_hide(bt);

	notify = elm_object_name_find(app->win, "notify", -1);
	elm_notify_allow_events_set(notify, EINA_TRUE);
	elm_notify_timeout_set(notify, 2.0);
	evas_object_show(notify);

	label = elm_object_name_find(app->win, "notify label", -1);
	elm_object_text_set(label, msg);

	ic = elm_object_name_find(app->win, "notify icon", -1);
	elm_image_file_set(ic, edams_edje_theme_file_get(), icon);
}


//
//
//
static void
_remove_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	char buf[256];
	Evas_Object *list;
	Elm_Object_Item *it;
	Location *location;

	Evas_Object *notify = (Evas_Object *) data;
	evas_object_hide(notify);

	list = (Evas_Object *) elm_object_name_find(app->win, "locations list", -1);
	it = (Elm_Object_Item *) elm_list_selected_item_get(list);

	if (it)
	{
		location = elm_object_item_data_get(it);
		cosm_location_feed_delete(app, location);
		location_remove(location);

		snprintf(buf, sizeof(buf), _("Location '%s' have been removed."),
				 location_name_get(location));
		elm_object_item_del(it);

		Evas_Object *naviframe = (Evas_Object *) elm_object_name_find(app->win, "naviframe", -1);
		elm_naviframe_item_pop(naviframe);

		_notify_set(buf, "elm/icon/info/default");
	}
}


//
// Remove currently selected location.
//
static void
_remove_location_bt_clicked_cb(void *data __UNUSED__,
							   Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *notify, *ic, *bt, *label, *list;
	Elm_Object_Item *it;

	list = (Evas_Object *) elm_object_name_find(app->win, "locations list", -1);
	it = (Elm_Object_Item *) elm_list_selected_item_get(list);

	if (!it)
	{
		_notify_set(_("Can't remove:no location selected!"), "elm/icon/warning-notify/default");
		return;
	}

	notify = elm_object_name_find(app->win, "notify", -1);
	elm_notify_allow_events_set(notify, EINA_TRUE);
	elm_notify_timeout_set(notify, 0.0);
	evas_object_show(notify);

	ic = elm_object_name_find(app->win, "notify icon", -1);
	elm_image_file_set(ic, edams_edje_theme_file_get(), "elm/icon/info-notify/default");

	label = elm_object_name_find(app->win, "notify label", -1);
	elm_object_text_set(label, _("Are you sure to want to remove selected location?"));

	bt = elm_object_name_find(app->win, "notify bt1", -1);
	elm_object_text_set(bt, _("Yes"));
	ic = elm_icon_add(app->win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "apply-window");
	elm_object_part_content_set(bt, "icon", ic);
	evas_object_smart_callback_add(bt, "clicked", _remove_apply_clicked_cb, notify);
	evas_object_show(bt);

	bt = elm_object_name_find(app->win, "notify bt2", -1);
	elm_object_text_set(bt, _("No"));
	ic = elm_icon_add(app->win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "window-close");
	elm_object_part_content_set(bt, "icon", ic);
	evas_object_show(bt);
}


static void
_location_item_del_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Location *location = (Location *) data;

	app->locations = locations_list_location_remove(app->locations, location);
	location_free(location);
	location = NULL;
}


void
edamsMessageSensorBasicHandler(xPL_ServicePtr theService __UNUSED__,
							   xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
	char buf[256] = "0";
	xPL_NameValueListPtr ListNomsValeursPtr;

	Ecore_Pipe *pipe = (Ecore_Pipe *) userValue;

	ListNomsValeursPtr = xPL_getMessageBody(theMessage);

	snprintf(buf, sizeof(buf), "%s.%s.%s",
			 xPL_getNamedValue(ListNomsValeursPtr, "device"),
			 xPL_getNamedValue(ListNomsValeursPtr, "type"),
			 xPL_getNamedValue(ListNomsValeursPtr, "current"));

	ecore_pipe_write(pipe, buf, strlen(buf));
}


static void do_lengthy_task(Ecore_Pipe * pipe __UNUSED__)
{
	for (;;)
	{
		xPL_processMessages(100);
		sleep(1);
	}
}



static void handler(void *data __UNUSED__, void *buf, unsigned int len)
{
	char s[PATH_MAX] = "0";
	char id[255];
	char name[255];
	char type[255];
	char sval[4] = "0";
	Device *device;

	char *str = malloc(sizeof(char) * len + 1);
	memcpy(str, buf, len);
	str[len] = '\0';
	sscanf(str, "%[^'.'].%[^'.'].%[^'.'].%s", name, id, type, sval);
	free(str);

	device = devices_list_device_with_id_get(app->devices, atoi(id));

	/* TODO:handle case of device has been changed(type, name). and inform user about it. */
	if (!device)
	{
		// Create new device file *only* if device isn't already
		// registered(filename already here)!
		snprintf(s, sizeof(s), "%s" DIR_SEPARATOR_S "%s-%s.eet",
				 edams_locations_data_path_get(), id, name);
		if (!(device = device_load(s)))
		{
			device = device_new(atoi(id), name);
			device_type_set(device, device_str_to_type(type));
			Eina_List *l;
			Eina_List *database;
			database = devices_database_list_get();
			Device *data;
			EINA_LIST_FOREACH(database, l, data)
			{
				if (strcmp(device_name_get(data), device_name_get(device)) == 0)
				{
					device_description_set(device, device_description_get(data));
					device_datasheeturl_set(device, device_datasheeturl_get(data));

					Evas_Object *eo;
					Ecore_Evas *ee;
					Evas *evas;
					eo = NULL;
					ee = ecore_evas_new(NULL, 10, 10, 50, 50, NULL);
					evas = ecore_evas_get(ee);
					eo = evas_object_image_filled_add(evas);
					evas_object_image_file_set(eo, device_filename_get(data), "/image/0");
					evas_object_image_alpha_set(eo, EINA_TRUE);
					evas_object_image_scale(eo, 50, 50);
					device_image_set(device, eo);
					if (eo)	evas_object_del(eo);
					break;
				}
			}

			devices_list_free(database);
			device_save(device);
		}
		app->devices = eina_list_append(app->devices, device);
	}
	device_data_set(device, sval);

	Eina_List *l;
	Location *location;
	EINA_LIST_FOREACH(app->locations, l, location)
	{
		Eina_List *l2, *widgets;
		Widget *widget;

		widgets = location_widgets_list_get(location);

		EINA_LIST_FOREACH(widgets, l2, widget)
		{
			// Update device widget affected to it's location.
			if (widget_device_id_get(widget) == device_id_get(device))
			{
				// If device is already here, so only update device data.
				_device_widget_data_update(widget);
				map_data_update(app, widget);
				cosm_device_datastream_update(app, location, device);
			}
		}

	}

	debug(stdout, _("Sensors registered:%d\n"), eina_list_count(app->devices));
}




static void
_clear_widget_from_location_bt_clicked_cb(void *data __UNUSED__,
										  Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	if (!app->location)
	{
		_notify_set(_
					("Can't clear location devices list:no location selected!"),
					"elm/icon/warning-notify/default");
		return;
	}
	location_widgets_list_clear(app->location);
	location_save(app->location);

	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	// elm_object_item_part_content_unset(naviframe, NULL);
	elm_object_item_part_content_set(elm_naviframe_top_item_get(naviframe),
									 NULL, _location_naviframe_content(app->location));
}




static void _dismissed(void *data __UNUSED__, Evas_Object * obj, void *event_info __UNUSED__)
{
	evas_object_del(obj);
}



static void _ctxpopup_item_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info)
{
	char *meter = elm_object_item_text_get(event_info);
	Widget *widget = data;
	widget_name_set(widget, meter);
	_device_widget_layout_update(widget);
	location_save(app->location);
}



static void
_layout_dbclicked__cb(void *data __UNUSED__, Evas_Object * obj,
					  const char *emission __UNUSED__, const char *source __UNUSED__)
{
	Evas_Object *ctxpopup;
	Evas_Coord x, y;
	Widget *widget = (Widget *) data;

	ctxpopup = elm_ctxpopup_add(obj);
	elm_ctxpopup_hover_parent_set(ctxpopup, app->win);
	evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed, NULL);

	Eina_List *l;
	char *meter;
	EINA_LIST_FOREACH(app->meters, l, meter)
	{
		elm_ctxpopup_item_append(ctxpopup, meter, NULL, _ctxpopup_item_cb, widget);
	}

	evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
	evas_object_size_hint_max_set(ctxpopup, 240, 240);
	evas_object_move(ctxpopup, x, y);
	evas_object_show(ctxpopup);
}


static void _device_widget_data_update(Widget * widget)
{
	// Update data device meter affected to location(can be affected on
	// different locations).
	Device *device = devices_list_device_with_id_get(app->devices,
													 widget_device_id_get(widget));

	if (device)
	{
		Eina_List *l;
		Location *location;

		EINA_LIST_FOREACH(app->locations, l, location)
		{
			char s[64];
			snprintf(s, sizeof(s), "%d %d %s layout",
					 widget_position_get(widget), device_id_get(device),
					 location_name_get(location));
			Evas_Object *layout = elm_object_name_find(app->win, s, -1);

			if (layout)
			{
				const char *t;
				// Special layout case(example:temperature values are floats).
				if ((t = elm_layout_data_get(layout, "tempvalue")))
				{
					int temp_x, temp_y;
					sscanf(device_data_get(device), "%d.%02d", &temp_x, &temp_y);

					Evas_Object *eo = elm_layout_edje_get(layout);
					Edje_Message_Float msg;
					double level =
						(double)((temp_x + (temp_y * 0.01)) -
								 TEMP_MIN) / (double)(TEMP_MAX - TEMP_MIN);

					if (level < 0.0)
						level = 0.0;
					else if (level > 1.0)
						level = 1.0;
					msg.val = level;
					edje_object_message_send(eo, EDJE_MESSAGE_FLOAT, 1, &msg);
				}


				if ((t = elm_layout_data_get(layout, "action")))
				{
					if (atoi(device_data_get(device)) == 0)
						elm_object_signal_emit(layout, "end", "over");
					else
						elm_object_signal_emit(layout, "animate", "over");
				}

				if ((t = elm_layout_data_get(layout, "title")))
				{
					snprintf(s, sizeof(s), "%d - %s", device_id_get(device),
							 device_name_get(device));
					elm_object_part_text_set(layout, "title.text", s);
				}

				if ((t = elm_layout_data_get(layout, "value")))
				{
					snprintf(s, sizeof(s), device_unit_format_get(device), device_data_get(device));
					elm_object_part_text_set(layout, "value.text", s);
				}
				elm_object_signal_emit(layout, "updated", "over");
			}
		}
	}
}



static void _device_widget_layout_update(Widget * widget)
{
	char s[64];

	// printf("SENSOR:%d-%s with data %s\n", device_id_get(device),
	// device_name_get(device), device_data_get(device));

	Eina_List *l;
	Location *location;
	EINA_LIST_FOREACH(app->locations, l, location)
	{
		snprintf(s, sizeof(s), "%d %d %s layout", widget_position_get(widget),
				 widget_device_id_get(widget), location_name_get(location));
		Evas_Object *layout = elm_object_name_find(app->win, s, -1);

		if (strstr(widget_name_get(widget), "default"))
			elm_layout_file_set(layout, edams_edje_theme_file_get(), "meter/counter");
		else
			elm_layout_file_set(layout, edams_edje_theme_file_get(), widget_name_get(widget));

		location_save(location);
	}
}


Evas_Object *_location_naviframe_content(Location * location)
{
	Evas_Object *layout;
	Evas_Object *gd;
	Evas_Object *vbx, *hbx;
	Evas_Object *ic, *img;
	Evas_Object *bt;
	Evas_Object *label;
	char s[256];

	if (!location)
		return NULL;

	vbx = elm_box_add(app->win);
	evas_object_size_hint_weight_set(vbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(vbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(vbx);

	gd = elm_grid_add(app->win);
	evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gd, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_grid_size_set(gd, 100, 100);
	elm_box_pack_end(vbx, gd);
	evas_object_show(gd);

	img = elm_image_add(app->win);
	elm_image_smooth_set(img, EINA_TRUE);
	elm_image_aspect_fixed_set(img, EINA_TRUE);
	elm_image_resizable_set(img, EINA_FALSE, EINA_TRUE);

	if (!elm_image_file_set(img, location_filename_get(location), "/image/0"))
	{
		elm_image_file_set(img, edams_edje_theme_file_get(), "default/nopicture");
	}
	elm_grid_pack(gd, img, 1, 1, 40, 40);

	evas_object_show(img);

	label = elm_label_add(app->win);
	snprintf(s, sizeof(s), _(_("Location:%s")), location_name_get(location));
	elm_object_text_set(label, s);
	elm_grid_pack(gd, label, 30, 1, 50, 8);
	evas_object_show(label);

	label = elm_label_add(app->win);
	snprintf(s, sizeof(s), _(_("Description:%s")), location_description_get(location));
	elm_object_text_set(label, s);
	elm_grid_pack(gd, label, 30, 8, 50, 8);
	evas_object_show(label);

	label = elm_label_add(app->win);
	snprintf(s, sizeof(s), _(_("Devices:")));
	elm_grid_pack(gd, label, 30, 15, 50, 8);
	evas_object_show(label);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Add..."));
	ic = elm_icon_add(app->win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "device-add");
	elm_object_part_content_set(bt, "icon", ic);
	elm_grid_pack(gd, bt, 55, 5, 20, 15);
	evas_object_smart_callback_add(bt, "clicked", devicespicker_add, app);
	evas_object_show(bt);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Remove..."));
	ic = elm_icon_add(app->win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "device-remove");
	elm_object_part_content_set(bt, "icon", ic);
	elm_grid_pack(gd, bt, 55, 15, 20, 15);
	evas_object_smart_callback_add(bt, "clicked", _clear_widget_from_location_bt_clicked_cb, NULL);
	evas_object_show(bt);

	hbx = elm_box_add(app->win);
	evas_object_name_set(hbx, "meters hbx");
	elm_box_horizontal_set(hbx, EINA_TRUE);
	//evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_homogeneous_set(hbx, EINA_FALSE);

	Eina_List *l, *widgets;
	Widget *widget;
	widgets = location_widgets_list_get(location);
	EINA_LIST_FOREACH(widgets, l, widget)
	{
		layout = elm_layout_add(app->win);
		snprintf(s, sizeof(s), "%d %d %s layout", widget_position_get(widget),
				 widget_device_id_get(widget), location_name_get(location));
		evas_object_name_set(layout, s);
		_device_widget_layout_update(widget);
		_device_widget_data_update(widget);
		evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_box_padding_set(hbx, 1, 0);
		elm_box_pack_end(hbx, layout);
		evas_object_show(layout);
		elm_layout_signal_callback_add(layout, "mouse,clicked,1", "*",
									   _layout_dbclicked__cb, widget);
	}

	elm_box_recalculate(hbx);
	elm_box_pack_end(vbx, hbx);
	evas_object_show(hbx);

	return vbx;
}





//
// Main.
//
EAPI_MAIN int elm_main(int argc, char **argv)
{
	char s[512];
	time_t timestamp;
	struct tm *t;
	Evas_Object *vbx, *vbx2, *bg, *frame;
	Evas_Object *sep;
	Evas_Object *tb, *bt, *ic, *label, *notify, *bx, *list, *naviframe;
	Eina_List *l;
	Ecore_Pipe *pipe;
	pid_t child_pid;

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

	// Setup main window.
	timestamp = time(NULL);
	t = localtime(&timestamp);

	snprintf(s, sizeof(s),
			 _
			 ("Enlightened Domotics Alarm Monitoring System %s - %02d/%02d/%d"),
			 PACKAGE_VERSION, (int)t->tm_mday, (int)t->tm_mon + 1, 1900 + (int)t->tm_year);

	app->locations = locations_list_get();
	Eina_List *groups = edje_file_collection_list(edams_edje_theme_file_get());
	if (groups)
	{
		char *group;
		EINA_LIST_FOREACH(groups, l, group)
		{
			if (strncmp(group, "meter/", 6) == 0)
			{
				app->meters = eina_list_append(app->meters, eina_stringshare_add(group));
			}
		}
		edje_file_collection_list_free(groups);
	}
	app->win = elm_win_add(NULL, "edams", ELM_WIN_BASIC);
	elm_win_title_set(app->win, s);
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


	// Setup notify for user informations.
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

	// Setup toolbar.
	app->toolbar = elm_toolbar_add(app->win);
	elm_toolbar_icon_order_lookup_set(app->toolbar, ELM_ICON_LOOKUP_FDO_THEME);
	evas_object_size_hint_align_set(app->toolbar, -1.0, 0.0);
	evas_object_size_hint_weight_set(app->toolbar, 1.0, 0.0);
	elm_toolbar_item_append(app->toolbar, "devices-creator",
							_("Devices Creator"), devices_creator_new, app);
	elm_toolbar_item_append(app->toolbar, "map", _("Locations Map"), map_new, app);
	elm_toolbar_item_append(app->toolbar, "preferences-browser",
							_("Preferences"), preferences_dlg_new, app);
	elm_toolbar_item_append(app->toolbar, "about-dlg", _("About"), about_dialog_new, app);
	elm_toolbar_item_append(app->toolbar, "close-application", _("Quit"), quit_bt_clicked_cb, app);
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
	evas_object_smart_callback_add(list, "selected", _locations_list_selected_cb, NULL);
	evas_object_show(list);
	elm_object_content_set(frame, list);
	elm_panes_content_left_size_set(panes, 0.15);
	elm_object_part_content_set(panes, "left", vbx2);

	// Table widget, contains group/subgroup genlist navigation and actions
	// buttons like add.
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
	elm_table_pack(tb, bt, 0, 0, 1, 1);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", _add_location_bt_clicked_cb, NULL);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Remove"));
	ic = elm_icon_add(app->win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "delete-edit");
	elm_object_part_content_set(bt, "icon", ic);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_table_pack(tb, bt, 0, 1, 1, 1);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", _remove_location_bt_clicked_cb, NULL);

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
		Evas_Object *ic;
		ic = elm_icon_add(app->win);
		// evas_object_size_hint_align_set(ic, 0.5, 0.5);
		// elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
		elm_image_file_set(ic, location_filename_get(location), "/image/0");
		Elm_Object_Item *it = elm_list_item_append(list, location_name_get(location), NULL, ic,
												   NULL, location);
		elm_object_item_del_cb_set(it, _location_item_del_cb);
		it = elm_naviframe_item_push(naviframe, location_name_get(location),
									 NULL, NULL, _location_naviframe_content(location), NULL);
		elm_naviframe_item_title_visible_set(it, EINA_FALSE);
		elm_object_item_data_set(it, location);
	}

	Elm_Object_Item *it = elm_naviframe_item_push(naviframe, NULL, NULL, NULL, NULL, NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
	elm_list_go(list);

	elm_object_content_set(frame, naviframe);

	evas_object_resize(app->win, 480, 500);
	evas_object_show(app->win);
	pipe = ecore_pipe_add(handler, NULL);
	xPL_addServiceListener(app->edamsService, edamsMessageSensorBasicHandler,
						   xPL_MESSAGE_TRIGGER, "sensor", "basic", (xPL_ObjectPtr) pipe);

	child_pid = fork();

	if (!child_pid)
	{
		ecore_pipe_read_close(pipe);
		do_lengthy_task(pipe);
	}
	else
	{
		ecore_pipe_write_close(pipe);
		elm_run();
	}

	ecore_pipe_del(pipe);
	kill(child_pid, SIGKILL);

	edams_shutdown(app);

	return EXIT_SUCCESS;
}

ELM_MAIN()
