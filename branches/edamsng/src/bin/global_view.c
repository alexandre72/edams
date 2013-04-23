/*
 * global_view.c
 * This file is part of EDAMS
 *
 * Copyright (C) 2013 - Alexandre Dussart
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


#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>

#include "edams.h"
#include "global_view.h"
#include "location.h"
#include "path.h"
#include "utils.h"
#include "widget.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

#define _evas_smart_group_type "Evas_Smart_Group"
#define EVT_CHILDREN_NUMBER_CHANGED "children,changed"

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
	{EVT_CHILDREN_NUMBER_CHANGED, "i"},
	{NULL, NULL}
};

typedef struct _Evas_Smart_Group_Data Evas_Smart_Group_Data;


#define MAX_CHILD 50


struct _Evas_Smart_Group_Data
{
	Evas_Object_Smart_Clipped_Data base;
	Evas_Object *children[MAX_CHILD], *border, *text;
	int child_count;
};


#define EVAS_SMART_GROUP_DATA_GET(o, ptr) \
  Evas_Smart_Group_Data * ptr = evas_object_smart_data_get(o)

#define EVAS_SMART_GROUP_DATA_GET_OR_RETURN(o, ptr)        \
  EVAS_SMART_GROUP_DATA_GET(o, ptr);                       \
  if (!ptr)                                                  \
    {                                                        \
       debug(MSG_ERROR, _("No widget data for object %p (%s)"), \
               o, evas_object_type_get(o));                  \
       return;                                               \
    }

#define EVAS_SMART_GROUP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EVAS_SMART_GROUP_DATA_GET(o, ptr);                         \
  if (!ptr)                                                    \
    {                                                          \
       debug(MSG_ERROR, _("No widget data for object %p (%s)"),   \
               o, evas_object_type_get(o));                    \
       return val;                                             \
    }

EVAS_SMART_SUBCLASS_NEW(_evas_smart_group_type, _evas_smart_group,
						Evas_Smart_Class, Evas_Smart_Class,
						evas_object_smart_clipped_class_get, _smart_callbacks);

/*Global objects*/
static Ecore_Evas *ee = NULL;
static Evas *evas = NULL;
static App_Info *app = NULL;
static Eet_File *ef = NULL;

static Evas_Object *cursor = NULL;
static Eina_Bool SCREEN_LOCK = EINA_FALSE;

/*Evas_Object Callbacks*/
static void _ecore_evas_resize_cb(Ecore_Evas * ee);
static void _on_mouse_in(void *data __UNUSED__, Evas * evas, Evas_Object * o __UNUSED__, void *einfo);
static void _on_mouse_out(void *data __UNUSED__, Evas * evas, Evas_Object * o __UNUSED__, void *einfo);
static void _on_mouse_move(void *data __UNUSED__, Evas * evas, Evas_Object * o, void *einfo);
static void _on_keydown(void *data, Evas * evas, Evas_Object * o, void *einfo);

/*Others funcs*/
static void global_view_edition_lock_set(Eina_Bool set);
static void global_view_cursor_set(const char *cur);

/*
 *
 */
static void
_on_child_del(void *data, Evas * evas __UNUSED__, Evas_Object * o, void *einfo __UNUSED__)
{
	Evas_Object *example_smart = data;
	long idx;

	EVAS_SMART_GROUP_DATA_GET(example_smart, priv);

	idx = (long)evas_object_data_get(o, "index");
	idx--;

	priv->children[idx] = NULL;

	evas_object_smart_member_del(o);
	evas_object_smart_changed(example_smart);
}/*_on_child_del*/


/*
 *
 */
static void
_evas_smart_group_child_callbacks_unregister(Evas_Object * obj)
{
	evas_object_data_set(obj, "index", NULL);
	evas_object_event_callback_del(obj, EVAS_CALLBACK_FREE, _on_child_del);
}/*_evas_smart_group_child_callbacks_unregister*/


/*
 *
 */
static void
_evas_smart_group_child_callbacks_register(Evas_Object * o, Evas_Object * child, long idx)
{
	evas_object_event_callback_add(child, EVAS_CALLBACK_FREE, _on_child_del, o);
	evas_object_data_set(child, "index", (void *)(++idx));
}/*_evas_smart_group_child_callbacks_register*/


/*
 *
 */
static void
_on_mouse_out(void *data __UNUSED__, Evas * evas __UNUSED__, Evas_Object * o,
			  void *einfo __UNUSED__)
{
	evas_object_focus_set(o, EINA_FALSE);
    global_view_cursor_set("cursors/left_ptr");
}/*_on_mouse_out*/


/*
 *
 */
static void
_on_mouse_in(void *data __UNUSED__, Evas * evas __UNUSED__, Evas_Object * o, void *einfo __UNUSED__)
{
	evas_object_focus_set(o, EINA_TRUE);
}/*_on_mouse_in*/


