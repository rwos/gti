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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define GIT_NAME "git"

#ifndef GTI_SPEED
#define GTI_SPEED 50
#endif

int  term_width(void);
void init_space(void);
void move_to_top(void);
void line_at(int start_x, const char *s);
void draw_car(int x);
void clear_car(int x);

int TERM_WIDTH;
FILE *TERM_FH;
int SLEEP_DELAY;

int main(int argc, char **argv)
{
    (void) argc;
    int i;
    TERM_FH = fopen("/dev/tty", "w");
    if (!TERM_FH)
        TERM_FH = stdout;
    TERM_WIDTH = term_width();
    SLEEP_DELAY = 1000000 / (TERM_WIDTH + GTI_SPEED);

    init_space();
    for (i = -20; i < TERM_WIDTH; i++) {
        draw_car(i);
        usleep(SLEEP_DELAY);
        clear_car(i);
    }
    move_to_top();
    fflush(TERM_FH);
    char *git_path = getenv("GIT");
    if (git_path) {
      execv(git_path, argv);
    } else {
      execvp(GIT_NAME, argv);
    }
    /* error in exec if we land here */
    perror(GIT_NAME);
    return 1;
}

int term_width(void)
{
    struct winsize w;
    ioctl(fileno(TERM_FH), TIOCGWINSZ, &w);
    return w.ws_col;
}

void init_space(void)
{
    fputs("\n\n\n\n\n\n", TERM_FH); /* 7 lines */
}

void move_to_top(void)
{
    fprintf(TERM_FH, "\033[7A");
}

void line_at(int start_x, const char *s)
{
    int x;
    size_t i;
    if (start_x > 1)
        fprintf(TERM_FH, "\033[%dC", start_x);
    for (x = start_x, i = 0; i < strlen(s); x++, i++) {
        if (x > 0 && x < TERM_WIDTH)
            fputc(s[i], TERM_FH);
    }
    fputc('\n', TERM_FH);
}

void draw_car(int x)
{
    move_to_top();
    line_at(x, "   ,---------------.");
    line_at(x, "  /  /``````|``````\\\\");
    line_at(x, " /  /_______|_______\\\\________");
    line_at(x, "|]      GTI |'       |        |]");
    if (x % 2) {
    line_at(x, "=  .-:-.    |________|  .-:-.  =");
    line_at(x, " `  -+-  --------------  -+-  '");
    line_at(x, "   '-:-'                '-:-'  ");
    } else {
    line_at(x, "=  .:-:.    |________|  .:-:.  =");
    line_at(x, " `   X   --------------   X   '");
    line_at(x, "   ':-:'                ':-:'  ");
    }
}

void clear_car(int x)
{
    move_to_top();
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
}

