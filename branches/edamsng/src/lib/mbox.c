/*
 * mbox.c
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

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include <stdio.h>

#include "settings.h"
#include "utils.h"
#include "global_view.h"



struct Mbox_Client
{
    Ecore_File_Monitor *monitor;
    const char *path;
    int num_new, num_total;
};

typedef struct Mbox_Client Mbox_Client;

static Mbox_Client *mbox = NULL;

static void _mbox_check_mail_monitor (void *data __UNUSED__, Ecore_File_Monitor * monitor __UNUSED__, Ecore_File_Event event, const char *path __UNUSED__);
static void _mbox_check_mail_parser();

/*
 *
 */
static void
_mbox_check_mail_monitor (void *data __UNUSED__, Ecore_File_Monitor * monitor __UNUSED__, Ecore_File_Event event __UNUSED__, const char *path __UNUSED__)
{

    if(event != ECORE_FILE_EVENT_MODIFIED) return;

    _mbox_check_mail_parser();

    global_view_new_mail_emit(mbox->num_new, mbox->num_total);
}/*_mbox_check_mail_monitor*/


/*
 *
 */
static void
_mbox_check_mail_parser()
{
  FILE *f;
  int header = 0;
  char buf[1024];

  if (!mbox->path) return;

  if (!(f = fopen (mbox->path, "r")))
    return;

  mbox->num_new = 0;
  mbox->num_total = 0;

    while (fgets (buf, sizeof (buf), f))
    {
        if (buf[0] == '\n')
            header = 0;
        else if (!strncmp (buf, "From ", 5))
	    {
	        header = 1;
	        mbox->num_total++;
	        mbox->num_new++;
        }
        else if (header)
	    {
	        if ((!strncmp (buf, "Status: ", 8)) && (strchr (buf, 'R')))
	            mbox->num_new--;

            /* Support for Mozilla/Thunderbird mbox format */
            else if (!strncmp (buf, "X-Mozilla-Status: ", 18))
            {
                if (!strstr (buf, "0000"))
		        {
                    mbox->num_new--;

                    if (strstr (buf, "0009"))
                        mbox->num_total--;
                }
	        }
    	}
    }

    fclose (f);
}/*_mbox_check_mail_parser*/


/*
 *
 */
void
mbox_monitoring_init()
{
    /*If mbox is already monitored don't monitor it again*/
    if(mbox) return;

    mbox = calloc(1, sizeof(Mbox_Client));

    mbox->num_new = 0;
    mbox->num_total = 0;
    mbox->path = eina_stringshare_add(edams_settings_mbox_path_get());
    mbox->monitor = ecore_file_monitor_add (mbox->path, _mbox_check_mail_monitor, NULL);
}/*mbox_monitoring_init*/


/*
 *
 */
void
mbox_monitoring_shutdown()
{
    if(!mbox) return;

    ecore_file_monitor_del(mbox->monitor);
    eina_stringshare_del(mbox->path);
    FREE(mbox);
}/*mbox_monitoring_shutdown*/