/*
 *
 */
static void
global_view_cursor_set(const char *cur)
{
    if(cursor)
        evas_object_del(cursor);

 	cursor = edje_object_add(evas);
	if (!cursor)
	{
		debug(MSG_E, _("Can't create Edje object"));
		return;
	}

    if (!edje_object_file_set(cursor, edams_edje_theme_file_get(), cur))
    {
        debug(MSG_E, _("Can't load Edje cursor '%s' from '%s'"), cur, edams_edje_theme_file_get());
        return;
    }
	evas_object_resize(cursor, 24, 24);
    evas_object_show(cursor);

    ecore_evas_object_cursor_set(ee, cursor, 0, 0, 0);
}/*global_view_cursor_set*/



/*
 *
 */
static void
_on_mouse_move(void *data __UNUSED__, Evas * evas, Evas_Object * o, void *einfo __UNUSED__)
{
	if(SCREEN_LOCK) return;

	if (evas_object_focus_get(o) == EINA_TRUE)
	{
	    if(strstr(evas_object_name_get(o), "_smart"))
            global_view_cursor_set("cursors/dotbox");
        else
            global_view_cursor_set("cursors/grab");

		int button_mask;
		button_mask = evas_pointer_button_down_mask_get(evas);

		if (button_mask && (1 << 0))
		{
            global_view_cursor_set("cursors/grabbing");

			int x, y;
			evas_pointer_canvas_xy_get(evas, &x, &y);
			Eina_Rectangle prect, crect;

	        if(strstr(evas_object_name_get(o), "_edje"))
			{
				    Widget *widget = evas_object_data_get(o, "widget");

                    if(widget_class_get(widget) != WIDGET_CLASS_VIRTUAL)
                    {
    				    EVAS_SMART_GROUP_DATA_GET(evas_object_smart_parent_get(o), priv);

    				    /*Get parent(grid) geometry*/
    				    evas_object_geometry_get(priv->border, &prect.x, &prect.y, &prect.w, &prect.h);
    				    evas_object_geometry_get(o, &crect.x, &crect.y, &crect.w, &crect.h);

                        prect.w = prect.w - crect.w;
                        prect.h = prect.h - crect.h;

    				    if(eina_rectangle_coords_inside(&prect, x, y))
                        {
				            evas_object_move(o, x, y);
    				    }
    				    else
    				    {
        				    return;
    				    }
                    }
                    else
                    {
                        evas_object_move(o, x, y);
                    }
				    char s[64];
					char key[64];
					snprintf(s, sizeof(s), "%d;%d", x, y);
					snprintf(key, sizeof(key), "global_view/%s", evas_object_name_get(o));
					eet_write(ef, key, s, strlen(s) + 1, 0);
			}
            else if(strstr(evas_object_name_get(o), "_smart"))
			{
				evas_object_move(o, x, y);

				char s[64];
				char key[64];
				evas_object_geometry_get(o, &prect.x, &prect.y, &prect.w, &prect.h);
				snprintf(s, sizeof(s), "%d;%d;%d;%d", x, y, prect.w, prect.h);
				snprintf(key, sizeof(key), "global_view/%s", evas_object_name_get(o));
				eet_write(ef, key, s, strlen(s) + 1, 0);

				/*Update childrens geometry of group moved.*/
				EVAS_SMART_GROUP_DATA_GET(o, priv);
				int x;
				for (x = 0; x != MAX_CHILD; x++)
				{
					if (priv->children[x])
					{
						evas_object_geometry_get(priv->children[x], &crect.x, &crect.y, &crect.w, &crect.h);

						snprintf(s, sizeof(s), "%d;%d", crect.x, crect.y);
						snprintf(key, sizeof(key), "global_view/%s", evas_object_name_get(priv->children[x]));
						eet_write(ef, key, s, strlen(s) + 1, 0);
					}
				}
			}
		}
	}
}/*_on_mouse_move*/


/*
 *Create and setup a new example smart object's internals
 */
static void
_evas_smart_group_smart_add(Evas_Object * o)
{
	EVAS_SMART_DATA_ALLOC(o, Evas_Smart_Group_Data);

	// This is a filled rectangle in the smart object's area, delimiting location size and position
	priv->border = evas_object_rectangle_add(evas_object_evas_get(o));
	evas_object_color_set(priv->border, 255, 255, 255, 50);
	evas_object_smart_member_add(priv->border, o);

	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN, _on_mouse_in, NULL);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out, NULL);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, NULL);
	evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

	priv->text = evas_object_text_add(evas);
	evas_object_text_style_set(priv->text, EVAS_TEXT_STYLE_PLAIN);
	evas_object_text_font_set(priv->text, "DejaVu", 20);
	evas_object_color_set(priv->text, 0, 0, 0, 255);
	evas_object_smart_member_add(priv->text, o);

	// Add border to smart
	_evas_smart_group_parent_sc->add(o);
}/*_evas_smart_group_add*/


