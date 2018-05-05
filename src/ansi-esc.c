/*
 * Copyright (C) 2004  Pekka Enberg <penberg@iki.fi>
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
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "error.h"
#include "edit-buffer.h"

/*
 *	ANSI escape sequence context keeps track of the current state while
 *	reading a file.
 */
struct ans_escape_seq_ctx {
	unsigned long current_line;
	unsigned long current_col;

	unsigned long saved_line;
	unsigned long saved_col;

	unsigned char bold;
	unsigned char fg_color;
	unsigned char bg_color;
};

#define DEFAULT_BOLD     0
#define DEFAULT_BG_COLOR 0x00
#define DEFAULT_FG_COLOR 0x07

static void __ans_move_cursor(struct ans_escape_seq_ctx * ctx,
			      unsigned long line, unsigned long col)
{
	ctx->current_line = line;
	ctx->current_col  = col;
}

static void ans_move_cursor(struct ans_escape_seq_ctx * ctx,
			    char * params, unsigned long len)
{
	unsigned long line, col;

	char * end = NULL;
	line = strtol(params, &end, 10) - 1;
	col  = strtol(++end, NULL, 10) - 1;

	__ans_move_cursor(ctx, line, col);
}

static void __ans_move_cursor_up(struct ans_escape_seq_ctx * ctx,
				 unsigned long lines)
{
	if (lines > ctx->current_line)
		lines = ctx->current_line;

	ctx->current_line -= lines;
}

static void ans_move_cursor_up(struct ans_escape_seq_ctx * ctx,
			       char * params, unsigned long len)
{
	unsigned long lines = 1;

	if (len != 0)
		lines = strtol(params, NULL, 10);
		
	__ans_move_cursor_up(ctx, lines);
}

static void __ans_move_cursor_down(struct ans_escape_seq_ctx * ctx, unsigned long lines)
{
	ctx->current_line += lines;
}

static void ans_move_cursor_down(struct ans_escape_seq_ctx * ctx,
				 char * params, unsigned long len)
{
	unsigned long lines = 1;

	if (len != 0)
		lines = strtol(params, NULL, 10);

	__ans_move_cursor_down(ctx, lines);
}

unsigned long clamp_max(unsigned long val, unsigned long max)
{
	return (val > max ? max : val);
}

#define MAX_COL 80

static void __ans_move_cursor_forward(struct ans_escape_seq_ctx * ctx, unsigned long spaces)
{
	ctx->current_col = clamp_max(ctx->current_col + spaces, MAX_COL);
}

static void ans_move_cursor_forward(struct ans_escape_seq_ctx * ctx,
				    char * params, unsigned long len)
{
	__ans_move_cursor_forward(ctx, strtol(params, NULL, 10));
}

static void __ans_move_cursor_back(struct ans_escape_seq_ctx * ctx, unsigned long spaces)
{
	if (spaces > ctx->current_col)
		spaces = ctx->current_col;

	ctx->current_col -= spaces;
}

static void ans_move_cursor_back(struct ans_escape_seq_ctx * ctx,
				 char * params, unsigned long len)
{
	__ans_move_cursor_back(ctx, strtol(params, NULL, 10));
}

static void __ans_save_cursor_pos(struct ans_escape_seq_ctx * ctx)
{
	ctx->saved_line = ctx->current_line;
	ctx->saved_col  = ctx->current_col;
}

static void __ans_restore_cursor_pos(struct ans_escape_seq_ctx * ctx)
{
	/* FIXME What if no one called ans_save_cursor_pos before?? */
	ctx->current_line = ctx->saved_line;
	ctx->current_col  = ctx->saved_col;
}

static void ans_clear_screen(struct edit_buffer * buf,
			     struct ans_escape_seq_ctx * ctx,
			     char * params, int len)
{
	assert(len == 1);
	assert(params[0] == '2');

	edit_buffer_clear(buf);
	ctx->current_col = 0;
	ctx->current_line = 0;
}

static void __ans_clear_eol(struct ans_escape_seq_ctx * ctx)
{
	assert(!"oops");
}

static void __ans_set_display_attr(struct ans_escape_seq_ctx * ctx,
				   unsigned long attr)
{
	/* Not supported.  */
	if (attr == 4 || attr == 5 || attr == 7 || attr == 8)
		return;

	if (attr == 0) {
		ctx->bold     = DEFAULT_BOLD;
		ctx->fg_color = DEFAULT_FG_COLOR;
		ctx->bg_color = DEFAULT_BG_COLOR;
	}
	else if (attr == 1)
		ctx->bold = 1;
	else if (attr >= 30 && attr <= 37)
		ctx->fg_color = attr - 30;
	else if (attr >= 40 && attr <= 47)
		ctx->bg_color = attr - 40;
	else
		error("unknown attr %i", attr);
}

static void ans_set_display_attrs(struct ans_escape_seq_ctx * ctx,
				  char * params, int len)
{
	char * end = NULL;
	for (;;) {
		unsigned long attr = strtol(params, &end, 10);
		if (end == params && attr == 0)
			break;
		__ans_set_display_attr(ctx, attr);
		params = end + 1;
	}
}

