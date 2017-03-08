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

#if defined (_WIN32) && !defined(WIN32)
#    define WIN32
#else
     /* fileno() */
#    define _POSIX_C_SOURCE
     /* usleep() */
#    define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) && !defined(__CYGWIN__)
#    include <process.h>
#    include <io.h>

    /* usleep() doesn't exist on MSVC, instead use Sleep() from Win32 API */
#    define usleep(a) Sleep((a) / 1000)

    /*
     * exec*() on MSVC makes the parent process exit; that means that gti.exe will finish as git is starting,
     * which causes cmd.exe to print its prompt over git's output (because it sees that the child process has
     * finished). The solution is to use synchronous spawn*(): it will make gti.exe to wait until git finishes.
     */
#    define execv(a, b) do { i = _spawnv(_P_WAIT, (a), (b)); if (i != -1) return i; } while(0)
#    define execvp(a, b) do { i = _spawnvp(_P_WAIT, (a), (b)); if (i != -1) return i; } while(0)

#else
#    include <unistd.h>
#endif

#ifndef WIN32
#    include <sys/ioctl.h>
#else
#    include <windows.h>
HANDLE WIN_CONSOLE;
#endif

/* SunOS defines winsize in termios.h */
#if defined(__sun) && defined(__SVR4)
#    include <sys/termios.h>
#endif

#define GIT_NAME "git"

#ifndef GTI_SPEED
#    define GTI_SPEED 50
#endif

int term_width(void);
void init_space(void);
void open_term();
void move_to_top(void);
void line_at(int start_x, const char *s);
void clear_car(int x);

typedef void (*draw_fn_t)(int x);
void draw_std(int x);
void draw_push(int x);
draw_fn_t select_command(int argc, char **argv);

int TERM_WIDTH;
FILE *TERM_FH;
int SLEEP_DELAY;

int main(int argc, char **argv)
{
    int i;
    char *git_path;
    draw_fn_t draw_fn;

    draw_fn = select_command(argc, argv);

    open_term();
    TERM_WIDTH = term_width();

    init_space();
    for (i = -20; i < TERM_WIDTH; i++) {
        draw_fn(i);
        clear_car(i);
    }
    move_to_top();
    fflush(TERM_FH);

    git_path = getenv("GIT");
    if (git_path) {
        execv(git_path, argv);
    } else {
        execvp(GIT_NAME, argv);
    }
    /* error in exec if we land here */
    perror(GIT_NAME);
    return 1;
}

draw_fn_t select_command(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-')
            continue;
        if (!strcmp(argv[i], "push"))
            return draw_push;
        break;
    }
    return draw_std;
}

void init_space(void)
{
    fputs("\n\n\n\n\n\n\n", TERM_FH);   /* 8 lines, to not remove the PS1 line */
    fflush(TERM_FH);
}

void open_term()
{
#ifndef WIN32
    TERM_FH = fopen("/dev/tty", "w");
    if (!TERM_FH)
        TERM_FH = stdout;
#else
    TERM_FH = fopen("CONOUT$", "w+");
    WIN_CONSOLE = (HANDLE)_get_osfhandle(fileno(TERM_FH));
#endif
}

int term_width(void)
{
#ifndef WIN32
    struct winsize w;
    ioctl(fileno(TERM_FH), TIOCGWINSZ, &w);
    return w.ws_col;
#else
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(WIN_CONSOLE, &ci);
    return ci.dwSize.X;
#endif
}

void move_to_top(void)
{
#ifndef WIN32
    fprintf(TERM_FH, "\033[7A");
#else
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(WIN_CONSOLE, &ci);
    ci.dwCursorPosition.X = 0;
    ci.dwCursorPosition.Y -= 7;
    SetConsoleCursorPosition(WIN_CONSOLE, ci.dwCursorPosition);
#endif
}

void move_to_x(int x)
{
#ifndef WIN32
    fprintf(TERM_FH, "\033[%dC", x);
#else
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(WIN_CONSOLE, &ci);
    ci.dwCursorPosition.X = x;
    SetConsoleCursorPosition(WIN_CONSOLE, ci.dwCursorPosition);
#endif
}

void line_at(int start_x, const char *s)
{
    int x;
    size_t i;
    if (start_x > 1)
        move_to_x(start_x);
    for (x = start_x, i = 0; i < strlen(s); x++, i++) {
        if (x > 0 && x < TERM_WIDTH)
            fputc(s[i], TERM_FH);
    }
#ifdef WIN32
    /*
     * It seems Windows wraps on whe cursor when it's about to overflow,
     * rather than after it has overflown (unless the overflowing character
     * is a newline), as other systems seems to do.
     */
    if (x < TERM_WIDTH)
#endif
        fputc('\n', TERM_FH);

    fflush(TERM_FH);
}

void draw_std(int x)
{
    /* *INDENT-OFF* */
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
    /* *INDENT-ON* */
    usleep(1000000 / (TERM_WIDTH + GTI_SPEED));
}

void draw_push(int x)
{
    /* *INDENT-OFF* */
    move_to_top();
    line_at(x, "   __      ,---------------.");
    line_at(x, "  /--\\   /  /``````|``````\\\\");
    line_at(x, "  \\__/  /  /_______|_______\\\\________");
    line_at(x, "   ||-< |]      GTI |'       |        |]");
    if (x % 2) {
    line_at(x, "   ||-< =  .-:-.    |________|  .-:-.  =");
    line_at(x, "   ||    `  -+-  --------------  -+-  '");
    line_at(x, "   ||      '-:-'                '-:-'  ");
    } else {
    line_at(x, "   ||-< =  .:-:.    |________|  .:-:.  =");
    line_at(x, "   /\\    `   X   --------------   X   '");
    line_at(x, "  /  \\     ':-:'                ':-:'  ");
    }
    /* *INDENT-ON* */
    usleep(20000000 / (TERM_WIDTH + GTI_SPEED));
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
