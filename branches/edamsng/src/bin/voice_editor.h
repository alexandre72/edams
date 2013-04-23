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


#ifndef __VOICEEDITOR_H
#define __VOICEEDITOR_H

#include <Evas.h>

typedef struct _VoiceEditor
{
	Evas_Object *win;
	Evas_Object *grid, *hbox, *frame;
	Evas_Object *message_entry, *progressbar;
	Evas_Object *icon, *cancel_button, *ok_button;
	char *sound_file;
} VoiceEditor;

VoiceEditor *voiceeditor_add();
void voiceeditor_close(VoiceEditor *voiceeditor);
void voiceeditor_sound_file_set(const char *file);
const char *voiceeditor_sound_file_get(VoiceEditor *voiceeditor);
#endif
