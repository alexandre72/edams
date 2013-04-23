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
minute_to_str(const char *minute)
{
    if(strcmp(minute, "*" ) == 0) return _("Every Minute");
    else if(strstr(minute, "*/"))
    {
        char *ret;
        char *s = strdup(minute);
        strdelstr(s, "*/");
        asprintf(&ret, "Every %s minutes", s);
        FREE(s);
        return ret;
    }
    else return minute;
}/*hour_to_str*/


/*
 *
 */
const char *
hour_to_str(const char *hour)
{
    if(strcmp(hour, "0" ) == 0) return _("12 Midnight");
    else if(strcmp(hour, "12" ) == 0) return _("12 Noon");
    else if(strcmp(hour, "*" ) == 0) return _("Every Hour");
    else if(strncmp(hour, "*/", 2) == 0)
    {
        char *ret;
        char *s = strdup(hour);
        strdelstr(s, "*/");
        asprintf(&ret, "Every %s hours", s);
        FREE(s);
        return ret;
    }
    else return hour;
}/*hour_to_str*/


/*
 *
 */
const char *
day_month_to_str(const char *day_month)
{
    if(strcmp(day_month, "*" ) == 0) return _("Every Day");
    else if(strstr(day_month, "*/" ))
    {
        char *ret;
        char *s = strdup(day_month);
        strdelstr(s, "*/");
        asprintf(&ret, "Every %s days", s);
        FREE(s);
        return ret;
    }
    else return day_month;
}/*hour_to_str*/


/*
 *
 */
const char *
month_to_str(const char *month)
{
    if(strcmp(month, "1" ) == 0) return _("January");
    else if(strcmp(month, "2" ) == 0) return _("February");
    else if(strcmp(month, "3" ) == 0) return _("March");
    else if(strcmp(month, "4" ) == 0) return _("April");
    else if(strcmp(month, "5" ) == 0) return _("May");
    else if(strcmp(month, "6" ) == 0) return _("June");
    else if(strcmp(month, "7" ) == 0) return _("July");
    else if(strcmp(month, "2" ) == 0) return _("August");
    else if(strcmp(month, "3" ) == 0) return _("September");
    else if(strcmp(month, "4" ) == 0) return _("October");
    else if(strcmp(month, "5" ) == 0) return _("November");
    else if(strcmp(month, "6" ) == 0) return _("December");
    else if(strcmp(month, "*" ) == 0) return _("Every Month");
    else if(strstr(month, "*/" ))
    {
        char *ret;
        char *s = strdup(month);
        strdelstr(s, "*/");
        asprintf(&ret, "Every %s months", s);
        FREE(s);
        return ret;
    }
    else return month;
}/*month_to_str*/


/*
 *
 */
const char *
day_week_to_str(const char *day_week)
{
    if(strcmp(day_week, "0" ) == 0) return _("Sunday");
    else if(strcmp(day_week, "1" ) == 0) return _("Monday");
    else if(strcmp(day_week, "2" ) == 0) return _("Tuesday");
    else if(strcmp(day_week, "3" ) == 0) return _("Wednesday");
    else if(strcmp(day_week, "4" ) == 0) return _("Thursday");
    else if(strcmp(day_week, "5" ) == 0) return _("Friday");
    else if(strcmp(day_week, "6" ) == 0) return _("Saturday");
    else if(strcmp(day_week, "*" ) == 0) return _("Every Weekday");
    else if(strstr(day_week, "*/" ))
    {
        char *ret;
        char *s = strdup(day_week);
        strdelstr(s, "*/");
        asprintf(&ret, "Every %s weeks", s);
        FREE(s);
        return ret;
    }
    else return day_week;
}/*day_week_to_str*/

/*
 *
 */
