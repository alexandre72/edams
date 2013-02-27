/*
 * init.c
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

#include <Elementary.h>

#include <Ecore_Con.h>
#include <Ecore_File.h>

#include "crontab.h"
#include "edams.h"
#include "path.h"
#include "settings.h"
#include "utils.h"


/*Others funcs*/
static int paths_init(App_Info * app);
static int efl_init(App_Info * app);

static int
efl_init(App_Info * app)
{
	debug(stdout, _("Initialize EFL..."));

	if (!eina_init())
	{
		debug(stderr, _("Can't init Eina"));
		return EXIT_FAILURE;
	}

	eet_init();
	ecore_init();

	if (!ecore_con_init() || !ecore_con_url_init())
	{
		debug(stderr, _("Can't init Ecore_Con or Ecore_Con_Url"));
		return EXIT_FAILURE;
	}

	if (!ecore_con_url_pipeline_get())
	{
		debug(stdout, _("Ecore_Con_Url pipeline has been enabled"));
		ecore_con_url_pipeline_set(EINA_TRUE);
	}

	ecore_evas_init();
	edje_init();

	if (!elm_init(app->argc, app->argv))
	{
		debug(stderr, _("Can't init Elementary"));
		return EXIT_FAILURE;
	}
	// Setting elementary options.
	elm_theme_extension_add(NULL, edams_edje_theme_file_get());
	elm_language_set("fr_FR.UTF-8");
#if ENABLE_NLS
	elm_language_set(setlocale(LC_ALL, NULL));
#endif

	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
	elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
	elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
	elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);

	return 0;
}


static int
paths_init(App_Info * app __UNUSED__)
{
	debug(stdout, _("Checking EDAMS useful paths..."));

	char s[PATH_MAX];

	// If no configurations path then create a new one in user home's.
	strcpy(s, edams_data_path_get());
	if (ecore_file_is_dir(s) == EINA_FALSE)
	{
		ecore_file_mkpath(edams_data_path_get());
	}

	// Create locations database directory.
	strcpy(s, edams_locations_data_path_get());
	if (ecore_file_is_dir(s) == EINA_FALSE)
		ecore_file_mkpath(s);

	// Create devices database directory.
	strcpy(s, edams_devices_data_path_get());
	if (ecore_file_is_dir(s) == EINA_FALSE)
		ecore_file_mkpath(s);

	return 0;
}


static int
 i18n_init(App_Info * app __UNUSED__)
{
#if ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE_NAME, edams_locale_path_get());
	bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
	textdomain(PACKAGE_NAME);
#endif

	return 0;
}



int
edams_init(App_Info * app)
{
	i18n_init(app);
	edams_settings_init();
	efl_init(app);
	paths_init(app);
	locations_init();
	xpl_init();
    crontab_init();

    if(edams_settings_mbox_path_get())
        mbox_monitoring_init();

	return 0;
}
