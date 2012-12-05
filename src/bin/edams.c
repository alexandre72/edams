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
#include "sensors.h"
#include "serial.h"
#include "sensors_picker.h"



static const int TEMP_MIN =  -30;
static const int TEMP_MAX =  50;


EAPI_MAIN int elm_main(int argc, char *argv[]);

int _log_dom = -1;


//Widgets callbacks.
static void quit_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _rooms_list_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _remove_room_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _add_room_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);

static void _notify_timeout(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _notify_close_bt_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _notify_set(const char *msg, const char *icon);
static void _room_item_del_cb(void *data, Evas_Object *obj, void *event_info);
static void _sensor_data_update(Sensor *sensor);
App_Info *app = NULL;

//
//Update current selected room informations.
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

	img = elm_object_name_find(win, "room image", -1);
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

	//Append room to rooms list.
	Evas_Object *list = elm_object_name_find(app->win, "rooms list", -1);
	Elm_Object_Item *it = elm_list_item_append(list, room_name_get(room), NULL, NULL, NULL, room);
	elm_object_item_del_cb_set(it, _room_item_del_cb);
	elm_list_item_bring_in(it);

	//Append room to naviframe and set it contents.
	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	it = elm_naviframe_item_push(naviframe, room_name_get(room), NULL, NULL, _room_naviframe_content(room), NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
    elm_object_item_data_set(it, room);
	elm_naviframe_item_promote(it);

	if(eo)
	{
		evas_object_del(eo);
    	elm_image_file_set(img, room_filename_get(room), "/image/1");
	}

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
	elm_grid_size_set(gd, 100, 100);
	elm_win_resize_object_add(win, gd);
	evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(gd);

	fr = elm_frame_add(win);
	elm_grid_pack(gd, fr, 1, 1, 30, 40);
	evas_object_show(fr);

	img = elm_image_add(win);
	evas_object_name_set(img, "room image");
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
	evas_object_name_set(entry, "room description entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_grid_pack(gd, entry, 51, 15, 40, 9);
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



static void
_rooms_list_selected_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    Eina_List *its, *l;
    Elm_Object_Item *it;

   app->room = elm_object_item_data_get(event_info);

    Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
    its = elm_naviframe_items_get(naviframe);

    EINA_LIST_FOREACH(its, l, it)
    {
		if(app->room == elm_object_item_data_get(it))
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
_remove_apply_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[256];
   Evas_Object *list;
   Elm_Object_Item *it;
   Room *room;

	Evas_Object *notify = (Evas_Object *)data;
	evas_object_hide(notify);

	list = (Evas_Object*)  elm_object_name_find(app->win, "rooms list", -1);
	it = (Elm_Object_Item *)elm_list_selected_item_get(list);

	if(it)
	{
	   	room = elm_object_item_data_get(it);

		snprintf(buf, sizeof(buf), _("Room '%s' have been removed."), room_name_get(room));
		elm_object_item_del(it);

		Evas_Object *naviframe = (Evas_Object*)  elm_object_name_find(app->win, "naviframe", -1);
		elm_naviframe_item_pop(naviframe);

		room_remove(room);
		_notify_set(buf, "elm/icon/info/default");
	}
}


//
//Remove currently selected room.
//
static void
_remove_room_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   	Evas_Object *notify, *ic, *bt, *label, *list;
    Elm_Object_Item *it;

	list = (Evas_Object*)  elm_object_name_find(app->win, "rooms list", -1);
	it = (Elm_Object_Item *)elm_list_selected_item_get(list);

     if(!it)
     {
		_notify_set(_("Can't remove:no room selected!"), "elm/icon/warning-notify/default");
        return;
     }

   	notify = elm_object_name_find(app->win, "notify", -1);
	elm_notify_allow_events_set(notify, EINA_TRUE);
	elm_notify_timeout_set(notify, 0.0);
	evas_object_show(notify);

    ic = elm_object_name_find(app->win, "notify icon", -1);
    elm_image_file_set(ic, edams_edje_theme_file_get(), "elm/icon/info-notify/default");

   	label = elm_object_name_find(app->win, "notify label", -1);
    elm_object_text_set(label, _("Are you sure to want to remove selected room?"));

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
    elm_icon_standard_set(ic, "close-window");
    elm_object_part_content_set(bt, "icon", ic);
	evas_object_show(bt);
}


static void
_room_item_del_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
 	Room *room = (Room *)data;

	app->rooms = rooms_list_room_remove(app->rooms, room);
	room_free(room);
	room = NULL;
}



static void
SeedRandomizer(void)
{
    srand((unsigned int)time((time_t *)NULL));
}

static int
Prandom(int max)
{
 //  return ((rand() % (int)(((max) + 1) - (min))) + (min));
//	return (int)(rand() / (double)RAND_MAX * (max - 1));;
// return rand() * (max / RAND_MAX);

    int partSize   = (max == RAND_MAX ? 0 : (RAND_MAX - max) / (max + 1));
    int maxUsefull = partSize * max + (partSize - 1);
    int draw;

    do {

        draw = rand();
    } while (draw > maxUsefull);

    return draw / partSize;

}


static void
do_lengthy_task(Ecore_Pipe *pipe)
{
    int fd = 0;
	char buf[256];
	char *samples[] = {"DS18B20","PIR","DHT11"};
	Elm_Prefs_Item_Type type;
	Eina_Value value;
	Eina_Bool softemu = EINA_FALSE;
	Eina_Bool hardemu = EINA_FALSE;

	elm_prefs_data_value_get(app->prefs_data, "main:softemu_checkb", &type, &value);
	softemu = eina_value_get(&value, &softemu);
	if(elm_prefs_data_value_get(app->prefs_data, "main:hardemu_checkb", &type, &value))
		hardemu = eina_value_get(&value, &hardemu);

	printf("OPTION:software emulation(snprintf) =%s\n", softemu?"TRUE":"FALSE");
	printf("OPTION:hardware emulation(serial loopback) =%s\n", hardemu?"TRUE":"FALSE");

	if(softemu == EINA_FALSE)
	{
		int baudrate = 115200;  // default
		if(hardemu == EINA_TRUE)
	   	fd = serialport_init("/dev/ttyUSB0", baudrate);	//CPL2103
		else
   		fd = serialport_init("/dev/ttyACM0", baudrate);	//ARDUINO
	}

	SeedRandomizer();
	for(;;)
	{
		//Serial loopback(TX<=>RX) emulation trame test.
		if(hardemu == EINA_TRUE)
			serialport_write(fd, "DEVICE;0;DS18B20;INT;17.296;OK\n");

		//Sotfware emulation trame test.
		if(softemu == EINA_TRUE)
			snprintf(buf, sizeof(buf), "DEVICE;%d;DS18B20;%d.%d;OK", Prandom(3),  Prandom(34), Prandom(99));

		if(softemu == EINA_FALSE)
			serialport_read_until(fd, buf, '\n');	//Disable it when software emulation, and enable it when serial loopback emulation test.
		ecore_pipe_write(pipe, buf, strlen(buf));
		sleep(3);
   }
}


static void
handler(void *data __UNUSED__, void *buf, unsigned int len)
{
	if(len < 10 || !buf)
	return;

	char *str = malloc(sizeof(char) * len + 1);

	memcpy(str, buf, len);
	str[len] = '\0';

	fprintf(stdout, _("INFO:Serial in content '%s'(%d bytes)\n"), (const char *)str, len);

	Sensor *sensor;
   	//Check if system msg...
	if(strncmp(str, "SYSTEM;", 7) == 0)
	{
		if(strstr(str, "STARTING"))
		{
			if(!app->waiting_win)
			{
			fprintf(stdout, _("INFO:Initialize EDAMS(calibrating sensors, init vars, setting serial port...)\n"));

			Evas_Object *bx, *pb, *bg;

			app->waiting_win = elm_win_add(app->win, "init_win", ELM_WIN_SPLASH);
			elm_win_title_set(app->waiting_win, _("Initialize EDAMS wait please..."));
			elm_win_center(app->waiting_win, EINA_TRUE,  EINA_TRUE);

		  	bg = elm_bg_add(app->waiting_win);
			evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_win_resize_object_add(app->win, bg );
			evas_object_show(bg );

		   bx = elm_box_add(app->waiting_win);
		   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		   elm_win_resize_object_add(app->waiting_win, bx);
		   evas_object_show(bx);

		   pb = elm_progressbar_add(app->waiting_win);
		   evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
   			evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   			elm_object_text_set(pb, _("calibrating sensors, setting serial port... "));
   			elm_progressbar_pulse_set(pb, EINA_TRUE);
   			elm_box_pack_end(bx, pb);
		   elm_progressbar_pulse(pb, EINA_TRUE);
   			evas_object_show(pb);

   			evas_object_show(app->waiting_win);
   			}
		}
		else if(strstr(str, "OPERATIONNAL"))
		{
			if(app->waiting_win)
			{
				fprintf(stdout, _("INFO:EDAMS is ready and operationnal\n"));
				evas_object_del(app->waiting_win);
			}
		}
	}
	//Check if new sensor...
	else if((sensor = sensor_detect(str)))
	{
		Eina_List *l;
		Sensor *data;
		Eina_Bool foundin = EINA_FALSE;

		EINA_LIST_FOREACH(app->sensors, l, data)
		{
			if(sensor_id_get(data) == sensor_id_get(sensor))
			{
					//If sensor is already here, so only update sensor data.
					fprintf(stdout, _("INFO:Updating sensor '%d-%s'  with data '%s'...\n"), sensor_id_get(sensor), sensor_name_get(sensor), sensor_data_get(sensor));
					_sensor_data_update(sensor);

					foundin = EINA_TRUE;
					break;
			}
		}

		char s[256];
		if(foundin == EINA_FALSE)
		{
			app->sensors = eina_list_append(app->sensors, sensor);
			snprintf(s, sizeof(s), _("Added new sensor '%d-%s'."), sensor_id_get(sensor), sensor_name_get(sensor));
			//fprintf(stdout, _("INFO:%d sensors registered on serial line...\n"), eina_list_count(app->sensors));
			_notify_set(s, "elm/icon/info/default");
			_sensor_data_update(sensor);
		}
	}

   free(str);
}


static void
_add_sensor_to_room_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
     if(!app->room)
     {
		_notify_set(_("Can't add a sensor to room:no room selected!"), "elm/icon/warning-notify/default");
        return;
     }

	sensorpicker_add_to_room(app);
}



