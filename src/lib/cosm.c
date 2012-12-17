/*
 * cosm.c
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

#include <Ecore_Con.h>
#include "cosm.h"
#include "edams.h"

Ecore_Con_Url *cosm_url = NULL;
Evas_Object *waiting_win = NULL;
Ecore_Event_Handler *url_event;

static Eina_Bool _url_feed_add_complete_cb(void *data, int type, void *event_info);
static Eina_Bool _url_datastream_update_complete_cb(void *data, int type, void *event_info);

static void waiting_win_create();


//Add new device's datastream.
Eina_Bool
cosm_device_datastream_update(App_Info *app, Location *location, Device *device)
{
	char s[512];
   	Eina_Bool r;
   	Settings *settings;

	//Don't add cosm device datastream if no feed!
	if(location_cosm_feedid_get(location) == 0)
		return EINA_FALSE;

	snprintf(s, sizeof(s), "http://api.cosm.com/v2/feeds/%d", location_cosm_feedid_get(location));
	cosm_url = ecore_con_url_custom_new(s, "PUT");
   	if (!cosm_url)
     {
     	if(app->settings->debugprintf)
	        fprintf(stdout, _("ERROR: Couldn't create Ecore_Con_Url object!\n"));
		return EINA_FALSE;
     }

	//if(app->settings->debugprintf)
		ecore_con_url_verbose_set(cosm_url, EINA_TRUE);

	cosm_apikey_set(app->settings->cosm_apikey);
   	url_event = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_datastream_update_complete_cb, NULL);

	snprintf(s, sizeof(s), ("{\"version\":\"1.0.0\", \"datastreams\":[{\"id\":\"%d-%s\", \"current_value\":\"%s\"}]"),
							device_id_get(device),
							device_name_get(device),
							device_data_get(device));

	r = ecore_con_url_post(cosm_url, s, strlen(s), "text/json");
	if (!r)
     {
        if(app->settings->debugprintf)
	    	fprintf(stdout, _("ERROR: Couldn't realize url PUT request!\n"));
		return r;
     }
	return EINA_TRUE;
}



//Add new location feed.
Eina_Bool
cosm_location_feed_add(App_Info *app, Location *location)
{
	char s[256];
   	Eina_Bool r;
   	Settings *settings;

	//Don't add cosm url feed if already there...
	if(location_cosm_feedid_get(location) != 0)
		return EINA_FALSE;

	waiting_win_create();
	snprintf(s, sizeof(s), _("Creating cosm feed for '%s'..."), location_name_get(location));
	Evas_Object *pb = elm_object_name_find(waiting_win, "pb",-1);
   	elm_object_text_set(pb, s);
   	evas_object_show(waiting_win);

   	cosm_url = ecore_con_url_custom_new("http://api.cosm.com/v2/feeds", "POST");

	if(app->settings->debugprintf)
		ecore_con_url_verbose_set(cosm_url, EINA_TRUE);

   	if (!cosm_url)
     {
     	if(app->settings->debugprintf)
	        fprintf(stdout, _("ERROR: Couldn't create Ecore_Con_Url object!\n"));
		return EINA_FALSE;
     }
	ecore_con_url_data_set(cosm_url, location);
   	url_event = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_feed_add_complete_cb, NULL);

	cosm_apikey_set(app->settings->cosm_apikey);

	snprintf(s, sizeof(s), "{\"title\":\"%s data\", \"version\":\"1.0.0\"}", location_name_get(location));
  	r = ecore_con_url_post(cosm_url, s, strlen(s), NULL);
	if (!r)
     {
        if(app->settings->debugprintf)
	    	fprintf(stdout, _("ERROR: Couldn't realize url PUT request!\n"));
		return r;
     }

	return EINA_TRUE;
}


Eina_Bool
cosm_apikey_set(const char *key)
{
   	ecore_con_url_additional_header_add(cosm_url, "X-ApiKey", key);

	return EINA_TRUE;
}

static Eina_Bool
_url_datastream_update_complete_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{

  Ecore_Con_Event_Url_Complete *url_complete = event_info;
   const Eina_List *headers, *l;
   char *str;

   headers = ecore_con_url_response_headers_get(url_complete->url_con);

   EINA_LIST_FOREACH(headers, l, str)
   {
		printf("%s", str);
	}

	ecore_event_handler_del(url_event);
	return EINA_FALSE;
 }


static Eina_Bool
_url_feed_add_complete_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Con_Event_Url_Complete *url_complete = event_info;
   const Eina_List *headers, *l;
   char *str;

   headers = ecore_con_url_response_headers_get(url_complete->url_con);

   EINA_LIST_FOREACH(headers, l, str)
   {
		if(strncmp(str, "Location:", 9) == 0)
		{
			unsigned int feedid;
			if (sscanf(str, "Location: http://api.cosm.com/v2/feeds/%d", &feedid)==1)
			{
                Location *location = ecore_con_url_data_get(cosm_url);
		     	location_cosm_feedid_set(location, feedid);
				location_save(location);
			}
     	}
	}

	evas_object_del(waiting_win);
	ecore_con_url_free(cosm_url);
	ecore_event_handler_del(url_event);
	return EINA_FALSE;
}



static void
waiting_win_create()
{
	Evas_Object *bx, *pb, *bg;
	waiting_win = elm_win_add(NULL, "waiting_win", ELM_WIN_SPLASH);
	elm_win_title_set(waiting_win, _("Generating feeds, wait please..."));
	elm_win_center(waiting_win, EINA_TRUE,  EINA_TRUE);

	bg = elm_bg_add(waiting_win);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(waiting_win, bg );
	evas_object_show(bg );

	bx = elm_box_add(waiting_win);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(waiting_win, bx);
	evas_object_show(bx);

	pb = elm_progressbar_add(waiting_win);
	evas_object_name_set(pb, "pb");
	evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, 0.5);
   	evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   	elm_progressbar_pulse_set(pb, EINA_TRUE);
   	elm_box_pack_end(bx, pb);
	elm_progressbar_pulse(pb, EINA_TRUE);
   	evas_object_show(pb);
}
