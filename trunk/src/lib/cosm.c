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

static Eina_Bool _url_feed_add_complete_cb(void *data, int type, Ecore_Con_Event_Url_Complete *event);
static Eina_Bool _url_feed_delete_complete_cb(void *data, int type,  Ecore_Con_Event_Url_Complete *event);
static Eina_Bool _url_datastream_update_complete_cb(void *data, int type,  Ecore_Con_Event_Url_Complete *event);


/*
 *
 */
const char *
cosm_server_error_to_str(int error)
{
    switch(error)
    {
        case 401: return _("401 Not Authorized");
        case 403: return _("403 Forbidden");
        case 404: return _("404 Not Found");
        case 422: return _("422 Unprocessable Entity");
        case 500: return _("500 Internal Server Error");
        case 503: return _("503 No server error");
    }

    return NULL;
}/*cosm_server_error_to_str*/



/*
 *Update datastream from cosm with device's data. Return EINA_TRUE if success.
 */
Eina_Bool
cosm_device_datastream_update(Location *location, Widget *widget)
{
	Ecore_Con_Url *cosm_url = NULL;
	char *s;

	/*Don't add cosm device datastream if no feed available*/
	if(!location || (location_cosm_feedid_get(location) == 0) || !edams_settings_cosm_apikey_get())
		return EINA_FALSE;

   	ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_datastream_update_complete_cb, NULL);
	asprintf(&s, "http://api.cosm.com/v2/feeds/%d", location_cosm_feedid_get(location));
	cosm_url = ecore_con_url_custom_new(s, "PUT");
   	if (!cosm_url)
     {
	    debug(MSG_COSM, _("Can't create Ecore_Con_Url object"));
		return EINA_FALSE;
     }
	FREE(s);

	//ecore_con_url_verbose_set(cosm_url, edams_settings_debug_get());
   	ecore_con_url_additional_header_add(cosm_url, "X-ApiKey", edams_settings_cosm_apikey_get());

	cJSON *root, *datastreams, *fmt,*unit;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"version", "1.0.0");

	cJSON_AddItemToObject(root, "datastreams", datastreams=cJSON_CreateArray());

	fmt=cJSON_CreateObject();
	cJSON_AddStringToObject(fmt,"current_value", widget_xpl_current_get(widget));
	cJSON_AddStringToObject(fmt,"id", widget_xpl_device_get(widget));

	if(xpl_type_to_unit_symbol(widget_xpl_type_get(widget)))
	{
		cJSON_AddItemToObject(fmt, "unit", unit=cJSON_CreateObject());
		cJSON_AddStringToObject(unit,"label", xpl_type_to_units(widget_xpl_type_get(widget)));
		cJSON_AddStringToObject(unit,"symbol", xpl_type_to_unit_symbol(widget_xpl_type_get(widget)));
	}

	cJSON_AddItemToArray(datastreams, fmt);

	s = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);

	if(!ecore_con_url_post(cosm_url, (void*)s, strlen(s), "text/json"))
	{
	   	debug(MSG_COSM, _("Can't realize url PUT request"));
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

	debug(MSG_COSM, _("Creating cosm feed '%s'..."), location_name_get(location));

   	ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, (Ecore_Event_Handler_Cb)_url_feed_add_complete_cb,NULL);
   	cosm_url = ecore_con_url_custom_new("http://api.cosm.com/v2/feeds", "POST");
   	if (!cosm_url)
     {
		debug(MSG_COSM, _("Can't create Ecore_Con_Url object"));
		return EINA_FALSE;
     }
	//ecore_con_url_verbose_set(cosm_url, edams_settings_debug_get());
   	ecore_con_url_additional_header_add(cosm_url, "X-ApiKey", edams_settings_cosm_apikey_get());;
    ecore_con_url_data_set(cosm_url, (void*)location);

    char *locale = setlocale(LC_NUMERIC, NULL);
     setlocale(LC_NUMERIC, "POSIX");

	cJSON *root,*fmt;
	root=cJSON_CreateObject();
	asprintf(&s, "%s sensor.basic", location_name_get(location));
	cJSON_AddItemToObject(root, "title", cJSON_CreateString(s));
	FREE(s);
	cJSON_AddItemToObject(root, "version", cJSON_CreateString("1.0.0"));
	cJSON_AddItemToObject(root, "location", fmt=cJSON_CreateObject());
	cJSON_AddStringToObject(fmt, "name", location_name_get(location));
	//cJSON_AddStringToObject(fmt, "description", location_description_get(location));
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

	if(!ecore_con_url_post(cosm_url, (void*)s, strlen(s), NULL))
	{
		debug(MSG_COSM, _("Can't realize url PUT request"));
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

	int feedid = location_cosm_feedid_get(location);

	debug(MSG_COSM, _("Deleting Cosm feed '%s'..."), location_name_get(location));

   	ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_feed_delete_complete_cb, (void*)feedid);
	asprintf(&s, "http://api.cosm.com/v2/feeds/%d", location_cosm_feedid_get(location));
	cosm_url = ecore_con_url_custom_new(s, "DELETE");
   	if (!cosm_url)
     {
	    debug(MSG_ERROR, _("Can't create Ecore_Con_Url object"));
		return EINA_FALSE;
     }
	//ecore_con_url_verbose_set(cosm_url, edams_settings_debug_get());
   	ecore_con_url_additional_header_add(cosm_url, "X-ApiKey", edams_settings_cosm_apikey_get());


	if(!ecore_con_url_post(cosm_url, (void*)s, strlen(s), NULL))
	{
		debug(MSG_ERROR, _("Can't realize url PUT request"));
		return EINA_FALSE;
	}
	FREE(s);

	return EINA_TRUE;
}/*cosm_location_feed_delete*/



