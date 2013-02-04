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
	char *cosm_apikey;			/*Cosm API key account. Eg 'h0154864887erz8erz8erz7rez'*/
	char *gnuplot_path;			/*Gnuplot path. Eg '/usr/bin/gnuplot'*/
	char *map_background;		/*Background image filename used in global view.*/
	char *smtp_server; 			/*SMTP server. Eg 'smtp://smtp.gmail.com:587'*/
	char *smtp_username;		/*SMTP user password. Eg 'myemailaddress@gmail.com'*/
	char *smtp_userpwd;			/*SMTP user password. Eg 'PASSWORD123'*/
	char *user_name;            /*User name. Eg 'John Doe'*/
	char *user_mail;            /*User mail. Eg 'john.doe@imail.net'*/
	Eina_Bool softemu;			/*TODO:Sotfware emulation, mainly used to test EDAMS.*/
	Eina_Bool debug;			/*Use printf to help to debug EDAMS.*/
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
edams_settings_map_background_get()
{
	EET_STRING_SETTINGS_READ("map/map_background", settings->map_background);
	return settings->map_background;
}/*edams_settings_map_background_get*/



/*
 *
 */
void
edams_settings_map_background_set(const char *map_background)
{
    eet_write(ef, "map/map_background", map_background, strlen(map_background)+1, 0);
    if(map_background && (strlen(map_background) > 0))
    	debug(stdout, _("Map background file set to '%s'"), map_background);
}/*edams_settings_map_background_set*/


/*
 *
 */
const char*
edams_settings_cosm_apikey_get()
{
	EET_STRING_SETTINGS_READ("edams/cosm_apikey",settings->cosm_apikey);
	return settings->cosm_apikey;
}/*edams_settings_cosm_apikey_get*/


/*
 *
 */
void
edams_settings_cosm_apikey_set(const char *cosm_apikey)
{
    eet_write(ef, "edams/cosm_apikey", cosm_apikey, strlen(cosm_apikey)+1, 0);;
	debug(stdout, _("Cosm data handling options is %s"), cosm_apikey?_("enabled"):_("disabled"));
}/*edams_settings_cosm_apikey_set*/

/*
 *
 */
const char*
edams_settings_gnuplot_path_get()
{
	EET_STRING_SETTINGS_READ("edams/gnuplot_path", settings->gnuplot_path);
	return settings->gnuplot_path;
}/*edams_settings_gnuplot_path_get*/

/*
 *
 */
void
edams_settings_gnuplot_path_set(const char *gnuplot_path)
{
    eet_write(ef, "edams/gnuplot_path", gnuplot_path, strlen(gnuplot_path)+1, 0);;
    if((gnuplot_path) && (strlen(gnuplot_path) > 0))
    	debug(stdout, _("Gnuplot path is '%s'"), gnuplot_path);
}/*edams_settings_gnuplot_path_set*/


/*
 *
 */
const char*
edams_settings_smtp_server_get()
{
	EET_STRING_SETTINGS_READ("edams/smtp_server", settings->smtp_server);
	return settings->smtp_server;
}/*edams_settings_smtp_server_get*/


/*
 *
 */
void
edams_settings_smtp_server_set(const char *smtp_server)
{
    eet_write(ef, "edams/smtp_server", smtp_server, strlen(smtp_server)+1, 0);;
    if(smtp_server && (strlen(smtp_server) > 0))
	    debug(stdout, _("Smtp server set to '%s'"), smtp_server);
}/*edams_settings_smtp_server_set*/


/*
 *
 */
const char*
edams_settings_smtp_username_get()
{
	EET_STRING_SETTINGS_READ("edams/smtp_username", settings->smtp_username);
	return settings->smtp_username;
}/*edams_settings_smtp_username_get*/


/*
 *
 */
void
edams_settings_smtp_username_set(const char *smtp_username)
{
    eet_write(ef, "edams/smtp_username", smtp_username, strlen(smtp_username)+1, 0);;
    if(smtp_username && (strlen(smtp_username) > 0))
	    debug(stdout, _("Smtp server username set to '%s'"), smtp_username);

}/*edams_settings_smtp_username_set*/


/*
 *
 */
const char*
edams_settings_smtp_userpwd_get()
{
	EET_STRING_SETTINGS_READ("edams/smtp_userpwd", settings->smtp_userpwd);
	return settings->smtp_userpwd;
}/*edams_settings_smtp_userpwd_get*/



/*
 *
 */
void
edams_settings_smtp_userpwd_set(const char *smtp_userpwd)
{
    eet_write(ef, "edams/smtp_userpwd", smtp_userpwd, strlen(smtp_userpwd)+1, 0);;
}/*edams_settings_smtp_username_set*/



/*
 *
 */
const char*
edams_settings_user_mail_get()
{
	EET_STRING_SETTINGS_READ("edams/user_mail", settings->user_mail);
	return settings->user_mail;
}/*edams_settings_user_mail_get*/



/*
 *
 */
void
edams_settings_user_mail_set(const char *mail)
{
    eet_write(ef, "edams/user_mail", mail, strlen(mail)+1, 0);;
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

	settings->gnuplot_path = NULL;
	settings->cosm_apikey = NULL;
	settings->map_background = NULL;
	settings->smtp_server = NULL;
	settings->smtp_username = NULL;
	settings->smtp_userpwd = NULL;
	settings->user_name = NULL;
	settings->user_mail = NULL;
	settings->softemu = EINA_FALSE;
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
	eina_stringshare_del(settings->map_background);
	eina_stringshare_del(settings->smtp_server);
	eina_stringshare_del(settings->smtp_username);
	eina_stringshare_del(settings->smtp_userpwd);
	eina_stringshare_del(settings->user_name);
	eina_stringshare_del(settings->user_mail);
	FREE(settings);
}/*edams_settings_shutdown*/
