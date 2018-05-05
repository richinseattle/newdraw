/*
 * Copyright (C) 2002-2003  Pekka Enberg <penberg@iki.fi>
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

#include <assert.h>
#include <stdlib.h>

#include "edit-buffer.h"
#include "error.h"
#include "screen.h"

void edit_buffer_put(struct edit_buffer *buf, unsigned long x,
		     unsigned long y, int value)
{
	assert(x < buf->width);
	assert(x >= 0);
	assert(y < buf->height);
	assert(y >= 0);

	buf->buffer[y * buf->width + x] = value;

	if (y + 1 > buf->max_height)
		buf->max_height = y + 1;
}

int edit_buffer_get(struct edit_buffer *buf, unsigned long x,
		    unsigned long y)
{
	assert(x < buf->width);
	assert(x >= 0);
	assert(y < buf->height);
	assert(y >= 0);

	return buf->buffer[y * buf->width + x];
}

void edit_buffer_clear(struct edit_buffer *buf)

{
	int x, y;

	for (y = 0; y < buf->height; y++) {
		for (x = 0; x < buf->width; x++) {
			edit_buffer_put(buf, x, y, 0x0720);
		}
	}
	buf->max_height = 0;
}

struct edit_buffer * edit_buffer_create(unsigned long width,
					unsigned long height)
{
	struct edit_buffer * ret = malloc(sizeof(struct edit_buffer));

	ret->buffer = malloc(height * width * sizeof(int));
	if (!ret->buffer)
		error("Could not allocate memory for edit buffer.");

	ret->height = height;
	ret->width = width;
	ret->max_height = 0;
	ret->start_x = 0;
	ret->start_y = 0;

	return ret;
}

void edit_buffer_release(struct edit_buffer *buf)
{
	if (buf->buffer)
		free(buf->buffer);

	buf->buffer = NULL;
	free(buf);
}
