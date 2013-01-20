/*
 * shutdown.c
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
#include <Ecore_Evas.h>
#include <Edje.h>
#include <Elementary.h>

#include "shutdown.h"
#include "settings.h"
#include "device.h"
#include "utils.h"

static int efl_shutdown(App_Info * app);
static int xpl_shutdown(App_Info * app);


static int xpl_shutdown(App_Info * app)
{
	debug(stdout, _("Shutdown xPL..."));
	xPL_setServiceEnabled(app->edamsService, EINA_FALSE);
	xPL_releaseService(app->edamsService);
	xPL_shutdown();

	return 0;
}


static int efl_shutdown(App_Info * app __UNUSED__)
{
	debug(stdout, _("Shutdown Enlightenment Foundation Libraries..."));
	eina_shutdown();
	ecore_evas_shutdown();
	ecore_shutdown();
	eet_shutdown();
	edje_shutdown();
	elm_shutdown();

	return 0;
}


int edams_shutdown(App_Info * app)
{
	xpl_shutdown(app);

	debug(stdout, _("Free allocated memory..."));
	app->locations = locations_list_free(app->locations);
	app->devices = devices_list_free(app->devices);
	locations_shutdown();
	devices_shutdown();
	edams_settings_shutdown();
	FREE(app);

	efl_shutdown(app);

	return EINA_TRUE;
}
