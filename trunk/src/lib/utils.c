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

/*
 *Return user home dir
 */

/*
 *Close a window by deleting evas object passing arg
 */
void
window_clicked_close_cb (void *data, Evas_Object * obj __UNUSED__,
                         void *event_info __UNUSED__)
{
   evas_object_del(data);
   data = NULL;
}


/*
 *Show a window message to inform user about something
 */
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
}/*msgbox*/


/*
 *Set debugging mode
 */
void
set_debug_mode(Eina_Bool debug)
 {
	_debug = debug;
}


/*
 *Write a debug message out (if we are in debugging mode)
 */
void
debug(FILE *stream, const char *format, ...)
{
	if(!_debug)
		return;

	char logMessageBuffer[LOG_BUFF_MAX] = "0";

  	va_list params;
  	/* Get access to the variable parms */
  	va_start(params, format);

  	/* Convert formatted message */
  	vsprintf(&logMessageBuffer[strlen(logMessageBuffer)], format, params);

  	/* Write to the console or system log file */
  	if(stream == stdout)
  	{
    	fprintf(stdout, "\033[32m[DBG]\033[0m");
	}
	else if(stream == stderr)
		fprintf(stderr, "\033[31m[ERROR]\033[0m");

	fprintf(stream, "%s\n", logMessageBuffer);

  	/* Release parms */
  	va_end(params);
}/*debug*/




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
}/*_strdelchr*/


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
}/*strdelstr*/


/*
 *
 */
const char *
home_dir_get(void)
{
    const char *home_dir = NULL;

    home_dir = getenv("HOME");

    if (!home_dir || home_dir[0] == '\0')
        home_dir = "/tmp";

    home_dir = eina_stringshare_add(home_dir);

    return home_dir;
}/*home_dir_get*/


/*
 *
 */
static const char *
_env_expand(const char *in)
{
    Eina_Strbuf *sb;
    const char *ret, *p, *e1 = NULL, *e2 = NULL, *val;
    char *env;

    if (!in) return NULL;
    sb = eina_strbuf_new();
    if (!sb) return NULL;

    /* maximum length of any env var is the input string */
    env = alloca(strlen(in) + 1);
    for (p = in; *p; p++)
    {
        if (!e1)
        {
            if (*p == '$') e1 = p + 1;
            else eina_strbuf_append_char(sb, *p);
        }
        else if (!(((*p >= 'a') && (*p <= 'z')) ||
                   ((*p >= 'A') && (*p <= 'Z')) ||
                   ((*p >= '0') && (*p <= '9')) ||
                   (*p == '_')))
        {
            size_t len;

            e2 = p;
            len = (size_t)(e2 - e1);
            if (len > 0)
            {
                memcpy(env, e1, len);
                env[len] = 0;
                val = getenv(env);
                if (val) eina_strbuf_append(sb, val);
            }
            e1 = NULL;
            eina_strbuf_append_char(sb, *p);
        }
    }
    ret = eina_stringshare_add(eina_strbuf_string_get(sb));
    eina_strbuf_free(sb);
    return ret;
}/*env_expand*/


/*
 *
 */
static const char *
_xdg_user_dir_get(const char *key, const char *fallback)
{
    Eina_File *file = NULL;
    Eina_File_Line *line;
    Eina_Iterator *it = NULL;
    const char *config_home;
    char path[PATH_MAX];
    char *ret = NULL;

    config_home = efreet_config_home_get();
    snprintf(path, sizeof(path), "%s/user-dirs.dirs", config_home);

    file = eina_file_open(path, EINA_FALSE);
    if (!file) goto fallback;
    it = eina_file_map_lines(file);
    if (!it) goto fallback;
    EINA_ITERATOR_FOREACH(it, line)
    {
        const char *eq, *end;

        if (line->length < 3) continue;
        if (line->start[0] == '#') continue;
        if (strncmp(line->start, "XDG", 3)) continue;
        eq = memchr(line->start, '=', line->length);
        if (!eq) continue;
        if (strncmp(key, line->start, eq - line->start)) continue;
        if (++eq >= line->end) continue;
        if (*eq == '"')
        {
            if (++eq >= line->end) continue;
            end = memchr(eq, '"', line->end - eq);
        }
        else
        {
            end = line->end;
            while (isspace(*end)) end--;
        }
        if (!end) continue;
        ret = alloca(end - eq + 1);
        memcpy(ret, eq, end - eq);
        ret[end - eq] = '\0';
        break;
    }
fallback:
    if (it) eina_iterator_free(it);
    if (file) eina_file_close(file);
    if (!ret)
    {
        const char *home;
        home = home_dir_get();
        ret = alloca(strlen(home) + strlen(fallback) + 2);
        sprintf(ret, "%s/%s", home, fallback);
    }
    return _env_expand(ret);
}/*user_dir_get*/


/*
 *
 */
const char *
xdg_pictures_dir_get(void)
{
    const char *xdg_pictures_dir = NULL;
    xdg_pictures_dir = _xdg_user_dir_get("XDG_PICTURES_DIR", _("Pictures"));
    return xdg_pictures_dir;
}/*xdg_pictures_dir_get*/
