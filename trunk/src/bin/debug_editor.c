#include <Elementary.h>
#include "cJSON.h"
#include "edams.h"

/*Global window elm object*/
static Evas_Object *win = NULL;


/*
 *
 */
const char*
debug_editor_values_get()
{
    const char *s;
	Evas_Object *entry = elm_object_name_find(win, "debug entry", -1);
	s = action_debug_data_format(elm_object_text_get(entry));

	return s;
}/*debug_editor_hbox_get*/


/*
 *
 */
Evas_Object *
debug_editor_hbox_get()
{
	Evas_Object *hbox = elm_object_name_find(win, "hbox", -1);
	return hbox;
}/*debug_editor_hbox_get*/


/*
 *
 */
Evas_Object *
debug_editor_add()
{
	Evas_Object *grid, *hbox, *frame;
	Evas_Object *entry;

	win = elm_win_util_standard_add("debug_editor", NULL);
	elm_win_title_set(win, _("Edit debug"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Printf message:"));
	elm_grid_pack(grid, frame, 1, 1, 99, 15);
	evas_object_show(frame);

    entry = elm_entry_add(win);
	evas_object_name_set(entry, "debug entry");
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
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
}/*debug_editor_add*/
