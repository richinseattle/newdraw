/*
 * Copyright (C) 2002-2004  Pekka Enberg <penberg@iki.fi>
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
#include <curses.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ansi-esc.h"
#include "bin-file.h"
#include "colors.h"
#include "edit-buffer.h"
#include "editor-context.h"
#include "error.h"
#include "screen.h"

/* Curses-like KEY_xxx macro for combining META key with an character.  */
#define KEY_META(ch) (0x1000 | ch)

static int get_char(void)
{
	int ch = getch();

#define META_KEY_CODE 0x1B
	if (ch == META_KEY_CODE) {
		/* After META comes the actual key we're interested in.  */
		ch = KEY_META(getch());
	}
	return ch;
}

static char highascii_sets[15][11] = {
	{ 218, 191, 192, 217, 196, 179, 195, 180, 193, 194, 197 }, /* single */
	{ 201, 187, 200, 188, 205, 186, 204, 185, 202, 203, 206 }, /* double horizontal */
	{ 214, 183, 211, 189, 196, 186, 199, 182, 208, 210, 215 }, /* double vertical */
	{ 213, 184, 212, 190, 205, 179, 198, 181, 207, 209, 216 }, /* double horizontal */
	{ 176, 177, 178, 219, 220, 223, 221, 222,  22, 254,  32 }, /* solid blocks */
	{ 197, 206, 215, 216,   1,   2,   3,   4,   5,   6,  32 },
	{  16,  17,  18,  19,  21,  23,  25,  29,  30,  31,  32 },
	{  28, 168, 127, 128, 129, 130, 131, 132, 133, 134,  32 },
	{ 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,  32 },
	{ 145, 146, 147, 148, 149, 150, 151, 152, 153, 154,  32 },
	{ 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,  32 },
	{ 165, 166, 167, 169, 170, 171, 172, 173, 174, 175,  32 },
	{ 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,  32 },
	{ 234, 235, 236, 237, 238, 239, 240, 241, 242, 243,  32 },
	{ 244, 245, 246, 247, 248, 249, 250, 251, 252, 253,  32 }
};

enum {
	INITIAL_HIGHASCII_SET = 4
};

static unsigned long selected_set = INITIAL_HIGHASCII_SET;

static int get_printable_char(int ch)
{
	switch (ch) {
		case KEY_F(1):
			return highascii_sets[selected_set][0];
		case KEY_F(2):
			return highascii_sets[selected_set][1];
		case KEY_F(3):
			return highascii_sets[selected_set][2];
		case KEY_F(4):
			return highascii_sets[selected_set][3];
		case KEY_F(5):
			return highascii_sets[selected_set][4];
		case KEY_F(6):
			return highascii_sets[selected_set][5];
		case KEY_F(7):
			return highascii_sets[selected_set][6];
		case KEY_F(8):
			return highascii_sets[selected_set][7];
		case KEY_F(9):
			return highascii_sets[selected_set][8];
		case KEY_F(10):
			return highascii_sets[selected_set][9];
	}
	return ch;
}

/*
 *	Commands issued in the editor
 */

static void cmd_resize(void)
{
	/* Currently we don't support resizing the terminal while
	   running.  */
	error("Terminal was resized.");
}

static void cmd_move_up(struct edit_buffer *buf, struct screen *scr)
{
	if (scr->cursor_y > 0)
		scr->cursor_y--;
	else if (buf->start_y > 0)
		buf->start_y--;
}

static void cmd_move_down(struct edit_buffer *buf, struct screen *scr)
{
	if (scr->cursor_y < (scr->height - 1))
		scr->cursor_y++;
	else if (buf->start_y + scr->height < buf->height)
		buf->start_y++;
}

static void cmd_move_left(struct edit_buffer *buf, struct screen *scr)
{
	if (scr->cursor_x > 0)
		scr->cursor_x--;
	else if (buf->start_x > 0)
		buf->start_x--;
}

static void cmd_move_right(struct edit_buffer *buf, struct screen *scr)
{
	if (scr->cursor_x < (scr->width - 1))
		scr->cursor_x++;
	else if (buf->start_x < (buf->width - scr->width))
		buf->start_x++;
}

static void cmd_move_to_end(struct edit_buffer *buf, struct screen *scr)
{
	scr->cursor_x = scr->width - 1;
	buf->start_x  = buf->width - scr->width;
}

