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

#ifndef _EDIT_BUFFER_H
#define _EDIT_BUFFER_H 1

struct screen;

#define CHAR_ATTR_TO_INT(attr, c) (((attr & 0xFF) << 8) | (c & 0xFF))

/* Off-screen edit buffer.  */
struct edit_buffer {
	unsigned long start_x;
	unsigned long start_y;
	unsigned long height;
	unsigned long width;
	unsigned long max_height;
	unsigned int *buffer;
};

struct edit_buffer * edit_buffer_create(unsigned long, unsigned long);
void edit_buffer_release(struct edit_buffer *);
void edit_buffer_clear(struct edit_buffer *);
void edit_buffer_draw_to_screen(struct edit_buffer *, struct screen *);
void edit_buffer_put(struct edit_buffer *, unsigned long, unsigned long, int);
int edit_buffer_get(struct edit_buffer *, unsigned long, unsigned long);

#endif
