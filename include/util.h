#pragma once
#include <stddef.h>

void util_sleep_ms(int ms);
void util_join_path(char* out, size_t cap, const char* a, const char* b);
int  util_file_exists(const char* path);
size_t util_utf8_strlen(const char* s);
