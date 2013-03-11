/*
 * settings.c
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

#include <Eina.h>
#include <Eet.h>
#include "settings.h"
#include "edams.h"
#include "path.h"
#include "utils.h"


typedef struct
{
	Eina_Stringshare *voicerss_apikey;			/*Voicerss API key account. Eg 'idde778445458778877989'*/
	Eina_Stringshare *cosm_apikey;			    /*Cosm API key account. Eg 'h0154864887erz8erz8erz7rez'*/
	Eina_Stringshare *gnuplot_path;			    /*Gnuplot path. Eg '/usr/bin/gnuplot'*/
	Eina_Stringshare *global_view_background;   /*Global view background image filename.*/
	Eina_Stringshare *mbox_path; 			    /*mbox path file. Eg '/home/jdoe/mbox'*/
	Eina_Stringshare *user_name;                /*User name. Eg 'John Doe'*/
	Eina_Stringshare *user_mail;                /*User mail. Eg 'john.doe@imail.net'*/
	Eina_Bool softemu;			                /*TODO:Sotfware emulation, mainly used to test EDAMS.*/
	Eina_Bool debug;			                /*Use printf to help to debug EDAMS.*/
} Settings;


static Settings *settings = NULL;
static Eet_File *ef = NULL;





#define EET_STRING_SETTINGS_READ(_field, _var) \
	char *ret; \
	int size; \
   	ret = eet_read(ef, _field, &size); \
   	if(ret) \
   	{ \
		_var = eina_stringshare_add(ret);	\
   		FREE(ret);	\
   	}	\

#define EET_BOOL_SETTINGS_READ(_field, _var) \
	char *ret; \
	int size; \
   	ret = eet_read(ef, _field, &size); \
   	if(ret) \
   	{ \
		_var = atoi(ret) ? EINA_TRUE : EINA_FALSE; \
   		FREE(ret);	\
   	} \

#define EET_BOOL_SETTINGS_WRITE(_field, _var) 			\
	if(_var == EINA_TRUE) 								\
	   	eet_write(ef, _field, "1", strlen("1")+1, 0);	\
	else 												\
	   	eet_write(ef, _field, "0", strlen("0")+1, 0); 	\


/*
 *
 */
Eina_Bool
edams_settings_debug_get()
{
	EET_BOOL_SETTINGS_READ("edams/debug", settings->debug);
	return settings->debug;
}/*edams_settings_debug_get*/


/*
 *
 */
void
edams_settings_debug_set(Eina_Bool isdebug)
{
	set_debug_mode(isdebug);
	EET_BOOL_SETTINGS_WRITE("edams/debug", isdebug);
	debug(stdout, _("Debug is %s"), isdebug?_("enabled"):_("disabled"));
}/*edams_settings_debug_set*/


/*
 *
 */
Eina_Bool
edams_settings_softemu_get()
{
	EET_BOOL_SETTINGS_READ("edams/softemu", settings->softemu);
	return settings->softemu;
}/*edams_settings_softemu_get*/


/*
 *
 */
void
edams_settings_softemu_set(Eina_Bool softemu)
{
	EET_BOOL_SETTINGS_WRITE("edams/softemu", softemu);
	debug(stdout, _("Software emulation is %s"), softemu?_("enabled"):_("disabled"));
}/*edams_settings_softemu_set*/


/*
 *
 */
const char*
edams_settings_global_view_background_get()
{
    settings->global_view_background = NULL;
	EET_STRING_SETTINGS_READ("global_view/background", settings->global_view_background);
	return settings->global_view_background;
}/*edams_settings_global_view_background_get*/



/*
 *
 */
void
edams_settings_global_view_background_set(const char *file)
{
    if((!file) || (strlen(file) == 0))
    {
        eet_delete(ef, "global_view/background");
        file = NULL;
    }
    else
    {
        eet_write(ef, "global_view/background", file, strlen(file)+1, 0);
    }

	debug(stdout, _("Global view background options is %s"), file ?_("enabled"):_("disabled"));
}/*edams_settings_global_view_background_set*/


/*
 *
 */
const char*
edams_settings_cosm_apikey_get()
{
    settings->cosm_apikey = NULL;
	EET_STRING_SETTINGS_READ("edams/cosm_apikey",settings->cosm_apikey);
	return settings->cosm_apikey;
}/*edams_settings_cosm_apikey_get*/


/*
 *
 */
void
edams_settings_cosm_apikey_set(const char *cosm_apikey)
{
    if((!cosm_apikey) || (strlen(cosm_apikey) == 0))
    {
        eet_delete(ef, "edams/cosm_apikey");
        cosm_apikey = NULL;
    }
    else
    {
        eet_write(ef, "edams/cosm_apikey", cosm_apikey, strlen(cosm_apikey)+1, 0);
    }
	debug(stdout, _("Cosm data handling options is %s"), cosm_apikey?_("enabled"):_("disabled"));
}/*edams_settings_cosm_apikey_set*/

/*
 *
 */
const char*
edams_settings_gnuplot_path_get()
{
    settings->gnuplot_path = NULL;
	EET_STRING_SETTINGS_READ("edams/gnuplot_path", settings->gnuplot_path);
	return settings->gnuplot_path;
}/*edams_settings_gnuplot_path_get*/

