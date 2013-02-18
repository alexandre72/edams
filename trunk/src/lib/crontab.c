/*
 * crontab.c
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

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_Getopt.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "crontab.h"
#include "utils.h"

Eina_List *crons = NULL;

/*
 *
 */
const char *
day_to_str(Day_Of_Week day)
{
    if(day == SUN) return _("Sunday");
    if(day == MON) return _("Monday");
    if(day == TUE) return _("Tuesday");
    if(day == WED) return _("Wednesday");
    if(day == THU) return _("Thursday)");
    if(day == FRI) return _("Friday");
    if(day == SAT) return _("Saturday");
    else           return _("Any");
}/*day_to_str*/

/*
 *
 */
const char *
month_to_str(Month month)
{
    if(month == JAN) return _("January");
    if(month == FEB) return _("February");
    if(month == MAR) return _("March");
    if(month == APR) return _("April");
    if(month == MAY) return _("May");
    if(month == JUN) return _("June");
    if(month == JUL) return _("July");
    if(month == AUG) return _("August");
    if(month == SEP) return _("September");
    if(month == OCT) return _("October");
    if(month == NOV) return _("November");
    if(month == DEC) return _("December");
    else             return _("Any");
}/*month_to_str*/



static int
crontab_line_read(FILE *pf,char *s)
{
	   int i=0;
	   s[i++]=fgetc(pf);
	   while ( !feof(pf) && s[i-1]!='\n' )
	      s[i++]=fgetc(pf);
	   s[i-1]='\0';
	   return(1);
}/*crontan_line_read*/


/*
 *
 */
static int
crontab_line_is_comment(char *line)
{
	unsigned int i=0;
	while(i<strlen(line))
	    if(line[i]=='#')
	       return(1);
	    else
	        if(line[i]==' ')
	            i++;
	       else
	           return(0);
	   return(0);
}/*crontab_line_is_comment*/


/*
 *
 */
static unsigned char
_c(const char value[50])
{
    if(strcmp(value, "*") == 0)
        return ANY;
    else if(strcmp(value, "/2") == 0)
        return PAIR;
    else
        return atoi(value);
}/*_c*/



/*
 *
 */
void
crontab_init(void)
{
    FILE *crontab = NULL;

    debug(stdout, _("Listing crons entries with crontab"));

    crontab = popen("crontab -l","r");
    if(crontab)
    {
        char s[255];
        while(!feof(crontab))
        {
            crontab_line_read(crontab, s);
            if(!s[0]) return;

            if(!crontab_line_is_comment(s))
            {
                char f[5][50];

                Cron_Entry *cron_entry;

                /*#min hour day Month Day_Of_Week Command*/
                sscanf(s,"%s %s %s %s %s %[^\n]",f[0],f[1],f[2],f[3],f[4],f[5]);

                //printf("On:%s %s %s\n", day_to_str(atoi(f[2])), f[3], month_to_str(atoi(f[4])));
               // printf("At:%sh%smin\n", f[1], f[0]);
                //printf("Command=%s\n", f[5]);

                if(strstr(f[5], "edams -a"))
                {
                    Action_Type action_type;
                    const char *buf = strdup(f[5]);
                    const char *needle;

                    needle = strstr(f[5], "edams -a ");

                    if (!needle) goto error;
                    needle+=9;
                    action_type = action_str_to_type(strtok(needle, " "));

                    strdelstr(buf, "'");
                    needle = strstr(buf, "-d ");
                    needle+=3;
                    cron_entry = cron_entry_new(_c(f[2]), _c(f[3]), _c(f[4]), _c(f[1]), _c(f[0]), action_type, needle);
                    FREE(buf);
                }
                else
                {
                    cron_entry = cron_entry_new(_c(f[2]), _c(f[3]), _c(f[4]), _c(f[1]), _c(f[0]), ACTION_TYPE_UNKNOWN, f[5]);
                }

                crons = eina_list_append(crons, cron_entry);
            }
        }
        pclose(crontab);
    }
    else
    {
        error:
        debug(stderr, _("Can't parse crontab list"));
    }
}/*crons_init*/


/*
 *
 */
