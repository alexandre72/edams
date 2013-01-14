/*
 * devicespicker.c
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

#include <Elementary.h>

#include "devices_picker.h"
#include "device.h"
#include "edams.h"
#include "location.h"
#include "path.h"
#include "utils.h"

static Elm_Gengrid_Item_Class *gic;


typedef struct _GenGridItem
{
	Elm_Object_Item *gg_it;
	Device *device;
} GenGridItem;


Evas_Object *win = NULL;

static char *_gengrid_devices_text_get(void *data, Evas_Object * obj __UNUSED__, const char *part __UNUSED__);
static Evas_Object *_gengrid_devices_content_get(void *data, Evas_Object * obj, const char *part);
static Eina_Bool _gengrid_devices_state_get(void *data __UNUSED__, Evas_Object * obj __UNUSED__, const char *part __UNUSED__);
static void _gengrid_devices_del(void *data __UNUSED__, Evas_Object * obj __UNUSED__);
static void _button_select_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);


/*
 *
 */
static char *
_gengrid_devices_text_get(void *data, Evas_Object * obj __UNUSED__, const char *part __UNUSED__)
{
	const GenGridItem *ti = data;
	char buf[256];

	snprintf(buf, sizeof(buf), "%d - %s", device_id_get(ti->device), device_name_get(ti->device));

	return strdup(buf);
}/*_gengrid_devices_text_get*/


/*
 *
 */
static Evas_Object *
_gengrid_devices_content_get(void *data, Evas_Object * obj, const char *part)
{
	const GenGridItem *ti = data;

	if (!strcmp(part, "elm.swallow.icon"))
	{
		Evas_Object *icon = elm_bg_add(obj);
		if (!elm_bg_file_set(icon, device_filename_get(ti->device), "/image/0"))
			elm_bg_file_set(icon, edams_edje_theme_file_get(), "elm/icon/device/default");
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		evas_object_show(icon);
		return icon;
	}

	return NULL;
}/*_gengrid_devices_content_get*/



/*
 *
 */
static Eina_Bool
_gengrid_devices_state_get(void *data __UNUSED__, Evas_Object * obj __UNUSED__, const char *part __UNUSED__)
{
	return EINA_FALSE;
}/*_gengrid_devices_state_get*/



/*
 *
 */
static void
_gengrid_devices_del(void *data __UNUSED__, Evas_Object * obj __UNUSED__)
{
	GenGridItem *ti = data;
	device_free(ti->device);
	FREE(ti);
}/*_gengrid_devices_del*/



/*
 *Callback called in gengrid "devices" when clickeddouble signal is emitted.
 *Callback called in button "select" when clicked signal is emitted.
 */
static void
_button_select_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	App_Info *app = data;
	Evas_Object *gg = (Evas_Object *) elm_object_name_find(win, "devices gengrid", -1);
	Elm_Object_Item *it = elm_gengrid_selected_item_get(gg);

	if (!it)
		return;
	GenGridItem *ggi = elm_object_item_data_get(it);
	Widget *widget;
	widget = widget_new(NULL, ggi->device);
	location_widgets_add(app->location, widget);
	location_save(app->location);

	char s[256];
	snprintf(s, sizeof(s), _("Widget for device '%s' have been added to location '%s'."),
							device_name_get(ggi->device),
							location_name_get(app->location));
	statusbar_text_set(s, "elm/icon/device/default");

	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	elm_object_item_part_content_set(elm_naviframe_top_item_get(naviframe), NULL, _location_naviframe_content_set(app->location));
}/*_button_select_clicked_cb*/


/*
 *Create devices picker: to allow user to select device found by xPL hub.
 */
void
devices_picker_add(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *gengrid, *bx, *hbx, *bt, *ic, *sp;
	App_Info *app = (App_Info *) data;

	if (!app->location)	return;

	// Setup a new window.
	win = elm_win_util_standard_add("device_picker", _("Select a device"));
	evas_object_data_set(win, "app", app);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
	elm_win_autodel_set(win, EINA_TRUE);

	bx = elm_box_add(win);
	evas_object_show(bx);

	// Setup gengrid to display devices files in a nice and modern way.
	gengrid = elm_gengrid_add(win);
	evas_object_name_set(gengrid, "devices gengrid");
	elm_gengrid_filled_set(gengrid, EINA_TRUE);
	elm_gengrid_align_set(gengrid, 0, 0);
	elm_gengrid_item_size_set(gengrid, 150, 150);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
	elm_gengrid_multi_select_set(gengrid, EINA_TRUE);
	elm_gengrid_select_mode_set(gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_gengrid_reorder_mode_set(gengrid, EINA_FALSE);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_min_set(gengrid, 400, 400);
	evas_object_smart_callback_add(gengrid, "clicked,double", _button_select_clicked_cb, app);
	elm_box_pack_end(bx, gengrid);
	evas_object_show(gengrid);

	// Add a separator bar to show user's actions like add or remove.
	sp = elm_separator_add(win);
	elm_separator_horizontal_set(sp, EINA_TRUE);
	elm_box_pack_end(bx, sp);
	evas_object_show(sp);

	// Buttons bar.
	hbx = elm_box_add(win);
	evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_box_horizontal_set(hbx, EINA_TRUE);
	elm_box_pack_end(bx, hbx);
	evas_object_show(hbx);

	bt = elm_button_add(win);
	elm_object_text_set(bt, _("Select"));
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "apply-window");
	elm_object_part_content_set(bt, "icon", ic);
	elm_box_pack_end(hbx, bt);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", _button_select_clicked_cb, app);

	bt = elm_button_add(win);
	elm_object_text_set(bt, _("Close"));
	ic = elm_icon_add(win);
	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "window-close");
	elm_object_part_content_set(bt, "icon", ic);
	elm_box_pack_end(hbx, bt);
	evas_object_show(bt);
	evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, win);

	sp = elm_separator_add(win);
	elm_separator_horizontal_set(sp, EINA_TRUE);
	evas_object_show(sp);
	elm_box_pack_end(bx, sp);

    // Defines gengrid item class.
  	gic = elm_gengrid_item_class_new();
   	gic->item_style = "default";
	gic->func.text_get = _gengrid_devices_text_get;
    gic->func.content_get = _gengrid_devices_content_get;
    gic->func.state_get = _gengrid_devices_state_get;
   	gic->func.del = _gengrid_devices_del;

     // Fill gengrid with detected devices.
    Eina_List *l;
    Device *device;

    EINA_LIST_FOREACH(app->devices, l, device)
    {
    	GenGridItem *ti;
        ti = calloc(1, sizeof(*ti));
       	ti->device = device_clone(device);
        ti->gg_it = elm_gengrid_item_append(gengrid, gic, ti, NULL, NULL);
    }

	// Item_class_ref is needed for gic.
	// Some items can be added in callbacks.
    elm_gengrid_item_class_ref(gic);
	elm_gengrid_item_class_free(gic);

	// Resize window.
	evas_object_resize(win, 400, 450);
	evas_object_show(win);
}/*devicespicker_add*/
