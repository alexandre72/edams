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

#include <stdio.h>
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
	Elm_Object_Item *gengrid_it;
	Device *device;
} GenGridItem;


Evas_Object *win = NULL;

static char *_gengrid_devices_text_get(void *data, Evas_Object * obj __UNUSED__, const char *part __UNUSED__);
static Evas_Object *_gengrid_devices_content_get(void *data, Evas_Object * obj, const char *part);
static Eina_Bool _gengrid_devices_state_get(void *data __UNUSED__, Evas_Object * obj __UNUSED__, const char *part __UNUSED__);
static void _gengrid_devices_del(void *data __UNUSED__, Evas_Object * obj __UNUSED__);
static void _button_apply_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);


/*
 *
 */
static char *
_gengrid_devices_text_get(void *data, Evas_Object * obj __UNUSED__, const char *part __UNUSED__)
{
	const GenGridItem *ti = data;

	return strdup(device_name_get(ti->device));
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
 *
 */

static void
_button_apply_name_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	App_Info *app = data;

	Evas_Object *gengrid = (Evas_Object *) elm_object_name_find(win, "devices gengrid", -1);
	Elm_Object_Item *it = elm_gengrid_selected_item_get(gengrid);

	if (!it) return;

	GenGridItem *ggi = elm_object_item_data_get(it);

	Evas_Object *entry = (Evas_Object *) elm_object_name_find(win, "device name entry", -1);

	if(elm_entry_is_empty(entry)) return;

	device_name_set(ggi->device, elm_object_text_get(entry));
	device_save(ggi->device);

	Widget *widget;
	widget = widget_new(NULL, ggi->device);
	location_widgets_add(app->location, widget);
	location_save(app->location);

	char *s;
	asprintf(&s, _("Widget for device '%s' have been added to location '%s'."),
											device_name_get(ggi->device),
											location_name_get(app->location));
	statusbar_text_set(s, "elm/icon/device/default");
	FREE(s);

	update_naviframe_content(app->location);


	Evas_Object *inwin = (Evas_Object *) elm_object_name_find(win, "inwin", -1);
	evas_object_del(inwin);
}/*_button_apply_name_clicked_cb*/




/*
 *Callback called in gengrid "devices" when clickeddouble signal is emitted.
 *Callback called in button "apply" when clicked signal is emitted.
 */
static void
_button_apply_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	App_Info *app = data;
	Evas_Object *gengrid = (Evas_Object *) elm_object_name_find(win, "devices gengrid", -1);
	Elm_Object_Item *it = elm_gengrid_selected_item_get(gengrid);

	if (!it) return;

	GenGridItem *ggi = elm_object_item_data_get(it);

	/*User must set a name for control.basic or virtual devices*/
	if( (device_class_get(ggi->device) == CONTROL_BASIC_CLASS) ||
		(device_class_get(ggi->device) == VIRTUAL_CLASS))
	{
		Evas_Object *inwin, *box, *label, *entry, *icon, *button;

	   	inwin = elm_win_inwin_add(win);
	   	evas_object_name_set(inwin, "inwin");
   		elm_object_style_set(inwin, "minimal_vertical");
		evas_object_show(inwin);

		box = elm_box_add(win);
		elm_box_horizontal_set(box, EINA_TRUE);
		evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_inwin_content_set(inwin, box);
		evas_object_show(box);

   		label = elm_label_add(win);
   		elm_object_text_set(label, _("Name:"));
		evas_object_show(label);
  		elm_box_pack_end(elm_win_inwin_content_get(inwin), label);

		entry = elm_entry_add(win);
		evas_object_name_set(entry, "device name entry");
		evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   		evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_entry_scrollable_set(entry, EINA_TRUE);
		elm_entry_editable_set(entry, EINA_TRUE);
		elm_entry_single_line_set(entry, EINA_TRUE);
		evas_object_show(entry);
  		elm_box_pack_end(elm_win_inwin_content_get(inwin), entry);

		button = elm_button_add(win);
		icon = elm_icon_add(win);
		elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
		elm_icon_standard_set(icon, "apply-window");
		elm_object_part_content_set(button, "icon", icon);
		elm_object_text_set(button, _("Ok"));
		elm_box_pack_end(box, button);
		evas_object_show(button);
		evas_object_smart_callback_add(button, "clicked", _button_apply_name_clicked_cb, app);

		button = elm_button_add(win);
		icon = elm_icon_add(win);
		elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
		elm_icon_standard_set(icon, "window-close");
		elm_object_part_content_set(button, "icon", icon);
		elm_object_text_set(button, _("Close"));
		elm_box_pack_end(box, button);
		evas_object_show(button);
		evas_object_smart_callback_add(button, "clicked", window_clicked_close_cb, inwin);
	}
	else
	{
		Widget *widget;
		widget = widget_new(NULL, ggi->device);
		location_widgets_add(app->location, widget);
		location_save(app->location);

		char *s;
		asprintf(&s, _("Widget for device '%s' have been added to location '%s'."),
													device_name_get(ggi->device),
													location_name_get(app->location));
		statusbar_text_set(s, "elm/icon/device/default");
		FREE(s);

		update_naviframe_content(app->location);
	}
}/*_button_select_clicked_cb*/


