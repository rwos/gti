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
#define WIN32
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) && !defined(__CYGWIN__)
#include <tchar.h>
#include <process.h>
#include <io.h>

// usleep() doesn't exist on MSVC, instead use Sleep() from Win32 API
#define usleep(a) Sleep((a) / 1000)

// exec*() on MSVC makes the parent process exit; that means that gti.exe will finish as git is starting,
// which causes cmd.exe to print its prompt over git's output (because it sees that the child process has
// finished). The solution is to use synchronous spawn*(): it will make gti.exe to wait until git finishes.
#define execv(a, b) return _tspawnv(_P_WAIT, (a), (b))
#define execvp(a, b) return _tspawnvp(_P_WAIT, TEXT(a), (b))

// Last but not least: Unicode. No idea how Cygwin deals with it, but for MSVC, you have to wide functions.
#define getenv(a) _tgetenv(TEXT(a))
#define perror(a) _tperror(TEXT(a))
#else
#include <unistd.h>

#define TEXT(a) a
#define TCHAR   char
#define _tmain  main
#endif

#ifndef WIN32
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif

#define GIT_NAME "git"

#ifndef GTI_SPEED
#define GTI_SPEED 50
#endif

int  term_width(void);
void init_space(void);
void open_term();
void move_to_top(void);
void line_at(int start_x, const char *s);
void draw_car(int x);
void clear_car(int x);

int TERM_WIDTH;
FILE *TERM_FH;
int SLEEP_DELAY;

int _tmain(int argc, TCHAR **argv)
{
    int i;
    TCHAR *git_path;
    (void) argc;

    open_term();
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

void init_space(void)
{
    fputs("\n\n\n\n\n\n\n", TERM_FH); /* 8 lines, to not remove the PS1 line */
}

#ifndef WIN32

void open_term()
{
    TERM_FH = fopen("/dev/tty", "w");
    if (!TERM_FH)
        TERM_FH = stdout;
}

int term_width(void)
{
    struct winsize w;
    ioctl(fileno(TERM_FH), TIOCGWINSZ, &w);
    return w.ws_col;
}

void move_to_top(void)
{
    fprintf(TERM_FH, "\033[7A");
}

void move_to_x(int x)
{
    fprintf(TERM_FH, "\033[%dC", x);
}

#else

HANDLE con;

void open_term()
{
    TERM_FH = fopen("CONOUT$", "w+");
    con = (HANDLE)_get_osfhandle(fileno(TERM_FH));

    /*
     * Both buffered and non-buffered access to the same handle is
     * recepie for disaster. Disable buffering.
     */
    setvbuf(TERM_FH, NULL, _IONBF, 0);
}

int term_width(void)
{
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(con, &ci);
    return ci.dwSize.X;
}

void move_to_top(void)
{
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(con, &ci);
    ci.dwCursorPosition.X = 0;
    ci.dwCursorPosition.Y -= 7;
    SetConsoleCursorPosition(con, ci.dwCursorPosition);
}

void move_to_x(int x)
{
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(con, &ci);
    ci.dwCursorPosition.X = x;
    SetConsoleCursorPosition(con, ci.dwCursorPosition);
}

#endif

void line_at(int start_x, const char *s)
{
    // fputc() is too slow in Windows for smooth animation. Instead, here is equivalent code that
    // writes into a buffer and then fputs() it. Proof of equivalence with the old version is
    // available at request from joker.vd@gmail.com
    int start, finish;
    char *ptr;
    static char * buff = 0;
    if (!buff) buff = malloc(2 * TERM_WIDTH);

    ptr = buff;

    if (start_x > 1) {
        memset(ptr, ' ', start_x - 1);
        ptr += start_x - 1;
    }

    start = -start_x + 1;
    if (start < 0) start = 0;

    finish = strlen(s);
    if (finish > TERM_WIDTH - start_x) finish = TERM_WIDTH - start_x;

    if (start < finish) {
        int len = finish - start;
        memcpy(ptr, s + start, len);
        ptr += len;
    }

    *ptr = '\n';
    ptr++;

    *ptr = '\0';
    fputs(buff, TERM_FH);
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

