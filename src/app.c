#define _CRT_SECURE_NO_WARNINGS
#include "app.h"
#include "menu.h"
#include "fs.h"
#include "autorun.h"
#include "banner.h"
#include "term.h"
#include "install.h"

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>   // fallback icin

static void open_notepad(const char* path) {
    ShellExecuteA(NULL, "open", "notepad.exe", path, NULL, SW_SHOWNORMAL);
}

static int get_exe_path(char* out, size_t cap) {
    DWORD n = GetModuleFileNameA(NULL, out, (DWORD)cap);
    return (n > 0 && n < cap) ? 1 : 0;
}

static int get_exe_dir(char* out, size_t cap) {
    char exe_path[512];
    if (!get_exe_path(exe_path, sizeof(exe_path))) return 0;

    size_t len = strlen(exe_path);
    if (len + 1 > cap) return 0;

    memcpy(out, exe_path, len + 1);
    for (int i = (int)len - 1; i >= 0; --i) {
        if (out[i] == '\\' || out[i] == '/') {
            out[i] = '\0';
            return 1;
        }
    }

    return 0;
}

// Klavye tusu veya mouse sol tik ile devam
static void wait_key_or_mouse(void) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) {
        _getch();
        return;
    }

    DWORD oldMode = 0;
    if (!GetConsoleMode(hIn, &oldMode)) {
        _getch();
        return;
    }

    DWORD mode = oldMode;
    mode |= ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    mode &= ~ENABLE_QUICK_EDIT_MODE; // drag/select kilitlemesin
    SetConsoleMode(hIn, mode);

    // Buffer'da kalmis eski eventleri bosalt (opsiyonel ama iyi)
    FlushConsoleInputBuffer(hIn);

    for (;;) {
        INPUT_RECORD ir;
        DWORD read = 0;

        if (!ReadConsoleInputA(hIn, &ir, 1, &read) || read == 0) {
            break;
        }

        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
            break;
        }

        if (ir.EventType == MOUSE_EVENT) {
            MOUSE_EVENT_RECORD* me = &ir.Event.MouseEvent;

            // Sol tik "press" (eventFlags==0 normal click)
            if ((me->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
                me->dwEventFlags == 0) {
                break;
            }
        }
    }

    SetConsoleMode(hIn, oldMode);
}

static void draw_footer_text_line(int y, int w, const char* text) {
    term_goto_xy(0, y);
    if (w <= 0) return;

    putchar('|');

    int inner = w - 2;
    if (inner < 0) inner = 0;

    int len = (text != NULL) ? (int)strlen(text) : 0;
    if (len > inner) len = inner;

    if (inner > 0) {
        if (len > 0) fwrite(text, 1, (size_t)len, stdout);
        for (int i = len; i < inner; ++i) putchar(' ');
    }

    if (w >= 2) putchar('|');
}

static void draw_footer_box(const char* line1, const char* line2) {
    int w = 0, h = 0;
    term_get_window_size(&w, &h);
    if (w <= 0 || h <= 0) return;

    const int box_h = 4;
    if (h < box_h) return;

    int y = h - box_h;

    term_goto_xy(0, y);
    if (w >= 1) putchar('+');
    for (int i = 0; i < w - 2; ++i) putchar('-');
    if (w >= 2) putchar('+');

    draw_footer_text_line(y + 1, w, line1);
    draw_footer_text_line(y + 2, w, line2);

    term_goto_xy(0, y + 3);
    if (w >= 1) putchar('+');
    for (int i = 0; i < w - 2; ++i) putchar('-');
    if (w >= 2) putchar('+');
}

static void show_footer_message(const char* line1) {
    draw_footer_box(line1, "Devam icin bir tusa bas veya mouse ile tikla...");
}

