#ifndef _ANSI_ESC_H_
#define _ANSI_ESC_H_ 1

#include <stdio.h>
struct edit_buffer;

void ans_read(FILE * input, struct edit_buffer * buffer);
void ans_write(FILE * output, struct edit_buffer * buffer);

#endif
