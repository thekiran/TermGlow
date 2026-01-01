#include "menu.h"
#include "menu_title.h"
#include "term.h"
#include "util.h"
#include <stdio.h>
#include <string.h>   // strlen
#include <windows.h>

static int max_line_len_utf8(const char* const* lines, int n) {
    int m = 0;
    for (int i = 0; i < n; ++i) {
        int L = (int)util_utf8_strlen(lines[i]);
        if (L > m) m = L;
    }
    return m;
}

static WORD reverse_attr(WORD attr) {
    WORD fg = attr & 0x0F;
    WORD bg = attr & 0xF0;
    return (attr & 0xFF00) | (fg << 4) | (bg >> 4);
}

static void draw_item_line(HANDLE hOut, WORD base_attr, int start_x, int row, const char* item, int selected) {
    term_goto_xy(start_x, row);
    if (hOut != INVALID_HANDLE_VALUE && selected) {
        SetConsoleTextAttribute(hOut, reverse_attr(base_attr));
        printf("> %s", item);
        SetConsoleTextAttribute(hOut, base_attr);
        return;
    }
    printf("%c %s", selected ? '>' : ' ', item);
}

static void draw_border_line(int x, int y, int w, const char* left, const char* mid, const char* right) {
    if (w < 2) return;
    term_goto_xy(x, y);
    printf("%s", left);
    for (int i = 0; i < w - 2; ++i) {
        printf("%s", mid);
    }
    printf("%s", right);
}

static void draw_top_border_with_close(int x, int y, int w) {
    draw_border_line(x, y, w, "┏", "━", "┓");
    
}

static void draw_vertical_borders(int x, int y, int h, int w) {
    if (w < 2 || h < 1) return;
    for (int i = 0; i < h; ++i) {
        term_goto_xy(x, y + i);
        printf("┃");
        term_goto_xy(x + w - 1, y + i);
        printf("┃");
    }
}

