/*
 * Copyright (C) 2002  Pekka Enberg <Pekka.Enberg@cs.helsinki.fi>
 *
 * Distributed under the terms of the GNU General Public License
 * version 2 or later.
 */

#include <assert.h>
#include <curses.h>
#include "error.h"

/*
 * Edit buffer attribute -> curses color pair lookup table.
 */
static char attr_to_color_pairs[8][8] = {
	{ 0,  1,  2,  3,  4,  5,  6,  7},
	{ 8,  9, 10, 11, 12, 13, 14, 15},
	{16, 17, 18, 19, 20, 21, 22, 23},
	{24, 25, 26, 27, 28, 29, 30, 31},
	{32, 33, 34, 35, 36, 37, 38, 39},
	{40, 41, 42, 43, 44, 45, 46, 47},
	{48, 49, 50, 51, 52, 53, 54, 55},
	{56, 57, 58, 59, 60, 61, 62, 63}
};

int attr_to_color_pair(int fg_color, int bg_color)
{
	/* Sanity checks.  */
	assert(fg_color >= 0);
	assert(fg_color <= 7);
	assert(bg_color >= 0);
	assert(bg_color <= 7);
	return attr_to_color_pairs[bg_color][fg_color];
}

/* All color pairs (foreground/background combinations) are
   initialized here.  */
void init_color_pairs()
{
	if (COLOR_PAIRS < 64) {
		error("Cannot initialize color pairs. Your system "
		      "supports only %i whereas 64 is required.",
		      COLOR_PAIRS);
	}

	assume_default_colors(0, COLOR_BLACK);

	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);
	init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(6, COLOR_CYAN, COLOR_BLACK);
	init_pair(7, COLOR_WHITE, COLOR_BLACK);

	init_pair(8, COLOR_BLACK, COLOR_RED);
	init_pair(9, COLOR_RED, COLOR_RED);
	init_pair(10, COLOR_GREEN, COLOR_RED);
	init_pair(11, COLOR_YELLOW, COLOR_RED);
	init_pair(12, COLOR_BLUE, COLOR_RED);
	init_pair(13, COLOR_MAGENTA, COLOR_RED);
	init_pair(14, COLOR_CYAN, COLOR_RED);
	init_pair(15, COLOR_WHITE, COLOR_RED);

	init_pair(16, COLOR_BLACK, COLOR_GREEN);
	init_pair(17, COLOR_RED, COLOR_GREEN);
	init_pair(18, COLOR_GREEN, COLOR_GREEN);
	init_pair(19, COLOR_YELLOW, COLOR_GREEN);
	init_pair(20, COLOR_BLUE, COLOR_GREEN);
	init_pair(21, COLOR_MAGENTA, COLOR_GREEN);
	init_pair(22, COLOR_CYAN, COLOR_GREEN);
	init_pair(23, COLOR_WHITE, COLOR_GREEN);

	init_pair(24, COLOR_BLACK, COLOR_YELLOW);
	init_pair(25, COLOR_RED, COLOR_YELLOW);
	init_pair(26, COLOR_GREEN, COLOR_YELLOW);
	init_pair(27, COLOR_YELLOW, COLOR_YELLOW);
	init_pair(28, COLOR_BLUE, COLOR_YELLOW);
	init_pair(29, COLOR_MAGENTA, COLOR_YELLOW);
	init_pair(30, COLOR_CYAN, COLOR_YELLOW);
	init_pair(31, COLOR_WHITE, COLOR_YELLOW);

	init_pair(32, COLOR_BLACK, COLOR_BLUE);
	init_pair(33, COLOR_RED, COLOR_BLUE);
	init_pair(34, COLOR_GREEN, COLOR_BLUE);
	init_pair(35, COLOR_YELLOW, COLOR_BLUE);
	init_pair(36, COLOR_BLUE, COLOR_BLUE);
	init_pair(37, COLOR_MAGENTA, COLOR_BLUE);
	init_pair(38, COLOR_CYAN, COLOR_BLUE);
	init_pair(39, COLOR_WHITE, COLOR_BLUE);

	init_pair(40, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(41, COLOR_RED, COLOR_MAGENTA);
	init_pair(42, COLOR_GREEN, COLOR_MAGENTA);
	init_pair(43, COLOR_YELLOW, COLOR_MAGENTA);
	init_pair(44, COLOR_BLUE, COLOR_MAGENTA);
	init_pair(45, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(46, COLOR_CYAN, COLOR_MAGENTA);
	init_pair(47, COLOR_WHITE, COLOR_MAGENTA);

	init_pair(48, COLOR_BLACK, COLOR_CYAN);
	init_pair(49, COLOR_RED, COLOR_CYAN);
	init_pair(50, COLOR_GREEN, COLOR_CYAN);
	init_pair(51, COLOR_YELLOW, COLOR_CYAN);
	init_pair(52, COLOR_BLUE, COLOR_CYAN);
	init_pair(53, COLOR_MAGENTA, COLOR_CYAN);
	init_pair(54, COLOR_CYAN, COLOR_CYAN);
	init_pair(55, COLOR_WHITE, COLOR_CYAN);

	init_pair(56, COLOR_BLACK, COLOR_WHITE);
	init_pair(57, COLOR_RED, COLOR_WHITE);
	init_pair(58, COLOR_GREEN, COLOR_WHITE);
	init_pair(59, COLOR_YELLOW, COLOR_WHITE);
	init_pair(60, COLOR_BLUE, COLOR_WHITE);
	init_pair(61, COLOR_MAGENTA, COLOR_WHITE);
	init_pair(62, COLOR_CYAN, COLOR_WHITE);
	init_pair(63, COLOR_WHITE, COLOR_WHITE);
}
