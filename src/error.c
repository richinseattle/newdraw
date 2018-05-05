/*
 * Copyright (C) 2002  Pekka Enberg <Pekka.Enberg@cs.helsinki.fi>
 *
 * Distributed under the terms of the GNU General Public License
 * version 2 or later.
 */
#include <curses.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR_MSG_BUFFER 1024

void
error(const char *format, ...)
{
	va_list args;
	char buffer[ERROR_MSG_BUFFER];

	endwin();
	va_start(args, format);
	vsnprintf(buffer, ERROR_MSG_BUFFER, format, args);
	va_end(args);
	printf("error: %s\n", buffer);
	exit(EXIT_FAILURE);
}
