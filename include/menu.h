#pragma once

typedef enum MenuAction {
    MENU_OPEN_BANNER = 0,
    MENU_DELETE_BANNER = 1,
    MENU_ENABLE_AUTORUN = 2,
    MENU_DISABLE_AUTORUN = 3,
    MENU_INSTALL = 4,
    MENU_EXIT = 5
} MenuAction;

MenuAction menu_run(void);  // yön tuşları + Enter
