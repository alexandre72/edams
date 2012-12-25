#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>


#include "map.h"
#include "edams.h"
#include "path.h"
#include "device.h"
#include "location.h"
#include "utils.h"


#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600



static const int TEMP_MIN =  -30;
static const int TEMP_MAX =  50;


//Globals.
static Ecore_Evas *ee;
static Evas *evas;
static Eina_Rectangle geometry;
static App_Info *app;
Eet_File *ef;

static void _ecore_evas_resize_cb(Ecore_Evas *ee);
static void _on_mouse_in(void *data __UNUSED__, Evas *evas, Evas_Object *o __UNUSED__, void *einfo);
static void _on_mouse_out(void *data __UNUSED__, Evas *evas, Evas_Object *o __UNUSED__, void *einfo);
static void _on_mouse_move(void  *data __UNUSED__, Evas  *evas, Evas_Object *o, void *einfo);

void
map_new(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	app = (App_Info *)data;

    //Set window geometry.
	geometry.w = 0;
	geometry.h = 0;
	geometry.x = 0;
	geometry.y = 0;

	if ((geometry.w <= 0) && (geometry.h <= 0))
	{
		geometry.w = DEFAULT_WIDTH;
		geometry.h = DEFAULT_HEIGHT;
	}

	//Alloc new evas screen.
   	ee = ecore_evas_new(NULL, 0, 0, geometry.w, geometry.h, NULL);
	if (!ee)
	{
		debug(stderr, _("Can't construct Ecore_Evas"));
		return;
	}
    debug(stdout, _("Ecore_Evas use %s engine"), ecore_evas_engine_name_get(ee));

	ecore_evas_shaped_set(ee, 0);
	ecore_evas_borderless_set(ee, 0);
	ecore_evas_title_set(ee, _("Location's map"));
	ecore_evas_callback_resize_set(ee, _ecore_evas_resize_cb);
	evas = ecore_evas_get(ee);

   	if (ecore_evas_ecore_evas_get(evas) == ee)
     	debug(stdout, _("Ecore_Evas has been correctly initalized"));

	Evas_Object *bg;
	if(app->settings->map_background)
	   	bg = evas_object_image_filled_add(evas);
	else
	   	bg = evas_object_rectangle_add(evas);

	evas_object_name_set(bg, "background image");
	evas_object_image_file_set(bg, app->settings->map_background, NULL);
    evas_object_image_alpha_set(bg, EINA_TRUE);
    Evas_Load_Error err = evas_object_image_load_error_get(bg);
    if (err != EVAS_LOAD_ERROR_NONE)
    {
		debug(stderr, _("Can't load image file from '%s'"), app->settings->map_background);
		evas_object_del(bg);
   		bg  = evas_object_rectangle_add(evas);
    }
   	evas_object_move(bg, 0, 0);
   	evas_object_resize(bg, geometry.w, geometry.h);
   	evas_object_show(bg);

	ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_READ_WRITE);

	int i = 0;
    Location *location;
    Eina_List *l;
    EINA_LIST_FOREACH(app->locations, l, location)
	{
			char key[64];
			char s[128];
			char *ret;
			int size;

		 	Evas_Object *table = evas_object_table_add(evas);
			snprintf(s, sizeof(s), "%s table", location_name_get(location));
			evas_object_name_set(table, s);
		   	evas_object_table_homogeneous_set(table, EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE);
		   	evas_object_table_padding_set(table, 0, 0);
		   	evas_object_resize(table, 400, 400);
		   	evas_object_show(table);

			Eina_Rectangle table_geometry;
			snprintf(key, sizeof(key), "map/%s", s);
   			ret = eet_read(ef, key, &size);
   			if(ret)
   			{
	   			sscanf(ret, "%d;%d", &table_geometry.x, &table_geometry.y);
   				free(ret);
   			}
   			else
   			{
   			   	table_geometry.x = ((geometry.w/eina_list_count(app->locations)) +5)*i;
   			   	table_geometry.y = 0;
			}
			evas_object_move(table, table_geometry.x,  table_geometry.y);
			evas_object_show(table);
			evas_object_event_callback_add(table, EVAS_CALLBACK_MOUSE_IN, _on_mouse_in, NULL);
			evas_object_event_callback_add(table, EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out, NULL);
			evas_object_event_callback_add(table, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, NULL);

			Evas_Object *rect = evas_object_rectangle_add(evas);
		   	evas_object_color_set(rect, 255, 0, 0, 125);
		   	evas_object_size_hint_min_set(rect, 100, 40);
		   	evas_object_show(rect);
		   	evas_object_table_pack(table, rect, 1, 1, 2, 1);

			Evas_Object *text = evas_object_text_add(evas);
			evas_object_text_style_set(text, EVAS_TEXT_STYLE_PLAIN);
			evas_object_text_font_set(text, "DejaVu", 20);
			evas_object_text_text_set(text, location_name_get(location));
			evas_object_color_set(text, 0, 0, 0, 255);
		   	evas_object_size_hint_min_set(text, 100, 20);
			evas_object_show(text);
			evas_object_table_pack(table, text, 1, 1, 2, 1);


			Eina_List *l2, *widgets;
			Widget *widget;
			widgets = location_widgets_list_get(location);
			int z = 0;
    		EINA_LIST_FOREACH(widgets, l2, widget)
    		{

 				Evas_Object *edje;
				edje = edje_object_add(evas);
				snprintf(s, sizeof(s), "%d %d %s edje", widget_position_get(widget), widget_device_id_get(widget), location_name_get(location));
				evas_object_name_set(edje, s);

				if (!edje)
				{
					debug(stderr,_("Couldn't create Edje_Object '%s'"), s);
					return;
				}

				if(strstr(widget_name_get(widget), "default"))
				{
					if (!edje_object_file_set(edje, edams_edje_theme_file_get(), "meter/counter"))
					{
						debug(stderr, _("Couldn't load group Edje group '%s' from Edje file '%s'"), widget_name_get(widget), edams_edje_theme_file_get());
						evas_object_del(edje);
						return;
					}
				}
				else
				{
					if (!edje_object_file_set(edje, edams_edje_theme_file_get(), widget_name_get(widget)))
					{
						debug(stderr, _("Couldn't load group Edje group '%s' from Edje file '%s'"), widget_name_get(widget), edams_edje_theme_file_get());
						evas_object_del(edje);
						return;
					}
				}

  				evas_object_table_pack(table, edje, 1, 2+z, 1, 2);
			   	evas_object_size_hint_min_set(edje, 100, 100);
				evas_object_show(edje);
				evas_object_propagate_events_set(edje, EINA_FALSE);
			   	//evas_object_event_callback_add(edje, EVAS_CALLBACK_MOUSE_IN, _on_mouse_in, NULL);
			   	//evas_object_event_callback_add(edje, EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out, NULL);
			   	//evas_object_event_callback_add(edje, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, NULL);
				z++;
			}

			i++;
	}

	ecore_evas_show(ee);
	ecore_main_loop_begin();
	eet_close(ef);
	ecore_evas_free(ee);

	return;
}