static void
_clear_sensor_from_room_bt_clicked_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
     if(!app->room)
     {
		_notify_set(_("Can't clear room sensors list:no room selected!"), "elm/icon/warning-notify/default");
        return;
     }

	room_sensors_list_clear(app->room);
	room_save(app->room);

	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	elm_object_item_part_content_set(elm_naviframe_top_item_get(naviframe) , NULL, _room_naviframe_content(app->room));
}



static void
_layout_dbclicked__cb(void *data __UNUSED__, Evas_Object *layout, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *edje;
   Evas_Coord w, h;

   elm_layout_sizing_eval(layout);
   edje = elm_layout_edje_get(layout);
   edje_object_size_min_calc(edje, &w, &h);
   printf("Minimum size for this theme: %dx%d\n", w, h);
}




static void
_sensor_data_update(Sensor *sensor)
{
	//Sync sensor data with room sensor data(if affected to any room!).
	Eina_List *l2, *l3, *sensors;
	Room *room;
	Sensor *data;

    EINA_LIST_FOREACH(app->rooms, l2, room)
    {
			sensors = room_sensors_list_get(room);
    		EINA_LIST_FOREACH(sensors, l3, data)
    		{
				if(sensor_id_get(sensor) == sensor_id_get(data))
					{
						sensor_data_set(data, sensor_data_get(sensor));
					}
    		}
    }

	if(sensor_data_get(sensor) && (app->room))
    {
		char s[64];
      	printf("SENSOR:%d-%s with data %s\n", sensor_id_get(sensor), sensor_name_get(sensor), sensor_data_get(sensor));
		snprintf(s, sizeof(s), "%d %s layout", sensor_id_get(sensor), room_name_get(app->room));
		Evas_Object * layout = elm_object_name_find(app->win, s, -1);


		const char *t;
		if((t = elm_layout_data_get(layout, "tempvalue")))
		{
			int temp_x, temp_y;
		   	elm_object_signal_emit(layout, "temp,state,known", "");
   			snprintf(s, sizeof(s), "%s°C", sensor_data_get(sensor));
    		elm_object_part_text_set(layout, "value.text", s);

		    sscanf(sensor_data_get(sensor), "%d.%02d", &temp_x, &temp_y);

			Evas_Object *eo = elm_layout_edje_get(layout);
			   Edje_Message_Float msg;
			double level =  (double)((temp_x + (temp_y*0.01)) - TEMP_MIN) /
    	          			(double)(TEMP_MAX - TEMP_MIN);

   			if (level < 0.0) level = 0.0;
   			else if (level > 1.0) level = 1.0;
   			msg.val = level;
		    edje_object_message_send(eo, EDJE_MESSAGE_FLOAT, 1, &msg);
		}


		if((t = elm_layout_data_get(layout, "action")))
		{
			 if(atoi(sensor_data_get(sensor)) == 0)
           		elm_object_signal_emit(layout, "end", "over");
    	      else
               	elm_object_signal_emit(layout, "animate", "over");
		}

		if((t = elm_layout_data_get(layout, "title")))
			elm_object_part_text_set(layout, "title.text", room_name_get(app->room));

		if((t = elm_layout_data_get(layout, "value")))
		{
			printf("%s\n", t);
			elm_object_part_text_set(layout, "value.text", sensor_data_get(sensor));
		}
	}
}




