/*
 * gnuplot.c
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

#include "gnuplot.h"
#include "path.h"
#include "utils.h"

static const char *gnuplot_device_data_file_get(Device *device);


/*
 *Return gnuplot device data file path.
 */
static const char *
gnuplot_device_data_file_get(Device *device)
{
	if(!device) return NULL;

	char s[PATH_MAX];
	snprintf(s, sizeof(s), "%s/%s.dat", edams_devices_data_path_get(), device_name_get(device));
	return strdup(s);
}


/*
 *Write png data graph from device data and return png data path.
 */
const char *
gnuplot_device_png_write(App_Info *app, Device *device)
{
	//TODO:should handle xrange to let user select data period(monthly, years, daily, hour).
	//fprintf(gnuplotPipe, "set xrange [\"01-01-2013-17:36:00\":\"31-01-2013-17:37:00\"]\n");
	//TODO:should handle options like grid, title, ylabel, xlabel, resolution format, histograms/plots...

	if(!device) return NULL;

	char s[PATH_MAX];

	if(!app->settings->gnuplot_path)
	{
		debug(stderr, _("Gnuplot binary path isn't set!"));
		return NULL;
	}

	FILE *gnuplot_pipe;
	snprintf(s, sizeof(s), "%s -persist", app->settings->gnuplot_path);
	gnuplot_pipe = popen(s, "w");
	if (gnuplot_pipe)
	{
		snprintf(s, sizeof(s), "%s/%s.png", edams_devices_data_path_get(), device_name_get(device));
		fprintf(gnuplot_pipe, "set term png size 600,300\n");
		fprintf(gnuplot_pipe, "set output \"%s\"\n", s);
		fprintf(gnuplot_pipe, "set xdata time\n");
 		fprintf(gnuplot_pipe, "set format x \"%%d/%%m\"\n");
  		fprintf(gnuplot_pipe, "set ylabel \"%s(%s)\"\n", device_units_get(device), device_unit_symbol_get(device));
  		fprintf(gnuplot_pipe, "set timefmt \"%%d-%%m-%%Y-%%H:%%M:%%S\"\n");
  		fprintf(gnuplot_pipe, "set grid\n");
		fprintf(gnuplot_pipe, "plot '%s' using 1:2 with lines title '%s'\n", gnuplot_device_data_file_get(device), device_name_get(device));
		fprintf(gnuplot_pipe,"exit\n");
		pclose(gnuplot_pipe);
	}
	else
	{
		debug(stderr, _("Couldn't found gnuplot binary in path:%s"), app->settings->gnuplot_path);
		return NULL;
	}

	return strdup(s);
}


/*
 *Write/update gnuplot device data file.
 */
Eina_Bool
gnuplot_device_data_write(App_Info *app, Device *device)
{
	if(!device) return EINA_FALSE;

	time_t timestamp;
	struct tm *t;
	FILE *dat;

	timestamp = time(NULL);
	t = localtime(&timestamp);

	if(!(dat = fopen(gnuplot_device_data_file_get(device), "a")))
	{
		debug(stderr, _("Couldn't open gnuplot device data file '%s'"),gnuplot_device_data_file_get(device));
		return EINA_FALSE;
	}

	fprintf(dat,  "%d-%d-%d-%d:%d:%d %s\n",
				t->tm_mday,
				t->tm_mon+1,
				1900+t->tm_year,
				t->tm_hour,
				 t->tm_min,
				 t->tm_sec,
				 device_data_get(device));
	fclose(dat);

	return EINA_TRUE;
}
