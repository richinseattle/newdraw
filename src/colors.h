/*
 * Copyright (C) 2002  Pekka Enberg <Pekka.Enberg@cs.helsinki.fi>
 *
 * Distributed under the terms of the GNU General Public License
 * version 2 or later.
 */
#ifndef _COLORS_H_
#define _COLORS_H_ 1

#define COLOR_ATTR(fg, bg) (bg * 0x10 + fg)

int attr_to_color_pair (int fg_color, int bg_color);
void init_color_pairs ();

#endif
