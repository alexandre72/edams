#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>

#include "map.h"
#include "path.h"

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
		CRITICAL(_("Can't construct Ecore_Evas object!"));
		return;
	}
    printf("Using %s engine!\n", ecore_evas_engine_name_get(ee));
	ecore_evas_shaped_set(ee, 0);
	ecore_evas_borderless_set(ee, 0);
	ecore_evas_title_set(ee, _("Room's map"));
	ecore_evas_callback_resize_set(ee, _ecore_evas_resize_cb);
	evas = ecore_evas_get(ee);

   	if (ecore_evas_ecore_evas_get(evas) == ee)
     	printf("Ecore_Evas has been correctly initalized.\n");

   	Evas_Object *bg  = evas_object_image_filled_add(evas);
	evas_object_name_set(bg, "background image");
	evas_object_image_file_set(bg, "./house.png", NULL);
    Evas_Load_Error err = evas_object_image_load_error_get(bg);
    if (err != EVAS_LOAD_ERROR_NONE)
    {
		ERR(_("Can't load Edje image from %s!"), "house.png");
    }

    evas_object_image_alpha_set(bg, EINA_TRUE);
   	evas_object_move(bg, 0, 0);
   	evas_object_resize(bg, geometry.w, geometry.h);
   	evas_object_show(bg);


	ef = eet_open("map.eet", EET_FILE_MODE_READ_WRITE);

    Room *room;
    Eina_List *l;
	int i = 0;
    EINA_LIST_FOREACH(app->rooms, l, room)
	{
			char s[64];
			char *ret;
			int size;

			Eina_Rectangle grid_geometry;

			Evas_Object *grid = evas_object_grid_add(evas);
			snprintf(s, sizeof(s), "%s grid", room_name_get(room));
			evas_object_name_set(grid, s);
			evas_object_grid_size_set(grid, 300, 300);
		 	evas_object_resize(grid, 300,  300);

   			ret = eet_read(ef, s, &size);
   			if(ret)
   			{
	   			sscanf(ret, "%d;%d", &grid_geometry.x, &grid_geometry.y);
   				free(ret);
   			}
   			else
   			{
   			   	grid_geometry.x = ((geometry.w/eina_list_count(app->rooms)) +5)*i;
   			   	grid_geometry.y = 0;
			}
				evas_object_move(grid, grid_geometry.x,  grid_geometry.y);
			evas_object_show(grid);
			evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_IN, _on_mouse_in, NULL);
			evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out, NULL);
			evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, NULL);

			Evas_Object *text = evas_object_text_add(evas);
			evas_object_text_style_set(text, EVAS_TEXT_STYLE_PLAIN);
			evas_object_text_font_set(text, "DejaVu", 20);
			evas_object_text_text_set(text, room_name_get(room));
			evas_object_color_set(text, 120, 15, 30, 255);
			evas_object_grid_pack(grid, text, 0, 0, 100, 30);
			evas_object_show(text);

			Eina_List *l2, *sensors;
			Sensor *sensor;
			sensors = room_sensors_list_get(room);
			int z = 0;
    		EINA_LIST_FOREACH(sensors, l2, sensor)
    		{
 				Evas_Object *edje;
				edje = edje_object_add(evas);
				snprintf(s, sizeof(s), "%d %s edje", sensor_id_get(sensor), room_name_get(room));
				evas_object_name_set(edje, s);

				if (!edje)
				{
					EINA_LOG_CRIT("could not create edje object!");
					return NULL;
				}

				if(!sensor_meter_get(sensor) || strstr(sensor_meter_get(sensor), "default"))
				{
					if (!edje_object_file_set(edje, edams_edje_theme_file_get(), "meter/counter"))
					{
						int err = edje_object_load_error_get(edje);
						const char *errmsg = edje_load_error_str(err);
						EINA_LOG_ERR("Couldn' load 'my_group' from edje_example.edj: %s", errmsg);
						evas_object_del(edje);
						return NULL;
					}
				}
				else
				{
					if (!edje_object_file_set(edje, edams_edje_theme_file_get(), sensor_meter_get(sensor)))
					{
						int err = edje_object_load_error_get(edje);
						const char *errmsg = edje_load_error_str(err);
						EINA_LOG_ERR("Couldn't load 'my_group' from edje_example.edj: %s", errmsg);
						evas_object_del(edje);
						return NULL;
					}
				}

				Eina_Rectangle edje_geometry;
	   			ret = eet_read(ef, s, &size);
  				if(ret)
   				{
	   				sscanf(ret, "%d;%d", &edje_geometry.x, &edje_geometry.y);
   					free(ret);
   				}
   				else
   				{
   					edje_geometry.x = z*5;
   					edje_geometry.y = 30;
   				}
				evas_object_grid_pack(grid, edje, edje_geometry.x , edje_geometry.y, 70, 150);
				evas_object_show(edje);
				evas_object_propagate_events_set(edje, EINA_FALSE);
			   	evas_object_event_callback_add(edje, EVAS_CALLBACK_MOUSE_IN, _on_mouse_in, NULL);
			   	evas_object_event_callback_add(edje, EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out, NULL);
			   	evas_object_event_callback_add(edje, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, NULL);
				z++;
			}
			i++;
	}

	ecore_evas_show(ee);
	ecore_main_loop_begin();
	eet_close(ef);
	ecore_evas_free(ee);
	return 0;
}