static void cmd_move_to_start(struct edit_buffer *buf, struct screen *scr)
{
	scr->cursor_x = 0;
	buf->start_x  = 0;
}

static void cmd_print_char(struct edit_buffer *buf, struct screen *scr,
		    struct editor_context *ctx, int ch)
{
	unsigned long attr = COLOR_ATTR(ctx->fg_color, ctx->bg_color);

	edit_buffer_put(buf,
			buf->start_x + scr->cursor_x,
			buf->start_y + scr->cursor_y,
			CHAR_ATTR_TO_INT(attr, get_printable_char(ch)));
}

void cmd_move_page_up(struct edit_buffer *buf, struct screen *scr)
{
	if (buf->start_y > scr->height) {
		buf->start_y -= scr->height;
	} else {
		buf->start_y  = 0;
		scr->cursor_y = 0;
	}
	screen_redraw();
}

void cmd_move_page_down(struct edit_buffer *buf, struct screen *scr)
{
	if (buf->start_y + (scr->height * 2) < buf->height) {
		buf->start_y += scr->height;
	} else {
		buf->start_y  = buf->height - scr->height;
		scr->cursor_y = scr->height - 1;
	}
	screen_redraw();
}

static unsigned long dec_wrap(unsigned long val, unsigned long max)
{
	return (val > 0) ? (val - 1) : max;
}

static unsigned long inc_wrap(unsigned long val, unsigned long max)
{
	return (val < max) ? (val + 1) : 0;
}

enum {
	MAX_FG_COLOR = 15
};

static void cmd_next_fg_color(struct editor_context *ctx)
{
	ctx->fg_color = inc_wrap(ctx->fg_color, MAX_FG_COLOR);
}

static void cmd_prev_fg_color(struct editor_context *ctx)
{
	ctx->fg_color = dec_wrap(ctx->fg_color, MAX_FG_COLOR);
}

enum {
	MAX_BG_COLOR = 7
};

static void cmd_next_bg_color(struct editor_context *ctx)
{
	ctx->bg_color = inc_wrap(ctx->bg_color, MAX_BG_COLOR);
}

static void cmd_prev_bg_color(struct editor_context *ctx)
{
	ctx->bg_color = dec_wrap(ctx->bg_color, MAX_BG_COLOR);
}

static bool cmd_select_highascii_set(int ch)
{
#define CASE_SELECT_HIGHASCII_SET(idx) \
	case KEY_META(KEY_F(idx)): \
		selected_set = idx - 1; \
		break; \

	switch (ch) {
		CASE_SELECT_HIGHASCII_SET(1)
		CASE_SELECT_HIGHASCII_SET(2)
		CASE_SELECT_HIGHASCII_SET(3)
		CASE_SELECT_HIGHASCII_SET(4)
		CASE_SELECT_HIGHASCII_SET(5)
		CASE_SELECT_HIGHASCII_SET(6)
		CASE_SELECT_HIGHASCII_SET(7)
		CASE_SELECT_HIGHASCII_SET(8)
		CASE_SELECT_HIGHASCII_SET(9)
		CASE_SELECT_HIGHASCII_SET(10)
		default:
			return false;
	}
	return true;
}

static bool cmd_move_cursor(int ch, struct edit_buffer * buf,
			    struct screen * scr)
{
#define CASE_MOVE_CURSOR(key, cmd) \
	case key: \
		cmd(buf, scr); \
		break;

	switch (ch) {
		CASE_MOVE_CURSOR(KEY_UP,    cmd_move_up)
		CASE_MOVE_CURSOR(KEY_DOWN,  cmd_move_down)
		CASE_MOVE_CURSOR(KEY_LEFT,  cmd_move_left)
		CASE_MOVE_CURSOR(KEY_RIGHT, cmd_move_right)
		CASE_MOVE_CURSOR(KEY_END,   cmd_move_to_end)
		CASE_MOVE_CURSOR(KEY_HOME,  cmd_move_to_start)
		CASE_MOVE_CURSOR(KEY_PPAGE, cmd_move_page_up)
		CASE_MOVE_CURSOR(KEY_NPAGE, cmd_move_page_down)
		default:
			return false;
	}
	return true;
}

