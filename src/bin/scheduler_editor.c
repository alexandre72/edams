/*
 * scheduler_editor.c
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

#include "cJSON.h"
#include "crontab.h"
#include "cmnd_editor.h"
#include "debug_editor.h"
#include "edams.h"
#include "exec_editor.h"
#include "mail_editor.h"
#include "myfileselector.h"
#include "osd_editor.h"
#include "path.h"
#include "settings.h"
#include "utils.h"
#include "voice_editor.h"


/*Global objects*/
static Evas_Object *win = NULL;


/*Evas_Object Callbacks*/
static void _button_add_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _button_remove_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__);
static void _hoversel_selected_cb(void *data, Evas_Object *obj, void *event_info);
/*Others funcs*/
static void _list_crons_add(Evas_Object *list, Cron_Entry *cron_elem);


/*
 *
 */
static void
_cmndeditor_button_ok_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	CmndEditor *cmndeditor = data;
    const char *s;

	Elm_Object_Item *selected_item = elm_list_selected_item_get(cmndeditor->list);

	if(!selected_item) return;

    Widget *widget = elm_object_item_data_get(selected_item);

    s = action_cmnd_data_format(widget_xpl_device_get(widget),
                            widget_xpl_type_get(widget),
                            widget_xpl_current_get(widget),
                            widget_xpl_data1_get(widget));


    evas_object_data_set(win, "action data", s);
	cmndeditor_close(cmndeditor);
}/*_execeditor_button_action_clicked_cb*/

/*
 *
 */
static void
_execeditor_button_ok_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	ExecEditor *execeditor = data;
    const char *s;

    s = action_exec_data_format(elm_object_text_get(execeditor->exec_entry),
                                elm_check_state_get(execeditor->terminal_check) ? "true" : "false");
    evas_object_data_set(win, "action data", s);
	execeditor_close(execeditor);
}/*_debugeditor_button_action_clicked_cb*/

/*
 *
 */
static void
_osdeditor_button_ok_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	OsdEditor *osdeditor = data;
    const char *s;

    s = action_osd_data_format(elm_object_text_get(osdeditor->command_entry),
                                elm_object_text_get(osdeditor->text_entry),
                                round(elm_slider_value_get(osdeditor->delay_slider)));

    evas_object_data_set(win, "action data", s);
	osdeditor_close(osdeditor);
}/*_debugeditor_button_action_clicked_cb*/

/*
 *
 */
static void
_debugeditor_button_ok_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	DebugEditor *debugeditor = data;
    const char *s;

    s = action_debug_data_format(elm_object_text_get(debugeditor->message_entry));
    evas_object_data_set(win, "action data", s);
	debugeditor_close(debugeditor);
}/*_debugeditor_button_action_clicked_cb*/

/*
 *
 */
static void
_maileditor_button_ok_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	MailEditor *maileditor = data;
    const char *s;

    s = action_mail_data_format(elm_object_text_get(maileditor->from_entry),
                                elm_object_text_get(maileditor->to_entry),
                                elm_object_text_get(maileditor->subject_entry),
                                elm_object_text_get(maileditor->body_entry));
    evas_object_data_set(win, "action data", s);
	maileditor_close(maileditor);
}/*_maileditor_button_action_clicked_cb*/


/*
 *
 */
static void
_voiceeditor_button_ok_clicked_cb(void *data, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	VoiceEditor *voiceeditor = data;
    char *to_file;
    const char *s;

    if(voiceeditor->sound_file)
    {
        asprintf(&to_file, "%s/%s", edams_sounds_data_path_get(), ecore_file_file_get(voiceeditor->sound_file));
        ecore_file_mv(voiceeditor->sound_file, to_file);
        FREE(voiceeditor->sound_file);

        s = action_voice_data_format(elm_object_text_get(voiceeditor->message_entry), to_file);
        FREE(to_file);
        evas_object_data_set(win, "action data", s);
    }

	voiceeditor_close(voiceeditor);
}/*_voiceeditor_button_action_clicked_cb*/



/*
 *Callback called in any hoversel objects when clicked signal is emitted.
 */