int app_run_manager(void) {
    UINT old_out_cp = GetConsoleOutputCP();
    UINT old_in_cp = GetConsoleCP();
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    fs_write_default_banner_if_missing();

    for (;;) {
        MenuAction a = menu_run();

        if (a == MENU_OPEN_BANNER) {
            char banner[512];
            fs_get_banner_path(banner, sizeof(banner));
            fs_write_default_banner_if_missing();
            open_notepad(banner);

            show_footer_message("Notepad acildi. Kaydedip kapatinca buraya don.");
            wait_key_or_mouse();
        }
        else if (a == MENU_DELETE_BANNER) {
            const char* msg = fs_delete_banner()
                ? "Banner silindi."
                : "Banner silinemedi (yok veya kilitli olabilir).";
            show_footer_message(msg);
            wait_key_or_mouse();
        }
        else if (a == MENU_ENABLE_AUTORUN) {
            const char* msg = NULL;
            char exe_path[512];
            if (!get_exe_path(exe_path, sizeof(exe_path))) {
                msg = "EXE yolu alinamadi.";
            } else if (autorun_enable_cmd_runner(exe_path)) {
                msg = "AutoRun acildi. CMD acilisinda calisacak.";
            } else {
                msg = "AutoRun acilamadi (yetki/registry sorunu).";
            }
            show_footer_message(msg);
            wait_key_or_mouse();
        }
        else if (a == MENU_DISABLE_AUTORUN) {
            const char* msg = autorun_disable_cmd()
                ? "AutoRun kapatildi. CMD acilisinda calismayacak."
                : "AutoRun kapatilamadi (yetki/registry sorunu).";
            show_footer_message(msg);
            wait_key_or_mouse();
        }
        else if (a == MENU_INSTALL) {
            const char* msg = NULL;
            char exe_path[512];
            char exe_dir[512];
            if (!get_exe_path(exe_path, sizeof(exe_path)) ||
                !get_exe_dir(exe_dir, sizeof(exe_dir))) {
                msg = "EXE yolu alinamadi.";
            } else {
                int ok_path = install_add_to_path(exe_dir);
                int ok_shortcut = install_create_start_menu_shortcut(exe_path);
                int ok_context = install_create_context_menu(exe_path);

                if (ok_path && ok_shortcut && ok_context) {
                    msg = "Sistem kurulumu tamamlandi (PATH + Baslat Menusu + Sag Tik).";
                } else if (!ok_path && !ok_shortcut && !ok_context) {
                    msg = "Kurulum basarisiz (hicbir oge eklenemedi).";
                } else if (!ok_path) {
                    msg = "PATH eklenemedi (Baslat Menusu/Sag Tik eklenmis olabilir).";
                } else if (!ok_shortcut) {
                    msg = "Baslat Menusu eklenemedi (PATH/Sag Tik eklenmis olabilir).";
                } else {
                    msg = "Sag tik eklenemedi (PATH/Baslat Menusu eklenmis olabilir).";
                }
            }

            show_footer_message(msg);
            wait_key_or_mouse();
        }
        else if (a == MENU_EXIT) {
            term_clear();
            break;
        }
    }

    SetConsoleOutputCP(old_out_cp);
    SetConsoleCP(old_in_cp);
    return 1;
}

int app_run_install(void) {
    const char* msg = NULL;
    char exe_path[512];
    char exe_dir[512];
    if (!get_exe_path(exe_path, sizeof(exe_path)) ||
        !get_exe_dir(exe_dir, sizeof(exe_dir))) {
        msg = "EXE yolu alinamadi.";
    } else {
        int ok_path = install_add_to_path(exe_dir);
        int ok_shortcut = install_create_start_menu_shortcut(exe_path);
        int ok_context = install_create_context_menu(exe_path);

        if (ok_path && ok_shortcut && ok_context) {
            msg = "Kurulum tamamlandi. Yeni bir terminal acin.";
        } else if (!ok_path && !ok_shortcut && !ok_context) {
            msg = "Kurulum basarisiz (hicbir oge eklenemedi).";
        } else if (!ok_path) {
            msg = "PATH eklenemedi (Baslat Menusu/Sag Tik eklenmis olabilir).";
        } else if (!ok_shortcut) {
            msg = "Baslat Menusu eklenemedi (PATH/Sag Tik eklenmis olabilir).";
        } else {
            msg = "Sag tik eklenemedi (PATH/Baslat Menusu eklenmis olabilir).";
        }
    }

    printf("%s\n", msg);
    return (msg && strstr(msg, "Kurulum tamamlandi") != NULL) ? 1 : 0;
}
int app_run_banner_runner(void) {
    fs_write_default_banner_if_missing();

    BannerConfig cfg = (BannerConfig){0};
    fs_get_banner_path(cfg.banner_path, sizeof(cfg.banner_path));
    cfg.fps_delay_ms = 60;

    return banner_run_once(&cfg);
}

int app_run_banner_loop(void) {
    fs_write_default_banner_if_missing();

    BannerConfig cfg = (BannerConfig){0};
    fs_get_banner_path(cfg.banner_path, sizeof(cfg.banner_path));
    cfg.fps_delay_ms = 60;

    return banner_run_loop(&cfg);
}


