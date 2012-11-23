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

void evas_object_image_scale(Evas_Object *obj, int width, int height);
void window_clicked_close_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
const char *user_home_get(void);
char *trimwhitespace(char *str);
void msgbox(const char *msg);
#endif /* __UTILS_H__ */