void
map_data_update(Sensor *sensor)
{
	//Sync sensor data with room sensor data(if affected to any room!).
	Eina_List *l, *l2, *sensors;
	Room *room;
	Sensor *data;

	if(!app)
	return;


    EINA_LIST_FOREACH(app->rooms, l, room)
    {
		sensors = room_sensors_list_get(room);
    	EINA_LIST_FOREACH(sensors, l2, data)
    	{
			if(sensor_id_get(sensor) == sensor_id_get(data))
			{
				sensor_data_set(data, sensor_data_get(sensor));
			}
    	}
    }


	//Updata data sensor meter affected to room(can be affected on different rooms).
	if(sensor_data_get(sensor))
    {
    	EINA_LIST_FOREACH(app->rooms, l, room)
    	{
			char s[64];
			snprintf(s, sizeof(s), "%d %s edje", sensor_id_get(sensor), room_name_get(room));
			Evas_Object * edje = evas_object_name_find(evas, s);

			if(edje)
			{
				const char *t;

				if((t = edje_object_data_get(edje, "tempvalue")))
				{
					int temp_x, temp_y;
   					snprintf(s, sizeof(s), "%s°C", sensor_data_get(sensor));
    				edje_object_part_text_set(edje, "value.text", s);

			    	sscanf(sensor_data_get(sensor), "%d.%02d", &temp_x, &temp_y);
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
				 	if(atoi(sensor_data_get(sensor)) == 0)
    	       			edje_object_signal_emit(edje, "end", "over");
    		      	else
    	          	 	edje_object_signal_emit(edje, "animate", "over");
				}

				if((t = edje_object_data_get(edje, "title")))
				{
					snprintf(s, sizeof(s), "%d - %s", sensor_id_get(sensor), sensor_name_get(sensor));
					edje_object_part_text_set(edje, "title.text", s);
				}
								if((t = edje_object_data_get(edje, "value")))
				{
					edje_object_part_text_set(edje, "value.text", sensor_data_get(sensor));
				}

				edje_object_signal_emit(edje, "updated", "over");
			}
		}
	}
}


static void
_on_mouse_move(void  *data __UNUSED__, Evas  *evas, Evas_Object *o, void *einfo)
{
    if(evas_object_focus_get(o) == EINA_TRUE)
    {
     	int button_mask, i;

 		button_mask = evas_pointer_button_down_mask_get(evas);

 		if(button_mask && (1 << 0))
 		{
 		    int x, y;
 		    evas_pointer_canvas_xy_get(evas, &x, &y);
			evas_object_move(o, x, y);

			char s[64];
			/*
			if(evas_object_smart_parent_get(o))
			{
				evas_object_smart_calculate(o);
				evas_object_grid_pack_get(evas_object_smart_parent_get(o), o, &x, &y, NULL, NULL);
			}
*/
			snprintf(s, sizeof(s), "%d;%d", x, y);
			eet_write(ef, evas_object_name_get(o), s, strlen(s) + 1, 0);
 		}
    }
}



static void
_on_mouse_out(void  *data __UNUSED__, Evas  *evas, Evas_Object *o, void *einfo)
{
		evas_object_focus_set(o, EINA_FALSE);
}



static void
_on_mouse_in(void  *data __UNUSED__, Evas  *evas, Evas_Object *o, void *einfo)
{
	    evas_object_focus_set(o, EINA_TRUE);
}


static void
_ecore_evas_resize_cb(Ecore_Evas *ee)
{
   	ecore_evas_geometry_get(ee, NULL, NULL, &geometry.w, &geometry.h);
   	evas_object_resize(evas_object_name_find(evas, "background image"), geometry.w, geometry.h);
}
