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

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "bin-file.h"
#include "edit-buffer.h"
#include "error.h"

bool bin_file_check(const char * filename)
{
	/* Not exactly bullet proof but... */
	return strstr(filename, ".bin") || strstr(filename, ".BIN");
}

void bin_file_read(FILE * input, struct edit_buffer * buf,
		   unsigned long max_cols)
{
	unsigned long i = 0;
	unsigned long column = 0;

	while (!feof(input)) {
		int character = fgetc(input);
		if (character == EOF)
			break;
		int attribute = fgetc(input);
		if (attribute == EOF)
			error("premature end of file");
		buf->buffer[i++] = CHAR_ATTR_TO_INT(attribute, character);
		column++;
		if (column >= max_cols) {
			column = 0;
			i += buf->width - max_cols;
		}
	}
}
