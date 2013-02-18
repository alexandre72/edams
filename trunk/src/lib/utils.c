/*
 * utils.c
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


#include <Elementary.h>

#include <ctype.h>

#include "utils.h"
#include "settings.h"


#define LOG_BUFF_MAX 512

Eina_Bool _debug = EINA_FALSE;



inline void
_free(const char * var, const char * filename, unsigned long line, void *ptr)
{
    //++free_count;
    //INF(_("Variable %s (%10p) at %s:%lu"), var, ptr, filename, line);
    if (ptr)
    {
        free((void *)ptr);
        ptr = NULL;
    }
    else
    {
        debug(stderr, _("Caught attempt to free NULL pointer variable %s at %s:%lu!"), var, filename, line);
    }
}//_free



/*Keep evas object image aspect ratio*/
void
evas_object_image_scale (Evas_Object * obj, int width, int height)
{
   int w3, h3;
   int w2, h2;

   evas_object_image_size_get (obj, &w2, &h2);

   w3 = width;
   h3 = ((double) w3 * h2) / (double) w2;

   if (h3 > height)
   {
      h3 = height;
      w3 = ((double) h3 * w2) / (double) h2;
   }

   evas_object_image_fill_set (obj, 0, 0, w3, h3);
   evas_object_resize (obj, w3, h3);
}

/*Return user home dir*/
const char *
user_home_get (void)
{
   return (getenv ("HOME"));
}

/*Close a window by deleting evas object passing arg*/
void
window_clicked_close_cb (void *data, Evas_Object * obj __UNUSED__,
                         void *event_info __UNUSED__)
{
   evas_object_del(data);
   data = NULL;
}


/*Show a window message to inform user about something*/
void
msgbox (const char *msg)
{
	Evas_Object *win, *bx;
	Evas_Object *label, *ic, *bt;

   	win = elm_win_util_standard_add ("confirmation", _("Confirmation"));
   	elm_win_autodel_set (win, EINA_TRUE);
   	elm_win_center (win, EINA_TRUE, EINA_TRUE);

	bx = elm_box_add(win);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bx);
	evas_object_show(bx);

	label = elm_label_add(win);
	elm_object_text_set(label, msg);
	elm_box_pack_end(bx, label);
	evas_object_show(label);

	bt = elm_button_add(win);
    elm_object_text_set(bt, _("Close"));
	ic = elm_icon_add(win);
   	elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
	elm_icon_standard_set(ic, "window-close");
   	elm_object_part_content_set(bt, "icon", ic);
    elm_box_pack_end(bx, bt);
    evas_object_show(bt);
    evas_object_smart_callback_add(bt, "clicked", window_clicked_close_cb , win);

   	evas_object_show (win);
}


/*Set debugging mode*/
void
set_debug_mode(Eina_Bool debug)
 {
	_debug = debug;
}


/*Write a debug message out (if we are in debugging mode)*/
void
debug(FILE *stream, const char *theFormat, ...)
{
	if(!_debug)
		return;

	char logMessageBuffer[LOG_BUFF_MAX] = "0";

  	va_list theParms;
  	/* Get access to the variable parms */
  	va_start(theParms, theFormat);

  	/* Convert formatted message */
  	vsprintf(&logMessageBuffer[strlen(logMessageBuffer)], theFormat, theParms);

  	/* Write to the console or system log file */
  	if(stream == stdout)
		fprintf(stdout, "\033[32mDBG:\033[0m");
	else if(stream == stderr)
		fprintf(stderr, "\033[31mERROR:\033[0m");

	fprintf(stream, "%s\n", logMessageBuffer);

  	/* Release parms */
  	va_end(theParms);
}


/*
 *
 */
const char *
filename_create(const char *filename)
{
    unsigned int i = 0;
    char *s;

    asprintf(&s, "%s", filename);
    while(ecore_file_exists(s) == EINA_TRUE)
	{
	    FREE(s);
	    asprintf(&s, "%s-%03d.eet", filename, i);
		i++;
	}

	return s;
}/*filename_create*/




/*
 * delete one character from a string
 */
void
_strdelchr( char *s, size_t i, size_t *a, size_t *b)
{
  size_t        j;

  if( *a == *b)
    *a = i - 1;
  else
    for( j = *b + 1; j < i; j++)
      s[++(*a)] = s[j];
  *b = i;
}

/*
 * delete all occurrences of characters in search from s
 * returns nr. of deleted characters
 */
size_t
strdelstr(char *s, const char *search)
{

    if(!s || !search) return 0;

  size_t        l               = strlen(s);
  size_t        n               = strlen(search);
  size_t        i;
  size_t        a               = 0;
  size_t        b               = 0;

  for( i = 0; i < l; i++)
    if( memchr( search, s[i], n))
      _strdelchr( s, i, &a, &b);
  _strdelchr( s, l, &a, &b);
  s[++a] = '\0';
  return l - a;
}