static void
_hoversel_selected_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
    Action_Type type = (Action_Type)elm_object_item_data_get(event_info);
    cJSON *root = NULL;

	Evas_Object *list = elm_object_name_find(win, "crons list", -1);
	Elm_Object_Item *selected_item = elm_list_selected_item_get(list);


	if(selected_item)
	{
        Cron_Entry *cron_elem = elm_object_item_data_get(selected_item);
        root = cJSON_Parse(cron_elem->action_data);
    }

	elm_object_text_set(obj, elm_object_item_text_get(event_info));
    evas_object_data_set(win, "action type", (void*)type);

    if(type == ACTION_TYPE_MAIL)
    {
        MailEditor *maileditor;
        maileditor = maileditor_add();
        evas_object_smart_callback_add(maileditor->ok_button, "clicked", _maileditor_button_ok_clicked_cb, maileditor);

        if(!root)
        {
    	    elm_object_text_set(maileditor->from_entry, edams_settings_user_mail_get());
        	elm_object_text_set(maileditor->subject_entry, _("[EDAMS]About..."));
        }
        else
        {
            cJSON *jfrom = cJSON_GetObjectItem(root, "FROM");
            cJSON *jto = cJSON_GetObjectItem(root, "TO");
            cJSON *jsubject = cJSON_GetObjectItem(root, "SUBJECT");
            cJSON *jbody = cJSON_GetObjectItem(root, "BODY");

            char *from = cJSON_PrintUnformatted(jfrom);
            char *to = cJSON_PrintUnformatted(jto);
            char *subject = cJSON_PrintUnformatted(jsubject);
            char *body = cJSON_PrintUnformatted(jbody);

            strdelstr(from, "\"");
            strdelstr(to, "\"");
            strdelstr(subject, "\"");
            strdelstr(body, "\"");

    	    elm_object_text_set(maileditor->from_entry, from);
            elm_object_text_set(maileditor->to_entry, to);
        	elm_object_text_set(maileditor->subject_entry, subject);
            elm_object_text_set(maileditor->body_entry, body);
        }
        return;
    }
    else if(type == ACTION_TYPE_DEBUG)
    {
        DebugEditor *debugeditor;
        debugeditor = debugeditor_add();
        evas_object_smart_callback_add(debugeditor->ok_button, "clicked", _debugeditor_button_ok_clicked_cb, debugeditor);
        //elm_object_text_set(debugeditor->message, );
        return;
    }
    else if(type == ACTION_TYPE_OSD)
    {
        OsdEditor *osdeditor;
        osdeditor = osdeditor_add();
        evas_object_smart_callback_add(osdeditor->ok_button, "clicked", _osdeditor_button_ok_clicked_cb, osdeditor);
        //elm_object_text_set(osdeditor->message, );
    }
    else if(type == ACTION_TYPE_EXEC)
    {
        ExecEditor *execeditor;
        execeditor = execeditor_add();
        evas_object_smart_callback_add(execeditor->ok_button, "clicked", _execeditor_button_ok_clicked_cb, execeditor);
        //elm_object_text_set(execeditor->message, );
    }
    else if(type == ACTION_TYPE_CMND)
    {
        CmndEditor *cmndeditor;
        cmndeditor = cmndeditor_add();
        evas_object_smart_callback_add(cmndeditor->ok_button, "clicked", _cmndeditor_button_ok_clicked_cb, cmndeditor);
        //elm_object_text_set(cmndeditor->message, );
    }
    else if(type == ACTION_TYPE_VOICE)
    {
        VoiceEditor *voiceeditor;
        voiceeditor = voiceeditor_add();
        evas_object_smart_callback_add(voiceeditor->ok_button, "clicked", _voiceeditor_button_ok_clicked_cb, voiceeditor);

        if(root)
        {
            cJSON *jtext = cJSON_GetObjectItem(root, "TEXT");
            char *text = cJSON_PrintUnformatted(jtext);
            strdelstr(text, "\"");
    	    elm_object_text_set(voiceeditor->message_entry, text);
        }
        return;
    }

}/*_hoversel_selected_cb*/


