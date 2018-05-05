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
#include <ctype.h>
#include <form.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "editor-context.h"
#include "edit-buffer.h"
#include "error.h"
#include "screen.h"

static void init_curses(void)
{
	savetty();

	if (initscr() == NULL)
		error("Could not initialize screen.");

	nonl();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	if (has_colors() == FALSE) {
		error("Your terminal doesn't support colors.");
	}
	start_color();
}

static void release_curses(void)
{
	resetty();
	endwin();
}

static bool char_set_forced = false;

struct screen * screen_init(bool force_ibm_cp437, unsigned long max_width)
{
	init_curses();
	init_color_pairs();

	struct screen * ret = malloc(sizeof(struct screen));
	if (!ret)
		error("Could not allocate memory for screen.");

	getmaxyx(stdscr, ret->height, ret->width);
	if (ret->height == -1 || ret->width == -1)
		error("Could not get screen dimensions from curses.");

	if (ret->width > max_width)
		ret->width = max_width;

	/* Leave a free line for the status bar.  */
	ret->height--;

	if (force_ibm_cp437) {
		char_set_forced = true;
		/* Set IBM CP437 character set.  Taken from Duh DRAW; seems
		   to work on regular Linux console.  */
		printf("\e(U");
	}
	return ret;
}

void screen_release(struct screen * screen)
{
	free(screen);
	release_curses();

	if (char_set_forced) {
		/* Set "UNIX" character set (whatever that means).  This is
		   from Duh DRAW as well.  */
		printf("\e(B");
	}
}

static void screen_set_unset_attr(int attribute, bool on)
{
	int fg_color = (attribute & 0x0F);
	int bg_color = (attribute & 0xF0) >> 4;

	if (fg_color > 7) {
		if (on)
			attron(A_BOLD);
		else
			attroff(A_BOLD);
		fg_color -= 8;
	}

	int attr = COLOR_PAIR(attr_to_color_pair(fg_color, bg_color));
	if (on)
		attron(attr);
	else
		attroff(attr);
}

void screen_draw_edit_buffer(struct screen * scr, struct edit_buffer *buf)
{
	assert(buf->start_x + scr->width <= buf->width);
	assert(buf->start_y + scr->height <= buf->height);
	assert(buf->start_x >= 0);
	assert(buf->start_y >= 0);

	int x, y;

	for (y = 0; y < scr->height; y++) {
		for (x = 0; x < scr->width; x++) {
			int attribute = (edit_buffer_get(buf,
							 buf->start_x + x,
							 buf->start_y +
							 y) & 0xFF00) >> 8;
			int character = edit_buffer_get(buf,
							buf->start_x + x,
							buf->start_y +
							y) & 0xFF;
			screen_set_unset_attr(attribute, true);
			mvprintw(y, x, "%c", character);
			screen_set_unset_attr(attribute, false);
		}
	}
}

void screen_print_status(struct edit_buffer *buf, struct screen *scr,
			 struct editor_context *ctx, char * highascii_set)
{
	/*
	 * Last line on the screen is the status bar
	 */
	move(scr->height, 0);
	clrtoeol();

#define RED_ON_BLACK COLOR_ATTR(1, 0)
	screen_set_unset_attr(RED_ON_BLACK, true);
	mvprintw(scr->height, 1, "(%2i, %2i)",
		 scr->cursor_x + buf->start_x + 1,
		 scr->cursor_y + buf->start_y + 1);
	screen_set_unset_attr(RED_ON_BLACK, false);

	move(scr->height, 13);
	screen_set_unset_attr(COLOR_ATTR(ctx->fg_color, ctx->bg_color), true);
	printw("Color");
	screen_set_unset_attr(COLOR_ATTR(ctx->fg_color, ctx->bg_color), false);

#define HIGHASCII_SET_STATUS_LEN 42
	move(scr->height, scr->width - HIGHASCII_SET_STATUS_LEN);

#define GREY_ON_BLACK COLOR_ATTR(7, 0)
	screen_set_unset_attr(GREY_ON_BLACK, true);

	int i;
	for (i = 0; i < 10; i++)
		printw(" %i=%c", i + 1, highascii_set[i]);
	screen_set_unset_attr(GREY_ON_BLACK, false);
}

void screen_move(unsigned long cursor_y, unsigned long cursor_x)
{
	move(cursor_y, cursor_x);
}

void screen_redraw(void)
{
	redrawwin(stdscr);
}

static char * trim_trailing(const char * str)
{
	unsigned long len;
	for (len = strlen(str); len > 0 && isspace(str[len - 1]); len--)
		;;
	
	char * ret = malloc(len + 1);
	strncpy(ret, str, len);
	ret[len] = 0;

	return ret;
}

char * screen_save_file_dialog(struct screen * scr)
{
	assume_default_colors(7, COLOR_BLACK);

#define SAVE_TEXT "Save to file:"
#define SAVE_TEXT_LEN strlen(SAVE_TEXT)
#define SAVE_FIELD_LEN 25

#define CENTER_START ((scr->width - SAVE_FIELD_LEN - SAVE_TEXT_LEN) / 2)

	FIELD * fields[2];

	fields[0] = new_field(1, SAVE_FIELD_LEN, scr->height / 2,
			      CENTER_START + SAVE_TEXT_LEN + 1, 0, 0);
	fields[1] = NULL;

	set_field_back(fields[0], A_UNDERLINE);

	FORM * form = new_form(fields);
	post_form(form);
	refresh();

	mvprintw(scr->height / 2, CENTER_START, SAVE_TEXT);
	mvprintw(1, 1, "Press Enter to save; double ESC to exit screen.");

#define FORM_KEY_ENTER 13
#define FORM_KEY_ESC   27
#define FORM_KEY_BACKSPACE 127

	bool quit = false;
	while (!quit) {
		int ch = getch();
		switch (ch) {
			case FORM_KEY_ENTER:
				form_driver(form, REQ_END_FIELD);
				quit = true;
				break;
			case FORM_KEY_ESC:
				/* Don't end field so we get an empty
				   filename.  */
				quit = true;
				break;
			case FORM_KEY_BACKSPACE:
				form_driver(form, REQ_DEL_PREV);
				break;
			default:
				form_driver(form, ch);
				break;
		}
	}

	char * ret = trim_trailing(field_buffer(fields[0], 0));
	if (strlen(ret) == 0) {
		free(ret);
		ret = NULL;
	}

	unpost_form(form);
	free_form(form);
	free_field(fields[0]);

	assume_default_colors(0, COLOR_BLACK);

	return ret;
}
