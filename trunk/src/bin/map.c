#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>

#include "device.h"
#include "gnuplot.h"
#include "location.h"
#include "map.h"
#include "path.h"
#include "utils.h"

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
       debug(stderr, _("No widget data for object %p (%s)"), \
               o, evas_object_type_get(o));                  \
       fflush(stderr);                                       \
       abort();                                              \
       return;                                               \
    }

#define EVAS_SMART_GROUP_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EVAS_SMART_GROUP_DATA_GET(o, ptr);                         \
  if (!ptr)                                                    \
    {                                                          \
       debug(stderr, _("No widget data for object %p (%s)"),   \
               o, evas_object_type_get(o));                    \
       fflush(stderr);                                         \
       abort();                                                \
       return val;                                             \
    }

EVAS_SMART_SUBCLASS_NEW(_evas_smart_group_type, _evas_smart_group,
						Evas_Smart_Class, Evas_Smart_Class,
						evas_object_smart_clipped_class_get, _smart_callbacks);



static Eina_Bool SCREEN_LOCK = EINA_FALSE;

/*Global Evas objects*/
static Ecore_Evas *ee = NULL;
static Evas *evas = NULL;
static Eina_Rectangle geometry;
static App_Info *app;
Eet_File *ef;

Evas_Object *cursor;

static void _ecore_evas_resize_cb(Ecore_Evas * ee);
static void _on_mouse_in(void *data __UNUSED__, Evas * evas, Evas_Object * o __UNUSED__, void *einfo);
static void _on_mouse_out(void *data __UNUSED__, Evas * evas, Evas_Object * o __UNUSED__, void *einfo);
static void _on_mouse_move(void *data __UNUSED__, Evas * evas, Evas_Object * o, void *einfo);
static void _on_keydown(void *data, Evas * evas, Evas_Object * o, void *einfo);

static void map_edition_lock_set(Eina_Bool set);
static void map_cursor_set(const char *cur);

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

    map_cursor_set("cursors/left_ptr");
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
map_cursor_set(const char *cur)
{
    if(cursor)
        evas_object_del(cursor);

 	cursor = edje_object_add(evas);
	if (!cursor)
	{
		debug(stderr, _("Can't create Edje object"));
		return;
	}

    if (!edje_object_file_set(cursor, edams_edje_theme_file_get(), cur))
    {
        debug(stderr, _("Can't load cursor '%s' from '%s'"), cur, edams_edje_theme_file_get());
        return;
    }
	evas_object_resize(cursor, 24, 24);
    evas_object_show(cursor);

    ecore_evas_object_cursor_set(ee, cursor, 0, 0, 0);
}/*map_cursor_set*/



/*
 *
 */
