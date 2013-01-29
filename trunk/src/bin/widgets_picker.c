/*
 * widgets_picker.c
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

#include <Elementary.h>

#include "widgets_picker.h"
#include "device.h"
#include "edams.h"
#include "path.h"
#include "utils.h"

static Evas_Object *win = NULL;

static void _list_item_widgets_selected_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _layout_samples_test(Evas_Object *layout);


/*
 * Return a random number between 1 & max.
 */
int prandom(int max)
{
    int partSize   = 1 + (max == RAND_MAX ? 0 : (RAND_MAX - max) / (max + 1));
    int maxUsefull = partSize * max + (partSize - 1);
    int draw;

    do {
        draw = rand();
    } while (draw > maxUsefull);

    return draw / partSize;

}


static const int TEMP_MIN = -30;
static const int TEMP_MAX = 50;

/*
 * Send random value to current selected widget to let user see effects on it.
 */
static void
_layout_samples_test(Evas_Object *layout)
{
	char s[32];
    srand((unsigned int)time((time_t *)NULL));

	Device *sample = evas_object_data_get(win, "device");
	snprintf(s, sizeof(s), "%d", prandom(24));
	device_current_set(sample, s);

	const char *t;
	//Special widget with drag part, so need to convert device current value to float.
	if ((t = elm_layout_data_get(layout, "drag")))
	{
		double level;
		Edje_Message_Float msg;

		if(device_type_get(sample) == TEMP_SENSOR_BASIC_TYPE)
		{
			int x, y;
			sscanf(device_current_get(sample), "%d.%02d", &x, &y);
			level =	(double)((x + (y * 0.01)) - TEMP_MIN) / (double)(TEMP_MAX - TEMP_MIN);

			if (level < 0.0) level = 0.0;
			else if (level > 1.0) level = 1.0;
		}
		else
		{
			level =	(double)device_current_to_int(sample) / 100;
		}

		msg.val = level;

		edje_object_message_send(elm_layout_edje_get(layout), EDJE_MESSAGE_FLOAT, 1, &msg);
	}

	if ((device_current_to_int(sample) == 0))
		elm_object_signal_emit(layout, "false", "over");
	else
		elm_object_signal_emit(layout, "true", "over");

	if ((t = elm_layout_data_get(layout, "title")))
	{
		snprintf(s, sizeof(s), "%s", device_name_get(sample));
		elm_object_part_text_set(layout, "title.text", s);
	}

	if ((t = elm_layout_data_get(layout, "value")))
	{
		snprintf(s, sizeof(s), "%d%s", device_current_to_int(sample), device_unit_symbol_get(sample) ? device_unit_symbol_get(sample) : "");
		elm_object_part_text_set(layout, "value.text", s);
	}
	elm_object_signal_emit(layout, "updated", "over");


   	Evas_Object *edje;
	Evas_Coord w, h;
	elm_layout_sizing_eval(layout);
   	edje = elm_layout_edje_get(layout);
	edje_object_size_max_get(edje, &w, &h);

   //edje_object_size_min_calc(edje, &w, &h);
   	printf("Minimum size for this theme: %dx%d\n", w, h);

	evas_object_resize(elm_layout_edje_get(layout), w, h);
	elm_layout_sizing_eval(layout);
}


static void
_update_cmnd_preview(Device *device)
{
	char *s;
	asprintf(&s, _("<ps><ps><em>control.basic<br>\
							{<br>\
							<tab>device=%s<br>\
 							<tab>type=%s<br>\
							<tab>current=%s<br>\
							}<br></em>"),
							device_name_get(device),
							device_type_to_str(device_type_get(device)),
							device_current_get(device));

	Evas_Object *entry = elm_object_name_find(win, "widget description entry", -1);
   	elm_object_text_set(entry, s);
	FREE(s);
}



/*
 *
 */
static void
_layout_signals_cb(void *data, Evas_Object *edje_obj __UNUSED__, const char  *emission, const char  *source)
{
	Device *device = data;

	if(strstr(emission, "mouse")) return;

	fprintf(stdout, "emission=%s\n", emission);
	fprintf(stdout, "source=%s\n", source);

	//fprintf(stdout, "data1=%s\n", data);

	//device_data1_set(device, data);
	device_current_set(device, emission);
	_update_cmnd_preview(device);
}



/*
 *Callback called in list "widgets" object when an item is selected
 */