static bool ans_param_char(int ch)
{
	return isdigit(ch) || ch == ';' || ch == '?';
}

static bool ans_eof(int ch)
{
#define DOS_EOF 26
	return ch == EOF || ch == DOS_EOF;
}

static void ans_parse_seq(struct edit_buffer * buf,
			  struct ans_escape_seq_ctx * ctx, FILE * input)
{
#define MAX_PARAMS_LEN 32 
	char params[MAX_PARAMS_LEN];
	int len = 0;
	int ch;

	memset(params, 0, MAX_PARAMS_LEN);

	/* Parse parameters.  */
	for (;;) {
		ch = fgetc(input);
		if (!ans_param_char(ch))
			break;

		assert(len < MAX_PARAMS_LEN);
		params[len++] = ch;
	}
	if (ans_eof(ch))
		error("corrupt escape sequence");

	/* Parse command.  */
	switch (ch) {
		case 'H':
		case 'f':
			ans_move_cursor(ctx, params, len);
			break;
		case 'A':
			ans_move_cursor_up(ctx, params, len);
			break;
		case 'B':
			ans_move_cursor_down(ctx, params, len);
			break;
		case 'C':
			ans_move_cursor_forward(ctx, params, len);
			break;
		case 'D':
			ans_move_cursor_back(ctx, params, len);
			break;
		case 'R':
			/* report cursor pos */
			break;
		case 's':
			__ans_save_cursor_pos(ctx);
			break;
		case 'u':
			__ans_restore_cursor_pos(ctx);
			break;
		case 'J':
			ans_clear_screen(buf, ctx, params, len);
			break;
		case 'K':
			__ans_clear_eol(ctx);
			break;
		case 'm':
			ans_set_display_attrs(ctx, params, len);
			break;
		case 'h':
			/* put screen in mode */
			break;
		case 'l':
			/* reset screen mode */
			break;
		default:
			error("escape code %c not supported", ch);
			break;
	}
}

static void ans_crlf(struct ans_escape_seq_ctx * ctx)
{
	ctx->current_col = 0;
	ctx->current_line++;
}

static void ans_write_char(struct edit_buffer * buf,
			   struct ans_escape_seq_ctx * ctx, int ch)
{
	assert(ch != 7);
	assert(ch != 8);
	assert(ch != 9);
	assert(ch != 11);

	if (ch == 10) {
		ans_crlf(ctx);
		return;
	}

	/* Ignore */
	if (ch == 12 || ch == 13)
		return;

	assert(ch != 14);
	assert(ch != 15);
	assert(ch != 26);
	assert(ch != 27);
	assert(ch != 127);

	assert(ctx->current_col <= MAX_COL);
	
	if (ctx->current_col == MAX_COL)
		ans_crlf(ctx);

	unsigned char fg_color =
		(ctx->bold == 1 ? ctx->fg_color + 8 : ctx->fg_color);

	unsigned long attr = COLOR_ATTR(fg_color, ctx->bg_color);

	edit_buffer_put(buf,
			ctx->current_col,
			ctx->current_line, 
                        CHAR_ATTR_TO_INT(attr, ch));


	if (ctx->current_col < MAX_COL)
		ctx->current_col++;
}

void ans_read(FILE * input, struct edit_buffer * buffer)
{
	struct ans_escape_seq_ctx ctx = {
		.current_line = 0,
		.current_col  = 0,
		.saved_line   = 0,
		.saved_col    = 0,
		.bold         = DEFAULT_BOLD,
		.fg_color     = DEFAULT_FG_COLOR,
		.bg_color     = DEFAULT_BG_COLOR
	};

	for (;;) {
		int ch = fgetc(input);
		if (ans_eof(ch))
			break;

#define ESC_PREFIX 27
		if (ch == ESC_PREFIX) {
			int next = fgetc(input);
			if (next != '[')
				error("corrupt escape sequence");
			ans_parse_seq(buffer, &ctx, input);
		} else {
			ans_write_char(buffer, &ctx, ch);
		}
	}
}

/*
 *	Writing
 */

static void ans_write_attr(FILE * output, int attr)
{
	bool bold = false;
	unsigned char fg_color = attr & 0x0F; 

	if (fg_color > 7) {
		fg_color -= 8;
		bold = true;
	}

	unsigned char bg_color = (attr & 0xF0) >> 4;
		
	fprintf(output, "\x1B[%i;%i;%im",
		(bold ? 1 : 0),
		fg_color + 30,
		bg_color + 40);
}

void ans_write(FILE * output, struct edit_buffer * buf)
{
	unsigned long x, y;
	for (y = 0; y < buf->max_height; y++) {
		int prev_attr = 0;

		for (x = 0; x < buf->width; x++) {
			int attr = (edit_buffer_get(buf, x, y) & 0xFF00) >> 8;

			if (attr != prev_attr)
				ans_write_attr(output, attr);
			prev_attr = attr;

			int ch = edit_buffer_get(buf, x, y) & 0xFF;
			fputc(ch, output);
		}
		fputc('\n', output);
	}
}
