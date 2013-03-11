/*
 * sound.c
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

#include "edams.h"
#include "sound.h"
#include "utils.h"

#include "config.h"

#ifdef HAVE_EMOTION

#include <Emotion.h>

static Evas        *evas       = NULL;
static Emotion_Vis  vis        = EMOTION_VIS_NONE;


/*
 *
 */
int
sound_init()
{
    debug(stdout, _("Sound support with Emotion has been enabled"));

    return 0;
}/*sound_init*/


/*
 *
 */
int
sound_shutdown()
{

	return 0;
}/*sound_shutdown*/


int
sound_file_play(const char *f)
{
    Evas_Object *em;

    App_Info *app = edams_app_info_get();

    evas = evas_object_evas_get(app->win);

    em = emotion_object_add(evas);

    if (!emotion_object_init(em, NULL))
    {
        debug(stdout, _("Can't play sound with Emotion gstreamer backend"));
         return -1;
    }
    emotion_object_file_set(em, f);

    evas_object_move(em, 0, 0);
    evas_object_resize(em, 10, 10);
    evas_object_show(em);

    emotion_object_play_set(em, EINA_TRUE);

    return 0;
}/*sound_file_play*/


#else
#include <Ecore.h>

int
sound_init()
{
	debug(stderr, _("No sound system for playing audio, using mplayer pipe"));
	return 0;
}/*sound_init*/


/*
 *
 */
int
sound_shutdown()
{
	return 0;
}/*sound_shutdown*/


int
sound_file_play(const char *f)
{
   	pid_t child_pid;
   	Ecore_Exe *child_handle;

    char *s;
    asprintf(&s, "mplayer %s", f);

	child_handle = ecore_exe_pipe_run(s,
                                    ECORE_EXE_PIPE_WRITE |
                                    ECORE_EXE_PIPE_READ_LINE_BUFFERED |
                                    ECORE_EXE_PIPE_READ, NULL);

   	if (child_handle == NULL)
	{
        debug(stderr, _("Can't create an Ecore_Exec_Pipe process"));
        FREE(s);
		return EINA_FALSE;
	}

    child_pid = ecore_exe_pid_get(child_handle);

	if (child_pid == -1)
   	{
        debug(stderr, _("Can't create get PID of Ecore_Exec_Pipe process"));
        FREE(s);
		return EINA_FALSE;
	}
	else
	{
		debug(stdout, _("Exec '%s' with PID '%d'"), s, child_pid);
	}

    ecore_exe_free(child_handle);
    FREE(s);
    return EINA_TRUE;
}/*sound_file_play*/
#endif
