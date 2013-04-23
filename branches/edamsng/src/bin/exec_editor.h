/*
 * debug_editor.h
 *
 * Copyright (C) 2012 - Alexandre Dussart
 *
 * Elm Extension Pack is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Elm Extension Pack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Elm Extension Pack. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __EXECEDITOR_H
#define __EXECEDITOR_H

#include <Evas.h>

typedef struct _ExecEditor
{
	Evas_Object *win;
	Evas_Object *grid, *hbox, *frame;
	Evas_Object *exec_entry, *terminal_check;
	Evas_Object *icon, *cancel_button, *ok_button;
} ExecEditor;

ExecEditor *execeditor_add();
void execeditor_close(ExecEditor *execeditor);
#endif
