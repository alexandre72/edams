#include <Elementary.h>
#include "cJSON.h"
#include "edams.h"



/*Global window elm object*/
static Evas_Object *win = NULL;


/*
 *
 */
const char*
cmnd_editor_values_get()
{
	cJSON *root;
    const char *s;

	Device *device = evas_object_data_get(win, "device");
    if((!device) || (!device_current_get(device)))
        return NULL;

	root=cJSON_CreateObject();
	cJSON_AddItemToObject(root, "DEVICE_FILENAME", cJSON_CreateString(device_filename_get(device)));
	cJSON_AddItemToObject(root, "CURRENT", cJSON_CreateString(device_current_get(device)));
	//cJSON_AddItemToObject(root, "DATA1", cJSON_CreateString(device_data1_get(device)));

    s = cJSON_PrintUnformatted(root);

	cJSON_Delete(root);

	return s;
}/*cmnd_editor_values_get*/



/*
 *
 */
Evas_Object *
cmnd_editor_hbox_get()
{
	Evas_Object *hbox = elm_object_name_find(win, "hbox", -1);
	return hbox;
}/*cmnd_editor_hbox_get*/


/*
 *
 */
static void
_slider_control_basic_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
    Device *device = data;
    char *s;

    double val = elm_slider_value_get(obj);
    asprintf(&s, "%1.0f", val);

    device_current_set(device, s);

    FREE(s);

}/*_slider_control_basic_changed_cb*/




/*
 *
 */
static void


_radio_control_basic_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
    Device *device = data;

    device_current_set(device, elm_object_text_get(obj));
}/*_radio_control_basic_changed_cb*/



/*
 *Callback called in 'control.basic' list when clicked signal is emitted.
 */
