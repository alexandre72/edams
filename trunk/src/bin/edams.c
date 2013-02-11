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
#include <stdio.h>

#include <Elementary.h>
#include <Ecore_File.h>

#include "action.h"
#include "actions_editor.h"
#include "about_dlg.h"
#include "cosm.h"
#include "edams.h"
#include "gnuplot.h"
#include "init.h"
#include "location.h"
#include "locations_creator.h"
#include "global_view.h"
#include "path.h"
#include "preferences_dlg.h"
#include "shutdown.h"
#include "utils.h"
#include "widget_editor.h"

/*Global objects*/
App_Info *app = NULL;

/*Evas_Object Callbacks*/
static void _list_locations_selected_cb(void *data, Evas_Object * obj, void *event_info);
static void _button_quit_clicked_cb(void *data __UNUSED__,  Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_remove_location_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__,void *event_info __UNUSED__);
static void _button_remove_widget_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_add_widget_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_edit_widget_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);

/*List sorts callbacks*/
static int _list_widgets_sort_cb(const void *pa, const void *pb);

/*Timers callbacks*/
static Eina_Bool _statusbar_timer_cb(void *data);

/*Others funcs*/
static Evas_Object *_item_provider(void *images __UNUSED__, Evas_Object *en, const char *item);
EAPI_MAIN int elm_main(int argc, char *argv[]);


/*
 *Return locations already read
 */
const Eina_List *
edams_locations_list_get()
{
    return app->locations;
}/*edams_locations_list_get*/


/*
 *Callback
 */
static void
_list_locations_selected_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Eina_List *its = NULL, *l;
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
}/*_list_locations_selected_cb*/

/*
 *
 */
static void
_button_quit_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	elm_exit();
}/*_button_quit_clicked_cb*/


/*
 *
 */
static Eina_Bool
_statusbar_timer_cb(void *data __UNUSED__)
{
	char *s;
	time_t timestamp;
	struct tm *t;

	Evas_Object *label = elm_object_name_find(app->win, "status text", -1);
	timestamp = time(NULL);
	t = localtime(&timestamp);
  	asprintf(&s, _("EDAMS v%s - %02d/%02d/%d"),PACKAGE_VERSION, (int)t->tm_mday, (int)t->tm_mon + 1, 1900 + (int)t->tm_year);
	elm_object_text_set(label, s);
	FREE(s);

	Evas_Object *icon = elm_object_name_find(app->win, "status icon", -1);
	elm_icon_standard_set(icon, "help-about");

	return EINA_FALSE;
}/*_statusbar_timer_cb*/

/*
 *Set elm label and elm icon of bottom status bar.
 */
void
statusbar_text_set(const char *msg, const char *ic)
{
	Evas_Object *label, *icon;

	label = elm_object_name_find(app->win, "status text", -1);
	elm_object_text_set(label, msg);

	icon = elm_object_name_find(app->win, "status icon", -1);

	if(!elm_icon_standard_set(icon, ic))
		elm_image_file_set(icon, edams_edje_theme_file_get(), ic);

	ecore_timer_add(6.0, _statusbar_timer_cb, NULL);
}/*statusbar_text_set*/

/*
 * Remove currently selected location.
 */
static void
_button_remove_location_clicked_cb(void *data __UNUSED__,Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *list;
	Elm_Object_Item *it;

	list = (Evas_Object *) elm_object_name_find(app->win, "locations list", -1);
	it = (Elm_Object_Item *) elm_list_selected_item_get(list);

	if (!it)
	{
		statusbar_text_set(_("Can't remove:no location selected!"), "dialog-error");
		return;
	}
	else
	{
		Location *location = elm_object_item_data_get(it);

		if(location)
		{
			char *s;
			cosm_location_feed_delete(location);
			global_view_location_del(location);
			location_remove(location);
			asprintf(&s, _("Location '%s' have been removed"), location_name_get(location));
			statusbar_text_set(s, "dialog-information");
			FREE(s);
			elm_object_item_del(it);

			app->locations = locations_list_location_remove(app->locations, location);
			location_free(location);

			Evas_Object *naviframe = (Evas_Object *) elm_object_name_find(app->win, "naviframe", -1);
			elm_naviframe_item_pop(naviframe);
		}
	}
}/*_button_remove_location_clicked_cb*/

/*
 *Callback called in remove widget button object when "clicked" signal is emitted
 */
