/*
 * Copyright (C) 2003  Pekka Enberg <penberg@iki.fi>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _SCREEN_H
#define _SCREEN_H 1

#include <stdbool.h>

struct editor_context;

/* Visible screen information.  */
struct screen {
	unsigned long cursor_x;
	unsigned long cursor_y;
	unsigned long height;
	unsigned long width;
};

struct screen * screen_init(bool, unsigned long);
void screen_release(struct screen * screen);
void screen_draw_edit_buffer(struct screen *, struct edit_buffer *);
void screen_print_status(struct edit_buffer *, struct screen *,
			 struct editor_context *, char *);
void screen_move(unsigned long, unsigned long);
void screen_redraw(void);
char * screen_save_file_dialog(struct screen *);

#endif
