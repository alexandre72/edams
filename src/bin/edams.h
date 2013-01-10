/*
 * edams.h
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

#ifndef __EDAMS_H__
#define __EDAMS_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Evas.h>
#include <Eina.h>

#include "utils.h"
#include "location.h"
#include "settings.h"
#include "xPL.h"

#define XPL_VERSION "20091005"

//
//Define App UI struct.
//
typedef struct
{
	int argc;
	char **argv;

	Evas_Object *win;
	Evas_Object *waiting_win;
	Evas_Object *toolbar;
	Eina_List *locations;
	Eina_List *devices;
	Location *location;
	Settings *settings;

	xPL_ServicePtr 	edamsService ;
	xPL_MessagePtr 	edamsMessageStat ;
	xPL_MessagePtr 	edamsMessageTrig ;
} App_Info;


Evas_Object*_location_naviframe_content(Location *location);

#endif /* __EDAMS_H__ */
