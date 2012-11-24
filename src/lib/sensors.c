/*
 * sensors.c
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
 
 
#include "sensors.h"


//
//
//
Sensor*
sensor_detect(char *s)
{
	char **arr;
	int i;
	Eina_List *l;
	Eina_List *database;
	Sensor *data, *sensor = NULL;
		
	printf("Detecting all sensors on serial line...\n");
	
	//Check if new device in trame.
	if(strncmp(s, "DEVICE;", 7) == 0)
	{
	   arr = eina_str_split(s, ";", 0);

		for (i = 0; arr[i]; i++)
		printf("%s\n", arr[i]);
     
		sensor = sensor_new(atoi(arr[1]), arr[2], NULL, NULL, NULL, NULL, NULL, NULL);
		sensor_datatype_set(sensor, arr[3]);
		sensor_data_set(sensor, arr[4]);
	
		printf("Get sensors informations from sensors database...\n");
		database = sensors_list_get();
		EINA_LIST_FOREACH(database, l, data)
		{
			if(strcmp(sensor_name_get(data), sensor_name_get(sensor)) == 0)
			{
				fprintf(stdout, "Found %s(%s)...\n", sensor_name_get(data), sensor_description_get(data));
				sensor_description_set(sensor, sensor_description_get(data));
				sensor_type_set(sensor, sensor_type_get(data));
				sensor_datasheeturl_set(sensor, sensor_datasheeturl_get(data));
				//sensor_picture_set(sensor, sensor_picture_get(data));
				break;
			}
		}
	}

	return sensor; 
}





