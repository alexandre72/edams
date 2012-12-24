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

#include <ctype.h>

#include "libedams.h"
#include "gettext.h"
#include "settings.h"


#define LOG_BUFF_MAX 512

Eina_Bool _debug = EINA_FALSE;

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
   evas_object_del (data);
}


/*Show a window message to inform user about something*/
void
msgbox (const char *msg)
{
   Evas_Object *win, *popup, *ic;
   Evas_Object *bt;

   win = elm_win_util_standard_add ("confirmation", _("Confirmation"));
   elm_win_autodel_set (win, EINA_TRUE);
   elm_win_center (win, EINA_TRUE, EINA_TRUE);
   evas_object_show (win);
   evas_object_resize (win, 480, 400);

   popup = elm_popup_add (win);
   evas_object_size_hint_weight_set (popup, EVAS_HINT_EXPAND,
                                     EVAS_HINT_EXPAND);
   elm_object_text_set (popup, msg);

   bt = elm_button_add (popup);
   elm_object_text_set (bt, _("Ok"));
   elm_object_part_content_set (popup, "button1", bt);
   ic = elm_icon_add (popup);
   elm_icon_order_lookup_set (ic, ELM_ICON_LOOKUP_FDO_THEME);
   elm_icon_standard_set (ic, "apply-window");
   elm_object_part_content_set (bt, "icon", ic);
   evas_object_smart_callback_add (bt, "clicked", window_clicked_close_cb,
                                   win);

   evas_object_show (popup);
}


/*Set debugging mode*/
void
set_debug_mode(Eina_Bool debug)
 {
  _debug = debug;
}


/*Write a debug message out (if we are in debugging mode)*/
void
debug(FILE *stream, char *theFormat, ...)
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
		fprintf(stdout, "\033[31mERROR:\033[0m");

	fprintf(stdout, "%s\n", logMessageBuffer);

  	/* Release parms */
  	va_end(theParms);
}