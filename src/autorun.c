#define _CRT_SECURE_NO_WARNINGS
#include "autorun.h"
#include <windows.h>
#include <stdio.h>

static const char* KEY_PATH = "Software\\Microsoft\\Command Processor";

int autorun_disable_cmd(void) {
    HKEY hKey;
    LONG rc = RegOpenKeyExA(HKEY_CURRENT_USER, KEY_PATH, 0, KEY_SET_VALUE, &hKey);
    if (rc != ERROR_SUCCESS) return 0;

    rc = RegDeleteValueA(hKey, "AutoRun");
    RegCloseKey(hKey);
    return (rc == ERROR_SUCCESS || rc == ERROR_FILE_NOT_FOUND);
}

int autorun_is_enabled_cmd(void) {
    HKEY hKey;
    LONG rc = RegOpenKeyExA(HKEY_CURRENT_USER, KEY_PATH, 0, KEY_QUERY_VALUE, &hKey);
    if (rc != ERROR_SUCCESS) return 0;

    DWORD type = 0;
    DWORD cb = 0;
    rc = RegQueryValueExA(hKey, "AutoRun", NULL, &type, NULL, &cb);
    RegCloseKey(hKey);
    return (rc == ERROR_SUCCESS && cb > 1);
}

int autorun_enable_cmd_runner(const char* exeFullPath) {
    // Run once and exit, guard against nested shells.
    // if not defined LIVEBANNER_DONE (set LIVEBANNER_DONE=1 & "C:\...\LiveBanner.exe" run)
    char value[1024];
    snprintf(value, sizeof(value),
        "if not defined LIVEBANNER_DONE (set LIVEBANNER_DONE=1 & \"%s\" run)",
        exeFullPath);

    HKEY hKey;
    LONG rc = RegCreateKeyExA(HKEY_CURRENT_USER, KEY_PATH, 0, NULL, 0,
                             KEY_SET_VALUE, NULL, &hKey, NULL);
    if (rc != ERROR_SUCCESS) return 0;

    rc = RegSetValueExA(hKey, "AutoRun", 0, REG_SZ,
                       (const BYTE*)value, (DWORD)(strlen(value) + 1));
    RegCloseKey(hKey);
    return (rc == ERROR_SUCCESS);
}
