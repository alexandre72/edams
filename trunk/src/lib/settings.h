/*
 * settings.h
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



#ifndef __SETTINGS_H__
#define __SETTINGS_H__


void edams_settings_init();
void edams_settings_shutdown();

Eina_Bool edams_settings_debug_get();
Eina_Bool edams_settings_softemu_get();
const char *edams_settings_global_view_background_get();
const char *edams_settings_cosm_apikey_get();
const char *edams_settings_voicerss_apikey_get();
const char *edams_settings_mbox_path_get();
const char *edams_settings_user_mail_get();
const char *edams_settings_user_name_get();

void edams_settings_debug_set(Eina_Bool debug);
void edams_settings_softemu_set(Eina_Bool softemu);
void edams_settings_global_view_background_set(const char *file);
void edams_settings_cosm_apikey_set(const char *cosm_apikey);
void edams_settings_voicerss_apikey_set(const char *apikey);
void edams_settings_mbox_path_set(const char *smtp_server);
void edams_settings_user_mail_set(const char *mail);
void edams_settings_user_name_set(const char *name);
#endif /* __SETTINGS_H__ */
