#define _CRT_SECURE_NO_WARNINGS
#include "fs.h"
#include "util.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

static int get_env(const char* name, char* out, size_t cap) {
    DWORD n = GetEnvironmentVariableA(name, out, (DWORD)cap);
    return (n > 0 && n < cap) ? 1 : 0;
}

int fs_get_appdata_dir(char* out, size_t cap) {
    char appdata[512];
    if (!get_env("APPDATA", appdata, sizeof(appdata))) return 0;
    if (cap == 0) return 0;
    size_t needed = strlen(appdata) + 1 /* \ */ + strlen("LiveBanner") + 1 /* NUL */;
    if (needed > cap) return 0;
    snprintf(out, cap, "%s\\LiveBanner", appdata);
    return 1;
}

int fs_ensure_appdata_dir(void) {
    char dir[512];
    if (!fs_get_appdata_dir(dir, sizeof(dir))) return 0;
    CreateDirectoryA(dir, NULL); // exists -> ok
    return 1;
}

int fs_get_banner_path(char* out, size_t cap) {
    char dir[512];
    if (!fs_get_appdata_dir(dir, sizeof(dir))) return 0;
    util_join_path(out, cap, dir, "banner.txt");
    return 1;
}

int fs_get_default_banner_path(char* out, size_t cap) {
    // Dev ortamında: LiveBanner\resources\banner.default.txt
    // Çalışma dizininden relative alıyoruz.
    snprintf(out, cap, "resources\\banner.default.txt");
    return 1;
}

static int copy_file_bytes(const char* src, const char* dst) {
    FILE* in = fopen(src, "rb");
    if (!in) return 0;
    FILE* out = fopen(dst, "wb");
    if (!out) { fclose(in); return 0; }

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { fclose(in); fclose(out); return 0; }
    }

    fclose(in);
    fclose(out);
    return 1;
}

int fs_write_default_banner_if_missing(void) {
    if (!fs_ensure_appdata_dir()) return 0;

    char banner[512];
    if (!fs_get_banner_path(banner, sizeof(banner))) return 0;

    if (util_file_exists(banner)) return 1;

    char def[512];
    fs_get_default_banner_path(def, sizeof(def));

    // resources dosyası yoksa, minimum fallback yaz
    if (!util_file_exists(def)) {
        FILE* f = fopen(banner, "wb");
        if (!f) return 0;
        const char* fallback =
            "LiveBanner\n"
            "Paste your ASCII banner here.\n";
        fwrite(fallback, 1, (unsigned)strlen(fallback), f);
        fclose(f);
        return 1;
    }

    return copy_file_bytes(def, banner);
}

int fs_delete_banner(void) {
    char banner[512];
    if (!fs_get_banner_path(banner, sizeof(banner))) return 0;
    return DeleteFileA(banner) ? 1 : 0;
}
