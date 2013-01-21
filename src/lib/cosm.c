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

#define _GNU_SOURCE
#include <stdio.h>

#include <Elementary.h>
#include <Ecore_Con.h>
#include "cJSON.h"
#include "cosm.h"
#include "edams.h"
#include "utils.h"

static Eina_Bool _url_feed_add_complete_cb(void *data, int type, void *event_info);
static Eina_Bool _url_feed_delete_complete_cb(void *data, int type, void *event_info);
static Eina_Bool _url_datastream_update_complete_cb(void *data, int type, void *event_info);


/*
 *Update datastream from cosm with device's data. Return EINA_TRUE if success.
 */
Eina_Bool
cosm_device_datastream_update(Location *location, Device *device)
{
	Ecore_Con_Url *cosm_url = NULL;
	char *s;

	//Don't add cosm device datastream if no feed!
	if(!location || (location_cosm_feedid_get(location) == 0) || !edams_settings_cosm_apikey_get())
		return EINA_FALSE;


	asprintf(&s, "http://api.cosm.com/v2/feeds/%d", location_cosm_feedid_get(location));
	cosm_url = ecore_con_url_custom_new(s, "PUT");
   	if (!cosm_url)
     {
	    debug(stderr, _("Can't create Ecore_Con_Url object"));
		return EINA_FALSE;
     }
	FREE(s);

	ecore_con_url_verbose_set(cosm_url, edams_settings_debug_get());
   	ecore_con_url_additional_header_add(cosm_url, "X-ApiKey", edams_settings_cosm_apikey_get());
   	ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_datastream_update_complete_cb, NULL);

	cJSON *root, *datastreams, *fmt,*unit;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"version", "1.0.0");

	cJSON_AddItemToObject(root, "datastreams", datastreams=cJSON_CreateArray());

	fmt=cJSON_CreateObject();
	cJSON_AddStringToObject(fmt,"current_value", device_data_get(device));
	cJSON_AddStringToObject(fmt,"id", device_name_get(device));

	if(strlen(device_unit_symbol_get(device)) > 0)
	{
		cJSON_AddItemToObject(fmt, "unit", unit=cJSON_CreateObject());
		cJSON_AddStringToObject(unit,"label", device_units_get(device));
		cJSON_AddStringToObject(unit,"symbol", device_unit_symbol_get(device));
	}

	cJSON_AddItemToArray(datastreams, fmt);

	s = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);

	if(!ecore_con_url_post(cosm_url, s, strlen(s), "text/json"))
	{
	   	debug(stderr, _("Can't realize url PUT request"));
		return EINA_FALSE;
	}
	FREE(s);
	return EINA_TRUE;
}/*cosm_device_datastream_update*/


/*
 *Add location feed to cosm. Return EINA_TRUE if success.
 */
Eina_Bool
cosm_location_feed_add(Location *location)
{
	Ecore_Con_Url *cosm_url = NULL;
	char *s;

	//Don't add cosm url feed if already there...
	if(!location || (location_cosm_feedid_get(location) != 0) || !edams_settings_cosm_apikey_get())
		return EINA_FALSE;

	debug(stderr, _("Creating cosm feed for '%s'..."), location_name_get(location));

   	cosm_url = ecore_con_url_custom_new("http://api.cosm.com/v2/feeds", "POST");

   	if (!cosm_url)
     {
		debug(stderr, _("Can't create Ecore_Con_Url object"));
		return EINA_FALSE;
     }
	ecore_con_url_verbose_set(cosm_url, edams_settings_debug_get());
   	ecore_con_url_additional_header_add(cosm_url, "X-ApiKey", edams_settings_cosm_apikey_get());;
   	ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_feed_add_complete_cb, (void*)location);

    char *locale = setlocale(LC_NUMERIC, NULL);
     setlocale(LC_NUMERIC, "POSIX");

	cJSON *root,*fmt;
	root=cJSON_CreateObject();
	asprintf(&s, "%s data", location_name_get(location));
	cJSON_AddItemToObject(root, "title", cJSON_CreateString(s));
	FREE(s);
	cJSON_AddItemToObject(root, "version", cJSON_CreateString("1.0.0"));
	cJSON_AddItemToObject(root, "location", fmt=cJSON_CreateObject());
	cJSON_AddStringToObject(fmt, "name", location_name_get(location));
	cJSON_AddStringToObject(fmt, "description", location_description_get(location));
	cJSON_AddStringToObject(fmt, "disposition", "fixed");
	cJSON_AddStringToObject(fmt, "exposure", "indoor");
	cJSON_AddStringToObject(fmt, "domain", "physical");

	if(location_latitude_get(location) != -1)
		cJSON_AddNumberToObject(fmt, "lat", location_latitude_get(location));

	if(location_longitude_get(location) != -1)
		cJSON_AddNumberToObject(fmt, "lon", location_longitude_get(location));

	s = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
    setlocale(LC_NUMERIC, locale);

	if(!ecore_con_url_post(cosm_url, s, strlen(s), NULL))
	{
		debug(stderr, _("Can't realize url PUT request"));
		return EINA_FALSE;
	}
	FREE(s);

	return EINA_TRUE;
}/*cosm_location_feed_add*/