/*
 *Callback called in button "remove" object when clicked signal is emitted.
 */
static void
_button_remove_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *list = elm_object_name_find(win, "crons list", -1);

	Elm_Object_Item *selected_item = elm_list_selected_item_get(list);

	if(!selected_item) return;

    Cron_Entry *cron_elem = elm_object_item_data_get(selected_item);

    crons_list_entry_remove(cron_elem);
    elm_object_item_del(selected_item);
}/*_button_remove_clicked_cb*/



/*
 *Callback called in button "add" object when clicked signal is emitted.
 */
static void
_button_add_clicked_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *spinner;
	Evas_Object *list = elm_object_name_find(win, "crons list", -1);
    Cron_Entry *cron_elem = NULL;
	Action_Type action_type = (Action_Type) evas_object_data_get(win, "action type");
	char *action_data = evas_object_data_get(win, "action data");
    double val;
    char *minute, *hour;
    char *day_month, *month, *day_week;

	if(!action_data) return;

	spinner = elm_object_name_find(win, "minute spinner", -1);
    val = elm_spinner_value_get(spinner);
        if(val == 60)
            asprintf(&minute, "*");
        else
            asprintf(&minute, "%d", (int)val);

	spinner = elm_object_name_find(win, "hour spinner", -1);
    val = elm_spinner_value_get(spinner);
        if(val == 24)
            asprintf(&hour, "*");
        else
            asprintf(&hour, "%d", (int)val);

	spinner = elm_object_name_find(win, "day month spinner", -1);
    val = elm_spinner_value_get(spinner);
        if(val == 32)
            asprintf(&day_month, "*");
        else
            asprintf(&day_month, "%d", (int)val);

	spinner = elm_object_name_find(win, "month spinner", -1);
    val = elm_spinner_value_get(spinner);
        if(val == 13)
            asprintf(&month, "*");
        else
            asprintf(&month, "%d", (int)val);

	spinner = elm_object_name_find(win, "day week spinner", -1);
    val = elm_spinner_value_get(spinner);
        if(val == 7)
            asprintf(&day_week, "*");
        else
            asprintf(&day_week, "%d", (int)val);

    cron_elem = cron_entry_new(minute, hour, day_month, month, day_week, action_type, action_data);
    crons_list_entry_add(cron_elem);
    FREE(minute);
    FREE(hour);
    FREE(day_month);
    FREE(month);
    FREE(day_week);

	_list_crons_add(list, cron_elem);
}/*_button_add_clicked_cb*/


/*
 *Callback called in list "actions" when item clicked signal is emitted.
 */
static void
_list_item_selected_cb(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info)
{
	Evas_Object *spinner;
	Cron_Entry *cron_elem = (Cron_Entry *)elm_object_item_data_get(event_info);

	spinner = elm_object_name_find(win, "minute spinner", -1);
    if(strcmp(cron_elem->minute, "*" ) == 0)
        elm_spinner_value_set(spinner , (double)60);
    else
        elm_spinner_value_set(spinner, (double)atoi(cron_elem->minute));

	spinner = elm_object_name_find(win, "hour spinner", -1);
    if(strcmp(cron_elem->hour, "*" ) == 0)
        elm_spinner_value_set(spinner , (double)24);
    else
        elm_spinner_value_set(spinner, (double)atoi(cron_elem->hour));

	spinner = elm_object_name_find(win, "day month spinner", -1);
    if(strcmp(cron_elem->day_month, "*" ) == 0)
        elm_spinner_value_set(spinner , (double)32);
    else
        elm_spinner_value_set(spinner, (double)atoi(cron_elem->day_month));

   	spinner = elm_object_name_find(win, "month spinner", -1);
    if(strcmp(cron_elem->month, "*" ) == 0)
        elm_spinner_value_set(spinner , (double)13);
    else
        elm_spinner_value_set(spinner, (double)atoi(cron_elem->month));

	spinner = elm_object_name_find(win, "day week spinner", -1);
    if(strcmp(cron_elem->day_week, "*" ) == 0)
        elm_spinner_value_set(spinner , (double)7);
    else
        elm_spinner_value_set(spinner, (double)atoi(cron_elem->day_week));

    evas_object_data_set(win, "action type", (void*)cron_elem->action_type);
    evas_object_data_set(win, "action data", (void*)cron_elem->action_data);
}/*_list_item_selected_cb*/