static void
_button_remove_widget_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	if (!app->location)
	{
		statusbar_text_set(_("Couldn't remove widget:no location selected!"), "dialog-error");
		return;
	}

	Evas_Object *list = data;
	Elm_Object_Item *selected_item = elm_list_selected_item_get(list);

	if(!selected_item) return;

	Widget *widget = elm_object_item_data_get(selected_item);
	location_widgets_del(app->location, widget);
   	elm_object_item_del(selected_item);
	location_save(app->location);

	char *s;
	asprintf(&s,  _("Widget '%s' have been removed from location '%s'"),
																widget_name_get(widget),
																location_name_get(app->location));

	statusbar_text_set(s, "elm/icon/device/default");
	FREE(s);
}/*_button_remove_widget_clicked_cb*/


/*
 *Callback called in remove widget button object when "clicked" signal is emitted
 */
static void
_button_add_widget_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	app->widget = NULL;
    widget_editor_add(app, NULL, NULL);
}/*_button_add_widget_clicked_cb*/


/*
 *Callback called in remove widget button object when "clicked" signal is emitted
 */
static void
_button_edit_widget_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *list = data;
	Elm_Object_Item *selected_item = elm_list_selected_item_get(list);

	if(!selected_item) return;

	app->widget = elm_object_item_data_get(selected_item);

    widget_editor_add(app, NULL, NULL);
}/*_button_remove_widget_clicked_cb*/


/*
 *Callback called in list "widgets" when sorted sorted_insert func is called.
 */
static int
_list_widgets_sort_cb(const void *pa, const void *pb)
{
   const Elm_Object_Item *ia = pa, *ib = pb;

   Widget *a = (Widget *)elm_object_item_data_get(ia);
   Widget *b = (Widget *)elm_object_item_data_get(ib);

   return strcoll(widget_name_get(a), widget_name_get(b));
}/*_list_widgets_sort_cb*/


/*
 *
 */
static Evas_Object *
_item_provider(void *images __UNUSED__, Evas_Object *en, const char *item)
{
   Evas_Object *o = NULL;;

   if (!strcmp(item, "cosm-logo"))
  {
   		o = elm_icon_add(en);
   		elm_icon_order_lookup_set(o, ELM_ICON_LOOKUP_FDO_THEME);
		elm_image_file_set(o, edams_edje_theme_file_get(), "elm/icon/cosm/default");
  }
   else if (!strcmp(item, "xpl-logo"))
   {
   		o = elm_icon_add(en);
   		elm_icon_order_lookup_set(o, ELM_ICON_LOOKUP_FDO_THEME);
		elm_image_file_set(o, edams_edje_theme_file_get(), "elm/icon/xpl/default");
   }

   return o;
}/*_item_provider*/


/*
 *
 */
void
update_naviframe_content(Location *location)
{
	Evas_Object *naviframe = elm_object_name_find(app->win, "naviframe", -1);
	elm_object_item_part_content_set(elm_naviframe_top_item_get(naviframe), NULL, _location_naviframe_content_set(location));
}/*update_naviframe_content*/

/*
 *
 */