static void
_on_mouse_move(void *data __UNUSED__, Evas * evas, Evas_Object * o, void *einfo __UNUSED__)
{
	if(SCREEN_LOCK) return;

	if (evas_object_focus_get(o) == EINA_TRUE)
	{
        if (evas_object_smart_parent_get(o))
            map_cursor_set("cursors/grab");
        else
            map_cursor_set("cursors/dotbox");

		int button_mask;
		button_mask = evas_pointer_button_down_mask_get(evas);

		if (button_mask && (1 << 0))
		{
            map_cursor_set("cursors/grabbing");

			int x, y;
			evas_pointer_canvas_xy_get(evas, &x, &y);
			Eina_Rectangle prect, crect;

			if (evas_object_smart_parent_get(o))
			{
				EVAS_SMART_GROUP_DATA_GET(evas_object_smart_parent_get(o), priv);

				// Get parent(grid) geometry
				evas_object_geometry_get(priv->border, &prect.x, &prect.y, &prect.w, &prect.h);
				evas_object_geometry_get(o, &crect.x, &crect.y, &crect.w, &crect.h);

				/* FIXME:Check to see ifn't out of the priv->border bounds. Check if children is in
				   fprintf(stdout, "Parent geometry:x=%d y=%d width=%d height=%d\n", prect.x, prect.y,
				   prect.w, prect.h); printf("Current child geometry:x=%d y=%d\n", x, y);
				   fprintf(stdout, "Mouse pointer is in?%s\n", eina_rectangle_coords_inside(&prect, x, y)?
				   "TRUE":"FALSE"); if(x>=prect.w || x<=prect.x) printf("Child is OUT\n"); */

				Widget *widget = evas_object_data_get(o, "widget");
				Device *device = widget_device_get(widget);

				if(device_class_get(device) != VIRTUAL_CLASS)
				{
					if(eina_rectangle_coords_inside(&prect, x, y))
					{
						evas_object_move(o, x, y);

						char s[64];
						char key[64];
						snprintf(s, sizeof(s), "%d;%d", x, y);
						snprintf(key, sizeof(key), "map/%s", evas_object_name_get(o));
						eet_write(ef, key, s, strlen(s) + 1, 0);
					}
				}
				else
				{
						evas_object_move(o, x, y);

						char s[64];
						char key[64];
						snprintf(s, sizeof(s), "%d;%d", x, y);
						snprintf(key, sizeof(key), "map/%s", evas_object_name_get(o));
						eet_write(ef, key, s, strlen(s) + 1, 0);
				}
			}
			else
			{
				evas_object_move(o, x, y);

				char s[64];
				char key[64];
				evas_object_geometry_get(o, &prect.x, &prect.y, &prect.w, &prect.h);
				snprintf(s, sizeof(s), "%d;%d;%d;%d", x, y, prect.w, prect.h);
				snprintf(key, sizeof(key), "map/%s", evas_object_name_get(o));
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
						snprintf(key, sizeof(key), "map/%s", evas_object_name_get(priv->children[x]));
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

	// Set border size/position.
	evas_object_move(priv->border, x, y);
	evas_object_resize(priv->border, w, h);

	// Set location title size/position.
	evas_object_size_hint_min_set(priv->text, 100, 20);
	evas_object_move(priv->text, x, y);

	// Set child's sizes/positions.
	int n = 0;
	for (n = 0; n != MAX_CHILD; n++)
	{
		if (priv->children[n])
		{
			Eina_Rectangle edje_geometry;
			asprintf(&key, "map/%s", evas_object_name_get(priv->children[n]));
			ret = eet_read(ef, key, &size);
			if (ret)
			{
				sscanf(ret, "%d;%d", &edje_geometry.x, &edje_geometry.y);
				free(ret);
			}
			else
			{
				edje_geometry.x = x;
				edje_geometry.y = y;
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
	// Specializing these two
	sc->add = _evas_smart_group_smart_add;
	sc->del = _evas_smart_group_smart_del;
	sc->show = _evas_smart_group_smart_show;
	sc->hide = _evas_smart_group_smart_hide;

	// Clipped smart object has no hook on resizes or calculations
	sc->resize = _evas_smart_group_smart_resize;
	sc->calculate = _evas_smart_group_smart_calculate;
}/*_evas_smart_group_smart_set_user*/




/*
 *
 */
static void
_edje_object_signals_cb(void *data, Evas_Object *edje_obj __UNUSED__, const char  *emission, const char  *source)
{
	Device *device = data;

    //Skip basic's edje signal emission.
	if(strstr(source, "edje")) return;
	if(strstr(emission, "mouse")) return;
	if(strstr(emission, "show")) return;
	if(strstr(emission, "hide")) return;
	if(strstr(emission, "resize")) return;
	if(strstr(emission, "load")) return;

	fprintf(stdout, "emission=%s\n", emission);
	fprintf(stdout, "source=%s\n", source);

    if((device_class_get(device) == VIRTUAL_CLASS))
    {
	    if(strstr(emission, "lock,on")) map_edition_lock_set(EINA_TRUE);
    	if(strstr(emission, "lock,off")) map_edition_lock_set(EINA_FALSE);
    	if(strstr(emission, "home")) map_quit();
    }

    if((device_class_get(device) == CONTROL_BASIC_CLASS))
    {
    	device_current_set(device, emission);
	    xpl_control_basic_cmnd_send(device);
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

	Eina_List *l2, *widgets;
	Widget *widget;
	widgets = location_widgets_list_get(location);

	int x = 0;
	EINA_LIST_FOREACH(widgets, l2, widget)
	{
		char *s;
		priv->children[x] = edje_object_add(evas_object_evas_get(o));
		asprintf(&s, "%d_%s_edje", widget_id_get(widget), location_name_get(location));
		evas_object_name_set(priv->children[x], s);
        FREE(s);
		evas_object_data_set(priv->children[x], "widget", (void *)widget);

		if (!priv->children[x])
		{
			debug(stderr, _("Can't create Edje_Object!"));
			continue;
		}

		if (strstr(widget_name_get(widget), "default"))
		{
			if(!edje_object_file_set(priv->children[x], edams_edje_theme_file_get(), "widget/counter"))
			{
				debug(stderr, _("Can't load group Edje group '%s' from Edje file '%s'"),
					 											 widget_name_get(widget),
					 											 edams_edje_theme_file_get());
				evas_object_del(priv->children[x]);
				continue;
			}
		}
		else
		{
			if (!edje_object_file_set(priv->children[x], edams_edje_theme_file_get(), widget_name_get(widget)))
			{
				debug(stderr, _ ("Can't load group Edje group '%s' from Edje file '%s'"),
					  											  	widget_name_get(widget),
					  							  					edams_edje_theme_file_get());
				evas_object_del(priv->children[x]);
				continue;
			}
		}

		Device *device = widget_device_get(widget);

		if((device_class_get(device) == CONTROL_BASIC_CLASS) ||
            (device_class_get(device) == VIRTUAL_CLASS))
        {
            if (edje_object_data_get(priv->children[x], "title"))
			{
				asprintf(&s, "%d - %s", widget_id_get(widget), device_name_get(device));
				edje_object_part_text_set(priv->children[x], "title.text", s);
                FREE(s);
			}

			edje_object_signal_callback_add(priv->children[x], "*", "*", _edje_object_signals_cb, device);
        }

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
		evas_object_smart_callback_call(o, EVT_CHILDREN_NUMBER_CHANGED,
										(void *)(long)priv->child_count);

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
 *Timer called when adding a new stat_img, must be deleted after some delay to make screen space.
 */
static Eina_Bool
_timer_cb(void *data)
{
	Evas_Object *img = data;

	evas_object_del(img);

	return EINA_FALSE;
}/*_timer_cb*/


/*
 *
 */
static void
widget_gnuplot_redraw(Widget *widget)
{
	Evas_Object *img = evas_object_name_find(evas, "stat_img");
	Device *device = widget_device_get(widget);

	evas_object_image_file_set(img, gnuplot_device_png_write(device), NULL);
	evas_object_image_reload(img);
}/*widget_gnuplot_redraw*/


/*
 *
 */
void
map_edition_lock_set(Eina_Bool set)
{
    SCREEN_LOCK = set;
	debug(stdout, _("Global view edition has been %s\n"), SCREEN_LOCK ? "enabled" : "disabled");
}/*map_edition_lock_set*/


/*
 *Callback called in smart object when a key is pressed
 */
static void
_on_keydown(void *data __UNUSED__, Evas * evas, Evas_Object * o, void *einfo)
{
	Evas_Event_Key_Down *ev = einfo;

	if(!strcmp(ev->keyname, "Escape"))
	{
	   	 	ecore_main_loop_quit();
	   	 	return;
	}

	if(evas_object_smart_parent_get(o))
	{
			// Key 's' show data device in graphics generated from gnuplot(PNG format).
			if (strcmp(ev->keyname, "s") == 0)
			{
				Widget *widget = evas_object_data_get(o, "widget");
				Device *device = widget_device_get(widget);

				if(device)
				{
					Evas_Object *stat_img  = evas_object_image_filled_add(evas);
					evas_object_data_set(o, "stat_img", stat_img);
					evas_object_image_alpha_set(stat_img, EINA_TRUE);
    				evas_object_image_file_set(stat_img, gnuplot_device_png_write(device), NULL);
					Evas_Load_Error err = evas_object_image_load_error_get(stat_img);
    				if (err != EVAS_LOAD_ERROR_NONE)
					{
						debug(stdout, _("Can't load Edje image!"));
						return;
					}

					evas_object_image_scale(stat_img, 600, 300);
					evas_object_move(stat_img, 0,  400);
				    evas_object_show(stat_img);
					ecore_timer_add(2.0, _timer_cb, stat_img);
					evas_object_show(stat_img);
			}
			return;
		}
	}
	else
	{
		if(SCREEN_LOCK) return;

		Eina_Rectangle o_geometry;
		evas_object_geometry_get(o, &o_geometry.x, &o_geometry.y, &o_geometry.w, &o_geometry.h);

		// Keys 'Right' or 'Left' keys are used to increase/decrease width.
		// Keys 'Up' or 'Down' keys are used to increase/decrease height.
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
			snprintf(key, sizeof(key), "map/%s", evas_object_name_get(o));
			eet_write(ef, key, s, strlen(s) + 1, 0);
			return;
		}

		// Key 'd' increase smart object's width and height size
		if (strcmp(ev->keyname, "i") == 0)
		{
			o_geometry.w *= 1.1;
			o_geometry.h *= 1.1;

			evas_object_resize(o, o_geometry.w, o_geometry.h);
			return;
		}

		// Key 'd' decrease smart object's width and height size
		if (strcmp(ev->keyname, "d") == 0)
		{
			o_geometry.w *= 0.9;
			o_geometry.h *= 0.9;

			evas_object_resize(o, o_geometry.w, o_geometry.h);
			return;
		}
/*
   		if (strcmp(ev->keyname, "w") == 0)
		{
			evas_smart_group_remove(o);
			evas_object_del(o);
			return;
		}
*/
	}
}/*_on_keydown*/



/*
 *
 */
void
map_new(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	app = (App_Info *) data;

	// Set window geometry.
	geometry.w = 0;
	geometry.h = 0;
	geometry.x = 0;
	geometry.y = 0;

	if ((geometry.w <= 0) && (geometry.h <= 0))
	{
		geometry.w = DEFAULT_WIDTH;
		geometry.h = DEFAULT_HEIGHT;
	}

	// Alloc new evas screen.
	ee = ecore_evas_new(NULL, 0, 0, geometry.w, geometry.h, NULL);
	if (!ee)
	{
		debug(stderr, _("Can't construct Ecore_Evas"));
		return;
	}

	debug(stdout, _("Using Ecore_Evas '%s' engine"), ecore_evas_engine_name_get(ee));
	ecore_evas_shaped_set(ee, 0);
	ecore_evas_borderless_set(ee, 0);
	ecore_evas_fullscreen_set(ee, EINA_TRUE);
	ecore_evas_title_set(ee, _("Global view"));
	ecore_evas_callback_resize_set(ee, _ecore_evas_resize_cb);
	evas = ecore_evas_get(ee);

	if (ecore_evas_ecore_evas_get(evas) == ee)
		debug(stdout, _("Ecore_Evas has been correctly initalized"));

	Evas_Object *bg;

	if (edams_settings_map_background_get())
		bg = evas_object_image_filled_add(evas);
	else
		bg = evas_object_rectangle_add(evas);

	evas_object_name_set(bg, "background image");
	evas_object_image_file_set(bg, edams_settings_map_background_get(), NULL);
	evas_object_image_alpha_set(bg, EINA_TRUE);
	Evas_Load_Error err = evas_object_image_load_error_get(bg);

	if (err != EVAS_LOAD_ERROR_NONE)
	{
		debug(stderr, _("Can't load image file from '%s'"),	edams_settings_map_background_get());
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
		map_location_add(location);
	}


    /*Install cursor*/
	//Set edje module data.
    map_cursor_set("cursors/left_ptr");

	/*
	cursor = edje_object_add(evas);
	if (!cursor)
	{
		debug(stderr, _("Can't create Edje object!"));
		return;
	}
    map_cursor_set("cursors/left_ptr");
	evas_object_resize(cursor, 24, 24);
    evas_object_show(cursor);
*/
	//FIXME:Should be a widget, can use of swallow type?
	/*
	Evas_Object *stat_img  = evas_object_image_filled_add(evas);
	evas_object_name_set(stat_img, "stat_img");
	evas_object_image_alpha_set(stat_img, EINA_TRUE);
    evas_object_image_file_set(stat_img, "/home/alex/DHT11.6.png", NULL);
	err = evas_object_image_load_error_get(stat_img);
    if (err != EVAS_LOAD_ERROR_NONE)
	{
		debug(stdout, _("Can't load Edje image!"));
	}
	evas_object_image_scale(stat_img, 200, 200);
	evas_object_move(stat_img, 0,  480);
	evas_object_show(stat_img);
*/

	ecore_evas_show(ee);
	ecore_main_loop_begin();
	eet_close(ef);
	ecore_evas_free(ee);
	return;
}/*map_new*/


/*
 *Quit map proprely(free allocated stuff).
 */
void
map_quit()
{
	if (!ee || !app) return;

	ecore_main_loop_quit();
}/*map_quit*/



/*
 *
 */
void
map_location_add(Location *location)
{
	/*TODO:Ok it's great, but it should be better to habe map_location_del too*/
	/*to avoid segfault when a location has been removed*/
	/*I know location associated to smart group no? maybe deleting group should works?*/
	if (!evas || !ee || !ef || !location) return;

	char s[64];
	char key[64];
	char *ret = NULL;
	int size;
	Eina_Rectangle smart_geometry;
	Evas_Object *smt;

	smt = evas_smart_group_add(evas);
	snprintf(s, sizeof(s), "%s_smart", location_name_get(location));
	evas_object_name_set(smt, s);
	snprintf(key, sizeof(key), "map/%s", evas_object_name_get(smt));

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
		smart_geometry.x = geometry.w / 4;
		smart_geometry.y = geometry.h / 4;
		smart_geometry.w = geometry.w / 2;
		smart_geometry.h = geometry.h / 2;
	}
	evas_object_move(smt, smart_geometry.x, smart_geometry.y);
	evas_object_resize(smt, smart_geometry.w, smart_geometry.h);
	evas_smart_group_location_add(smt, location);
	evas_object_show(smt);
}/*map_location_add*/



static const int TEMP_MIN = -30;
static const int TEMP_MAX = 50;


/*
 *Sync map widgets with device data.
 */
void
map_widget_data_update(App_Info * app, Location *location, Device *device)
{
	/*Sync device data with location device data(if affected to any location!)*/
	if (!evas || !ee || !ef || !app || !location || !device) return;

	/*Parse all location widget's and update Edje widget's objects.*/
	Eina_List *l, *widgets;
	Widget *widget;

	widgets = location_widgets_list_get(location);

	EINA_LIST_FOREACH(widgets, l, widget)
	{
		if(strcmp(widget_device_filename_get(widget), device_filename_get(device)) == 0)
		{
			widget_gnuplot_redraw(widget);

			/*Edje widget's object name follow same scheme _widgetid_locationame.*/
			char s[128];
			snprintf(s, sizeof(s), 	"%d_%s_edje",
								 	widget_id_get(widget),
						 			location_name_get(location));
			Evas_Object *edje = evas_object_name_find(evas, s);

			/*Parse Edje widget's messages/actions and update them.*/
			if(!edje) continue;

			const char *t;

			//Special widget with drag part, so need to convert device current value to float.
			if ((t = edje_object_data_get(edje, "drag")))
			{
				double level;
				Edje_Message_Float msg;

				if(device_type_get(device) == TEMP_SENSOR_BASIC_TYPE)
				{
					int x, y;
					sscanf(device_current_get(device), "%d.%02d", &x, &y);
					level =	(double)((x + (y * 0.01)) - TEMP_MIN) / (double)(TEMP_MAX - TEMP_MIN);
				}
				else
				{
					level =	(double)device_current_to_int(device) / (double)(100);
				}

				if (level < 0.0) level = 0.0;
				else if (level > 1.0) level = 1.0;

				msg.val = level;
				edje_object_message_send(edje, EDJE_MESSAGE_FLOAT, 1, &msg);
			}

			if ((device_current_to_int(device) == 0))
				edje_object_signal_emit(edje, "false", "over");
			else
				edje_object_signal_emit(edje, "true", "over");

			if ((t = edje_object_data_get(edje, "title")))
			{
				snprintf(s, sizeof(s), "%d - %s", widget_id_get(widget), device_name_get(device));
				edje_object_part_text_set(edje, "title.text", s);
			}

			if ((t = edje_object_data_get(edje, "value")))
			{
				snprintf(s, sizeof(s), "%4.d%s", device_current_to_int(device), device_unit_symbol_get(device));
				edje_object_part_text_set(edje, "value.text", s);
			}
			edje_object_signal_emit(edje, "updated", "over");
		}
	}
}/*map_data_update*/


/*
 *
 */
Eina_Bool
xpl_control_basic_cmnd_send(Device *device)
{
	debug(stdout, "\
					\ncontrol.basic\n\
					{\n\
					device=%s\n\
 					type=%s\n\
					current=%s\n\
					}\n",
					device_name_get(device),
					device_type_to_str(device_type_get(device)),
					device_current_get(device));

    if ((app->xpl_edams_message_cmnd = xPL_createBroadcastMessage(app->xpl_edams_service, xPL_MESSAGE_COMMAND)) == NULL)
    {
      fprintf(stderr, "Unable to create broadcast message\n");
		return EINA_FALSE;
    }

  	xPL_setSchema(app->xpl_edams_message_cmnd, "control", "basic");

    /*Install the value(s) and send the message*/
  	xPL_setMessageNamedValue(app->xpl_edams_message_cmnd, "device", device_name_get(device));
  	xPL_setMessageNamedValue(app->xpl_edams_message_cmnd, "type", device_type_to_str(device_type_get(device)));
  	xPL_setMessageNamedValue(app->xpl_edams_message_cmnd, "current", device_current_get(device));

	if(device_data1_get(device))
	  	xPL_setMessageNamedValue(app->xpl_edams_message_cmnd, "data1", device_data1_get(device));

	/*Broadcast the message*/
	if (!xPL_sendMessage(app->xpl_edams_message_cmnd))
	{
		fprintf(stderr, "Unable to send xPL message\n");
		return EINA_FALSE;
	}

    return EINA_TRUE;
}/*xpl_control_basic_cmnd_send*/



/*
 *Callback called when resizing Ecore_Evas canvas.
 */
static void
_ecore_evas_resize_cb(Ecore_Evas * ee)
{
	//Resize background image to fit in new canvas size.
	ecore_evas_geometry_get(ee, NULL, NULL, &geometry.w, &geometry.h);
	evas_object_resize(evas_object_name_find(evas, "background image"), geometry.w, geometry.h);
}/*_ecore_evas_resize_cb*/