static void
_evas_smart_group_smart_del(Evas_Object * o)
{
	EVAS_SMART_GROUP_DATA_GET(o, priv);

	int x = 0;
	for (x = 0; x != MAX_CHILD; x++)
	{
		if (priv->children[x])
		{
			_evas_smart_group_child_callbacks_unregister(priv->children[0]);
			priv->children[x] = NULL;
		}
	}

	_evas_smart_group_parent_sc->del(o);
}/*_evas_smart_group_smart_del*/


/*
 *
 */
static void
_evas_smart_group_smart_show(Evas_Object * o)
{
	EVAS_SMART_GROUP_DATA_GET(o, priv);

	int x = 0;
	for (x = 0; x != MAX_CHILD; x++)
	{
		if (priv->children[x])
			evas_object_show(priv->children[x]);
	}

	evas_object_show(priv->border);
	evas_object_show(priv->text);
	_evas_smart_group_parent_sc->show(o);
}/*_evas_smart_group_smart_show*/


/*
 *
 */
static void
_evas_smart_group_smart_hide(Evas_Object * o)
{
	EVAS_SMART_GROUP_DATA_GET(o, priv);

	int x = 0;
	for (x = 0; x != MAX_CHILD; x++)
	{
		if (priv->children[x])
			evas_object_hide(priv->children[x]);
	}

	evas_object_hide(priv->border);

	_evas_smart_group_parent_sc->hide(o);
}/*_evas_smart_group_smart_hide*/


/*
 *
 */
static void
_evas_smart_group_smart_resize(Evas_Object * o, Evas_Coord w, Evas_Coord h)
{
	Evas_Coord ow, oh;
	evas_object_geometry_get(o, NULL, NULL, &ow, &oh);
	if ((ow == w) && (oh == h))
		return;

	//Trigger recalculation.
	evas_object_smart_changed(o);
}/*_evas_smart_group_smart_resize*/


/*
 * Act on child objects' properties, before rendering
 */
static void
_evas_smart_group_smart_calculate(Evas_Object * o)
{
	char *key;
	char *ret;
	int size;
	Evas_Coord x, y, w, h;


	EVAS_SMART_GROUP_DATA_GET_OR_RETURN(o, priv);
	evas_object_geometry_get(o, &x, &y, &w, &h);

	/*Set border size/position*/
	evas_object_move(priv->border, x, y);
	evas_object_resize(priv->border, w, h);

	/*Set location title size/position*/
	evas_object_size_hint_min_set(priv->text, 100, 20);
	evas_object_move(priv->text, x, y);

	/*Set child's sizes/positions*/
	int n = 0;
	for (n = 0; n != MAX_CHILD; n++)
	{
		if (priv->children[n])
		{
			Eina_Rectangle edje_geometry;
			asprintf(&key, "global_view/%s", evas_object_name_get(priv->children[n]));
			ret = eet_read(ef, key, &size);
			if (ret)
			{
				sscanf(ret, "%d;%d", &edje_geometry.x, &edje_geometry.y);
				free(ret);
			}
			else
			{
				edje_geometry.x = x+50;
				edje_geometry.y = y+50;
			}
			evas_object_move(priv->children[n], edje_geometry.x, edje_geometry.y);
			FREE(key);

			edje_object_size_min_get(priv->children[n], &w, &h);
			edje_object_size_max_get(priv->children[n], &w, &h);
			evas_object_resize(priv->children[n], w, h);
			evas_object_event_callback_add(priv->children[n], EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);
		}
	}
}/*_evas_smart_group_smart_calculate*/

/*
 * Setup our smart interface
 */
static void
_evas_smart_group_smart_set_user(Evas_Smart_Class * sc)
{
	/*Specializing these two*/
	sc->add = _evas_smart_group_smart_add;
	sc->del = _evas_smart_group_smart_del;
	sc->show = _evas_smart_group_smart_show;
	sc->hide = _evas_smart_group_smart_hide;

	/*Clipped smart object has no hook on resizes or calculations*/
	sc->resize = _evas_smart_group_smart_resize;
	sc->calculate = _evas_smart_group_smart_calculate;
}/*_evas_smart_group_smart_set_user*/


/*
 *
 */
