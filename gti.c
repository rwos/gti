/*
 * gti - a git launcher
 *
 * Copyright 2012 by Richard Wossal <richard@r-wos.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation.  No representations are made about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 */

#include "animation.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define GIT_NAME "git"

int  term_width(void);
void init_space(void);
void move_to_top(void);
void line_at(int start_x, const char *s);
void draw_car(int x);
void clear_car(int x);

int TERM_WIDTH;

int main(int argc, char **argv)
{
    int i;
    TERM_WIDTH = term_width();
    init_space();
    for (i = -20; i < TERM_WIDTH; i++) {
        draw_car(i);
        usleep(20*1000);
        clear_car(i);
    }
    if (argc > 1) {
        execvp(GIT_NAME, argv);
        /* error in exec if we land here */
        perror(GIT_NAME);
    }
    return 1;
}

int term_width(void)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

void init_space(void)
{
    size_t line;
    for (line = 0; line < ANIM_HEIGHT; line++)
        putchar('\n');
}

void move_to_top(void)
{
    printf("\033[%dA", ANIM_HEIGHT);
}

void line_at(int start_x, const char *s)
{
    int x;
    size_t i;
    if (start_x > 1)
        printf("\033[%dC", start_x);
    for (x = start_x, i = 0; i < strlen(s); x++, i++) {
        if (x > 0 && x < TERM_WIDTH)
            putchar(s[i]);
    }
    putchar('\n');
}

void draw_car(int x)
{
    size_t frame = abs(x % ANIM_FRAMES);
    size_t line;
    move_to_top();
    for (line = 0; line < ANIM_HEIGHT; line++)
        line_at(x, animation[frame][line]);
}

void clear_car(int x)
{
    size_t line;
    move_to_top();
    for (line = 0; line < ANIM_HEIGHT; line++)
        line_at(x, "  ");
}

// vim:expandtab:tabstop=4:shiftwidth=4
