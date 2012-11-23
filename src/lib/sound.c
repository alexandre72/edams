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




#include "sound.h"


#ifdef HAVE_EMOTION	
static Evas        *evas       = NULL;
static Emotion_Vis  vis        = EMOTION_VIS_NONE;



extern int
sound_file_play(const char *f);
{
   Evas_Object *o;

   DBG("Initialize EMOTION.");
 	
 	o = emotion_object_add(evas);

   if (!emotion_object_init(o, "gstreamer"))
     return -1;

   emotion_object_vis_set(o, vis);
   emotion_object_file_set(o, f);
   emotion_object_play_set(o, 1);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 320, 240);
   emotion_object_smooth_scale_set(o, 1);
   evas_object_show(o);

	
return 0;
}

#else
extern int
sound_file_play(const char *f);
{
	ERR("No sound system for playing audio.");
	return 0;
}
#endif

