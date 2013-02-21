/*
 * crontab.h
 * This file is part of EDAMS
 *
 * Copyright (C) 2013 - Alexandre Dussart
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

#ifndef __CRONTAB_H
#define __CRONTAB_H

#include <Eina.h>

#include "action.h"


typedef struct Cron_Entry
{
    Eina_Stringshare *minute;       /*0 to 59*/
    Eina_Stringshare *hour;         /*0 à 23*/
    Eina_Stringshare *day_month;    /*1 to 31*/
    Eina_Stringshare *month;        /*1 to 12*/
    Eina_Stringshare *day_week;     /*0 to 6*/

    Action_Type action_type;    /*Action type. Eg 'EXEC'*/
    const char *action_data;   /*Action data*/
}Cron_Entry;


void crontab_init();
void crontab_shutdown();

const Eina_List *crons_list_get();

Eina_Bool crons_list_entry_remove(Cron_Entry *cron_elem);
Eina_Bool crons_list_entry_add(Cron_Entry *cron_elem);

Cron_Entry *
cron_entry_new(char *minute, char *hour,
              char * day_month, char * month,  char * day_week,
                Action_Type action_type,   char *action_data);

const char *minute_to_str(const char *minute);
const char *hour_to_str(const char * hour);
const char *day_month_to_str(const char * day_month);
const char *month_to_str(const char *month);
const char *day_week_to_str(const char *day_week);

#endif /*__#ifndef __CRONTAB_H_H*/
