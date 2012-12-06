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




#include "shutdown.h"


int
edams_shutdown(App_Info *app)
{
	INF(_("Shutdown Edams..."));
	app->rooms = rooms_list_free(app->rooms);
	app->sensors = sensors_list_free(app->sensors);
	void *data;
	EINA_LIST_FREE(app->meters, data)
		eina_stringshare_del(data);
	rooms_shutdown();
	elm_prefs_data_unref(app->prefs_data);
	FREE(app);

	return EINA_TRUE;
}
