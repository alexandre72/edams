/*
 * cosm.h
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
 * MERCHANTABILITY or FITNESS FOR A PAR TICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EDAMS. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __COSM_H
#define __COSM_H

#include "edams.h"

Eina_Bool cosm_location_feed_add(Location *location);
Eina_Bool cosm_location_feed_delete(Location *location);
Eina_Bool cosm_device_datastream_update(Location *location, Widget *widget);

#endif /*__COSM_H*/