Evas_Object *
_location_naviframe_content_set(Location * location)
{
	Evas_Object *grid;
	Evas_Object *bx, *frame;
	Evas_Object *list;
	Evas_Object *icon;
	Evas_Object *entry, *bt;
	char *s;

	if (!location) return NULL;

	grid = elm_grid_add(app->win);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_grid_size_set(grid, 100, 100);
	evas_object_show(grid);

   	entry = elm_entry_add(app->win);
   	elm_entry_scrollable_set(entry, EINA_TRUE);
    asprintf(&s, 	_("Name:%s<br>Cosm feedid:%d"),
    				location_name_get(location),
    				location_cosm_feedid_get(location));
   	elm_object_text_set(entry, s);
   	FREE(s);
    elm_entry_editable_set(entry, EINA_FALSE);
   	elm_entry_context_menu_disabled_set(entry, EINA_TRUE);
	elm_entry_item_provider_append(entry, _item_provider, NULL);
   	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   	elm_grid_pack(grid, entry, 1, 1, 99, 20);
	evas_object_show(entry);

	if(location_cosm_feedid_get(location) != 0)
		elm_entry_entry_append(entry, "<item size=65x16 vsize=full href=cosm-logo></item>");
/*
	if((location_latitude_get(location) != -1) && (location_longitude_get(location) != -1))
		elm_entry_entry_append(entry, "<item size=65x16 vsize=full href=cosm-logo></item>");
*/
	frame = elm_frame_add(app->win);
	elm_object_text_set(frame, _("Widgets"));
	elm_grid_pack(grid, frame, 1, 21, 99, 59);
	evas_object_show(frame);

	list = elm_list_add(app->win);
	asprintf(&s, "%s widgets list", location_name_get(location));
	evas_object_name_set(list, s);
	FREE(s);
	elm_scroller_policy_set(list, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_ON);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_show(list);
	elm_object_content_set(frame, list);

	Eina_List *l, *widgets;
	Widget *widget;
	widgets = location_widgets_list_get(location);
/*
	if(widgets)
		elm_entry_entry_append(entry, "<item size=65x16 vsize=full href=xpl-logo></item>");
*/
    /*Append widgets from location to list*/
	EINA_LIST_FOREACH(widgets, l, widget)
	{
		Evas_Object *icon;
		icon = elm_icon_add(app->win);
		asprintf(&s, "%s/icon", widget_group_get(widget));
   		elm_image_file_set(icon, edams_edje_theme_file_get(), s);
   		FREE(s);
		elm_image_aspect_fixed_set(icon, EINA_TRUE);
		elm_image_resizable_set(icon, 1, 0);

		elm_list_item_sorted_insert(list,  strdup(widget_name_get(widget)), icon, NULL, NULL, widget, _list_widgets_sort_cb);
	}
	elm_list_go(list);

	bx = elm_box_add(app->win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_grid_pack(grid, bx, 1, 81, 90, 10);
	evas_object_show(bx);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Edit"));
    icon = elm_icon_add(app->win);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(icon, "document-properties");
    elm_object_part_content_set(bt, "icon", icon);
	elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", _button_edit_widget_clicked_cb, list);
    evas_object_show(bt);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Actions"));
    icon = elm_icon_add(app->win);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(icon, "document-properties");
    elm_object_part_content_set(bt, "icon", icon);
	elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", actions_editor_add, app);
    evas_object_show(bt);

    bt = elm_button_add(app->win);
    elm_object_text_set(bt, _("Add"));
	icon = elm_icon_add(app->win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "device-add");
	elm_object_part_content_set(bt, "icon", icon);
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", _button_add_widget_clicked_cb, app);
	evas_object_show(bt);

	bt = elm_button_add(app->win);
	elm_object_text_set(bt, _("Remove"));
    icon = elm_icon_add(app->win);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(icon, "device-remove");
    elm_object_part_content_set(bt, "icon", icon);
	elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", _button_remove_widget_clicked_cb, list);
    evas_object_show(bt);

	return grid;
}/*_location_naviframe_content_set*/

/*
 * Edams main loop.
 */
EAPI_MAIN int
elm_main(int argc, char **argv)
{
    Evas_Object *vbx, *vbx2, *bg, *frame;
    Evas_Object *sep;
    Evas_Object *tb, *bt, *icon, *label, *bx, *list, *naviframe;
    Eina_List *l;

    /*Alloc and initialize App_Info struct*/
    app = calloc(1, sizeof(App_Info));

    if (!app)
    {
        fprintf(stderr, _("\033[31mERROR:\033[0mCan't calloc App_Info struct!\n"));
        exit(-1);
    }

    app->argc = argc;
    app->argv = argv;

    /*Initialize edams*/
    edams_init(app);

    /*Setup main window*/
    app->locations = locations_list_get();

    app->win = elm_win_add(NULL, "edams", ELM_WIN_BASIC);
    elm_win_title_set(app->win, _("Enlightened Domotics Alarm Monitoring System"));
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

    /*Setup toolbar.*/
    app->toolbar = elm_toolbar_add(app->win);
    elm_toolbar_icon_order_lookup_set(app->toolbar, ELM_ICON_LOOKUP_FDO_THEME);
    evas_object_size_hint_align_set(app->toolbar, -1.0, 0.0);
    evas_object_size_hint_weight_set(app->toolbar, 1.0, 0.0);
    elm_toolbar_item_append(app->toolbar, "global-view", _("Global View"), global_view_new, app);
    elm_toolbar_item_append(app->toolbar, "applications-utilities", _("Preferences"), preferences_dlg_new, app);
    elm_toolbar_item_append(app->toolbar, "help-about", _("About"), about_dialog_new, app);
    elm_toolbar_item_append(app->toolbar, "application-exit", _("Quit"), _button_quit_clicked_cb, app);
    elm_box_pack_end(vbx, app->toolbar);
    evas_object_show(app->toolbar);

    sep = elm_separator_add(app->win);
    elm_separator_horizontal_set(sep, EINA_TRUE);
    elm_box_pack_end(vbx, sep);
    evas_object_show(sep);

    /*Create locations list panel selector*/
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
    evas_object_smart_callback_add(list, "selected", _list_locations_selected_cb, NULL);
    evas_object_show(list);
    elm_object_content_set(frame, list);
    elm_panes_content_left_size_set(panes, 0.20);
    elm_object_part_content_set(panes, "left", vbx2);

    /*Table widget, contains group/subgroup genlist navigation and actions buttons like add...*/
    tb = elm_table_add(app->win);
    elm_win_resize_object_add(app->win, tb);
    evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_box_pack_end(vbx2, tb);
    evas_object_show(tb);

    bt = elm_button_add(app->win);
    elm_object_text_set(bt, _("Add"));
    icon = elm_icon_add(app->win);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(icon, "list-add");
    elm_object_part_content_set(bt, "icon", icon);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, bt, 0, 0, 1, 1);
    evas_object_show(bt);
    evas_object_smart_callback_add(bt, "clicked", locations_creator_add, app);

    bt = elm_button_add(app->win);
    elm_object_text_set(bt, _("Remove"));
    icon = elm_icon_add(app->win);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
    elm_icon_standard_set(icon, "list-remove");
    elm_object_part_content_set(bt, "icon", icon);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, bt, 0, 1, 1, 1);
    evas_object_show(bt);
    evas_object_smart_callback_add(bt, "clicked", _button_remove_location_clicked_cb, list);

    naviframe = elm_naviframe_add(app->win);
    elm_naviframe_content_preserve_on_pop_set(naviframe, EINA_FALSE);
    evas_object_name_set(naviframe, "naviframe");
    evas_object_size_hint_weight_set(naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(naviframe);

    /*Insert location page to naviframe*/
    Location *location;
    EINA_LIST_FOREACH(app->locations, l, location)
    {
        icon = elm_icon_add(app->win);
        if(!elm_image_file_set(icon, location_filename_get(location), "/image/0"))
        {
            evas_object_size_hint_min_set(icon, 48, 48);
            evas_object_size_hint_align_set(icon, 0.5, EVAS_HINT_FILL);
            evas_object_show(icon);
            elm_list_item_append(list, location_name_get(location), icon, icon,  NULL, location);
        }
        else
        {
            elm_list_item_append(list, location_name_get(location), NULL, NULL,  NULL, location);
        }

        Elm_Object_Item *it = elm_naviframe_item_push(naviframe, location_name_get(location), NULL, NULL, _location_naviframe_content_set(location), NULL);
         elm_naviframe_item_title_visible_set(it, EINA_FALSE);
        elm_object_item_data_set(it, location);
     }

    Elm_Object_Item *it = elm_naviframe_item_push(naviframe, NULL, NULL, NULL, NULL, NULL);
    elm_naviframe_item_title_visible_set(it, EINA_FALSE);
    elm_list_go(list);
    elm_object_part_content_set(panes, "right", naviframe);

    /*Setup status bar to inform user.*/
    sep = elm_separator_add(app->win);
    elm_separator_horizontal_set(sep, EINA_TRUE);
    elm_box_pack_end(vbx, sep);
    evas_object_show(sep);

    bx = elm_box_add(app->win);
    elm_box_horizontal_set(bx, EINA_TRUE);
    elm_box_pack_end(vbx, bx);
    evas_object_show(bx);

    icon = elm_icon_add(bx);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
    evas_object_name_set(icon, "status icon");
    evas_object_size_hint_min_set(icon, 24, 24);
    evas_object_size_hint_align_set(icon, 0.5, EVAS_HINT_FILL);
    evas_object_show(icon);
    elm_box_pack_end(bx, icon);

    label = elm_label_add(app->win);
    elm_label_slide_duration_set(label, 3);
    elm_label_slide_set(label, EINA_TRUE);
    elm_object_style_set(label, "slide_bounce");
    evas_object_name_set(label, "status text");
    evas_object_show(label);
    elm_box_pack_end(bx, label);
    evas_object_show(label);

    elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
    elm_image_aspect_fixed_set(icon, EINA_TRUE);
    elm_image_fill_outside_set(icon, EINA_FALSE);

    sep = elm_separator_add(app->win);
    elm_separator_horizontal_set(sep, EINA_TRUE);
    elm_box_pack_end(vbx, sep);
    evas_object_show(sep);

    evas_object_resize(app->win, 700, 500);
    evas_object_show(app->win);

    //mail_action("{\"FROM\":\"alexandre.dussart@laposte.net\",\"TO\":\"alexandre.dussart@laposte.net\",\"SUBJECT\":\"Bot EDAMS\",\"BODY\":\"TEST\"}");
    //exec_action("{\"EXEC\":\"/usr/bin/gedit\",\"TERMINAL\":\"false\"}");
     //debug_action("{\"PRINT\":\"pwet\"}");

    Ecore_Pipe *pipe = xpl_start();

    elm_run();

    if(pipe)
        ecore_pipe_del(pipe);
    global_view_quit();
    edams_shutdown(app);

    return EXIT_SUCCESS;
}/*elm_main*/
ELM_MAIN()
