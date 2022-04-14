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
#    define _POSIX_C_SOURCE 199506L
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
#    define GTI_SPEED 1000
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
void draw_pull(int x);
void draw_tag(int x);
draw_fn_t select_command(int argc, char **argv);

FILE *TERM_FH;
int TERM_WIDTH;
unsigned int FRAME_TIME;

int main(int argc, char **argv)
{
    int i;
    char *git_path;
    char *tmp;
    unsigned int gti_speed;
    draw_fn_t draw_fn;

    tmp = getenv("GTI_SPEED");
    if (!tmp || sscanf(tmp, "%u", &gti_speed) != 1) {
        gti_speed = GTI_SPEED;
    }
    open_term();
    TERM_WIDTH = term_width();
    FRAME_TIME = 1000 * 1000 * 10 / (gti_speed + TERM_WIDTH + 1);

    draw_fn = select_command(argc, argv);
    init_space();
    for (i = -20; i < TERM_WIDTH; i++) {
        draw_fn(i);
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
        if (!strcmp(argv[i], "pull"))
            return draw_pull;
        if (!strcmp(argv[i], "tag"))
            return draw_tag;
        break;
    }
    return draw_std;
}

void init_space(void)
{
    fputs("\n\n\n\n\n\n\n\n", TERM_FH); /* 9 lines, to not remove the PS1 line */
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
#    ifndef __MVS__
    fprintf(TERM_FH, "\033[8A");
#    else
    fprintf(TERM_FH, "\047[8A");
#    endif
#else
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(WIN_CONSOLE, &ci);
    ci.dwCursorPosition.X = 0;
    ci.dwCursorPosition.Y -= 8;
    SetConsoleCursorPosition(WIN_CONSOLE, ci.dwCursorPosition);
#endif
}

void move_to_x(int x)
{
#ifndef WIN32
#    ifndef __MVS__
    fprintf(TERM_FH, "\033[%dC", x);
#    else
    fprintf(TERM_FH, "\047[%dC", x);
#    endif
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
    line_at(x, " ");
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
    usleep(FRAME_TIME);

    clear_car(x);
}

void draw_push(int x)
{
    /* *INDENT-OFF* */
    move_to_top();
    line_at(x, " ");
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
    usleep(FRAME_TIME * 10);

    clear_car(x);
}

void draw_pull(int x)
{
    /* *INDENT-OFF* */
    move_to_top();
    line_at(x, "                                              ------.");
    line_at(x, "   ,---------------.                          |      |       ,");
    line_at(x, "  /  /``````|``````\\\\                         |      |       ||");
    line_at(x, " /  /_______|_______\\\\________            ,-------.--+-------++--,");
    if (x % 2) {
    line_at(x, "|]      GTI |'       |        |]           / .:-:.     |          |");
    line_at(x, "=  .-:-.    |________|  .-:-. = -----------     .   `-------  .-:-.");
    line_at(x, " `  -+-  --------------  -+-  '               '       `----'    +");
    line_at(x, "   '-:-'                '-:-'                '-:-'            '-:-'");
    } else {
    line_at(x, "|]      GTI |'       |        |]           / .-:-.     |          |");
    line_at(x, "=  .:-:.    |________|  .:-:. = -----------   ,     `-------  .:-:.");
    line_at(x, " `   X   --------------   X   '                 '     `----'    X");
    line_at(x, "   ':-:'                ':-:'                ':-:'            ':-:'");
    }
    /* *INDENT-ON* */
    usleep(FRAME_TIME * 8);

    clear_car(x);
}

void draw_tag(int iteration)
{
    const int car_x = 4;
    int car_y = 0;
    int keyframe = ((iteration + 20) / 4) % 3;

    /* *INDENT-OFF* */
    move_to_top();
    line_at(car_x, "     ,-------------, .     ");
    line_at(car_x, "    /     [_o_]     \\|    ");
    line_at(car_x, " []/_________________|[]   ");
    line_at(car_x, "  /__/_____________\\__\\  ");
    line_at(car_x, "d|/``\\=(_)=====(_)=/``\\|b");
    line_at(car_x, " |\\__/=============\\__/| ");
    line_at(car_x, " \\-----|__G_T_I__|-----/  ");
    if (keyframe == 1)
    line_at(car_x, "  !\\/!             !\\/!");
    else if (keyframe == 2)
    line_at(car_x, "  ;/\\;             ;/\\;");
    else
    line_at(car_x, "  ||||             ||||");
    /* *INDENT-ON* */
    usleep(FRAME_TIME * 2);

    /* clear it */
    move_to_top();
    for (car_y = 0; car_y < 8; car_y++) {
        line_at(car_x, "                           ");
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
    line_at(x, "  ");
}
