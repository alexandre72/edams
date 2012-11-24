/*
 * myfileselector.h
 * This file is part of Elm Extension Pack
 *
 * Copyright (C) 2012 - Alexandre Dussart
 *
 * Elm Extension Pack is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Elm Extension Pack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Elm Extension Pack. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __MYFILESELECTOR_H
#define __MYFILESELECTOR_H

#include "libedams.h"
#include "mem.h"
#include "path.h"
#include "utils.h"


typedef struct
{
	Evas_Object *win;
	Evas_Object *fs;
    Evas_Object *preview_img;
    Evas_Object *preview_frame;    
	Evas_Object *filename_entry;
	Evas_Object *permissions_entry;	
	Evas_Object *owner_entry;	
	Evas_Object *size_entry;	
	Evas_Object *mimetype_entry;	
	Evas_Object *cancel_bt;
	Evas_Object *action_bt;
} MyFileSelector;


MyFileSelector *myfileselector_add();
void myfileselector_set_title(MyFileSelector *myfs, const char *title);
void myfileselector_close(MyFileSelector *myfs);
#endif
