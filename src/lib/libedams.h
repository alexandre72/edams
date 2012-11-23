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

#ifdef HAVE_EVIL
#include <windows.h>
#include <shlobj.h>
#endif

#undef EINA_LOG_DOMAIN_DEFAULT
#define EINA_LOG_DOMAIN_DEFAULT _log_dom
extern int _log_dom;

#include "gettext.h"


#ifdef HAVE_EVIL
#define DIR_SEPARATOR '\\'
#define DIR_SEPARATOR_S "\\"
#else
#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_S "/"
#endif

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#define __UNUSED__ __attribute__((unused))

#ifdef HAVE_EVIL
#define CRITICAL(...) printf( __VA_ARGS__); printf("\n");
#define ERR(...) printf( __VA_ARGS__); printf("\n");
#define WRN(...) printf(__VA_ARGS__); printf("\n");
#define INF(...) printf(__VA_ARGS__); printf("\n");
#define DBG(...) printf( __VA_ARGS__); printf("\n");
#else
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#endif

#endif /* __LIBEDAMS_H__ */