MenuAction menu_run(void) {
    const char* items[] = {
        "Banner dosyasini ac (yapistir/duzenle)",
        "Banner'i sil (banner.txt)",
        "AutoRun'u etkinlestir (CMD acilisinda calissin)",
        "AutoRun'u devre disi birak",
        "Sisteme kur (PATH + Baslat Menusu)",
        "Cikis yap"
    };
    const int n = (int)(sizeof(items) / sizeof(items[0]));

    int sel = 0;

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD old_in_mode = 0;
    int has_in_mode = 0;
    if (hIn != INVALID_HANDLE_VALUE && GetConsoleMode(hIn, &old_in_mode)) {
        DWORD mode = old_in_mode;
        mode |= ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT;
        mode &= ~ENABLE_QUICK_EDIT_MODE;
        if (SetConsoleMode(hIn, mode)) {
            has_in_mode = 1;
        }
    }

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD base_attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    if (hOut != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
            base_attr = csbi.wAttributes;
        }
    }

    term_set_cursor_visible(0);

    int last_w = -1, last_h = -1;
    int box_x = 0, box_y = 0;
    int content_x = 0, content_y = 0;
    int content_w = 0;
    int box_w = 0, box_h = 0;
    int items_row = 0;
    int need_full_redraw = 1;
    int close_x = 0, close_y = 0, close_w = 0, close_h = 0;
    int has_close = 0;

    const int pad_x = 1;
    const int pad_y = 0;

    for (;;) {
        int win_w = 0, win_h = 0;
        term_get_window_size(&win_w, &win_h);
        if (win_w != last_w || win_h != last_h) {
            need_full_redraw = 1;
        }

        const char* const* title = menu_title_lines();
        int title_count = menu_title_line_count();
        const char* help = "YON TUSLARI: secim  |  ENTER: onay  |  ESC: cikis  |  MOUSE: sec";

        if (need_full_redraw) {
            // Width includes the prefix ("> " or "  ")
            int items_max = max_line_len_utf8(items, n);
            content_w = items_max + 2;
            int hp = (int)util_utf8_strlen(help);
            if (hp > content_w) content_w = hp;
            for (int i = 0; i < title_count; ++i) {
                int t = (int)util_utf8_strlen(title[i]);
                if (t > content_w) content_w = t;
            }

            // Height: title lines, separator, help, separator, items
            int content_h = title_count + 1 + 1 + 1 + n;
            box_w = content_w + (pad_x * 2) + 2;
            box_h = content_h + (pad_y * 2) + 2;

            box_x = (win_w - box_w) / 2;
            box_y = (win_h - box_h) / 2;

            if (box_x < 0) box_x = 0;
            if (box_y < 0) box_y = 0;

            content_x = box_x + 1 + pad_x;
            content_y = box_y + 1 + pad_y;

            term_clear();

            has_close = (box_w >= 6);
            if (has_close) {
                close_w = 3;
                close_h = 1;
                close_x = box_x + box_w - 5;
                close_y = box_y;
            }

            draw_top_border_with_close(box_x, box_y, box_w);
            draw_border_line(box_x, box_y + box_h - 1, box_w, "┗", "━", "┛");
            draw_vertical_borders(box_x, box_y + 1, box_h - 2, box_w);

            int row = content_y;

            for (int i = 0; i < title_count; ++i) {
                term_goto_xy(content_x, row++);
                printf("%s", title[i]);
            }

            draw_border_line(box_x, row++, box_w, "┣", "━", "┫");

            term_goto_xy(content_x, row++);
            printf("%s", help);

            draw_border_line(box_x, row++, box_w, "┣", "━", "┫");

            items_row = row;
            for (int i = 0; i < n; ++i) {
                draw_item_line(hOut, base_attr, content_x, row++, items[i], i == sel);
            }

            last_w = win_w;
            last_h = win_h;
            need_full_redraw = 0;
        }

        for (;;) {
            INPUT_RECORD ir;
            DWORD read = 0;
            if (hIn == INVALID_HANDLE_VALUE || !ReadConsoleInputA(hIn, &ir, 1, &read)) {
                break;
            }

            if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
                WORD vk = ir.Event.KeyEvent.wVirtualKeyCode;
                if (vk == VK_ESCAPE) {
                    if (has_in_mode) SetConsoleMode(hIn, old_in_mode);
                    term_set_cursor_visible(1);
                    return MENU_EXIT;
                } else if (vk == VK_RETURN) {
                    if (has_in_mode) SetConsoleMode(hIn, old_in_mode);
                    term_set_cursor_visible(1);
                    return (MenuAction)sel;
                } else if (vk == VK_UP) {
                    int old = sel;
                    sel = (sel - 1 + n) % n;
                    if (sel != old) {
                        draw_item_line(hOut, base_attr, content_x, items_row + old, items[old], 0);
                        draw_item_line(hOut, base_attr, content_x, items_row + sel, items[sel], 1);
                    }
                    continue;
                } else if (vk == VK_DOWN) {
                    int old = sel;
                    sel = (sel + 1) % n;
                    if (sel != old) {
                        draw_item_line(hOut, base_attr, content_x, items_row + old, items[old], 0);
                        draw_item_line(hOut, base_attr, content_x, items_row + sel, items[sel], 1);
                    }
                    continue;
                }
            } else if (ir.EventType == MOUSE_EVENT) {
                MOUSE_EVENT_RECORD* me = &ir.Event.MouseEvent;
                int mx = (int)me->dwMousePosition.X;
                int my = (int)me->dwMousePosition.Y;
                int in_rows = (my >= items_row && my < items_row + n);
                int in_cols = (mx >= content_x && mx < content_x + content_w);
                int in_close = has_close &&
                    mx >= close_x && mx < close_x + close_w &&
                    my >= close_y && my < close_y + close_h;

                if (me->dwEventFlags == MOUSE_WHEELED) {
                    SHORT delta = (SHORT)HIWORD(me->dwButtonState);
                    if (delta > 0) {
                        int old = sel;
                        sel = (sel - 1 + n) % n;
                        if (sel != old) {
                            draw_item_line(hOut, base_attr, content_x, items_row + old, items[old], 0);
                            draw_item_line(hOut, base_attr, content_x, items_row + sel, items[sel], 1);
                        }
                    } else if (delta < 0) {
                        int old = sel;
                        sel = (sel + 1) % n;
                        if (sel != old) {
                            draw_item_line(hOut, base_attr, content_x, items_row + old, items[old], 0);
                            draw_item_line(hOut, base_attr, content_x, items_row + sel, items[sel], 1);
                        }
                    }
                    continue;
                }

                if (in_close) {
                    if ((me->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
                        me->dwEventFlags == 0) {
                        term_clear();
                        need_full_redraw = 1;
                        break;
                    }
                }

                if (in_rows && in_cols) {
                    int new_sel = my - items_row;
                    if (new_sel != sel) {
                        int old = sel;
                        sel = new_sel;
                        draw_item_line(hOut, base_attr, content_x, items_row + old, items[old], 0);
                        draw_item_line(hOut, base_attr, content_x, items_row + sel, items[sel], 1);
                    }

                    if ((me->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
                        me->dwEventFlags == 0) {
                        if (has_in_mode) SetConsoleMode(hIn, old_in_mode);
                        term_set_cursor_visible(1);
                        return (MenuAction)sel;
                    }
                }
            } else if (ir.EventType == WINDOW_BUFFER_SIZE_EVENT) {
                need_full_redraw = 1;
                break;
            }
        }
    }
}