static int
_crontab_line_read(FILE *pf,char *s)
{
	   int i=0;
	   s[i++]=fgetc(pf);
	   while ( !feof(pf) && s[i-1]!='\n' )
	      s[i++]=fgetc(pf);
	   s[i-1]='\0';
	   return(1);
}/*crontad_line_read*/


/*
 *
 */
static int
_crontab_line_is_comment(char *line)
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
}/*_crontab_line_is_comment*/


/*
 *
 */
void
crontab_init(void)
{
    FILE *crontab = NULL;

    debug(MSG_INFO, _("Listing crons entries with crontab"));

    crontab = popen("crontab -l","r");
    if(crontab)
    {
        char s[255];
        while(!feof(crontab))
        {
            _crontab_line_read(crontab, s);
            if(!s[0]) return;

            if(!_crontab_line_is_comment(s))
            {
                char f[5][50];

                Cron_Entry *cron_entry;

                /*#min hour day Month Day_Of_Week Command*/
                sscanf(s,"%s %s %s %s %s %[^\n]",f[0],f[1],f[2],f[3],f[4],f[5]);

                //printf("Run '%s' %s %s ", f[5], minute_to_str(f[0]), hour_to_str(f[1]));
                //printf("%s %s %s\n", day_month_to_str(f[2]), month_to_str(f[3]), day_week_to_str(f[4]));

                if(strstr(f[5], "edams -a"))
                {
                    Action_Type action_type;
                    char *buf = strdup(f[5]);
                    char *needle;

                    needle = strstr(f[5], "edams -a ");

                    if (!needle) goto error;
                    needle+=9;
                    action_type = action_str_to_type(strtok(needle, " "));

                    strdelstr(buf, "'");
                    needle = strstr(buf, "-d ");
                    needle+=3;
                    cron_entry = cron_entry_new(f[0], f[1], f[2], f[3], f[4],
                                                action_type, needle);
                    FREE(buf);
                }
                else
                {
                    cron_entry = cron_entry_new(f[0], f[1], f[2], f[3], f[4],
                                                ACTION_TYPE_UNKNOWN, f[5]);
                }

                crons = eina_list_append(crons, cron_entry);
            }
        }
        pclose(crontab);
    }
    else
    {
        error:
        debug(MSG_ERROR, _("Can't parse crontab list"));
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
        debug(MSG_ERROR, _("Can't create a valid temp name file"));
        return;
    }

    EINA_LIST_FOREACH(crons, l, cron_elem)
   	{
        fprintf(crontab, "%s\t", cron_elem->minute);
        fprintf(crontab, "%s\t", cron_elem->hour);
        fprintf(crontab, "%s\t", cron_elem->day_month);
        fprintf(crontab, "%s\t", cron_elem->month);
        fprintf(crontab, "%s\t", cron_elem->day_week);

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

	Cron_Entry *cron_elem = NULL;

    /*Free all location node of Eina_List*/
	EINA_LIST_FREE(crons, cron_elem)
	{
        eina_stringshare_del(cron_elem->minute);
        eina_stringshare_del(cron_elem->hour);
        eina_stringshare_del(cron_elem->day_month);
        eina_stringshare_del(cron_elem->month);
        eina_stringshare_del(cron_elem->day_week);
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
cron_entry_new(char *minute, char *hour,
              char * day_month, char * month,  char * day_week,
                Action_Type action_type,   char *action_data)
{
    Cron_Entry *cron_entry = NULL;

    cron_entry = calloc(1, sizeof(Cron_Entry));

    if(!cron_entry)
        return NULL;

    cron_entry->minute = eina_stringshare_add(minute);
    cron_entry->hour = eina_stringshare_add(hour);
    cron_entry->day_month = eina_stringshare_add(day_month);
    cron_entry->month = eina_stringshare_add(month);
    cron_entry->day_week = eina_stringshare_add(day_week);

    cron_entry->action_type = action_type;
    cron_entry->action_data = eina_stringshare_add(action_data);

    return cron_entry;
}/*cron_entry_new*/