static void
_edje_object_signals_cb(void *data, Evas_Object *edje_obj, const char  *emission, const char  *source)
{
	Widget *widget = data;
    double val = 0;
    char *s = NULL;
    const char *type = widget_device_type_get(widget);

    /*Skip basic's edje signal emission*/
	if(strstr(source, "edje")) return;
	if(strstr(emission, "mouse")) return;
	if(strstr(emission, "show")) return;
	if(strstr(emission, "hide")) return;
	if(strstr(emission, "resize")) return;
	if(strstr(emission, "load")) return;
	if(strstr(emission, "updated")) return;
	if(strstr(emission, "drag,stop")) return;

    if((widget_class_get(widget) == WIDGET_CLASS_SENSOR))
    {
	    if(strstr(emission, "sensor.basic,cmnd"))  device_sensor_cmnd_send(widget);
    }
    else if((widget_class_get(widget) == WIDGET_CLASS_VIRTUAL))
    {
        /*Handle virtual widgets, special system actions*/
	    if(strstr(emission, "lock,on")) global_view_edition_lock_set(EINA_TRUE);
    	if(strstr(emission, "lock,off")) global_view_edition_lock_set(EINA_FALSE);
    	if(strstr(emission, "home")) global_view_quit();
    }
    else if((widget_class_get(widget) == WIDGET_CLASS_CONTROL))
    {
        if(strstr(emission, "drag"))
        {
            Edje_Drag_Dir dir = edje_object_part_drag_dir_get(edje_obj, source);

            if(dir == EDJE_DRAG_DIR_X)
                edje_object_part_drag_value_get(edje_obj, source, &val, NULL);
            else if(dir == EDJE_DRAG_DIR_Y)
                edje_object_part_drag_value_get(edje_obj, source, NULL, &val);
            //else if(dir == EDJE_DRAG_DIR_XY)
            // edje_object_part_drag_value_get(o, source, &hval, &vval);
        }
        else
        {
            val = atoi(emission);
        }

        /*Scale to device type format*/
        if(strcmp(type, DEVICE_TYPE_PERCENTAGE_CONTROL) == 0)
        {
            val = (val * 100);
            asprintf(&s, "%d%%", (int)val);
        }
        else
        {
            asprintf(&s, "%d", (int)val);
        }

        widget_device_current_set(widget, s);
	    edje_object_part_text_set(edje_obj, "value.text", s);
	    FREE(s);

	    device_control_cmnd_send(widget);
    }
}/*_edje_object_signals_cb*/



/*
 *
 *BEGINS smart object's own interface
 *
 */
Eina_List *
evas_smart_group_location_add(Evas_Object * o, Location * location)
{
	Eina_List *childs = NULL;

	EVAS_SMART_GROUP_DATA_GET_OR_RETURN_VAL(o, priv, NULL);

	evas_object_text_text_set(priv->text, location_name_get(location));

	Eina_List *l, *widgets = NULL;
	Widget *widget;
	widgets = location_widgets_list_get(location);

	int x = 0;
	EINA_LIST_FOREACH(widgets, l, widget)
	{
        if((widget_class_get(widget) == WIDGET_CLASS_VIRTUAL)) continue;

		char *s;
		priv->children[x] = edje_object_add(evas_object_evas_get(o));
		asprintf(&s, "%d_%s_edje", widget_id_get(widget), location_name_get(location));
		evas_object_name_set(priv->children[x], s);
        FREE(s);
		evas_object_data_set(priv->children[x], "widget", (void *)widget);

		if (!priv->children[x])
		{
			debug(MSG_E, _("Can't create Edje_Object object"));
			continue;
		}

		if (!edje_object_file_set(priv->children[x], edams_edje_theme_file_get(), widget_group_get(widget)))
		{
			debug(MSG_E, _ ("Can't load group Edje group '%s' from Edje file '%s'"),
		                                        				  	widget_group_get(widget),
					  							  					edams_edje_theme_file_get());
			evas_object_del(priv->children[x]);
			continue;
		}

        if(edje_object_part_exists(priv->children[x], "title.text"))
		{
			edje_object_part_text_set(priv->children[x], "title.text", widget_name_get(widget));
		}
    	edje_object_signal_callback_add(priv->children[x], "*", "*", _edje_object_signals_cb, widget);

		evas_object_propagate_events_set(priv->children[x], EINA_FALSE);
		evas_object_event_callback_add(priv->children[x], EVAS_CALLBACK_MOUSE_IN, _on_mouse_in, NULL);
		evas_object_event_callback_add(priv->children[x], EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out, NULL);
		evas_object_event_callback_add(priv->children[x], EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, NULL);

		_evas_smart_group_child_callbacks_register(o, priv->children[x], 0);
		evas_object_smart_member_add(priv->children[x], o);
		evas_object_smart_changed(o);
		priv->child_count++;

		childs = eina_list_append(childs, priv->children[x]);
		x++;
	}

	if (!childs)
		evas_object_smart_callback_call(o, EVT_CHILDREN_NUMBER_CHANGED, (void *)(long)priv->child_count);

	return childs;
}/*evas_smart_group_location_add*/




/*
 *Add a new smart object to a canvas.
 */