const Eina_List *
crons_list_get()
{
    return crons;
}/*crons_list_get*/


/*
 *
 */
static void
crons_list_write()
{
    Eina_List *l;
    Cron_Entry *cron_elem = NULL;
	FILE *crontab = NULL;
    char *s, *exe;

    s = tempnam(NULL, "edams.");
    if((crontab = fopen(s, "w+")) == NULL)
    {
        debug(stderr, _("Can't create a valid temp name file"));
        return;
    }

    EINA_LIST_FOREACH(crons, l, cron_elem)
   	{
        if(cron_elem->minute == ANY)
            fprintf(crontab, "*\t");
        else
            fprintf(crontab, "%d\t", cron_elem->minute);

        if(cron_elem->hour == ANY)
            fprintf(crontab, "*\t");
        else
            fprintf(crontab, "%d\t", cron_elem->hour);

        if(cron_elem->mday == ANY)
            fprintf(crontab, "*\t");
        else
            fprintf(crontab, "%d\t", cron_elem->mday);

        if(cron_elem->month == ANY)
            fprintf(crontab, "*\t");
        else
            fprintf(crontab, "%d\t", cron_elem->month);

        if(cron_elem->day == ANY)
            fprintf(crontab, "*\t");
        else
            fprintf(crontab, "%d\t", cron_elem->day);

        if(cron_elem->action_type != ACTION_TYPE_UNKNOWN)
            fprintf(crontab, "edams -a %s -d '%s'\n", action_type_to_str(cron_elem->action_type), cron_elem->action_data);
        else
            fprintf(crontab, "%s\n", cron_elem->action_data);
    }
    fclose(crontab);
    fflush(crontab);

    asprintf(&exe, "crontab %s", s);
    ecore_exe_run(exe, NULL);
    FREE(exe);
    FREE(s);
}/*crons_list_write*/


/*
 *
 */
void
crontab_shutdown()
{
    EINA_SAFETY_ON_NULL_RETURN(crons);
    debug(stdout, _("Free cron allocated entries"));

	Cron_Entry *cron_elem = NULL;

    /*Free all location node of Eina_List*/
	EINA_LIST_FREE(crons, cron_elem)
	{
        eina_stringshare_del(cron_elem->action_data);
        free(cron_elem);
        cron_elem = NULL;
	}
	eina_list_free(crons);
    crons = NULL;
}/*crons_shutdown*/


/*
 *
 */
Eina_Bool
crons_list_entry_add(Cron_Entry *cron_elem)
{
    crons = eina_list_append(crons, cron_elem);
    crons_list_write(crons);
    return EINA_TRUE;
}/*crons_list_entry_add*/


/*
 *
 */
Eina_Bool
crons_list_entry_remove(Cron_Entry *cron_elem)
{
    if(!crons) return EINA_FALSE;
    crons = eina_list_remove(crons, cron_elem);
    crons_list_write(crons);
    return EINA_TRUE;
}/*crons_list_entry_remove*/


/*
 *
 */
Cron_Entry *
cron_entry_new(unsigned char day, unsigned char mday, unsigned char month,
                unsigned char hour, unsigned char minute,
                Action_Type action_type,  const char *action_data)
{
    Cron_Entry *cron_entry = NULL;

    cron_entry = calloc(1, sizeof(Cron_Entry));

    if(!cron_entry)
        return NULL;

    cron_entry->day = day;
    cron_entry->mday = mday;
    cron_entry->month = month;
    cron_entry->hour = hour;
    cron_entry->minute = minute;

    cron_entry->action_type = action_type;
    cron_entry->action_data = eina_stringshare_add(action_data);

    return cron_entry;
}/*cron_entry_new*/


/*
 *
 */
 /*
static void
crontab_test()
{
    Cron_Entry *cron_elem = NULL;

    crons_init();

    cron_elem = cron_entry_new(MON, ANY, JAN, 12, 30,  "edams -a \"{\"TYPE\":\"EXEC\",\"DATA\":{\"EXEC\":\"/usr/bin/gedit\"}}\"");
    crons_list_entry_add(cron_elem);

    crons_shutdown();
}*/
/*crontab_test*/
