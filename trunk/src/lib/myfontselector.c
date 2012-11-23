/*
 * myfontselector.c
 * This file is part of Elm Extension Pack
 *
 * Copyright (C) 2012 - Alexandre Dussart
 *
 * Elm Extension Pack is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Elm Extension Pack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Elm Extension Pack. If not, see <http://www.gnu.org/licenses/>.
 */
 
 
#include "myfontselector.h"


#define TEST_STRING "oislOIS.015!|,"


static Evas_Object *op_fontslider, *op_fontlist, *op_fsml, *op_fbig;


typedef struct _Font
{
   Elm_Object_Item *item;
   const char *name;
   Eina_Bool bitmap : 1;
} Font;


//Global vars.
static Eina_List *fonts = NULL;
static Eina_Hash *fonthash = NULL;
static Evas_Coord tsize_w = 0, tsize_h = 0;
static int expecting_resize = 0;



static void
_cb_op_font_sel(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   Font *f = data;

	printf("FONT=%s\n", f->name);

   elm_object_disabled_set(op_fsml, f->bitmap);
   elm_object_disabled_set(op_fontslider, f->bitmap);
   elm_object_disabled_set(op_fbig, f->bitmap);
}

static void
_cb_op_fontsize_sel(void *data, Evas_Object *obj, void *event __UNUSED__)
{
   int size = elm_slider_value_get(obj) + 0.5;

   elm_genlist_realized_items_update(op_fontlist);
}

static int
_cb_op_font_sort(const void *d1, const void *d2)
{
   return strcasecmp(d1, d2);
}



static void
_cb_op_font_preview_eval(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event __UNUSED__)
{
   Font *f = data;
   Evas_Object *o;
   Evas_Coord ox, oy, ow, oh, vx, vy, vw, vh;
   char buf[4096];
   
   if (!evas_object_visible_get(obj)) return;
   if (edje_object_part_swallow_get(obj, "terminology.text.preview")) return;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   if ((ow < 2) || (oh < 2)) return;
   evas_output_viewport_get(evas_object_evas_get(obj), &vx, &vy, &vw, &vh);
   if (ELM_RECTS_INTERSECT(ox, oy, ow, oh, vx, vy, vw, vh))
     {
        o = evas_object_text_add(evas_object_evas_get(obj));
        evas_object_color_set(o, 0, 0, 0, 255);
        evas_object_text_text_set(o, TEST_STRING);
        evas_object_scale_set(o, elm_config_scale_get());
        if (f->bitmap)
          {
             snprintf(buf, sizeof(buf), "%s/fonts/%s",
                      elm_app_data_dir_get(), f->name);
            // evas_object_text_font_set(o, buf, config->font.size);
          }
        else
        {
          //evas_object_text_font_set(o, f->name, config->font.size);
          }
        evas_object_geometry_get(o, NULL, NULL, &ow, &oh);
        evas_object_size_hint_min_set(o, ow, oh);
        edje_object_part_swallow(obj, "terminology.text.preview", o);
     }
}



static char *
_cb_op_font_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   Font *f = data;
   char buf[4096], *p;

   eina_strlcpy(buf, f->name, sizeof(buf));
   buf[0] = toupper(buf[0]);
   p = strrchr(buf, '.');
   if (p) *p = 0;
   return strdup(buf);
}

static char *
_cb_op_font_group_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return strdup(data);
}



void
options_font_clear(void)
{
   Font *f;

   op_fontslider = NULL;
   op_fontlist = NULL;
   op_fsml = NULL;
   op_fbig = NULL;
   
   EINA_LIST_FREE(fonts, f)
     {
        eina_stringshare_del(f->name);
        free(f);
     }
   if (fonthash)
     {
        eina_hash_free(fonthash);
        fonthash = NULL;
     }
}




//
//Close a window by deleting evas object passing arg.
//
static void _cancel_bt_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    myfontselector_close(data);
}


//
//
//
void myfontselector_set_title(MyFontSelector *myfs, const char *title)
{
	elm_win_title_set(myfs->win, title);
}


//
//
//
void myfontselector_close(MyFontSelector *myfs)
{
    evas_object_del(myfs->win);
    FREE(myfs);
}