Evas_Object *
evas_smart_group_add(Evas * evas)
{
	return evas_object_smart_add(evas, _evas_smart_group_smart_class_new());
}/*evas_smart_group_add*/



/*
 *Remove a child element, return its pointer (or NULL on errors)
 */
Evas_Object *
evas_smart_group_remove(Evas_Object * o )
{
	EVAS_SMART_GROUP_DATA_GET_OR_RETURN_VAL(o, priv, NULL);

	int n = 0;
	for (n = 0; n != MAX_CHILD; n++)
	{
		if (priv->children[n])
		{
			_evas_smart_group_child_callbacks_unregister(priv->children[n]);
			evas_object_smart_member_del(priv->children[n]);
			priv->children[n] = NULL;
			priv->child_count--;
		}
	}

	evas_object_smart_callback_call(o, EVT_CHILDREN_NUMBER_CHANGED, (void *)(long)priv->child_count);
	evas_object_smart_changed(o);

	return NULL;
}/*evas_smart_group_remove*/



/*
 *
 */
void
global_view_edition_lock_set(Eina_Bool set)
{
    SCREEN_LOCK = set;
	debug(MSG_INFO, _("Global view edition has been %s\n"), SCREEN_LOCK ? "enabled" : "disabled");
}/*global_view_edition_lock_set*/




static Evas_Object *line[100];
static Evas_Object *mid;
static int column = 0;

/*
 *
 */
void
grapher_value_add(int value)
{
    if(column == 100)
    {
        column = 0;
        int x;
        for(x = 0; x != 100;x++)
        {
            evas_object_color_set(line[x], 255, 255, 255, 255);
            evas_object_line_xy_set(line[x], column, 100, column, 100);
        }
    }

    if(value > 50) value = 50;

    evas_object_color_set(line[column], 255, 0, 255, 255);
    evas_object_line_xy_set(line[column], column, (100/2), column, (100/2)-value);
    column++;
}/*grapher_value_add*/


/*
 *
 */
void
grapher_show()
{
    int x;

    for(x = 0; x != 100;x++)
        evas_object_show(line[x]);

    evas_object_show(mid);
}/*grapher_show*/


/*
 *
 */
static Eina_Bool
grapher_hide(void *data __UNUSED__)
{
    int x;

    for(x = 0; x != 100;x++)
        evas_object_hide(line[x]);

    evas_object_hide(mid);

    return EINA_FALSE;
}/*grapher_hide*/

/*
 *
 */
void
grapher_init()
{
    int x;

    for(x = 0; x != 100;x++)
    {
        line[x] = evas_object_line_add(evas);
    }

    mid = evas_object_line_add(evas);
    evas_object_color_set(mid, 10, 255, 10, 255);
    evas_object_line_xy_set(mid, 0, (100/2), 100, (100/2));
}/*grapher_init*/



/*
 *
 */
void
grapher_redraw(int value)
{
    grapher_value_add(value);
}/*grapher_redraw*/



/*
 *Callback called in smart object when a key is pressed
 */
static void
_on_keydown(void *data __UNUSED__, Evas * evas __UNUSED__, Evas_Object * o, void *einfo)
{
	Evas_Event_Key_Down *ev = einfo;

	if(!strcmp(ev->keyname, "Escape"))
	{
	   	 	ecore_main_loop_quit();
	   	 	return;
	}

	if(evas_object_smart_parent_get(o))
	{
		/*Key 's' show data device in grapher widget*/
		if (strcmp(ev->keyname, "s") == 0)
		{
			Widget *widget = evas_object_data_get(o, "widget");

			if(widget)
			{
                grapher_show();
                grapher_redraw(5);
				ecore_timer_add(2.0, grapher_hide, NULL);
			}
		}
		return;
	}
	else
	{
		if(SCREEN_LOCK) return;

		Eina_Rectangle o_geometry;
		evas_object_geometry_get(o, &o_geometry.x, &o_geometry.y, &o_geometry.w, &o_geometry.h);

		/*Keys 'Right' or 'Left' keys are used to increase/decrease width*/
		/*Keys 'Up' or 'Down' keys are used to increase/decrease height*/
		if (strcmp(ev->keyname, "Right") == 0 || strcmp(ev->keyname, "Left") == 0
			|| strcmp(ev->keyname, "Up") == 0 || strcmp(ev->keyname, "Down") == 0)
		{
			switch (ev->keyname[0])
			{
			case 'R':
				o_geometry.w *= 1.02;
				break;
			case 'L':
				o_geometry.w *= 0.98;
				break;
			case 'U':
				o_geometry.h *= 1.02;
				break;
			case 'D':
				o_geometry.h *= 0.98;
				break;
			}

			evas_object_resize(o, o_geometry.w, o_geometry.h);
			char s[64];
			char key[64];
			snprintf(s, sizeof(s), "%d;%d;%d;%d", o_geometry.x, o_geometry.y, o_geometry.w, o_geometry.h);
			snprintf(key, sizeof(key), "global_view/%s", evas_object_name_get(o));
			eet_write(ef, key, s, strlen(s) + 1, 0);
			return;
		}

		/*Key 'i' increase smart object's width and height size*/
		if (strcmp(ev->keyname, "i") == 0)
		{
			o_geometry.w *= 1.1;
			o_geometry.h *= 1.1;

			evas_object_resize(o, o_geometry.w, o_geometry.h);
			return;
		}

		/*Key 'd' decrease smart object's width and height size*/
		if (strcmp(ev->keyname, "d") == 0)
		{
			o_geometry.w *= 0.9;
			o_geometry.h *= 0.9;

			evas_object_resize(o, o_geometry.w, o_geometry.h);
			return;
		}
	}
}/*_on_keydown*/