/*
 *Add cron_elem to list object
 */
static void
_list_crons_add(Evas_Object *list, Cron_Entry *cron_elem)
{
    Action_Type type = cron_elem->action_type;

    if(type == ACTION_TYPE_UNKNOWN) return;

    char *s;
    asprintf(&s, _("On %s %s %s %s %s"),
                    day_week_to_str(cron_elem->day_week), day_month_to_str(cron_elem->day_month),
                    month_to_str(cron_elem->month),
                    hour_to_str(cron_elem->hour), minute_to_str(cron_elem->minute));

	Evas_Object *icon;
	icon = elm_icon_add(win);
    elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
    evas_object_size_hint_min_set(icon, 48, 48);
    evas_object_size_hint_align_set(icon, 0.5, EVAS_HINT_FILL);

    if(type == ACTION_TYPE_CMND)
        elm_icon_standard_set(icon, "xpl-logo");
    else if(type == ACTION_TYPE_MAIL)
        elm_icon_standard_set(icon, "mail-message-new");
    else if(type == ACTION_TYPE_EXEC)
        elm_icon_standard_set(icon, "system-run");
    else if(type == ACTION_TYPE_DEBUG)
        elm_icon_standard_set(icon, "debug-action");
    else if(type == ACTION_TYPE_DEBUG)
        elm_icon_standard_set(icon, "osd-action");
    else if(type == ACTION_TYPE_VOICE)
        elm_icon_standard_set(icon, "voice-action");
    else
       	elm_image_file_set(icon, edams_edje_theme_file_get(), "");

    elm_list_item_append(list, s, icon, NULL, _list_item_selected_cb, cron_elem);
	elm_list_go(list);
    FREE(s);
}/*_list_action_add*/


/*
 *
 */
