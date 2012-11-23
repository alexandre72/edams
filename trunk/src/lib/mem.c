/*
 * mem.c
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


#include "mem.h"


static int malloc_count = 0;    //Count calls to MALLOC().  Count calls to MALLOC().
static int calloc_count = 0;    //Count calls to CALLOC().  Count calls to CALLOC().
static int realloc_count = 0;   //Count calls to REALLOC().
static int free_count = 0;      //Count calls to FREE(). 


//
//LibAST implementation of malloc().
//
void *
_malloc(const char *var, const char *filename, unsigned long line, size_t size)
{
    void *temp;

    ++malloc_count;
    
    //INF(_("Variable %s %lu bytes requested at %s:%lu."), var, size, filename, line);

    temp = (void *) malloc(size);

    return (temp);
}



//
//LibAST implementation of realloc().
//
void *
_realloc(const char *var, const char * filename, unsigned long line, void *ptr, size_t size)
{
    void *temp;

    ++realloc_count;

    //INF(_("Variable %s (%10p -> %lu) at %s:%lu."), var, ptr, (unsigned long) size, filename, line);
    if (!ptr) {
        temp = (void *) _malloc(var, filename, line, size);
    } 
    else if (size == 0) 
    {
        _free(var, filename, line, ptr);
        temp = NULL;
    } else 
    {
        temp = (void *) realloc(ptr, size);
    }
    return (temp);
}


//
//LibAST implementation of calloc().
//
void *
_calloc(const char *filename, unsigned long line, size_t count, size_t size)
{
    void *temp;

    ++calloc_count;
    //INF(_("%lu units of %lu bytes each requested at %s:%lu."), count, size, filename, line);
    temp = (void *) calloc(count, size);
    
    return (temp);
}

//LibAST implementation of free().
void
_free(const char * var, const char * filename, unsigned long line, void *ptr)
{
    ++free_count;
    //INF(_("Variable %s (%10p) at %s:%lu"), var, ptr, filename, line);
    if (ptr) 
    {
        free(ptr);
        ptr = NULL;
    } 
    //else 
   // {
   //     ERR(_("Caught attempt to free NULL pointer!"));
   // }
}

//
//LibAST implementation of strdup().
//
char *
_mystrdup(const char * var, const char * filename, unsigned long line, const char * str)
{
    register char * newstr;
    register size_t len;

    //INF(_("Variable %s (%10p) at %s:%lu."), var, str, filename, line);
    //Copy NUL byte also.
    len = strlen((char *) str) + 1;
    newstr = (char *) _malloc(var, (char *) filename, line, len);
    strcpy((char *) newstr, (char *) str);
    return (newstr);
}



//
//Dump listing of tracked pointers.
//
void
_dump_mem(void)
{
    INF(_("Dumping memory usage:"));
    INF(_("Calls to malloc(): %d."), malloc_count);
    INF(_("Calls to realloc(): %d."), realloc_count);
    INF(_("Calls to calloc(): %d."), calloc_count);    
    INF(_("Calls to free(): %d."), free_count);
}
