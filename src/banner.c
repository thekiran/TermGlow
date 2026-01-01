#define _CRT_SECURE_NO_WARNINGS
#include "banner.h"
#include "term.h"
#include "util.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmdline_has_switch_c(void) {
    const char* cmdline = getenv("CMDCMDLINE");
    if (!cmdline) return 0;
    for (const char* p = cmdline; *p; ++p) {
        if (p[0] == '/' && (p[1] == 'c' || p[1] == 'C')) return 1;
    }
    return 0;
}

static int is_interactive_console(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    if (hOut == INVALID_HANDLE_VALUE || hIn == INVALID_HANDLE_VALUE) return 0;
    if (!GetConsoleMode(hOut, &mode)) return 0;
    if (!GetConsoleMode(hIn, &mode)) return 0;
    return 1;
}

static int read_lines(const char* path, char lines[][512], int maxLines) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;

    int n = 0;
    while (n < maxLines && fgets(lines[n], 512, f)) {
        size_t len = strlen(lines[n]);
        while (len && (lines[n][len - 1] == '\r' || lines[n][len - 1] == '\n')) {
            lines[n][len - 1] = '\0';
            len--;
        }
        n++;
    }
    fclose(f);
    return n;
}

static void print_lines(const char lines[][512], int count, int r, int g, int b, int use_vt) {
    if (use_vt) {
        printf("\x1b[38;2;%d;%d;%dm", r, g, b);
    }
    for (int i = 0; i < count; i++) {
        printf("%s\n", lines[i]);
    }
    if (use_vt) {
        printf("\x1b[0m");
    }
}

int banner_run_once(const BannerConfig* cfg) {
    if (!is_interactive_console()) return 1;
    if (cmdline_has_switch_c()) return 1;

    char lines[80][512];
    int count = read_lines(cfg->banner_path, lines, 80);
    if (count <= 0) {
        printf("Banner dosyasi okunamadi: %s\n", cfg->banner_path);
        return 0;
    }

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return 0;

    DWORD old_mode = 0;
    int vt_ok = term_enable_vt_ex(&old_mode);

    CONSOLE_CURSOR_INFO curInfo;
    int haveCursor = GetConsoleCursorInfo(hOut, &curInfo);
    CONSOLE_CURSOR_INFO savedCur = curInfo;

    term_clear();
    print_lines(lines, count, 255, 80, 40, vt_ok);
    fflush(stdout);

    COORD pos = {0, (SHORT)count};
    SetConsoleCursorPosition(hOut, pos);

    if (haveCursor) {
        SetConsoleCursorInfo(hOut, &savedCur);
    }
    if (vt_ok) {
        term_restore_out_mode(old_mode);
    }
    return 1;
}

int banner_run_loop(const BannerConfig* cfg) {
    if (!is_interactive_console()) return 1;

    char lines[80][512];
    int count = read_lines(cfg->banner_path, lines, 80);
    if (count <= 0) {
        printf("Banner dosyasi okunamadi: %s\n", cfg->banner_path);
        return 0;
    }

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return 0;

    DWORD old_mode = 0;
    int vt_ok = term_enable_vt_ex(&old_mode);

    int min_y = count;
    int r = 255, g = 0, b = 0;
    int dr = -2, dg = 3, db = 4;
    COORD top = {0, 0};

    for (;;) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD saved = {0, 0};
        if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
            saved = csbi.dwCursorPosition;
        }
        if (saved.Y < min_y) {
            saved.Y = (SHORT)min_y;
            saved.X = 0;
        }
        SetConsoleCursorPosition(hOut, top);

        r += dr; g += dg; b += db;
        if (r <= 0 || r >= 255) dr *= -1;
        if (g <= 0 || g >= 255) dg *= -1;
        if (b <= 0 || b >= 255) db *= -1;

        print_lines(lines, count, r, g, b, vt_ok);
        SetConsoleCursorPosition(hOut, saved);
        fflush(stdout);

        util_sleep_ms(cfg->fps_delay_ms > 0 ? cfg->fps_delay_ms : 60);
    }
}