/*
 *
 */
static void
_virtual_widgets_add(Location *location)
{
	Eina_List *l2, *widgets = NULL;
    Widget *widget;
    char *key;
	char *ret;
	int size;
	Evas_Coord w, h;

	widgets = location_widgets_list_get(location);
    EINA_LIST_FOREACH(widgets, l2, widget)
    {
        if((widget_class_get(widget) != WIDGET_CLASS_VIRTUAL)) continue;
		char *s;
    	Evas_Object *virtual;

        virtual = edje_object_add(evas);
		asprintf(&s, "%d_%s_edje", widget_id_get(widget), location_name_get(location));
		evas_object_name_set(virtual, s);
        FREE(s);
		evas_object_data_set(virtual, "widget", (void *)widget);

		if (!virtual)
		{
			debug(MSG_E, _("Can't create Edje_Object object"));
			continue;
		}

		if (!edje_object_file_set(virtual, edams_edje_theme_file_get(), widget_group_get(widget)))
		{
			debug(MSG_E, _ ("Can't load group Edje group '%s' from Edje file '%s'"),
		                                        				widget_group_get(widget),
					  							  				edams_edje_theme_file_get());
			 evas_object_del(virtual);
			  continue;
		  }

		evas_object_propagate_events_set(virtual, EINA_FALSE);
    	edje_object_signal_callback_add(virtual, "*", "*", _edje_object_signals_cb, widget);
		evas_object_event_callback_add(virtual, EVAS_CALLBACK_MOUSE_IN, _on_mouse_in, virtual);
		evas_object_event_callback_add(virtual, EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out, virtual);
		evas_object_event_callback_add(virtual, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, virtual);

		Eina_Rectangle edje_geometry;
		asprintf(&key, "global_view/%s", evas_object_name_get(virtual));
		ret = eet_read(ef, key, &size);
		if (ret)
		{
		    sscanf(ret, "%d;%d", &edje_geometry.x, &edje_geometry.y);
			free(ret);
		}
		else
		{
			edje_geometry.x = 100;
			edje_geometry.y = 100;
		}
		evas_object_move(virtual, edje_geometry.x, edje_geometry.y);
		FREE(key);

		edje_object_size_min_get(virtual, &w, &h);
		edje_object_size_max_get(virtual, &w, &h);
		evas_object_resize(virtual, w, h);
		evas_object_show(virtual);
    }
}/*_virtual_widgets_add*/


/*
 *
 */