/*
 *Create devices picker: to allow user to select device found by xPL hub.
 */
void
devices_picker_add(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *gengrid, *box, *button, *icon, *separator, *grid;

	App_Info *app = (App_Info *) data;

	if (!app->location)	return;

	// Setup a new window.
	win = elm_win_util_standard_add("device_picker", _("Select a device"));
	evas_object_data_set(win, "app", app);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);
	elm_win_autodel_set(win, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(grid);

	// Setup gengrid to display devices files in a nice and modern way.
	gengrid = elm_gengrid_add(win);
	evas_object_name_set(gengrid, "devices gengrid");
	elm_gengrid_filled_set(gengrid, EINA_TRUE);
	elm_gengrid_align_set(gengrid, 0, 0);
	elm_gengrid_item_size_set(gengrid, 70, 70);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
	elm_gengrid_multi_select_set(gengrid, EINA_FALSE);
	elm_gengrid_select_mode_set(gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_gengrid_reorder_mode_set(gengrid, EINA_TRUE);
	elm_scroller_policy_set(gengrid,  ELM_SCROLLER_POLICY_AUTO,  ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(gengrid, "clicked,double", _button_apply_clicked_cb, app);
	elm_grid_pack(grid, gengrid, 1, 1, 99, 80);
	evas_object_show(gengrid);

	// Add a separator bar to show user's actions like add or remove.
	box = elm_box_add(win);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_box_homogeneous_set(box, EINA_TRUE);
	elm_grid_pack(grid, box, 1, 90, 99, 10);
	evas_object_show(box);

	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_box_pack_end(box, separator);
	evas_object_show(separator);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "apply-window");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Ok"));
	elm_box_pack_end(box, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_apply_clicked_cb, app);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "window-close");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Close"));
	elm_box_pack_end(box, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", window_clicked_close_cb, win);

	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_box_pack_end(box, separator);
	evas_object_show(separator);

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
    GenGridItem *ti;

	//Add a registered virtual control.basic device, to allow adding xPL that can be controlled .
	device = device_new(_("control.basic"));
	device_class_set(device, CONTROL_BASIC_CLASS);
	ti = calloc(1, sizeof(*ti));
	ti->device = device_clone(device);
	ti->gengrid_it = elm_gengrid_item_append(gengrid, gic, ti, NULL, NULL);
	device_free(device);

	//Add a registered virtual device, to allow adding special widgets like mail checker, clock.
	device = device_new(_("virtual"));
	device_class_set(device, VIRTUAL_CLASS);
	ti = calloc(1, sizeof(*ti));
	ti->device = device_clone(device);
	ti->gengrid_it = elm_gengrid_item_append(gengrid, gic, ti, NULL, NULL);
	device_free(device);

    EINA_LIST_FOREACH(app->devices, l, device)
    {
    	if((device_class_get(device) == VIRTUAL_CLASS) ||
			(device_class_get(device) == CONTROL_BASIC_CLASS)) 	continue;
        ti = calloc(1, sizeof(*ti));
       	ti->device = device_clone(device);
        ti->gengrid_it = elm_gengrid_item_append(gengrid, gic, ti, NULL, NULL);
    }

	// Item_class_ref is needed for gic.
	// Some items can be added in callbacks.
    elm_gengrid_item_class_ref(gic);
	elm_gengrid_item_class_free(gic);

	// Resize window.
	evas_object_resize(win, 400, 450);
	evas_object_show(win);
}/*devicespicker_add*/