/*
 *
 */
static Eina_Bool
_url_datastream_update_complete_cb(void *data __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Url_Complete *event_info)
{
    const Eina_List *headers = NULL;
    //const char *header = NULL;

    if((event_info->status != 201) && (event_info->status != 200))
    {
        char *s;
        asprintf(&s, _("A datastream feed hasn't been updated, Cosm server returned error '%s'"), cosm_server_error_to_str(event_info->status));
        debug(MSG_COSM, s);
        FREE(s);
        return ECORE_CALLBACK_RENEW;
    }
    else
    {
    	headers = ecore_con_url_response_headers_get(event_info->url_con);

    }

    ecore_con_url_free(event_info->url_con);
    return ECORE_CALLBACK_CANCEL;
 }/*_url_datastream_update_complete_cb*/


/*
 *
 */
static Eina_Bool
_url_feed_add_complete_cb(void *data __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Url_Complete *event_info)
{
    const Eina_List *headers = NULL, *it;
    const char *header = NULL;


    if((event_info->status != 201) && (event_info->status != 200))
    {
        debug(MSG_COSM, _("Cosm server returned code: '%d'"), event_info->status);
        char *s;
        asprintf(&s, _("A location feed hasn't been added, Cosm server returned error '%s'"), cosm_server_error_to_str(event_info->status));
       debug(MSG_COSM, s);
        FREE(s);
        return ECORE_CALLBACK_RENEW;
    }
    else
    {
        headers = ecore_con_url_response_headers_get(event_info->url_con);

         EINA_LIST_FOREACH(headers, it, header)
        {
		    if(strncmp(header, "Location:", 9) == 0)
		    {
			    unsigned int feedid;
			    if (sscanf(header, "Location: http://api.cosm.com/v2/feeds/%d", &feedid)==1)
			    {
				    Location *location = ecore_con_url_data_get(event_info->url_con);;
				    location_cosm_feedid_set(location, feedid);
				    location_save(location);
				    char *s;
				    asprintf(&s, _("Location has been added to Cosm with feedid '%d'"), feedid);
                    debug(MSG_COSM, s);
				    FREE(s);
				    update_naviframe_content(location);
                    ecore_con_url_free(event_info->url_con);
				    break;
			    }
     	    }
	    }
    }

    return ECORE_CALLBACK_CANCEL;
}/*_url_feed_add_complete_cb*/

/*
 *
 */
static Eina_Bool
_url_feed_delete_complete_cb(void *data, int type __UNUSED__, Ecore_Con_Event_Url_Complete *event_info)
{
    const Eina_List *headers = NULL, *it;
    const char *header = NULL;

    if((event_info->status != 201) && (event_info->status != 200))
    {
        debug(MSG_COSM, _("Cosm server returned code: '%d'"), event_info->status);
        char *s;
        asprintf(&s, _("A location feed hasn't been deleted, Cosm server returned error '%s'"), cosm_server_error_to_str(event_info->status));
        debug(MSG_COSM, s);
        FREE(s);
        return ECORE_CALLBACK_RENEW;
    }
    else
    {
        headers = ecore_con_url_response_headers_get(event_info->url_con);

   	    EINA_LIST_FOREACH(headers, it, header)
   	    {
   	    	if(strncmp(header, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK")) == 0)
	    	{
	    		char *s;
				asprintf(&s, _("Location with feedid '%d' has been removed from Cosm"), (int)data);
                debug(MSG_COSM, s);
				FREE(s);
                ecore_con_url_free(event_info->url_con);
				break;
		    }
	    }
    }

    return ECORE_CALLBACK_CANCEL;
}/*_url_feed_delete_complete_cb*/