void
scheduler_editor_new(void *data __UNUSED__, Evas_Object * obj __UNUSED__, void *event_info __UNUSED__)
{
	Evas_Object *grid;
	Evas_Object *icon, *bx, *frame, *separator;
	Evas_Object *list, *button, *hoversel, *spinner;
	unsigned int x = 0;

	win = elm_win_util_standard_add("scheduler_editor", _("Scheduler editor"));
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_center(win, EINA_TRUE, EINA_TRUE);

	grid = elm_grid_add(win);
	elm_grid_size_set(grid, 100, 100);
	elm_win_resize_object_add(win, grid);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(grid);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Events"));
	elm_grid_pack(grid, frame, 1, 1, 99, 67);
	evas_object_show(frame);

	list = elm_list_add(win);
	elm_list_select_mode_set(list ,ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_name_set(list, "crons list");
	evas_object_show(list);
	elm_object_content_set(frame, list);

    Eina_List *crons, *l;
    Cron_Entry *cron_elem;

    crons = crons_list_get();
    EINA_LIST_FOREACH(crons, l, cron_elem)
        _list_crons_add(list, cron_elem);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Weekday"));
	elm_grid_pack(grid, frame, 1, 68, 15, 12);
	evas_object_show(frame);

    spinner = elm_spinner_add(win);
   	evas_object_name_set(spinner, "day week spinner");
    elm_spinner_min_max_set(spinner, 0, 7);
    elm_spinner_special_value_add(spinner, 0, _("Sunday"));
    elm_spinner_special_value_add(spinner, 1, _("Monday"));
    elm_spinner_special_value_add(spinner, 2, _("Tuesday"));
    elm_spinner_special_value_add(spinner, 3, _("Wednesday"));
    elm_spinner_special_value_add(spinner, 4, _("Thursday"));
    elm_spinner_special_value_add(spinner, 5, _("Friday"));
    elm_spinner_special_value_add(spinner, 6, _("Saturday"));
    elm_spinner_special_value_add(spinner, 7, _("Every Weekday"));
    evas_object_show(spinner);
	elm_object_content_set(frame, spinner);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Day"));
	elm_grid_pack(grid, frame, 16, 68, 15, 12);
	evas_object_show(frame);

    spinner = elm_spinner_add(win);
   	evas_object_name_set(spinner, "day month spinner");
    elm_spinner_min_max_set(spinner, 1, 32);
    evas_object_show(spinner);
	elm_object_content_set(frame, spinner);

	for(x = 1;x != 32;x++)
	{   char *buf;
	    asprintf(&buf, "%d", x);
        elm_spinner_special_value_add(spinner, x, buf);
        FREE(buf);
	}
    elm_spinner_special_value_add(spinner, 32, _("Every Day"));

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Month"));
	elm_grid_pack(grid, frame, 31, 68, 15, 12);
	evas_object_show(frame);

    spinner = elm_spinner_add(win);
   	evas_object_name_set(spinner, "month spinner");
    elm_spinner_min_max_set(spinner, 1, 13);
    elm_spinner_special_value_add(spinner, 1, _("January"));
    elm_spinner_special_value_add(spinner, 2, _("February"));
    elm_spinner_special_value_add(spinner, 3, _("March"));
    elm_spinner_special_value_add(spinner, 4, _("April"));
    elm_spinner_special_value_add(spinner, 5, _("May"));
    elm_spinner_special_value_add(spinner, 6, _("June"));
    elm_spinner_special_value_add(spinner, 7, _("July"));
    elm_spinner_special_value_add(spinner, 8, _("August"));
    elm_spinner_special_value_add(spinner, 9, _("September"));
    elm_spinner_special_value_add(spinner, 10, _("October"));
    elm_spinner_special_value_add(spinner, 11, _("November"));
    elm_spinner_special_value_add(spinner, 12, _("December"));
    elm_spinner_special_value_add(spinner, 13, _("Every Month"));
    evas_object_show(spinner);
	elm_object_content_set(frame, spinner);


	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Hour"));
	elm_grid_pack(grid, frame, 47, 68, 15, 12);
	evas_object_show(frame);

    spinner = elm_spinner_add(win);
   	evas_object_name_set(spinner, "hour spinner");
    elm_spinner_min_max_set(spinner, 0, 24);
    elm_spinner_special_value_add(spinner, 0, _("12 Midnight"));
    elm_spinner_special_value_add(spinner, 1, _("1 AM"));
    elm_spinner_special_value_add(spinner, 2, _("2 AM"));
    elm_spinner_special_value_add(spinner, 3, _("3 AM"));
    elm_spinner_special_value_add(spinner, 4, _("4 AM"));
    elm_spinner_special_value_add(spinner, 5, _("5 AM"));
    elm_spinner_special_value_add(spinner, 6, _("6 AM"));
    elm_spinner_special_value_add(spinner, 7, _("7 AM"));
    elm_spinner_special_value_add(spinner, 8, _("8 AM"));
    elm_spinner_special_value_add(spinner, 9, _("9 AM"));
    elm_spinner_special_value_add(spinner, 10, _("10 AM"));
    elm_spinner_special_value_add(spinner, 11, _("11 AM"));
    elm_spinner_special_value_add(spinner, 12, _("12 Noon"));
    elm_spinner_special_value_add(spinner, 13, _("1 PM"));
    elm_spinner_special_value_add(spinner, 14, _("2 PM"));
    elm_spinner_special_value_add(spinner, 15, _("3 PM"));
    elm_spinner_special_value_add(spinner, 16, _("4 PM"));
    elm_spinner_special_value_add(spinner, 17, _("5 PM"));
    elm_spinner_special_value_add(spinner, 18, _("6 PM"));
    elm_spinner_special_value_add(spinner, 19, _("7 PM"));
    elm_spinner_special_value_add(spinner, 20, _("8 PM"));
    elm_spinner_special_value_add(spinner, 21, _("9 PM"));
    elm_spinner_special_value_add(spinner, 22, _("10 PM"));
    elm_spinner_special_value_add(spinner, 23, _("11 PM"));
    elm_spinner_special_value_add(spinner, 24, _("Every Hour"));
    evas_object_show(spinner);
	elm_object_content_set(frame, spinner);

	frame = elm_frame_add(win);
	elm_object_text_set(frame, _("Minute"));
	elm_grid_pack(grid, frame, 64, 68, 15, 12);
	evas_object_show(frame);

    spinner = elm_spinner_add(win);
   	evas_object_name_set(spinner, "minute spinner");
    elm_spinner_min_max_set(spinner, 0, 60);
    evas_object_show(spinner);
	elm_object_content_set(frame, spinner);

	for(x = 0;x != 60;x++)
	{   char *buf;
	    asprintf(&buf, "%d", x);
        elm_spinner_special_value_add(spinner, x, buf);
        FREE(buf);
	}
    elm_spinner_special_value_add(spinner, 60, _("Every Minute"));

   	hoversel = elm_hoversel_add(grid);
   	evas_object_name_set(hoversel, "type hoversel");
   	elm_object_text_set(hoversel, _("Action"));
	elm_grid_pack(grid, hoversel, 80, 70, 20, 8);
	for(x = 0;x < ACTION_TYPE_LAST;x++)
	{
		if(x == ACTION_TYPE_UNKNOWN) continue;
		if((x == ACTION_TYPE_DEBUG) && (!edams_settings_debug_get())) continue;

		Elm_Object_Item *it = elm_hoversel_item_add(hoversel, action_type_to_desc(x), ELM_ICON_NONE, ELM_ICON_NONE, NULL, (void*)(unsigned int)x);

        if(x == ACTION_TYPE_CMND)
            elm_hoversel_item_icon_set(it, "xpl-logo", NULL, ELM_ICON_STANDARD);
        else if(x == ACTION_TYPE_MAIL)
            elm_hoversel_item_icon_set(it, "mail-message-new", NULL, ELM_ICON_STANDARD);
        else if(x == ACTION_TYPE_EXEC)
    		elm_hoversel_item_icon_set(it, "system-run", NULL, ELM_ICON_STANDARD);
        else if(x == ACTION_TYPE_DEBUG)
    		elm_hoversel_item_icon_set(it, "debug-action", NULL, ELM_ICON_STANDARD);
        else if(x == ACTION_TYPE_OSD)
    		elm_hoversel_item_icon_set(it, "osd-action", NULL, ELM_ICON_STANDARD);
        else if(x == ACTION_TYPE_VOICE)
    		elm_hoversel_item_icon_set(it, "voice-action", NULL, ELM_ICON_STANDARD);
        else
    		elm_hoversel_item_icon_set(it, NULL, NULL, ELM_ICON_NONE);
	}
	evas_object_show(hoversel);
	evas_object_smart_callback_add(hoversel, "selected", _hoversel_selected_cb, NULL);

	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_grid_pack(grid, separator, 1, 89, 99, 1);
	evas_object_show(separator);

	bx = elm_box_add(win);
	elm_box_horizontal_set(bx, EINA_TRUE);
	elm_box_homogeneous_set(bx, EINA_TRUE);
	elm_grid_pack(grid, bx, 1, 90, 99, 10);
	evas_object_show(bx);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "list-add");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Add"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_add_clicked_cb, NULL);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "list-remove");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Remove"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", _button_remove_clicked_cb, NULL);

	button = elm_button_add(win);
	icon = elm_icon_add(win);
	elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(icon, "window-close");
	elm_object_part_content_set(button, "icon", icon);
	elm_object_text_set(button, _("Close"));
	elm_box_pack_end(bx, button);
	evas_object_show(button);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(button, "clicked", window_clicked_close_cb, win);

	/*
	separator = elm_separator_add(win);
	elm_separator_horizontal_set(separator, EINA_TRUE);
	elm_grid_pack(grid, bx, 1, 89, 99, 1);
	evas_object_show(separator);
*/

	evas_object_resize(win, 920, 500);
	evas_object_show(win);
}/*scheduler_editor_new*/
