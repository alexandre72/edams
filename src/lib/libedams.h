/*
 * libedams.h
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
#ifndef __LIBEDAMS_H__
#define __LIBEDAMS_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Elementary.h>

#ifdef HAVE_EFREET
#include <Efreet.h>
#endif

#include "gettext.h"

#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_S "/"

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#define __UNUSED__ __attribute__((unused))

#define FREE(ptr)  do { _free( #ptr, __FILE__, __LINE__, (ptr)); (ptr) = NULL; } while (0)

inline void
_free(const char * var, const char * filename, unsigned long line, void *ptr)
{
    //++free_count;
    //INF(_("Variable %s (%10p) at %s:%lu"), var, ptr, filename, line);
    if (ptr)
    {
        free(ptr);
        ptr = NULL;
    }
    else
    {
        fprintf(stderr, _("Caught attempt to free NULL pointer variable %s at %s:%lu!"), var, filename, line);
    }
}//_free


#endif /* __LIBEDAMS_H__ */
