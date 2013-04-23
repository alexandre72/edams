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

#include "crontab.h"
#include "settings.h"
#include "shutdown.h"
#include "sound.h"
#include "utils.h"


static int efl_shutdown(App_Info * app);


/*
 *
 */
static int
efl_shutdown(App_Info * app __UNUSED__)
{
	debug(MSG_INFO, _("Shutdown Enlightenment Foundation Libraries..."));
	eina_shutdown();
	ecore_evas_shutdown();
	ecore_shutdown();
	eet_shutdown();
	edje_shutdown();
	elm_shutdown();

	return 0;
}/*efl_shutdown*/


/*
 *
 */
int
edams_shutdown(App_Info * app)
{
	devices_shutdown();
    sound_shutdown();

	debug(MSG_INFO, _("Free allocated memory..."));
	app->locations = locations_list_free(app->locations);
	locations_shutdown();
	edams_settings_shutdown();
	FREE(app);

    mbox_monitoring_shutdown();

    crontab_shutdown();
	efl_shutdown(app);

	return EINA_TRUE;
}/*edams_shutdown*/
