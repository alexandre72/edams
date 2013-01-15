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

#include <Elementary.h>
#include <Ecore_Con.h>
#include "cosm.h"
#include "edams.h"
#include "utils.h"

Evas_Object *waiting_win = NULL;
Ecore_Event_Handler *url_event;

static Eina_Bool _url_feed_add_complete_cb(void *data, int type, void *event_info);
static Eina_Bool _url_feed_delete_complete_cb(void *data, int type, void *event_info);
static Eina_Bool _url_datastream_update_complete_cb(void *data, int type, void *event_info);
static Eina_Bool cosm_apikey_set(Ecore_Con_Url *cosm_url, const char *key);
static void waiting_win_create();



//Add new device's datastream.
Eina_Bool
cosm_device_datastream_update(App_Info *app, Location *location, Device *device)
{
	Ecore_Con_Url *cosm_url = NULL;
	char s[512];
   	Eina_Bool r;

	//Don't add cosm device datastream if no feed!
	if(!location || (location_cosm_feedid_get(location) == 0) || !app->settings->cosm_apikey)
		return EINA_FALSE;

	snprintf(s, sizeof(s), "http://api.cosm.com/v2/feeds/%d", location_cosm_feedid_get(location));
	cosm_url = ecore_con_url_custom_new(s, "PUT");
   	if (!cosm_url)
     {
	    debug(stderr, _("Couldn't create Ecore_Con_Url object"));
		return EINA_FALSE;
     }
	ecore_con_url_verbose_set(cosm_url, app->settings->debug);
	cosm_apikey_set(cosm_url, app->settings->cosm_apikey);
   	url_event = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_datastream_update_complete_cb, location);

	if(strlen(device_unit_symbol_get(device)) > 0)
		snprintf(s, sizeof(s), ("{\"version\":\"1.0.0\", \"datastreams\":[{\"id\":\"%s\", \"current_value\":\"%s\", \"unit\":{\"label\":\"%s\", \"symbol\": \"%s\"}}]}"),
							device_name_get(device),
							device_data_get(device),
							device_units_get(device),
							device_unit_symbol_get(device));

	else
		snprintf(s, sizeof(s), ("{\"version\":\"1.0.0\", \"datastreams\":[{\"id\":\"%s\", \"current_value\":\"%s\"}]}"),
							device_name_get(device),
							device_data_get(device));

	r = ecore_con_url_post(cosm_url, s, strlen(s), "text/json");
	if (!r)
     {
	   	debug(stderr, _("Couldn't realize url PUT request"));
		return r;
     }
	return EINA_TRUE;
}



//Add new location feed.
Eina_Bool
cosm_location_feed_add(App_Info *app, Location *location)
{
	Ecore_Con_Url *cosm_url = NULL;
	char s[512];
   	Eina_Bool r;

	//Don't add cosm url feed if already there...
	if(!location || (location_cosm_feedid_get(location) != 0) || !app->settings->cosm_apikey)
		return EINA_FALSE;

	waiting_win_create();
	snprintf(s, sizeof(s), _("Creating cosm feed for '%s'..."), location_name_get(location));
	Evas_Object *pb = elm_object_name_find(waiting_win, "pb",-1);
   	elm_object_text_set(pb, s);
   	evas_object_show(waiting_win);

   	cosm_url = ecore_con_url_custom_new("http://api.cosm.com/v2/feeds", "POST");

   	if (!cosm_url)
     {
		debug(stderr, _("Couldn't create Ecore_Con_Url object"));
		return EINA_FALSE;
     }
	ecore_con_url_verbose_set(cosm_url, app->settings->debug);
	cosm_apikey_set(cosm_url, app->settings->cosm_apikey);
   	url_event = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_feed_add_complete_cb, location);

	char *locale = setlocale(LC_NUMERIC, NULL);
	setlocale(LC_NUMERIC, "POSIX");
	snprintf(s, sizeof(s), ("{\"title\":\"%s data\", \"version\":\"1.0.0\", \"location\": { \"disposition\":\"fixed\", \"name\":\"%s\", \"lat\":%'.2f, \"exposure\":\"indoor\", \"lon\":%'.2f, \"domain\":\"physical\"}}"),
					location_name_get(location),
					location_name_get(location),
					location_latitude_get(location),
					location_longitude_get(location));
	setlocale(LC_NUMERIC, locale);

  	r = ecore_con_url_post(cosm_url, s, strlen(s), NULL);
	if (!r)
     {
	    debug(stderr, _("Couldn't realize url PUT request"));
		return r;
     }

	return EINA_TRUE;
}