void
global_view_new(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	app = (App_Info *) data;
    Eina_Rectangle geometry;


	/*Set window geometry*/
	geometry.w = 0;
	geometry.h = 0;
	geometry.x = 0;
	geometry.y = 0;

	if ((geometry.w <= 0) && (geometry.h <= 0))
	{
		geometry.w = DEFAULT_WIDTH;
		geometry.h = DEFAULT_HEIGHT;
	}

	/*Alloc new Ecore_Evas*/
	ee = ecore_evas_new(NULL, 0, 0, geometry.w, geometry.h, NULL);
	if (!ee)
	{
		debug(MSG_E, _("Can't construct Ecore_Evas object"));
		return;
	}

	ecore_evas_callback_resize_set(ee, _ecore_evas_resize_cb);
	ecore_evas_shaped_set(ee, EINA_FALSE);
	ecore_evas_borderless_set(ee, EINA_FALSE);
	ecore_evas_fullscreen_set(ee, EINA_TRUE);
	ecore_evas_title_set(ee, _("Global view"));

	evas = ecore_evas_get(ee);
	ecore_evas_geometry_get(ee, NULL, NULL, &geometry.w, &geometry.h);


	if (ecore_evas_ecore_evas_get(evas) == ee)
	    debug(MSG_E, _("Using Ecore_Evas '%s' engine"), ecore_evas_engine_name_get(ee));

	Evas_Object *bg;

	if (edams_settings_global_view_background_get())
		bg = evas_object_image_filled_add(evas);
	else
		bg = evas_object_rectangle_add(evas);

	evas_object_name_set(bg, "background image");
	evas_object_image_file_set(bg, edams_settings_global_view_background_get(), NULL);
	evas_object_image_alpha_set(bg, EINA_TRUE);
	Evas_Load_Error err = evas_object_image_load_error_get(bg);

	if (err != EVAS_LOAD_ERROR_NONE)
	{
		debug(MSG_E, _("Can't load image file from '%s'"),	edams_settings_global_view_background_get());
		evas_object_del(bg);
		bg = evas_object_rectangle_add(evas);
	}
	evas_object_move(bg, 0, 0);
	evas_object_resize(bg, geometry.w, geometry.h);
	evas_object_show(bg);

	ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_READ_WRITE);

	Location *location;
	Eina_List *l;
	EINA_LIST_FOREACH(app->locations, l, location)
	{
		global_view_location_add(location);

		/*Now add virtual widget's, they're not dependents to a smart_group*/
        _virtual_widgets_add(location);
	}

    /*Install normal cursor*/
    global_view_cursor_set("cursors/left_ptr");

	Evas_Object *osd_text = evas_object_text_add(evas);
	evas_object_name_set(osd_text, "osd text");
	evas_object_text_style_set(osd_text, EVAS_TEXT_STYLE_PLAIN);
	evas_object_text_font_set(osd_text, "DejaVu", 20);
	evas_object_color_set(osd_text, 0, 0, 255, 255);
	evas_object_resize(osd_text, 100, 20);
    evas_object_move(osd_text, 0, geometry.h / 2);

	//FIXME:Grapher should be a widget, can use of swallow type?
    grapher_init();

	ecore_evas_show(ee);
	ecore_main_loop_begin();
	eet_close(ef);
	ecore_evas_free(ee);
	return;
}/*global_view_new*/


/*
 *Quit global_view proprely(free allocated stuff).
 */
void
global_view_quit()
{
	if (!ee || !app) return;

	ecore_main_loop_quit();
}/*global_view_quit*/


/*
 *
 */
void
global_view_location_del(Location *location)
{
	Evas_Object *smt;
    char *s;
	if (!evas || !location) return;

	asprintf(&s,  "%s_smart", location_name_get(location));
   	smt = evas_object_name_find(evas, s);
	FREE(s);

	if(smt)
	{
    	evas_smart_group_remove(smt);
    	evas_object_del(smt);
    }
}/*global_view_location_del*/



/*
 *
 */
void
global_view_location_add(Location *location)
{
	Evas_Object *smt;
	char s[64];
	char key[64];
	char *ret = NULL;
	int size;
	Eina_Rectangle geometry, smart_geometry;

	if (!evas || !ef || !location) return;

	smt = evas_smart_group_add(evas);
	snprintf(s, sizeof(s), "%s_smart", location_name_get(location));
	evas_object_name_set(smt, s);
	snprintf(key, sizeof(key), "global_view/%s", evas_object_name_get(smt));

	ret = eet_read(ef, key, &size);
	if (ret)
	{
		sscanf(ret, "%d;%d;%d;%d",
					&smart_geometry.x,
					&smart_geometry.y,
					&smart_geometry.w,
					&smart_geometry.h);
		free(ret);
	}
	else
	{
        ecore_evas_geometry_get(ee, NULL, NULL, &geometry.w, &geometry.h);
		smart_geometry.x = geometry.w / 4;
		smart_geometry.y = geometry.h / 4;
		smart_geometry.w = geometry.w / 2;
		smart_geometry.h = geometry.h / 2;
	}
	evas_object_move(smt, smart_geometry.x, smart_geometry.y);
	evas_object_resize(smt, smart_geometry.w, smart_geometry.h);
	evas_smart_group_location_add(smt, location);
	evas_object_show(smt);
}/*global_view_location_add*/


/*
 *Sync global_view widgets with xPL device's current.
 */
