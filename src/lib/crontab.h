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

#define ANY 99
#define PAIR 88
#define UNPAIR 89


typedef enum Day_Of_Week
{
    SUN = (0),
    MON = (1),
    TUE = (2),
    WED = (3),
    THU = (4),
    FRI = (5),
    SAT = (6),
} Day_Of_Week;


typedef enum Month
{
    JAN = (1),
    FEB = (2),
    MAR = (3),
    APR = (4),
    MAY = (5),
    JUN = (6),
    JUL = (7),
    AUG = (8),
    SEP = (9),
    OCT = (10),
    NOV = (11),
    DEC = (12)
} Month;


typedef struct Cron_Entry
{
    unsigned char minute;       /*0 to 59*/
    unsigned char hour;         /*1 à 23*/
    unsigned char mday;         /*1 to 31*/
    unsigned char month;        /*1 to 12*/
    unsigned char day;          /*0 to 7*/

    Action_Type action_type;    /*Action type. Eg 'EXEC'*/
    const char *action_data;   /*Action data*/
}Cron_Entry;


void crontab_init();
void crontab_shutdown();

const Eina_List *crons_list_get();

Eina_Bool crons_list_entry_remove(Cron_Entry *cron_elem);
Eina_Bool crons_list_entry_add(Cron_Entry *cron_elem);

Cron_Entry *
cron_entry_new(unsigned char day, unsigned char mday, unsigned char month,
                unsigned char hour, unsigned char minute,
                Action_Type action_type,  const char *action_data);

const char *month_to_str(Month month);
const char *day_to_str(Day_Of_Week day);

#endif /*__#ifndef __CRONTAB_H_H*/
