/*
 * myfileselector.c
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


#include "myfileselector.h"


//Taken from enlightenment DR17 credits goes to his authors.
static char * _file_user_get(uid_t st_uid);
static char *_file_perms_get(mode_t st_mode, uid_t st_uid, gid_t st_gid);
static char *_file_size_get(off_t size);


#ifdef HAVE_EVIL
#define 	getuid()   1000
//Get the real group ID of the calling process.
gid_t getgid ()
{
  return 0;
}
#endif



static char *
_file_user_get(uid_t st_uid)
{
   char name[4096];
   struct passwd *pwd;

    if (getuid() == st_uid)
        snprintf(name, sizeof(name), _("You"));
    else
    {
        pwd = getpwuid(st_uid);
        if (pwd)
            snprintf(name, sizeof(name), "%s", pwd->pw_name);
        else
            snprintf(name, sizeof(name), "%-8d", (int)st_uid);
    }

   return strdup(name);
}



static char *
_file_perms_get(mode_t st_mode, uid_t st_uid, gid_t st_gid)
{
   char perms[256];
   int acc = 0;
   int owner = 0;
   int group = 0;
   int user_read = 0;
   int user_write = 0;
   int group_read = 0;
   int group_write = 0;
   int other_read = 0;
   int other_write = 0;

   if (getuid() == st_uid)
      owner = 1;
   if (getgid() == st_gid)
      group = 1;

   if ((S_IRUSR & st_mode) == S_IRUSR)
     user_read = 1;
   if ((S_IWUSR & st_mode) == S_IWUSR)
     user_write = 1;

   if ((S_IRGRP & st_mode) == S_IRGRP)
     group_read = 1;
   if ((S_IWGRP & st_mode) == S_IWGRP)
     group_write = 1;

   if ((S_IROTH & st_mode) == S_IROTH)
     other_read = 1;
   if ((S_IWOTH & st_mode) == S_IWOTH)
     other_write = 1;

   if (owner)
     {
        if ((!user_read) && (!user_write))
	  snprintf(perms, sizeof(perms), _("Protected"));
        else if ((user_read) && (!user_write))
	  snprintf(perms, sizeof(perms), _("Read Only"));
        else if ((user_read) && (user_write))
	  acc = 1;
     }
   else if (group)
     {
        if ((!group_read) && (!group_write))
	  snprintf(perms, sizeof(perms), _("Forbidden"));
        else if ((group_read) && (!group_write))
	  snprintf(perms, sizeof(perms), _("Read Only"));
        else if ((group_read) && (group_write))
	  acc = 1;
     }
   else
     {
        if ((!other_read) && (!other_write))
	  snprintf(perms, sizeof(perms), _("Forbidden"));
        else if ((other_read) && (!other_write))
	  snprintf(perms, sizeof(perms), _("Read Only"));
        else if ((other_read) && (other_write))
	  acc = 1;
     }
   if (!acc)
     return strdup(perms);
   else
     return strdup(_("Read-Write"));
}


static char *
_file_size_get(off_t size)
{
   double dsize;
   char buf[256];

   dsize = (double)size;
   if (dsize < 1024.0) snprintf(buf, sizeof(buf), _("%'.0f Bytes"), dsize);
   else
     {
	dsize /= 1024.0;
	if (dsize < 1024) snprintf(buf, sizeof(buf), _("%'.0f KB"), dsize);
	else
	  {
	     dsize /= 1024.0;
	     if (dsize < 1024) snprintf(buf, sizeof(buf), _("%'.0f MB"), dsize);
	     else
	       {
		  dsize /= 1024.0;
		  snprintf(buf, sizeof(buf), _("%'.1f GB"), dsize);
	       }
	  }
     }
   return strdup(buf);
}


static void
_fs_selected_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
    struct stat st;
	MyFileSelector *myfs = (MyFileSelector *) data;
	char *size, *owner, *perms;

   if (stat(event_info, &st) < 0) return;

	elm_object_text_set(myfs->filename_entry, event_info);
    size =  _file_size_get(st.st_size);
    owner = _file_user_get(st.st_uid);
    perms = _file_perms_get(st.st_mode, st.st_uid, st.st_gid);

    elm_object_text_set(myfs->size_entry, size);
    elm_object_text_set(myfs->owner_entry, owner);
    elm_object_text_set(myfs->permissions_entry, perms);

#ifdef HAVE_EVIL
    elm_object_text_set(myfs->mimetype_entry, _("*FIXME*"));
#else
    elm_object_text_set(myfs->mimetype_entry, efreet_mime_type_get(event_info));
#endif

    if((eina_str_has_extension(event_info, ".png") == EINA_TRUE) ||
    (eina_str_has_extension(event_info, ".jpg") == EINA_TRUE) ||
    (eina_str_has_extension(event_info, ".jpeg") == EINA_TRUE) ||
    (eina_str_has_extension(event_info, ".gif") == EINA_TRUE) ||
    (eina_str_has_extension(event_info, ".bmp") == EINA_TRUE))
    {
        elm_image_file_set(myfs->preview_img, event_info, NULL);
    }
    else
    {
        elm_image_file_set(myfs->preview_img, "", NULL);
    }

   FREE(perms);
   FREE(owner);
}


//
//Close a window by deleting evas object passing arg.
//
static void
_cancel_bt_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    myfileselector_close(data);
}


//
//
//
void
myfileselector_set_title(MyFileSelector *myfs, const char *title)
{
	elm_win_title_set(myfs->win, title);
}


//
//
//
void
myfileselector_close(MyFileSelector *myfs)
{
    evas_object_del(myfs->win);
    FREE(myfs);
}


MyFileSelector *
myfileselector_add()
{
	Evas_Object *label, *ic;

    MyFileSelector *myfs = calloc(1, sizeof(MyFileSelector));

    myfs->win = elm_win_util_standard_add("myfileselector", _("Select a file..."));
    elm_win_center(myfs->win, EINA_TRUE, EINA_TRUE);
	elm_win_autodel_set(myfs->win, EINA_TRUE);


    Evas_Object *gd = elm_grid_add(myfs->win);
    elm_grid_size_set(gd, 100, 100);
    elm_win_resize_object_add(myfs->win, gd);
    evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(gd);

    //TODO:implement shotcut adds(and remove too!).
    //bt =elm_button_add(myfs->win);
	//elm_object_text_set(bt, "Add to shortcuts");
	//ic = elm_icon_add(bt);
   	//elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	//elm_icon_standard_set(ic, "go-up");
   	//elm_object_part_content_set(bt, "icon", ic);
    //elm_grid_pack(gd, bt, 0, 0, 20, 7);
    //evas_object_show(bt);


    myfs->fs = elm_fileselector_add(myfs->win);
    elm_fileselector_buttons_ok_cancel_set(myfs->fs, EINA_FALSE);
    elm_fileselector_is_save_set(myfs->fs, EINA_FALSE);
    elm_fileselector_path_set(myfs->fs, user_home_get());
    evas_object_size_hint_weight_set(myfs->fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(myfs->fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_grid_pack(gd, myfs->fs , 1, 1, 50, 80);
    evas_object_show(myfs->fs);
    evas_object_smart_callback_add(myfs->fs, "selected", _fs_selected_cb, myfs);


    //Set preview panel.
    myfs->preview_img =  elm_image_add(myfs->win);
    elm_image_smooth_set(myfs->preview_img, EINA_TRUE);
    elm_image_aspect_fixed_set(myfs->preview_img, EINA_TRUE);
    evas_object_size_hint_weight_set(myfs->preview_img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(myfs->preview_img);

    myfs->preview_frame= elm_frame_add(myfs->win);
    elm_object_text_set(myfs->preview_frame, _("Preview"));
    elm_object_content_set(myfs->preview_frame, myfs->preview_img);
    elm_grid_pack(gd, myfs->preview_frame, 60, 10, 30, 30);
    evas_object_show(myfs->preview_frame);

    label = elm_label_add(myfs->win);
    elm_object_text_set(label, _("Type :"));
    elm_grid_pack(gd, label, 55, 40, 20, 7);
	evas_object_show(label);

    myfs->mimetype_entry = elm_entry_add(myfs->win);
    elm_entry_editable_set(myfs->mimetype_entry, EINA_FALSE);
    elm_entry_scrollable_set(myfs->mimetype_entry , EINA_TRUE);
    elm_entry_single_line_set(myfs->mimetype_entry , EINA_TRUE);
    elm_grid_pack(gd, myfs->mimetype_entry , 70, 40, 20, 7);
    evas_object_show(myfs->mimetype_entry );

    label = elm_label_add(myfs->win);
    elm_object_text_set(label, _("Size :"));
    elm_grid_pack(gd, label, 55, 50, 20, 7);
	evas_object_show(label);

    myfs->size_entry  = elm_entry_add(myfs->win);
    elm_entry_editable_set(myfs->size_entry, EINA_FALSE);
    elm_entry_scrollable_set(myfs->size_entry, EINA_TRUE);
    elm_entry_single_line_set(myfs->size_entry, EINA_TRUE);
    elm_grid_pack(gd, myfs->size_entry, 70, 50, 20, 7);
    evas_object_show(myfs->size_entry);

    label = elm_label_add(myfs->win);
    elm_object_text_set(label,  _("Owner :"));
    elm_grid_pack(gd, label, 55, 60, 20, 7);
	evas_object_show(label);

    myfs->owner_entry = elm_entry_add(myfs->win);
    elm_entry_editable_set(myfs->owner_entry, EINA_FALSE);
    elm_entry_scrollable_set(myfs->owner_entry, EINA_TRUE);
    elm_entry_single_line_set(myfs->owner_entry, EINA_TRUE);
    elm_grid_pack(gd, myfs->owner_entry, 70, 60, 20, 7);
    evas_object_show(myfs->owner_entry);

    label = elm_label_add(myfs->win);
    elm_object_text_set(label,  _("Permissions :"));
    elm_grid_pack(gd, label, 55, 70, 20, 7);
	evas_object_show(label);

    myfs->permissions_entry = elm_entry_add(myfs->win);
    elm_entry_editable_set(myfs->permissions_entry, EINA_FALSE);
    elm_entry_scrollable_set(myfs->permissions_entry, EINA_TRUE);
    elm_entry_single_line_set(myfs->permissions_entry, EINA_TRUE);
    elm_grid_pack(gd, myfs->permissions_entry, 70, 70, 20, 7);
    evas_object_show(myfs->permissions_entry);
    //End of preview panel.

    //Show cucrent selected file.
    myfs->filename_entry = elm_entry_add(myfs->win);
    elm_entry_scrollable_set(myfs->filename_entry, EINA_TRUE);
    elm_entry_single_line_set(myfs->filename_entry, EINA_TRUE);
    elm_grid_pack(gd, myfs->filename_entry, 0, 83, 100, 7);
    evas_object_show(myfs->filename_entry);

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
