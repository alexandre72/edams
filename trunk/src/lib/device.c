/*
 * devices.c
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


#include "device.h"
#include "libedams.h"

//
//
//
Device*
device_detect(char *s)
{
	char **arr;
	Device *data, *device = NULL;
	unsigned int n;

	//Check if new device in trame.
	if(strncmp(s, "DEVICE;", 7) == 0)
	{
	   arr = eina_str_split_full(s, ";", 5, &n);

		if(n == 5 && (strcmp(arr[4], "OK") == 0))
		{

			if(!arr[1] || !arr[2] || !arr[3])
			{
				FREE(arr[0]);
				FREE(arr);
				return device;
			}


			device = device_new(atoi(arr[1]), arr[2], NULL, NULL, NULL);
			device_data_set(device, arr[3]);

			Eina_List *l;
			Eina_List *database;
			database = devices_list_get();
			EINA_LIST_FOREACH(database, l, data)
			{
				if(strcmp(device_name_get(data), device_name_get(device)) == 0)
				{
					//fprintf(stdout, "INFO:Found %s(%s) device on serial buffer.\n", device_name_get(data), device_description_get(data));
					device_description_set(device, device_description_get(data));
					device_type_set(device, device_type_get(data));
					device_datasheeturl_set(device, device_datasheeturl_get(data));
					device_meter_set(device, device_meter_get(data));
					break;
				}
			}
			devices_list_free(database);
		}
		FREE(arr[0]);
		FREE(arr);
	}

	return device;
}
