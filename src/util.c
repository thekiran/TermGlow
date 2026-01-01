#define _CRT_SECURE_NO_WARNINGS
#include "util.h"
#include <windows.h>
#include <stdio.h>

void util_sleep_ms(int ms) { Sleep((DWORD)ms); }

void util_join_path(char* out, size_t cap, const char* a, const char* b) {
    snprintf(out, cap, "%s\\%s", a, b);
}

int util_file_exists(const char* path) {
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

size_t util_utf8_strlen(const char* s) {
    size_t count = 0;
    const unsigned char* p = (const unsigned char*)s;
    while (*p) {
        if ((*p & 0xC0u) != 0x80u) {
            count++;
        }
        p++;
    }
    return count;
}