static bool cmd_change_color(int ch, struct editor_context * ctx)
{
#define CASE_CHANGE_COLOR(key, cmd) \
	case key: \
		cmd(ctx); \
		break;

	switch (ch) {
		CASE_CHANGE_COLOR(KEY_META(KEY_UP),    cmd_next_bg_color)
		CASE_CHANGE_COLOR(KEY_META(KEY_DOWN),  cmd_prev_bg_color)
		CASE_CHANGE_COLOR(KEY_META(KEY_RIGHT), cmd_next_fg_color)
		CASE_CHANGE_COLOR(KEY_META(KEY_LEFT),  cmd_prev_fg_color)
		default:
			return false;
	}
	return true;
}

static void cmd_save_file(struct screen * scr, struct edit_buffer * buf)
{
	char * filepath = screen_save_file_dialog(scr);

	if (filepath) {
		char *filename = strrchr(filepath, '/');
		if(!filename) { filename = filepath; }
		
		char *p = filename;
		while((p = strchr(p, '\\'))) { *p = '-'; }

		char * save_path = "./art";
		char *output_path = calloc(1, strlen(filename) + strlen(save_path) + 2);
		sprintf(output_path, "%s/%s", save_path, filename);
		printf("\n%s\n", output_path);

		FILE *output = fopen(output_path, "w");
		if (!output)
			error("Could not open '%s' for writing.", filename);

		ans_write(output, buf);

		fclose(output);
		free(output_path);
		free(filepath);
	}
}

/*
 *	Main editor loop
 */

static void edit_loop(struct edit_buffer *buf, struct screen *scr)
{
	struct editor_context ctx = {
		.fg_color = 0x07,
		.bg_color = 0x00
	};

	bool quit = false;

	while (!quit) {
		screen_draw_edit_buffer(scr, buf);
		screen_print_status(buf, scr, &ctx,
				    highascii_sets[selected_set]);
		screen_move(scr->cursor_y, scr->cursor_x);

		int ch = get_char();
		if (ch == ERR)
			error("getch() returned ERR");

		if (cmd_select_highascii_set(ch)
		    || cmd_move_cursor(ch, buf, scr)
		    || cmd_change_color(ch, &ctx))
			continue;

		switch (ch) {
			case KEY_META('x'):
			case KEY_META('X'):
				quit = true;
				break;
			case KEY_META('s'):
			case KEY_META('S'):
				cmd_save_file(scr, buf);
				break;
			case KEY_RESIZE:
				cmd_resize();
				break;
			case KEY_BACKSPACE:
				cmd_move_left(buf, scr);
				cmd_print_char(buf, scr, &ctx, ' ');
				break;
			default:
				cmd_print_char(buf, scr, &ctx, ch);
				cmd_move_right(buf, scr);
		}
	}
}

static void usage(char * argv[])
{
	printf("usage: %s [-h -f -c <columns> -r <rows>] [filename]\n", argv[0]);
}

int main(int argc, char *argv[])
{
	unsigned long edit_buffer_cols = 80;
	unsigned long edit_buffer_rows = 1000;
	bool force_ibm_cp437 = false;

	for (;;) {
		int arg_index = getopt(argc, argv, "hfc:r:");
		if (arg_index == -1) {
			break;
		}
		switch (arg_index) {
			case 'h':
				usage(argv);
				return EXIT_SUCCESS;
			case 'f':
				force_ibm_cp437 = true;
				break;
			case 'c':
				edit_buffer_cols = strtol(optarg, NULL, 10);
				break;
			case 'r':
				edit_buffer_rows = strtol(optarg, NULL, 10);
				break;
		default:
			usage(argv);
			return EXIT_FAILURE;
		}
	}

	struct edit_buffer *buf =
		edit_buffer_create(edit_buffer_cols, edit_buffer_rows);
	edit_buffer_clear(buf);

	if (argv[optind] != NULL) {
		FILE *input = fopen(argv[optind], "r");
		if (input) {
			if (bin_file_check(argv[optind]))
				bin_file_read(input, buf, buf->width);
			else
				ans_read(input, buf);
			fclose(input);
		}
	}
	struct screen *scr = screen_init(force_ibm_cp437, edit_buffer_cols);

	edit_loop(buf, scr);

	edit_buffer_release(buf);
	screen_release(scr);

	return EXIT_SUCCESS;
}