void
map_data_update(App_Info *app, Widget *widget)
{
	//Sync device data with location device data(if affected to any location!).
	if(!app)
	return;

	//Update data device meter affected to location(can be affected on different locations).
	Device *device = devices_list_device_with_id_get(app->devices, widget_device_id_get(widget));

	if(device)
    {
    	Eina_List *l;
    	Location *location;

    	EINA_LIST_FOREACH(app->locations, l, location)
    	{
			char s[128];
			snprintf(s, sizeof(s), "%d %d %s edje", widget_position_get(widget), widget_device_id_get(widget), location_name_get(location));
			Evas_Object * edje = evas_object_name_find(evas, s);

			if(edje)
			{
				const char *t;
				//Special layout case(example:temperature values are floats).
				if((t = edje_object_data_get(edje, "tempvalue")))
				{
					int temp_x, temp_y;
			    	sscanf(device_data_get(device), "%d.%02d", &temp_x, &temp_y);

					Edje_Message_Float msg;
					double level =  (double)((temp_x + (temp_y*0.01)) - TEMP_MIN) /
    		          			(double)(TEMP_MAX - TEMP_MIN);

   					if (level < 0.0) level = 0.0;
   					else if (level > 1.0) level = 1.0;
   					msg.val = level;
			    	edje_object_message_send(edje, EDJE_MESSAGE_FLOAT, 1, &msg);
				}


				if((t = edje_object_data_get(edje, "action")))
				{
				 	if(atoi(device_data_get(device)) == 0)
    	       			edje_object_signal_emit(edje, "end", "over");
    		      	else
    	          	 	edje_object_signal_emit(edje, "animate", "over");
				}

				if((t = edje_object_data_get(edje, "title")))
				{
					snprintf(s, sizeof(s), "%d - %s", device_id_get(device), device_name_get(device));
					elm_object_part_text_set(edje, "title.text", s);
				}

				if((t = edje_object_data_get(edje, "value")))
				{
					snprintf(s, sizeof(s), device_unit_format_get(device), device_data_get(device));
					elm_object_part_text_set(edje, "value.text", s);
				}


				edje_object_signal_emit(edje, "updated", "over");
			}
		}
	}
}


static void
_on_mouse_move(void  *data __UNUSED__, Evas  *evas, Evas_Object *o, void *einfo __UNUSED__)
{
    if(evas_object_focus_get(o) == EINA_TRUE)
    {
     	int button_mask;

 		button_mask = evas_pointer_button_down_mask_get(evas);

 		if(button_mask && (1 << 0))
 		{
 		    int x, y;
 		    evas_pointer_canvas_xy_get(evas, &x, &y);
			evas_object_move(o, x, y);


			char s[64];
			char key[64];
			snprintf(s, sizeof(s), "%d;%d", x, y);
			snprintf(key, sizeof(key), "map/%s", evas_object_name_get(o));
			eet_write(ef, key, s, strlen(s) + 1, 0);
 		}
    }
}



static void
_on_mouse_out(void  *data __UNUSED__, Evas  *evas __UNUSED__, Evas_Object *o, void *einfo __UNUSED__)
{
		evas_object_focus_set(o, EINA_FALSE);
}



static void
_on_mouse_in(void  *data __UNUSED__, Evas  *evas __UNUSED__, Evas_Object *o, void *einfo __UNUSED__)
{
	    evas_object_focus_set(o, EINA_TRUE);
}


static void
_ecore_evas_resize_cb(Ecore_Evas *ee)
{
   	ecore_evas_geometry_get(ee, NULL, NULL, &geometry.w, &geometry.h);
   	evas_object_resize(evas_object_name_find(evas, "background image"), geometry.w, geometry.h);
}
