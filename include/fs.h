#pragma once
#include <stddef.h>

int  fs_get_appdata_dir(char* out, size_t cap);        // %APPDATA%\LiveBanner
int  fs_ensure_appdata_dir(void);
int  fs_get_banner_path(char* out, size_t cap);        // ...\banner.txt
int  fs_get_default_banner_path(char* out, size_t cap);// resources\banner.default.txt (dev ortamı)
int  fs_write_default_banner_if_missing(void);
int  fs_delete_banner(void);
