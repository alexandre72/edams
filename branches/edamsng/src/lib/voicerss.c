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
#include <sys/stat.h>
#include <fcntl.h>

#include <ao/ao.h>

#include <Elementary.h>
#include <Ecore_Con.h>
#include "cJSON.h"
#include "cosm.h"
#include "edams.h"
#include "path.h"
#include "sound.h"
#include "utils.h"
#include "voice_editor.h"

static Eina_Bool _url_complete_cb(void *data, int type, void *event_info);
static Eina_Bool _url_progress_cb(void *data, int type, void *event_info);
const char *voicerss_server_error_to_str(int error);
static char to_hex(char code);
static char *url_encode(char *str);
const char *_voicerss_locale_get();


VoiceEditor *voiceeditor = NULL;

/*
 *Converts an integer value to its hex character
 */
static char
to_hex(char code)
 {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}/*to_hex*/


/*
 *Returns a url-encoded version of str
 */
static char *
url_encode(char *str)
{
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}/*url_encode*/

/*
 *
 */
const char *
_voicerss_locale_get()
{
    const char *l;

    l = setlocale(LC_CTYPE, NULL);

    if(strstr(l, "de"))
        return "de-de";
    else if(strstr(l, "du"))
        return "nl-nl";
    else if(strstr(l, "es"))
        return "es-es";
    else if(strstr(l, "fr"))
        return "fr-fr";
     else if(strstr(l, "it"))
        return "it-it";
    else if(strstr(l, "zh"))
        return "zh-cn";
    else
        return "en-us";
}/*_voicerss_locale_get*/


/*
 *Return EINA_TRUE if success.
 */
Eina_Bool
voicerss_play(const char *text, VoiceEditor *ve)
{
	Ecore_Con_Url *url = NULL;
    int fd;
    char *s;
    char *tmpfile;

    voiceeditor = ve;

    asprintf(&tmpfile, "/tmp/edamsXXXXXX.wav");

	if(!text || (edams_settings_voicerss_apikey_get() == 0))
        return EINA_FALSE;

    fd = mkostemps(tmpfile, 4, O_CREAT | O_WRONLY | O_TRUNC);
    if (fd == -1)
    {
        debug(MSG_ERROR, _("Can't create a valid temp name file"));
        return -1;
    }

    char *enc;
    enc = url_encode(text);
	asprintf(&s, "http://api.voicerss.org/?key=%s&src=%s&hl=%s&r=-3", edams_settings_voicerss_apikey_get(), enc, _voicerss_locale_get());
    url = ecore_con_url_new(s);
    FREE(enc);
	FREE(s);

    if (!url)
    {
	    debug(MSG_ERROR, _("Can't create Ecore_Con_Url object"));
		return EINA_FALSE;
    }

    ecore_con_url_data_set(url, eina_stringshare_add(tmpfile));
    FREE(tmpfile);
    ecore_con_url_fd_set(url, fd);

    ecore_event_handler_add(ECORE_CON_EVENT_URL_PROGRESS, _url_progress_cb, NULL);
    ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, NULL);

    if(!ecore_con_url_get(url))
    {

	    debug(MSG_ERROR, _("Can't realize Ecore_Con_Url GET request"));
        return EINA_FALSE;
    }
    return EINA_TRUE;
}/*voicerss_play*/


/*
 *
 */
static Eina_Bool
_url_complete_cb(void *data __UNUSED__, int type __UNUSED__, void *event_info)
{
    Ecore_Con_Event_Url_Complete *url_complete = event_info;

    Eina_Stringshare *file = ecore_con_url_data_get(url_complete->url_con);

    if((url_complete->status != 201) && (url_complete->status != 200))
    {
        debug(MSG_VOICERSS, _("Voicerss server returned code: '%d'"), url_complete->status);
        if(file)
        {
            ecore_file_remove(file);
            eina_stringshare_del(file);
        }
        return ECORE_CALLBACK_RENEW;
    }

    if(file)
    {
		debug(MSG_VOICERSS, _("Voice has been created from voicerss.org"), (int)data);
        sound_file_play(file);
        voiceeditor->sound_file = strdup(file);
        eina_stringshare_del(file);
        elm_progressbar_pulse(voiceeditor->progressbar, EINA_FALSE);
        elm_object_disabled_set(voiceeditor->ok_button, EINA_FALSE);
    }

    ecore_con_url_free(url_complete->url_con);

    return ECORE_CALLBACK_CANCEL;
}/*_url_complete_cb*/



/*
 *
 */
static Eina_Bool
_url_progress_cb(void *data, int type, void *event_info)
{
   Ecore_Con_Event_Url_Progress *url_progress = event_info;

   return EINA_TRUE;
}/*_url_progress_cb*/