Evas_Object*
_room_naviframe_content(Room *room)
{
	Evas_Object *layout;
    Evas_Object *gd;
    Evas_Object *vbx, *hbx;
	Evas_Object *ic, *img;
 	Evas_Object *bt;
    Evas_Object *label;
    char s[256];

	if(!room) return NULL;

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

    if(!elm_image_file_set(img, room_filename_get(room), "/image/1"))
	{
		elm_image_file_set(img, edams_edje_theme_file_get(), "default/nopicture");
	}
        elm_grid_pack(gd, img, 1, 1, 25, 25);

	evas_object_show(img);

	label = elm_label_add(app->win);
	snprintf(s, sizeof(s), _(_("Room:%s")), room_name_get(room));
	elm_object_text_set(label, s);
    elm_grid_pack(gd, label, 30, 1, 50, 8);
	evas_object_show(label);

	label = elm_label_add(app->win);
	snprintf(s, sizeof(s), _(_("Description:%s")), room_description_get(room));
	elm_object_text_set(label, s);
	elm_grid_pack(gd, label, 30, 8, 50, 8);
	evas_object_show(label);

	label = elm_label_add(app->win);
	snprintf(s, sizeof(s), _(_("Sensors:")));
	elm_grid_pack(gd, label, 30, 15, 50, 8);
	evas_object_show(label);

	Eina_List *l, *sensors;
	sensors = room_sensors_list_get(room);
	Sensor *sensor;
    EINA_LIST_FOREACH(sensors, l, sensor)
    {
		strcat(s, sensor_name_get(sensor));
		strcat(s, ",");
    }
	elm_object_text_set(label, s);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Add..."));
	ic = elm_icon_add(app->win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   	elm_icon_standard_set(ic, "sensors-add");
   	elm_object_part_content_set(bt, "icon", ic);
	elm_grid_pack(gd, bt , 55, 5, 20, 8);
	evas_object_smart_callback_add(bt, "clicked", _add_sensor_to_room_bt_clicked_cb, NULL);
    evas_object_show(bt);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Remove..."));
	ic = elm_icon_add(app->win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   	elm_icon_standard_set(ic, "sensors-remove");
   	elm_object_part_content_set(bt, "icon", ic);
	elm_grid_pack(gd, bt , 55, 15, 20, 8);
	evas_object_smart_callback_add(bt, "clicked", _clear_sensor_from_room_bt_clicked_cb, NULL);
    evas_object_show(bt);

	hbx = elm_box_add(app->win);
	elm_box_horizontal_set(hbx, EINA_TRUE);
	evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_homogeneous_set(hbx, EINA_TRUE);
    elm_box_pack_end(vbx, hbx);
	evas_object_show(hbx);

	sensors = room_sensors_list_get(room);

    EINA_LIST_FOREACH(sensors, l, sensor)
    {
    	layout = elm_layout_add(app->win);
		snprintf(s, sizeof(s), "%d %s layout", sensor_id_get(sensor), room_name_get(room));
		evas_object_name_set(layout, s);
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   		evas_object_size_hint_align_set(layout, EVAS_HINT_FILL,  EVAS_HINT_FILL);
		elm_box_padding_set(hbx, 5, 5);
    	elm_box_pack_end(hbx, layout);
		evas_object_show(layout);

		if(!strstr(sensor_meter_get(sensor), "default"))
	      	elm_layout_file_set(layout, edams_edje_theme_file_get(), sensor_meter_get(sensor));
		else
			elm_layout_file_set(layout, edams_edje_theme_file_get(), "meter/thermometer2");

		//elm_object_signal_callback_add(layout, "emit,bt,doubleclicked", "", _layout_dbclicked__cb, layout);
		_sensor_data_update(sensor);
	}

	elm_box_recalculate(hbx);
	return vbx;
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
	Evas_Object *tb, *bt, *ic, *label, *notify, *bx, *list, *naviframe;
	Eina_List *l;
	Ecore_Pipe *pipe;
	pid_t child_pid;

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

	//Set elm locale based on setlocale returns.
	#if ENABLE_NLS
    	elm_language_set(locale);
    #endif

   	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   	elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   	elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
	elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);

	INF(_("Initialize Application..."));
    app = calloc(1, sizeof(App_Info));

	//Init edams.
	edams_init();

	app->prefs_data = elm_prefs_data_new(edams_cfg_file_get(), NULL, EET_FILE_MODE_READ_WRITE);

	//Setup main window.
	timestamp = time(NULL);
	t = localtime(&timestamp);

	snprintf(s, sizeof(s), _("Enlightened Domotics Alarm Monitoring System %s - %02d/%02d/%d"),
	   				PACKAGE_VERSION,
	   				(int)t->tm_mday,
	  				(int)t->tm_mon+1,
	  				1900+(int)t->tm_year);


	Eina_List *groups = edje_file_collection_list(edams_edje_theme_file_get());
	if(groups)
	{
		char *group;
		EINA_LIST_FOREACH(groups, l, group)
		{
			if(strncmp(group, "meter/" , 6) == 0)
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
	elm_win_resize_object_add(app->win, bg );
	evas_object_show(bg );

	vbx = elm_box_add(app->win);
	evas_object_size_hint_weight_set(vbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(app->win, vbx);
	evas_object_show(vbx);


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

	//Setup toolbar.
	app->toolbar = elm_toolbar_add(app->win);
	elm_toolbar_icon_order_lookup_set(app->toolbar, ELM_ICON_LOOKUP_FDO_THEME);
	evas_object_size_hint_align_set(app->toolbar, -1.0, 0.0);
	evas_object_size_hint_weight_set(app->toolbar, 1.0, 0.0);
	elm_toolbar_item_append(app->toolbar,"sensors-creator", _("Sensors Creator"), sensors_creator_new, app);
	elm_toolbar_item_append(app->toolbar,"preferences-browser", _("Preferences"), preferences_dlg_new, app);
	elm_toolbar_item_append(app->toolbar,"about-dlg", _("About"), about_dialog_new, app);
	elm_toolbar_item_append(app->toolbar, "close-window", _("Quit"), quit_bt_clicked_cb, app);
	elm_box_pack_end(vbx, app->toolbar);
	evas_object_show(app->toolbar);

	sep = elm_separator_add(app->win);
	elm_separator_horizontal_set(sep, EINA_TRUE);
	elm_box_pack_end(vbx, sep);
	evas_object_show(sep);

	//Create rooms list panel selector.
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

	list = elm_list_add(app->win);
	elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_list_mode_set(list, ELM_LIST_EXPAND);
	evas_object_name_set(list, "rooms list");
	evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(list, "selected", _rooms_list_selected_cb, NULL);
	evas_object_show(list);
	elm_object_content_set(frame, list);
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
   	elm_table_pack(tb, bt, 0, 0, 1, 1);
    evas_object_show(bt);
    evas_object_smart_callback_add(bt, "clicked", _add_room_bt_clicked_cb, NULL);

	bt = elm_button_add(app->win);
    elm_object_text_set(bt, _("Remove"));
    ic = elm_icon_add(app->win);
    elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(ic,  "delete-edit");
    elm_object_part_content_set(bt, "icon", ic);
   	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   	elm_table_pack(tb, bt, 0, 1, 1, 1);
    evas_object_show(bt);
    evas_object_smart_callback_add(bt, "clicked", _remove_room_bt_clicked_cb, NULL);

	frame = elm_frame_add(app->win);
	evas_object_name_set(frame, "room frame");
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

    app->rooms = rooms_list_get();
    Room *room;
    EINA_LIST_FOREACH(app->rooms, l, room)
	{
		Evas_Object *ic;
		ic = elm_icon_add(app->win);
		//evas_object_size_hint_align_set(ic, 0.5, 0.5);
		//elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
		elm_image_file_set(ic, room_filename_get(room), "/image/1");
		Elm_Object_Item *it = elm_list_item_append(list, room_name_get(room), NULL, ic, NULL, room);
		elm_object_item_del_cb_set(it, _room_item_del_cb);
		it = elm_naviframe_item_push(naviframe, room_name_get(room), NULL, NULL, _room_naviframe_content(room), NULL);
		elm_naviframe_item_title_visible_set(it, EINA_FALSE);
    	elm_object_item_data_set(it, room);
	}

	Elm_Object_Item *it = elm_naviframe_item_push(naviframe, NULL, NULL, NULL, NULL, NULL);
	elm_naviframe_item_title_visible_set(it, EINA_FALSE);
    elm_list_go(list);

	elm_object_content_set(frame, naviframe);

	evas_object_resize(app->win, 480, 500);
	evas_object_show(app->win);

   pipe = ecore_pipe_add(handler, NULL);

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

	edams_shutdown(app);
	ecore_pipe_del(pipe);
	kill(child_pid, SIGKILL);

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
