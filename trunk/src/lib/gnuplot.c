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

static const char *gnuplot_device_data_file_get(Widget *widget);


/*
 *Return gnuplot device data file path.
 */
static const char *
gnuplot_device_data_file_get(Widget *widget)
{
	if(!widget) return NULL;

	const char *s;
	asprintf(&s, "%s/%s.dat", edams_devices_data_path_get(), widget_xpl_device_get(widget));
	return s;
}


/*
 *Write png data graph from device data and return png data path.
 */
const char *
gnuplot_device_png_write(Widget *widget)
{
	char *ret = NULL;
	char *f = NULL;

	//TODO:should handle xrange to let user select data period(monthly, years, daily, hour).
	//fprintf(gnuplotPipe, "set xrange [\"01-01-2013-17:36:00\":\"31-01-2013-17:37:00\"]\n");
	//TODO:should handle options like grid, title, ylabel, xlabel, resolution format, histograms/plots...

	if(!widget) return f;

	if(!edams_settings_gnuplot_path_get())
	{
		debug(stderr, _("Gnuplot binary path isn't set!"));
		return f;
	}

	FILE *gnuplot_pipe;
	gnuplot_pipe = popen(edams_settings_gnuplot_path_get(), "w");
	if (gnuplot_pipe)
	{
		asprintf(&ret,"%s/%s.png", edams_devices_data_path_get(), widget_xpl_device_get(widget));
		fprintf(gnuplot_pipe, "set term png size 600,300\n");
		fprintf(gnuplot_pipe, "set output \"%s\"\n", ret);
		fprintf(gnuplot_pipe, "set xdata time\n");
 		fprintf(gnuplot_pipe, "set format x \"%%d/%%m\"\n");
  		fprintf(gnuplot_pipe, "set ylabel \"%s(%s)\"\n", xpl_type_to_units(widget_xpl_type_get(widget)), xpl_type_to_unit_symbol(widget_xpl_type_get(widget)));
  		fprintf(gnuplot_pipe, "set timefmt \"%%d-%%m-%%Y-%%H:%%M:%%S\"\n");
  		fprintf(gnuplot_pipe, "set grid\n");
        f = gnuplot_device_data_file_get(widget);
		fprintf(gnuplot_pipe, "plot '%s' using 1:2 with lines title '%s'\n", f, widget_xpl_device_get(widget));
        FREE(f);
		fprintf(gnuplot_pipe,"exit\n");
		pclose(gnuplot_pipe);
	}
	else
	{
		debug(stderr, _("Couldn't found gnuplot binary in path:%s"), edams_settings_gnuplot_path_get());
	}

	return ret;
}


/*
 *Write/update gnuplot device data file.
 */
Eina_Bool
gnuplot_device_data_write(Widget *widget)
{
	time_t timestamp;
	struct tm *t;
	FILE *dat;
    const char *f;

	if(!widget) return EINA_FALSE;

	timestamp = time(NULL);
	t = localtime(&timestamp);

    f = gnuplot_device_data_file_get(widget);
	if(!(dat = fopen(f, "a")))
	{
		debug(stderr, _("Can't open gnuplot device data file '%s'"), gnuplot_device_data_file_get(widget));
        FREE(f);
		return EINA_FALSE;
	}
    FREE(f);

	fprintf(dat,  "%d-%d-%d-%d:%d:%d %s\n",
				t->tm_mday,
				t->tm_mon+1,
				1900+t->tm_year,
				t->tm_hour,
				 t->tm_min,
				 t->tm_sec,
				 widget_xpl_current_get(widget));
	fclose(dat);

	return EINA_TRUE;
}