/*
 *Delete location feed from cosm. Return EINA_TRUE if success.
 */
Eina_Bool
cosm_location_feed_delete(Location *location)
{
	Ecore_Con_Url *cosm_url = NULL;
	char *s;

	//Don't delete if no cosm feedid cosm or null location
	if(!location || (location_cosm_feedid_get(location) == 0) || !edams_settings_cosm_apikey_get())
		return EINA_FALSE;

	debug(stderr, _("Delete cosm feed for '%s'..."), location_name_get(location));

	asprintf(&s, "http://api.cosm.com/v2/feeds/%d", location_cosm_feedid_get(location));
	cosm_url = ecore_con_url_custom_new(s, "DELETE");
   	if (!cosm_url)
     {
	    debug(stderr, _("Can't create Ecore_Con_Url object"));
		return EINA_FALSE;
     }
	ecore_con_url_verbose_set(cosm_url, edams_settings_debug_get());
   	ecore_con_url_additional_header_add(cosm_url, "X-ApiKey", edams_settings_cosm_apikey_get());
   	ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_feed_delete_complete_cb, (void*)(int)location_cosm_feedid_get(location));

	if(!ecore_con_url_post(cosm_url, s, strlen(s), NULL))
	{
		debug(stderr, _("Can't realize url PUT request"));
		return EINA_FALSE;
	}
	FREE(s);

	return EINA_TRUE;
}/*cosm_location_feed_delete*/



/*
 *
 */
static Eina_Bool
_url_datastream_update_complete_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
	Ecore_Con_Event_Url_Complete *url_complete = event_info;

	Eina_List *headers;

	headers = ecore_con_url_response_headers_get(url_complete->url_con);

	return EINA_TRUE;
 }/*_url_datastream_update_complete_cb*/


/*
 *
 */
static Eina_Bool
_url_feed_add_complete_cb(void *data, int type __UNUSED__, void *event_info)
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
			return EINA_FALSE;
		}
		else if(strncmp(str, "Location:", 9) == 0)
		{
			unsigned int feedid;
			if (sscanf(str, "Location: http://api.cosm.com/v2/feeds/%d", &feedid)==1)
			{
				Location *location = data;
				location_cosm_feedid_set(location, feedid);
				location_save(location);
				char *s;
				asprintf(&s, _("Location has been added to cosm with feedid '%d'"), feedid);
				statusbar_text_set(s, "elm/icon/cosm/default");
				FREE(s);
				return EINA_TRUE;
			}
     	}
	}

	return EINA_TRUE;
}/*_url_feed_add_complete_cb*/

/*
 *
 */
static Eina_Bool
_url_feed_delete_complete_cb(void *data, int type __UNUSED__, void *event_info)
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
			return EINA_FALSE;
		}
   		else if(strncmp(str, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK")) == 0)
		{
				char *s;
				asprintf(&s, _("Location with feedid '%d' has been removed from cosm"), (int)data);
				statusbar_text_set(s, "elm/icon/cosm/default");
				FREE(s);
				return EINA_TRUE;
		}
	}

	return EINA_TRUE;
}/*_url_feed_delete_complete_cb*/
