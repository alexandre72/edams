/*
 * gnuplot.h
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


#ifndef __GNUPLOT_H
#define __GNUPLOT_H

#include "device.h"
#include "edams.h"

const char *gnuplot_device_png_write(App_Info *app, Device *device);
Eina_Bool gnuplot_device_data_write(App_Info *app, Device *device);

#endif /*__GNUPLOT_H*/
