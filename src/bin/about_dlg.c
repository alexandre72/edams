/*
 * about_dlg.c
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




#include "edams.h"
#include "utils.h"

static const char *names[] =
{
     "bub1", "sh1",
     "bub2", "sh2",
     "bub3", "sh3",
};


//
//
//
static void _del(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Ecore_Animator *ani = evas_object_data_get(win, "animator");

   ecore_animator_del(ani);
}


//
//
//
static Eina_Bool anim(void *data)
{
   Evas_Object *win = data;
   Evas_Object *bub;
   Evas_Coord x, y, w, h, vw, vh;
   double t, xx, yy, zz, r, fac;
   unsigned int i;

   evas_output_viewport_get(evas_object_evas_get(win), 0, 0, &vw, &vh);
   r = 48;
   t = ecore_loop_time_get();
   fac = 2.0 / (double)((sizeof(names) / sizeof(char *) / 2));
   evas_pointer_canvas_xy_get(evas_object_evas_get(win), &x, &y);


   for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
     {
        bub = evas_object_data_get(win, names[i * 2]);
        zz = (((2 + sin(t * 6 + (M_PI * (i * fac)))) / 3) * 64) * 2;
        xx = (cos(t * 3 + (M_PI * (i * fac))) * r) * 2;
        yy = (sin(t * 2 + (M_PI * (i * fac))) * r) * 2;

        w = zz;
        h = zz;
        x = (vw/2) + xx - w;
        y = (vh/2) + yy - h;

        evas_object_move(bub, x, y);
        evas_object_resize(bub, w, h);
     }
   return ECORE_CALLBACK_RENEW;
}


//
//
//
void about_dialog_new(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    unsigned int i;
    char buf[PATH_MAX];
    Evas_Object *win, *bg, *bub;
    Evas_Object *bx, *ic, *bt;
    Ecore_Animator *ani;
    App_Info *app = (App_Info *)data;

    win = elm_win_add(app->win, "aboutdlg", ELM_WIN_BASIC);
    elm_win_title_set(win, _("About edams..."));
    elm_win_autodel_set(win, EINA_TRUE);

    bg = elm_bg_add(win);
    elm_bg_file_set(bg, edams_edje_theme_file_get(), "bg/edams-bg");
    elm_bg_option_set(bg, ELM_BG_OPTION_TILE);
    elm_win_resize_object_add(win, bg);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(bg);

    snprintf(buf, sizeof(buf), "%s"DIR_SEPARATOR_S"share"DIR_SEPARATOR_S"edams"DIR_SEPARATOR_S"icons"DIR_SEPARATOR_S"bubble.png", edams_install_path_get());
    for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
    {
        bub = evas_object_image_filled_add(evas_object_evas_get(win));
        evas_object_image_file_set(bub, buf, NULL);
        evas_object_resize(bub, 64, 64);
        evas_object_show(bub);
        evas_object_data_set(win, names[(i * 2)], bub);
    }
    evas_object_resize(win, 500, 300);
    evas_object_show(win);

    ani = ecore_animator_add(anim, win);
    evas_object_data_set(win, "animator", ani);
    evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _del, win);

	bx = elm_box_add(win);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bx, 0.5, 0.5);
	elm_win_resize_object_add(win, bx);
	evas_object_show(bx);

    ic = elm_icon_add(win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   	elm_image_file_set(ic, edams_edje_theme_file_get(), "bg/edams-header");
	elm_box_pack_end(bx, ic);
    evas_object_show(ic);
    evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);

/*
    entry =  elm_entry_add(win);
    elm_entry_scrollable_set(entry, EINA_TRUE);
    evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
  	elm_entry_editable_set(entry, EINA_FALSE);
	elm_entry_line_wrap_set(entry, ELM_WRAP_MIXED);
	snprintf(s, sizeof(s),  _(""
            "<item absize=250x70 vsize=full href=file://%s/icons/edams_header.png></item>"
            "<br><em>%s (build %s)</em></>"
            "<ps></ps>"
            "<b>Authors:</b>"
            "<br><em>Alexandre Dussart(main, plugins, website...)</em> <link>alexandre.dussart@laposte.net</link></>"
            "<br><em>Remi Samier(frilexie, suggestions, testings, doc)</em> <link>r.samier@gmail.com</link></>"
            "<br><em>Corine Berte(suggestions...)</em></>"
            "<br><em>Sonia Dussart(some design and items parts)</em></>"
            "<br><em>Simion Ploscariu(learntocount based code)</em></>"
            "<ps></ps>"
            "<b>Web:</b>"
			"<br><link>http://www.edams.org</link></>"),
			edams_system_data_path_get(),
			PACKAGE_VERSION,
			__DATE__);
	elm_object_text_set(entry, s);
	elm_box_pack_end(bx, entry);
	evas_object_show(entry);
*/


/*
    label = elm_label_add(win);
    elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
    elm_object_text_set(label, "FriLogos");
    evas_object_size_hint_align_set(label, 0.5, 0.5);
    elm_label_slide_duration_set(label, 3);
    elm_label_slide_set(label, EINA_TRUE);
    elm_object_style_set(label, "marker");
    evas_object_color_set(label, 255, 255, 255, 255);
    evas_object_move(label, 50, 20);
    evas_object_resize(label, 400, 250);
	elm_box_pack_end(bx, entry);
    evas_object_show(label);
	ecore_timer_add(0.5, _timer_cb, label);
*/

    bt = elm_button_add(win);
    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	evas_object_size_hint_align_set(bt, 0.5, 1.0);
	ic = elm_icon_add(win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   	elm_icon_standard_set(ic, "window-close");
   	elm_object_part_content_set(bt, "icon", ic);
	elm_object_text_set(bt, _("Close"));
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb, win);
	evas_object_show(bt);
}
