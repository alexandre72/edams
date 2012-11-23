/*
 * modules.h
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



#ifndef __MODULE_H__
#define __MODULE_H__

#include <Eet.h>
#include <Evas.h>

#include "libedams.h"
#include "mem.h"
#include "path.h"
#include "rooms.h"
#include "settings.h"

//#include "edams.h"

//Define Module Infos.
typedef struct 
{
	Evas_Object *settings;
	char *id;
	char *name;
	char *description;
	char *icon;
} Module_Info;


//Modules data functions.
extern Module_Info *module_free(Module_Info *module);
extern Eina_List *modules_list_get(Evas_Object *obj);
extern Eina_List *modules_list_free(Eina_List *modules);

#endif /* __MODULE_H__ */