static void
_list_item_widgets_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	const char *widget = data;
	const char *t;

	Evas_Object *layout = elm_object_name_find(win, "widget layout", -1);
	elm_layout_file_set(layout, edams_edje_theme_file_get(), widget);
	_layout_samples_test(layout);


	Evas_Object *entry = elm_object_name_find(win, "widget description entry", -1);
	if((t = elm_layout_data_get(layout, "description")))
	{
		char *s;
		asprintf(&s, _("Description:%s"), t);
   		elm_object_text_set(entry, t);
   		FREE(s);
   	}
   	else
   	{
   		elm_object_text_set(entry, NULL);
   	}

	/*Show cmnd schema specific parameters for control.basic widget according to edc item flags values.*/
	Device *device = evas_object_data_get(win, "device");
	if(device_class_get(device) == CONTROL_BASIC_CLASS)
	{
		if((t = elm_layout_data_get(layout, "tags")))
		{
			//FIXME:Hack to avoid setting input or ouput from sensor.basic class!
			if(strcmp(t, "input") == 0)			device_type_set(device, INPUT_CONTROL_BASIC_TYPE);
			else if(strcmp(t, "ouput") == 0)	device_type_set(device, OUTPUT_CONTROL_BASIC_TYPE);
			else								device_type_set(device, device_str_to_type(t));

			elm_layout_signal_callback_add(layout, "*", "*", _layout_signals_cb, device);
		}
	}
}


/*
 *Callback called in button "apply" object when clicked signal is emitted.
 */
static void
_button_apply_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	char *s;
	Elm_Object_Item *item;

	App_Info *app = data;
	Evas_Object *list = elm_object_name_find(win, "widgets picker list", -1);

	item = elm_list_selected_item_get(list);

	if (!item) return;

	asprintf(&s, "%s widgets list", location_name_get(app->location));
	Evas_Object *widgets_list = elm_object_name_find(app->win, s, -1);
	FREE(s);
	Elm_Object_Item *selected_item = elm_list_selected_item_get(widgets_list);

	Widget *widget = elm_object_item_data_get(selected_item);
	Device *device = widget_device_get(widget);

	if(device_class_get(device) == CONTROL_BASIC_CLASS)
	{
		Device *device_sel = evas_object_data_get(win, "device");
		device_type_set(device, device_type_get(device_sel));
	}

	device_save(device);
    widget_name_set(widget, elm_object_item_data_get(item));
	location_save(app->location);

	update_naviframe_content(app->location);

	evas_object_del(win);
}


/*
 *
 */
void
widgets_picker_add(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *grid;
	Evas_Object *icon, *box, *frame;
	Evas_Object *button;
	Evas_Object *entry, *list, *layout;
	Eina_List *l, *groups;

	App_Info *app = (App_Info *) data;

	char *s;
	asprintf(&s, "%s widgets list", location_name_get(app->location));
	Evas_Object *widgets_list = elm_object_name_find(app->win, s, -1);
	FREE(s);
	Elm_Object_Item *selected_item = elm_list_selected_item_get(widgets_list);

	if(!selected_item) return;

	Widget *widget = elm_object_item_data_get(selected_item);
	Device *device = widget_device_get(widget);

	asprintf(&s, _("Edit widget properties for '%s' xPL device"), device_name_get(device));
	win = elm_win_util_standard_add("widgets_picker", s);
	FREE(s);
	evas_object_data_set(win, "device", device);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	evas_object_name_set(grid, "grid");
	elm_grid_size_set(grid, 100, 100);
   	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_win_resize_object_add(win, grid);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Preview"));
	elm_grid_pack(grid, frame, 1, 1, 40, 50);
	evas_object_show(frame);

	layout = elm_layout_add(win);
	evas_object_name_set(layout, "widget layout");
   	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(frame, layout);
	evas_object_show(layout);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Widgets"));
	elm_grid_pack(grid, frame, 41, 1, 58, 50);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS );
	evas_object_name_set(list, "widgets picker list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

	groups = edje_file_collection_list(edams_edje_theme_file_get());
	if (groups)
	{
		char *group;
		EINA_LIST_FOREACH(groups, l, group)
		{
			asprintf(&s, "widget/%s", device_class_to_str(device_class_get(device)));
			if((strncmp(group, s, strlen(s)) == 0) && (!strstr(group, "/icon")))
			{
				Evas_Object *icon;
				icon = elm_icon_add(win);
				asprintf(&s, "%s/icon", group);
			   	elm_image_file_set(icon, edams_edje_theme_file_get(), s);
			   	FREE(s);
				elm_image_aspect_fixed_set(icon, EINA_TRUE);
				elm_image_resizable_set(icon, 1, 0);
				elm_list_item_append(list, group, icon, NULL, _list_item_widgets_selected_cb, group);
			}
			FREE(s);
		}
		edje_file_collection_list_free(groups);
	}
	elm_list_go(list);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Description"));
	elm_grid_pack(grid, frame, 1, 51, 99, 30);
	evas_object_show(frame);

   	entry = elm_entry_add(win);
   	elm_entry_editable_set(entry, EINA_FALSE);
   	evas_object_name_set(entry, "widget description entry");
   	elm_entry_line_wrap_set(entry, ELM_WRAP_MIXED);
   	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(frame, entry);
	evas_object_show(entry);

	box = elm_box_add(win);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_box_homogeneous_set(box, EINA_TRUE);
	elm_grid_pack(grid, box, 1, 90, 99, 10);
	evas_object_show(box);

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

	evas_object_resize(win, 600, 500);
	evas_object_show(win);
}/*widgets_picker_add*/