void
global_view_widget_data_update(Location *location, Widget *widget)
{
	if (!evas || !location || !widget) return;

	/*Update Edje widget objects.*/
    grapher_redraw(atoi(widget_device_current_get(widget)));

    /*All Edje widget's object name follow same scheme _widgetid_locationame_edje*/
	char s[128];
    snprintf(s, sizeof(s), 	"%d_%s_edje",
								 	widget_id_get(widget),
						 			location_name_get(location));

	Evas_Object *edje = evas_object_name_find(evas, s);

	/*Parse Edje widget's messages/actions and update them*/
	if(!edje) return;

	const char *t;

	/*Special widget with drag part, so need to convert device current value to float*/
	if ((t = edje_object_data_get(edje, "drag")))
	{
    	double level;
    	Edje_Message_Float msg;

	    if(strcmp(widget_device_type_get(widget), DEVICE_TYPE_TEMP_SENSOR) == 0)
	    {
			int x, y;
			sscanf(widget_device_current_get(widget), "%d.%02d", &x, &y);
			level =	(double)((x + (y * 0.01)) - device_type_current_min_get(widget_device_type_get(widget))) / (double)(device_type_current_max_get(widget_device_type_get(widget)) - device_type_current_min_get(widget_device_type_get(widget)));
		}
		else
		{
	         level = (double)atoi(widget_device_current_get(widget)) / 100;
		}

    	if (level < 0.0) level = 0.0;
		else if (level > 1.0) level = 1.0;

		msg.val = level;
		edje_object_message_send(edje, EDJE_MESSAGE_FLOAT, 1, &msg);
	}

    edje_object_signal_emit(edje, widget_device_current_get(widget), "whole");

	if(edje_object_part_exists(edje, "title.text"))
	{
		edje_object_part_text_set(edje, "title.text", widget_name_get(widget));
	}

	if(edje_object_part_exists(edje, "value.text"))
	{
	   	snprintf(s, sizeof(s), "%s%s",
	   	        widget_device_current_get(widget),
        	   	device_type_to_unit_symbol(widget_device_type_get(widget)) ? device_type_to_unit_symbol(widget_device_type_get(widget)) : "");
		edje_object_part_text_set(edje, "value.text", s);
	}

    if(edje_object_part_exists(edje, "text.highest"))
	{
	    edje_object_part_text_set(edje, "text.highest", widget_device_highest_get(widget));
	}

    if(edje_object_part_exists(edje, "text.lowest"))
	{
	    edje_object_part_text_set(edje, "text.lowest", widget_device_lowest_get(widget));
	}


	edje_object_signal_emit(edje, "updated", "whole");
}/*global_view_data_update*/


/*
 *Timer called when changing OSD and must be hidden after some delay.
 */
static Eina_Bool
_osd_timer_cb(void *data)
{
	Evas_Object *obj = data;

	evas_object_hide(obj);

	return EINA_FALSE;
}/*_timer_cb*/


/*
 *
 */
void global_view_osd_write(const char *text, double delay)
{
	if (!evas) return;

    Evas_Object *osd_text = evas_object_name_find(evas, "osd text");

    if(text)
        evas_object_text_text_set(osd_text, text);
    else
        evas_object_text_text_set(osd_text, NULL);

    evas_object_show(osd_text);

    if(delay != -1)
        ecore_timer_add(delay, _osd_timer_cb, osd_text);
}



/*
 *
 */
void
global_view_new_mail_emit(int num_new, int num_total)
{
	if (!evas) return;

    Eina_List *l, *l2;
	Location *location;
	EINA_LIST_FOREACH(app->locations, l, location)
	{
        Widget *widget;
        Eina_List *widgets = location_widgets_list_get(location);
        EINA_LIST_FOREACH(widgets, l2, widget)
        {
            if((widget_class_get(widget) != WIDGET_CLASS_VIRTUAL)) continue;

            if(strstr(widget_group_get(widget), "widget/virtual/mail"))
            {
                char *s;
                asprintf(&s, "%d_%s_edje",
								 	    widget_id_get(widget),
						 			    location_name_get(location));

                Evas_Object *mail_obj = evas_object_name_find(evas, s);
                FREE(s);

                edje_object_part_text_set(mail_obj, "title.text", ecore_file_file_get(edams_settings_mbox_path_get()));

                if(!mail_obj) continue;

                if (num_new > 0)
                {
                    asprintf(&s, "%d", num_total);
                    edje_object_part_text_set(mail_obj, "mail.count.total.text", s);
                    FREE(s);
                    asprintf(&s, "%d", num_new);
                    edje_object_part_text_set(mail_obj, "mail.count.unseen.text", s);
                    FREE(s);
                    edje_object_signal_emit(mail_obj, "mail,new", "whole");
                }
                else
                {
                    edje_object_part_text_set(mail_obj, "mail.count.unseen.text", "");
                    edje_object_part_text_set(mail_obj, "mail.count.total.text", "");
                    edje_object_signal_emit(mail_obj, "mail,empty", "whole");
                }
            }
        }
    }
}

/*
 *Callback called when resizing Ecore_Evas canvas.
 */
static void
_ecore_evas_resize_cb(Ecore_Evas * ee)
{
    Eina_Rectangle geometry;
    Evas_Object *bg;

	/*Resize background image to fit in new canvas size*/
	ecore_evas_geometry_get(ee, NULL, NULL, &geometry.w, &geometry.h);
    bg = evas_object_name_find(evas, "background image");
	evas_object_resize(bg, geometry.w, geometry.h);
}/*_ecore_evas_resize_cb*/
