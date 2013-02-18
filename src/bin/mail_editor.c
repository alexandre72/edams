#include <Elementary.h>
#include "cJSON.h"
#include "edams.h"



/*Global window elm object*/
static Evas_Object *win = NULL;


/*
 *
 */
const char*
mail_editor_values_get()
{
    const char *s;

	Evas_Object *from_entry = elm_object_name_find(win, "from entry", -1);
	Evas_Object *to_entry = elm_object_name_find(win, "to entry", -1);
	Evas_Object *subject_entry = elm_object_name_find(win, "subject entry", -1);
	Evas_Object *body_entry = elm_object_name_find(win, "body entry", -1);

    s = action_mail_data_format(    elm_object_text_get(from_entry),
                                   	elm_object_text_get(to_entry),
	                                elm_object_text_get(subject_entry),
	                                elm_object_text_get(body_entry));
	return s;
}/*_button_edit_arg_apply_clicked_cb*/


Evas_Object *
mail_editor_hbox_get()
{
	Evas_Object *hbox = elm_object_name_find(win, "hbox", -1);
	return hbox;
}



/*
 *
 */
Evas_Object *
mail_editor_add()
{
	Evas_Object *grid, *hbox, *frame;
	Evas_Object *entry;

	win = elm_win_util_standard_add("mail_editor", NULL);
	elm_win_title_set(win, _("Edit mail"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("From:"));
	elm_grid_pack(grid, frame, 1, 1, 99, 15);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "from entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_object_text_set(entry, edams_settings_user_name_get());
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("To:"));
	elm_grid_pack(grid, frame, 1, 17, 99, 15);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "to entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Subject:"));
	elm_grid_pack(grid, frame, 1, 32, 99, 15);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "subject entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_object_text_set(entry, _("[EDAMS]About..."));
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Body:"));
	elm_grid_pack(grid, frame, 1, 47, 99, 40);
	evas_object_show(frame);

	entry = elm_entry_add(win);
	evas_object_name_set(entry, "body entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_FALSE);
	evas_object_show(entry);
	elm_object_content_set(frame, entry);

	hbox = elm_box_add(win);
	evas_object_name_set(hbox, "hbox");
	elm_box_horizontal_set(hbox, EINA_TRUE);
	elm_box_homogeneous_set(hbox, EINA_TRUE);
	elm_grid_pack(grid, hbox, 1, 89, 99, 10);
	evas_object_show(hbox);

	evas_object_resize(win, 400, 400);
	evas_object_show(win);

	return win;
}/*mail_editor_add*/