static void
_list_control_basic_item_selected_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
    Evas_Object *bx;
    Evas_Object *group, *radio, *slider;
    Evas_Object *frame;
    Evas_Object *grid = elm_object_name_find(win, "grid", -1);
    Device *device = data;
    Device_Type type = device_type_get(device);

	frame = elm_frame_add(win);
    elm_object_text_set(frame, _("Value"));
    elm_grid_pack(grid, frame, 1, 42, 99, 40);
    evas_object_show(frame);

    switch(type)
    {
        case INPUT_CONTROL_BASIC_TYPE:
   		case MUTE_CONTROL_BASIC_TYPE:
        case MACRO_CONTROL_BASIC_TYPE:
        case FLAG_CONTROL_BASIC_TYPE:
        case OUTPUT_CONTROL_BASIC_TYPE:
		case INFRARED_CONTROL_BASIC_TYPE:
        case PERIODIC_CONTROL_BASIC_TYPE:
        case SCHEDULED_CONTROL_BASIC_TYPE:
        case TIMER_CONTROL_BASIC_TYPE:

                bx = elm_box_add(win);
                elm_box_horizontal_set(bx, EINA_FALSE);
                evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
                evas_object_show(bx);

                group = radio = elm_radio_add(win);

                    if(type == MUTE_CONTROL_BASIC_TYPE)
                        elm_object_text_set(radio, "Yes");
                      else if((type == INPUT_CONTROL_BASIC_TYPE) ||
                            (type == MACRO_CONTROL_BASIC_TYPE) ||
                            (type == SCHEDULED_CONTROL_BASIC_TYPE) ||
                            (type == PERIODIC_CONTROL_BASIC_TYPE))
                        elm_object_text_set(radio, "Enable");
                    else if(type == FLAG_CONTROL_BASIC_TYPE)
                        elm_object_text_set(radio, "Set");
                    else if(type == INFRARED_CONTROL_BASIC_TYPE)
                        elm_object_text_set(radio, "Send");
                    else if(type == TIMER_CONTROL_BASIC_TYPE)
                        elm_object_text_set(radio, "Off");

                elm_radio_state_value_set(radio, 0);
                evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                elm_box_pack_end(bx, radio);
                evas_object_show(radio);
                evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, device);

                radio = elm_radio_add(win);
                elm_radio_group_add(radio, group);

                    if(type == MUTE_CONTROL_BASIC_TYPE)
                        elm_object_text_set(radio, "No");
                    else if((type == INPUT_CONTROL_BASIC_TYPE) ||
                            (type == MACRO_CONTROL_BASIC_TYPE) ||
                            (type == SCHEDULED_CONTROL_BASIC_TYPE) ||
                            (type == PERIODIC_CONTROL_BASIC_TYPE))
                        elm_object_text_set(radio, "Disable");
                    else if(type == FLAG_CONTROL_BASIC_TYPE)
                        elm_object_text_set(radio, "Clear");
                    else if(type == INFRARED_CONTROL_BASIC_TYPE)
                        elm_object_text_set(radio, "Enable_rx");
                    else if(type == TIMER_CONTROL_BASIC_TYPE)
                            elm_object_text_set(radio, "Start");

                elm_radio_state_value_set(radio, 1);
                evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                elm_box_pack_end(bx, radio);
                evas_object_show(radio);
                evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, device);

                    if((type == MACRO_CONTROL_BASIC_TYPE) ||
                        (type == FLAG_CONTROL_BASIC_TYPE) ||
                        (type == PERIODIC_CONTROL_BASIC_TYPE) ||
                        (type == SCHEDULED_CONTROL_BASIC_TYPE) ||
                        (type == TIMER_CONTROL_BASIC_TYPE) ||
                        (type == OUTPUT_CONTROL_BASIC_TYPE))
                    {
                        radio = elm_radio_add(win);
                        elm_radio_group_add(radio, group);

                            if((type == MACRO_CONTROL_BASIC_TYPE))
                                elm_object_text_set(radio, "Do");
                            else if(type == FLAG_CONTROL_BASIC_TYPE)
                                elm_object_text_set(radio, "Neutral");
                            else if(type == OUTPUT_CONTROL_BASIC_TYPE)
                                elm_object_text_set(radio, "high");
                            else if(type == INFRARED_CONTROL_BASIC_TYPE)
                                elm_object_text_set(radio, "Disable_rx");
                            else if((type == PERIODIC_CONTROL_BASIC_TYPE) ||
                                    (type == SCHEDULED_CONTROL_BASIC_TYPE))
                                elm_object_text_set(radio, "Started");
                            else if(type == TIMER_CONTROL_BASIC_TYPE)
                                 elm_object_text_set(radio, "Stop");

                        elm_radio_state_value_set(radio, 2);
                        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                        elm_box_pack_end(bx, radio);
                        evas_object_show(radio);
                        evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, device);
                    }

                    if((type == INFRARED_CONTROL_BASIC_TYPE) ||
                        (type == TIMER_CONTROL_BASIC_TYPE) ||
                        (type == OUTPUT_CONTROL_BASIC_TYPE))
                    {
                        radio = elm_radio_add(win);
                        elm_radio_group_add(radio, group);

                            if(type == OUTPUT_CONTROL_BASIC_TYPE)
                                elm_object_text_set(radio, "low");
                            else if(type == INFRARED_CONTROL_BASIC_TYPE)
                                elm_object_text_set(radio, "Enable_tx");
                            else if(type == TIMER_CONTROL_BASIC_TYPE)
                                 elm_object_text_set(radio, "Halt");

                        elm_radio_state_value_set(radio, 3);
                        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                        elm_box_pack_end(bx, radio);
                        evas_object_show(radio);
                        evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, device);
                    }

                   if((type == INFRARED_CONTROL_BASIC_TYPE) ||
                        (type == TIMER_CONTROL_BASIC_TYPE) ||
                        (type == OUTPUT_CONTROL_BASIC_TYPE))
                    {
                        radio = elm_radio_add(win);
                        elm_radio_group_add(radio, group);

                            if(type == OUTPUT_CONTROL_BASIC_TYPE)
                                elm_object_text_set(radio, "pulse");
                          else if(type == INFRARED_CONTROL_BASIC_TYPE)
                                elm_object_text_set(radio, "Disable_tx");
                            else if(type == TIMER_CONTROL_BASIC_TYPE)
                                 elm_object_text_set(radio, "Resume");

                        elm_radio_state_value_set(radio, 4);
                        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                        elm_box_pack_end(bx, radio);
                        evas_object_show(radio);
                        evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, device);
                    }


                    if(type == OUTPUT_CONTROL_BASIC_TYPE)
                    {
                        radio = elm_radio_add(win);
                        elm_radio_group_add(radio, group);

                            if(type == OUTPUT_CONTROL_BASIC_TYPE)
                                elm_object_text_set(radio, "toggle");

                        elm_radio_state_value_set(radio, 5);
                        evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                        evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
                        elm_box_pack_end(bx, radio);
                        evas_object_show(radio);
                        evas_object_smart_callback_add(radio, "changed", _radio_control_basic_changed_cb, device);
                    }


                elm_object_content_set(frame, bx);
                break;

	            case BALANCE_CONTROL_BASIC_TYPE:
        		case SLIDER_CONTROL_BASIC_TYPE:
        		case VARIABLE_CONTROL_BASIC_TYPE:
                                slider = elm_slider_add(win);
                                elm_slider_unit_format_set(slider, "%1.0f");

                                if(type == BALANCE_CONTROL_BASIC_TYPE)
                                    elm_slider_min_max_set(slider, -100, 100);
                                else if((type == SLIDER_CONTROL_BASIC_TYPE) ||
                                        (type == SLIDER_CONTROL_BASIC_TYPE))
                                    elm_slider_min_max_set(slider, 0, 255);

                                evas_object_show(slider);
                                evas_object_smart_callback_add(slider, "changed", _slider_control_basic_changed_cb, NULL);
                                elm_object_content_set(frame, slider);
	                            break;

                case UNKNOWN_DEVICE_TYPE:
                break;
    }

    evas_object_data_set(win, "device", device);
}/*_list_control_basic_item_selected_cb*/


