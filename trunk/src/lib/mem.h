/*
 * mem.h
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



#ifndef __MEM_H__
#define __MEM_H__

#include "libedams.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


# define MALLOC(sz)                             _malloc(#sz, __FILE__, __LINE__, (sz))
# define CALLOC(type,n)                         _calloc( __FILE__, __LINE__, (n), (sizeof(type)))
# define REALLOC(mem,sz)                        _realloc((mem, __FILE__, __LINE__, (mem), (sz))
# define FREE(ptr)                              do { _free( #ptr, __FILE__, __LINE__, (ptr)); (ptr) = NULL; } while (0)
# define STRDUP(s)                              _mystrdup(#s, __FILE__, __LINE__, (s))
# define MALLOC_DUMP()                          _dump_mem()


//mem.c:my malloc/free/calloc reimplementation..
void _free(const char * var, const char * filename, unsigned long line, void *ptr);
void *_calloc(const char *filename, unsigned long line, size_t count, size_t size);
void *_realloc(const char *var, const char * filename, unsigned long line, void *ptr, size_t size);
void *_malloc(const char *var, const char *filename, unsigned long line, size_t size);
void _dump_mem(void);
char *_mystrdup(const char *var, const char * filename, unsigned long line, const char * str);
#endif /* __MEM_H__ */
