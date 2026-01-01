#pragma once
#include <windows.h>

int term_enable_vt(void);
int term_enable_vt_ex(DWORD* old_mode);
void term_restore_out_mode(DWORD old_mode);
void term_clear(void);
void term_goto_xy(int x, int y);
void term_get_window_size(int* w, int* h);
int term_set_cursor_visible(int visible);