//Delete location feed.
Eina_Bool
cosm_location_feed_delete(App_Info *app, Location *location)
{
	Ecore_Con_Url *cosm_url = NULL;
	char s[512];
   	Eina_Bool r;

	//Don't delete if no cosm feedid cosm or null location
	if(!location || (location_cosm_feedid_get(location) == 0) || !app->settings->cosm_apikey)
		return EINA_FALSE;

	waiting_win_create();
	snprintf(s, sizeof(s), _("Delete cosm feed for '%s'..."), location_name_get(location));
	Evas_Object *pb = elm_object_name_find(waiting_win, "pb",-1);
   	elm_object_text_set(pb, s);
   	evas_object_show(waiting_win);

	snprintf(s, sizeof(s), "http://api.cosm.com/v2/feeds/%d", location_cosm_feedid_get(location));
	cosm_url = ecore_con_url_custom_new(s, "DELETE");
   	if (!cosm_url)
     {
	    debug(stderr, _("Couldn't create Ecore_Con_Url object"));
		return EINA_FALSE;
     }
	ecore_con_url_verbose_set(cosm_url, app->settings->debug);
	cosm_apikey_set(cosm_url, app->settings->cosm_apikey);
   	url_event = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_feed_delete_complete_cb, strdup(location_name_get(location)));

  	r = ecore_con_url_post(cosm_url, NULL, 0, NULL);
	if (!r)
     {
	    debug(stderr, _("Couldn't realize url DELETE request"));
		return r;
     }

	return EINA_TRUE;
}


static Eina_Bool
cosm_apikey_set(Ecore_Con_Url *cosm_url, const char *key)
{
   	ecore_con_url_additional_header_add(cosm_url, "X-ApiKey", key);

	return EINA_TRUE;
}


static Eina_Bool
_url_datastream_update_complete_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{

	Ecore_Con_Event_Url_Complete *url_complete = event_info;

	const Eina_List *headers;

	headers = ecore_con_url_response_headers_get(url_complete->url_con);

	//FIXME:Handle debug mode and check if valid response from cosm server!
	/*
	Eina_List *l;
	const char *str;
      EINA_LIST_FOREACH(headers, l, str)
   {
		fprintf(stdout, "%s", str);
	}
	*/
	return EINA_TRUE;
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
	   	if(strncmp(str, "HTTP/1.1 40", strlen("HTTP/1.1 40")) == 0)
		{
			statusbar_text_set(_("A location feed hasn't been created, maybe cosm server is down or it's an internet connection problem?"), "elm/icon/cosm/default");
			evas_object_del(waiting_win);
			return EINA_FALSE;
		}
		if(strncmp(str, "Location:", 9) == 0)
		{
			unsigned int feedid;
			if (sscanf(str, "Location: http://api.cosm.com/v2/feeds/%d", &feedid)==1)
			{
                Location *location = (Location *)data;
		     	location_cosm_feedid_set(location, feedid);
				location_save(location);
				char s[128];
				snprintf(s, sizeof(s), _("Location with cosm feedid '%d' for '%s' has been added with success"), location_cosm_feedid_get(location), location_name_get(location));
				statusbar_text_set(s, "elm/icon/cosm/default");
			}
     	}
	}

	evas_object_del(waiting_win);
	return EINA_TRUE;
}


static Eina_Bool
_url_feed_delete_complete_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
   Ecore_Con_Event_Url_Complete *url_complete = event_info;
   const Eina_List *headers, *l;
   char *str;

   headers = ecore_con_url_response_headers_get(url_complete->url_con);

   	EINA_LIST_FOREACH(headers, l, str)
   	{
	   	if(strncmp(str, "HTTP/1.1 40", strlen("HTTP/1.1 40")) == 0)
		{
			statusbar_text_set(_("A location feed hasn't been created, maybe cosm server is down or it's an internet connection problem?"), "elm/icon/cosm/default");
			evas_object_del(waiting_win);
			return EINA_FALSE;
		}
   		if(strncmp(str, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK")) == 0)
		{
                char *location = (char *)data;
				char s[128];
				snprintf(s, sizeof(s), _("Location '%s' has been deleted with success"), location);
				statusbar_text_set(s, "elm/icon/cosm/default");
				FREE(location);
		}
	}

	evas_object_del(waiting_win);
	return EINA_TRUE;
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
