/*
 * utils.h
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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <Evas.h>

#include "gettext.h"

#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_S "/"

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#define __UNUSED__ __attribute__((unused))

#define FREE(ptr)  do { _free( #ptr, __FILE__, __LINE__, (ptr)); (ptr) = NULL; } while (0)

#ifndef RANDOM
#define RANDOM(num) (int)(rand()/(double)RAND_MAX * (num))
#endif

#ifndef RANDOMIZE
#define RANDOMIZE() srand((unsigned)time(NULL))
#endif

void _free(const char * var, const char * filename, unsigned long line, void *ptr);
void set_debug_mode(Eina_Bool debug);
void debug(FILE *stream, const char *theFormat, ...);
void evas_object_image_scale(Evas_Object *obj, int width, int height);
void window_clicked_close_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
void msgbox(const char *msg);
const char *filename_create(const char *filename);
size_t strdelstr(char *s, const char *search);
const char *home_dir_get(void);
const char *xdg_pictures_dir_get(void);
#endif /* __UTILS_H__ */
