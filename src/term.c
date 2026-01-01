#include "term.h"
#include <windows.h>

int term_enable_vt(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return 0;

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return 0;

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(hOut, mode) ? 1 : 0;
}

int term_enable_vt_ex(DWORD* old_mode) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return 0;

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return 0;
    if (old_mode) *old_mode = mode;

    if (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) return 1;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(hOut, mode) ? 1 : 0;
}

void term_restore_out_mode(DWORD old_mode) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    SetConsoleMode(hOut, old_mode);
}

void term_clear(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;

    int width = (csbi.srWindow.Right - csbi.srWindow.Left) + 1;
    int height = (csbi.srWindow.Bottom - csbi.srWindow.Top) + 1;

    COORD topLeft;
    topLeft.X = csbi.srWindow.Left;
    topLeft.Y = csbi.srWindow.Top;

    DWORD cells = (DWORD)(width * height);
    DWORD written = 0;
    FillConsoleOutputCharacterA(hOut, ' ', cells, topLeft, &written);
    FillConsoleOutputAttribute(hOut, csbi.wAttributes, cells, topLeft, &written);
    SetConsoleCursorPosition(hOut, topLeft);
}

void term_goto_xy(int x, int y) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    COORD p;
    p.X = (SHORT)x;
    p.Y = (SHORT)y;
    SetConsoleCursorPosition(hOut, p);
}

void term_get_window_size(int* w, int* h) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;

    int width = (csbi.srWindow.Right - csbi.srWindow.Left) + 1;
    int height = (csbi.srWindow.Bottom - csbi.srWindow.Top) + 1;

    if (w) *w = width;
    if (h) *h = height;
}

int term_set_cursor_visible(int visible) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return 0;

    CONSOLE_CURSOR_INFO info;
    if (!GetConsoleCursorInfo(hOut, &info)) return 0;

    info.bVisible = visible ? TRUE : FALSE;
    return SetConsoleCursorInfo(hOut, &info) ? 1 : 0;
}