/*
 *
 */
static void
_list_control_basic_item_add(Evas_Object *list, Device *device)
{
	const char *s;

	asprintf(&s, "%s %s", device_name_get(device), device_type_to_str(device_type_get(device)));

	elm_list_item_append(list, s, NULL, NULL, _list_control_basic_item_selected_cb, device);
	elm_list_go(list);
}/*_list_control_basic_item_selected_cb*/


/*
 *
 */
Evas_Object *
cmnd_editor_add(App_Info *app)
{
	Evas_Object *grid, *hbox, *frame;
	Evas_Object *list;

	win = elm_win_util_standard_add("cmnd_editor", NULL);
	elm_win_title_set(win, _("Edit cmnd control.basic"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	evas_object_name_set(grid, "grid");
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Select control.basic"));
	elm_grid_pack(grid, frame, 1, 1, 99, 40);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_scroller_policy_set(list, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_ON);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_name_set(list, "control.basic list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

	Eina_List *l;
	Device *device;
	EINA_LIST_FOREACH(app->devices, l, device)
	{
        if(device_class_get(device) == CONTROL_BASIC_CLASS)
    	    _list_control_basic_item_add(list, device);
    }

	hbox = elm_box_add(win);
	evas_object_name_set(hbox, "hbox");
	elm_box_horizontal_set(hbox, EINA_TRUE);
	elm_box_homogeneous_set(hbox, EINA_TRUE);
	elm_grid_pack(grid, hbox, 1, 89, 99, 10);
	evas_object_show(hbox);

	evas_object_resize(win, 400, 400);
	evas_object_show(win);

	return win;
}/*cmnd_editor_add*/