/*
 *
 */
void
edams_settings_gnuplot_path_set(const char *gnuplot_path)
{
    if((!gnuplot_path) || (strlen(gnuplot_path) == 0))
    {
        eet_delete(ef, "edams/gnuplot_path");
        gnuplot_path = NULL;
    }
    else
    {
        eet_write(ef, "edams/gnuplot_path", gnuplot_path, strlen(gnuplot_path)+1, 0);
    }
	debug(stdout, _("Gnuplot data handling options is %s"), gnuplot_path ? _("enabled"):_("disabled"));
}/*edams_settings_gnuplot_path_set*/


/*
 *
 */
const char*
edams_settings_mbox_path_get()
{
    settings->mbox_path = NULL;
	EET_STRING_SETTINGS_READ("edams/mbox_path", settings->mbox_path);
	return settings->mbox_path;
}/*edams_settings_smtp_server_get*/


/*
 *
 */
void
edams_settings_mbox_path_set(const char *mbox_path)
{
    if((!mbox_path) || (strlen(mbox_path) == 0))
    {
        eet_delete(ef, "edams/mbox_path");
        mbox_monitoring_shutdown();
        mbox_path = NULL;
    }
    else
    {
        eet_write(ef, "edams/mbox_path", mbox_path, strlen(mbox_path)+1, 0);
        mbox_monitoring_init();
    }

	debug(stdout, _("Mbox monitoring is %s"), mbox_path ? _("enabled"):_("disabled"));
}/*edams_settings_smtp_server_set*/


/*
 *
 */
const char*
edams_settings_user_mail_get()
{
    settings->user_mail = NULL;
    EET_STRING_SETTINGS_READ("edams/user_mail", settings->user_mail);
	return settings->user_mail;
}/*edams_settings_user_mail_get*/


/*
 *
 */
void
edams_settings_user_mail_set(const char *mail)
{
    if((!mail) || (strlen(mail) == 0))
    {
        eet_delete(ef, "edams/user_mail");
    }
    else
    {
        eet_write(ef, "edams/user_mail", mail, strlen(mail)+1, 0);;
    }

}/*edams_settings_user_mail_set*/


/*
 *
 */
const char*
edams_settings_user_name_get()
{
	EET_STRING_SETTINGS_READ("edams/user_name", settings->user_name);
	return settings->user_name;
}/*edams_settings_user_mail_get*/



/*
 *
 */
void
edams_settings_user_name_set(const char *user_name)
{
    eet_write(ef, "edams/user_name", user_name, strlen(user_name)+1, 0);;
}/*edams_settings_user_mail_set*/



/*
 *
 */
const char*
edams_settings_voicerss_apikey_get()
{
    settings->voicerss_apikey = NULL;
	EET_STRING_SETTINGS_READ("edams/voicerss_apikey",settings->voicerss_apikey);
	return settings->voicerss_apikey;
}/*edams_settings_voicerss_apikey_get*/


/*
 *
 */
void
edams_settings_voicerss_apikey_set(const char *voicerss_apikey)
{
    if((!voicerss_apikey) || (strlen(voicerss_apikey) == 0))
    {
        eet_delete(ef, "edams/voicerss_apikey");
        voicerss_apikey = NULL;
    }
    else
    {
        eet_write(ef, "edams/voicerss_apikey", voicerss_apikey, strlen(voicerss_apikey)+1, 0);
    }
	debug(stdout, _("Voicerss handling options is %s"), voicerss_apikey?_("enabled"):_("disabled"));
}/*edams_settings_voicerss_apikey_set*/



/*
 *
 */
void
edams_settings_init()
{
    settings = calloc(1, sizeof(Settings));

	if(!settings)
	{
		debug(stderr, _("Can't calloc Settings struct"));
		return;
	}

	ef = eet_open(edams_settings_file_get(), EET_FILE_MODE_READ_WRITE);

	settings->gnuplot_path = eina_stringshare_add("/usr/bin/gnuplot");
	settings->cosm_apikey = NULL;
	settings->voicerss_apikey = NULL;
	settings->global_view_background = NULL;
	settings->user_name = eina_stringshare_add(getlogin());
	char *s;
	asprintf(&s, "%s@localhost", getlogin());
	settings->user_mail = eina_stringshare_add(s);
    FREE(s);
	settings->softemu = EINA_FALSE;
    if(home_dir_get())
        asprintf(&s, "%s/mbox", home_dir_get());
	settings->mbox_path = eina_stringshare_add(s);
	FREE(s);
	settings->debug = edams_settings_debug_get();

	set_debug_mode(settings->debug);
}/*edams_settings_init*/


/*
 *
 */
void
edams_settings_shutdown()
{
	eet_close(ef);

	eina_stringshare_del(settings->gnuplot_path);
	eina_stringshare_del(settings->cosm_apikey);
	eina_stringshare_del(settings->voicerss_apikey);
	eina_stringshare_del(settings->global_view_background);
	eina_stringshare_del(settings->mbox_path);
	eina_stringshare_del(settings->user_name);
	eina_stringshare_del(settings->user_mail);
	FREE(settings);
}/*edams_settings_shutdown*/