MyFontSelector *myfontselector_add()
{
	MyFontSelector *myfs;
	Evas_Object *label, *ic;
   Evas_Object *o, *bx, *fr, *bx0;
   char buf[4096], *file, *fname, *s;
   Eina_List *files, *fontlist, *l;
   Font *f;
   Elm_Object_Item *it, *sel_it = NULL, *grp_it = NULL;
   Elm_Genlist_Item_Class *it_class, *it_group;


    myfs = MALLOC(sizeof(*myfs));

   options_font_clear();

    myfs->win = elm_win_util_standard_add("myfontselector", _("Select a font..."));
    elm_win_center(myfs->win, EINA_TRUE, EINA_TRUE);
	elm_win_autodel_set(myfs->win, EINA_TRUE);
  
    Evas_Object *gd = elm_grid_add(myfs->win);
    elm_grid_size_set(gd, 100, 100);
    elm_win_resize_object_add(myfs->win, gd);
    evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(gd);
    
	fr = o = elm_frame_add(myfs->win);
	evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_text_set(o, "Font");
    elm_grid_pack(gd, fr , 1, 1, 90, 90); 
	evas_object_show(o);
   
   bx0 = o = elm_box_add(myfs->win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, o);
   evas_object_show(o);
   
   bx = o = elm_box_add(myfs->win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);
   elm_box_horizontal_set(o, EINA_TRUE);
   
   op_fsml = o = elm_label_add(myfs->win);
   elm_object_text_set(o, "<font_size=6>A</font_size>");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   op_fontslider = o = elm_slider_add(myfs->win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(o, 40);
   elm_slider_unit_format_set(o, "%1.0f");
   elm_slider_indicator_format_set(o, "%1.0f");
   elm_slider_min_max_set(o, 5, 45);
   //elm_slider_value_set(o, config->font.size);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   op_fbig = o = elm_label_add(myfs->win);
   elm_object_text_set(o, "<font_size=24>A</font_size>");
   elm_box_pack_end(bx, o);
   evas_object_show(o);
   
   elm_box_pack_end(bx0, bx);
   evas_object_show(bx);
   
   it_class = elm_genlist_item_class_new();
   it_class->item_style = "end_icon";
   it_class->func.text_get = _cb_op_font_text_get;
   it_class->func.content_get = NULL;
   
   it_group = elm_genlist_item_class_new();
   it_group->item_style = "group_index";
   it_group->func.text_get = _cb_op_font_group_text_get;

   op_fontlist = o = elm_genlist_add(myfs->win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_genlist_mode_set(o, ELM_LIST_COMPRESS);
   elm_genlist_homogeneous_set(o, EINA_TRUE);
   
   snprintf(buf, sizeof(buf), "%s/fonts", elm_app_data_dir_get());
   files = ecore_file_ls(buf);
   
   if (files)
     {
        grp_it = elm_genlist_item_append(o, it_group, "Bitmap", NULL,
                                         ELM_GENLIST_ITEM_GROUP,
                                         NULL, NULL);
        elm_genlist_item_select_mode_set(grp_it,
                                         ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
     }
   
   EINA_LIST_FREE(files, file)
     {
        f = calloc(1, sizeof(Font));
        f->name = eina_stringshare_add(file);
        f->bitmap = EINA_TRUE;
        fonts = eina_list_append(fonts, f);
        
        f->item = it = elm_genlist_item_append(o, it_class, f, grp_it,
                                     ELM_GENLIST_ITEM_NONE,
                                     _cb_op_font_sel, f);
                                     /*
        if ((config->font.bitmap) && (config->font.name) && 
            (!strcmp(config->font.name, f->name)))
          {
             elm_genlist_item_selected_set(it, EINA_TRUE);
             sel_it = it;
             elm_object_disabled_set(op_fsml, EINA_TRUE);
             elm_object_disabled_set(op_fontslider, EINA_TRUE);
             elm_object_disabled_set(op_fbig, EINA_TRUE);
          }
          */
        free(file);
     }

   fontlist = evas_font_available_list(evas_object_evas_get(myfs->win));
   fontlist = eina_list_sort(fontlist, eina_list_count(fontlist),
                             _cb_op_font_sort);
   fonthash = eina_hash_string_superfast_new(NULL);

   if (fonts)
     {
        grp_it = elm_genlist_item_append(o, it_group, "Standard", NULL,
                                         ELM_GENLIST_ITEM_GROUP,
                                         NULL, NULL);
        elm_genlist_item_select_mode_set(grp_it,
                                         ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
     }
   EINA_LIST_FOREACH(fontlist, l, fname)
     {
        snprintf(buf, sizeof(buf), "%s", fname);
        s = strchr(buf, ':');
        if (s) *s = 0;
        fname = buf;
        if (!eina_hash_find(fonthash, fname))
          {
             f = calloc(1, sizeof(Font));
             f->name = eina_stringshare_add(fname);
             f->bitmap = EINA_FALSE;
             eina_hash_add(fonthash, fname, f);
             fonts = eina_list_append(fonts, f);
             f->item = it = elm_genlist_item_append(o, it_class, f, grp_it,
                                          ELM_GENLIST_ITEM_NONE,
                                          _cb_op_font_sel, f);
                                          /*
             if ((!config->font.bitmap) && (config->font.name) && 
                 (!strcmp(config->font.name, f->name)))
               {
                  elm_genlist_item_selected_set(it, EINA_TRUE);
                  sel_it = it;
               }
               */
          }
     }
   if (fontlist)
     evas_font_available_list_free(evas_object_evas_get(myfs->win), fontlist);
   
   elm_genlist_item_show(sel_it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
   
   elm_genlist_item_class_free(it_class);
   elm_genlist_item_class_free(it_group);
   
   elm_box_pack_end(bx0, o);
   evas_object_size_hint_weight_set(myfs->win, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(myfs->win, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(o);
   expecting_resize = 0;

    //Buttons bar.
    myfs->action_bt =elm_button_add(myfs->win);
    elm_object_text_set(myfs->action_bt,  _("Open"));
	ic = elm_icon_add(myfs->win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   	elm_icon_standard_set(ic, "document-open");
   	elm_object_part_content_set(myfs->action_bt, "icon", ic);
    elm_grid_pack(gd, myfs->action_bt , 35, 92, 15, 7);   	
    evas_object_show(myfs->action_bt );
    
    myfs->cancel_bt =elm_button_add(myfs->win);
	elm_object_text_set(myfs->cancel_bt,  _("Close"));
	ic = elm_icon_add(myfs->win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "window-close");
   	elm_object_part_content_set(myfs->cancel_bt, "icon", ic);
    elm_grid_pack(gd, myfs->cancel_bt , 50, 92, 15, 7);
    evas_object_show(myfs->cancel_bt );
	evas_object_smart_callback_add(myfs->cancel_bt, "clicked", _cancel_bt_clicked_cb, myfs);    
    //End of buttons bar.

    ///Forcing min height.
    evas_object_resize(myfs->win, 600, 500);
    evas_object_show(myfs->win);        

	return myfs;
}
