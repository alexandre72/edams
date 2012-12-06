/*
 * sensorspicker.c
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


#include "sensors_picker.h"

#include "sensors.h"
#include "utils.h"
#include "path.h"

static Elm_Gengrid_Item_Class *gic;


typedef struct _GenGridItem
{
    Elm_Object_Item *gg_it;
    Sensor *sensor;
} GenGridItem;


Evas_Object *win = NULL;

static char *_gg_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__);
static Evas_Object *_gg_content_get(void *data, Evas_Object *obj, const char *part);
static void _gg_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__);
static void _gg_clickeddouble_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info);
static Eina_Bool _gg_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__);


//
//
//
static char *
_gg_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
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
_gg_content_get(void *data, Evas_Object *obj, const char *part)
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
_gg_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}



//
//
//
static void
_gg_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
	//GenGridItem *ggi = data;
	//FREE(ggi);
}



//
//Display ctxpopup when double-click on an patient's gengrid.
//
static void
_gg_clickeddouble_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
	App_Info *app = data;
   	Evas_Object *gg = (Evas_Object*) elm_object_name_find(win, "sensors gengrid", -1);
   	Elm_Object_Item *it = elm_gengrid_selected_item_get(gg);

   if (!it) return;
	GenGridItem *ggi = elm_object_item_data_get(it);

	room_sensors_add(app->room, ggi->sensor);
	room_save(app->room);

	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	elm_object_item_part_content_set(elm_naviframe_top_item_get(naviframe) , NULL, _room_naviframe_content(app->room));
}



//
//Create sensors picker.
//
Eina_Bool
sensorpicker_add_to_room(App_Info *app)
{
    Evas_Object *grid, *bx, *hbx, *bt, *ic, *sp;

    //Setup a new window.
   	win = elm_win_util_standard_add("sensor_picker", _("Select a sensor"));
   	evas_object_data_set(win, "app", app);
   	elm_win_autodel_set(win, EINA_TRUE);

   	bx = elm_box_add(win);
   	evas_object_show(bx);

	//Setup gengrid to display sensors files in a nice and modern way.
   	grid = elm_gengrid_add(win);
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
    evas_object_smart_callback_add(grid, "clicked,double",_gg_clickeddouble_cb, app);
    elm_box_pack_end(bx, grid);
   	evas_object_show(grid);

    //Add a separator bar to show user's actions like add or remove.
    sp = elm_separator_add(win);
    elm_separator_horizontal_set(sp, EINA_TRUE);
    elm_box_pack_end(bx, sp);
    evas_object_show(sp);

    //Buttons bar.
   	hbx = elm_box_add(win);
   	evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	elm_box_horizontal_set(hbx, EINA_TRUE);
   	elm_box_pack_end(bx, hbx);
   	evas_object_show(hbx);

	bt = elm_button_add(win);
    elm_object_text_set(bt, _("Close"));
	ic = elm_icon_add(win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "close-window");
   	elm_object_part_content_set(bt, "icon", ic);
    elm_box_pack_end(hbx, bt);
    evas_object_show(bt);
    evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb , win);

    sp = elm_separator_add(win);
    elm_separator_horizontal_set(sp, EINA_TRUE);
    evas_object_show(sp);
    elm_box_pack_end(bx, sp);

    //Defines gengrid item class.
   	gic = elm_gengrid_item_class_new();
   	gic->item_style = "default";
   	gic->func.text_get = _gg_text_get;
   	gic->func.content_get = _gg_content_get;
   	gic->func.state_get = _gg_state_get;
   	gic->func.del = _gg_del;

	//Fill gengrid with detected sensors.
    Eina_List *l;
    Sensor *sensor;
    EINA_LIST_FOREACH(app->sensors, l, sensor)
    {
        GenGridItem *ti;
        ti = calloc(1, sizeof(*ti));
        ti->sensor = sensor;
   		ti->gg_it = elm_gengrid_item_append(grid, gic, ti, NULL, NULL);
       	//ti->item = elm_gengrid_item_sorted_insert(grid, gic, ti, compare_cb, grid_sel, NULL);
	}

   	//Item_class_ref is needed for gic.
   	//Some items can be added in callbacks.
   	elm_gengrid_item_class_ref(gic);
   	elm_gengrid_item_class_free(gic);

    //Resize window.
   	evas_object_resize(win, 400, 450);
   	evas_object_show(win);

   	return EINA_TRUE;
}
