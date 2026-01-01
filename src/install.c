#include "install.h"
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

static int path_contains_dir(const char* path, const char* dir) {
    if (!path || !dir) return 0;

    const char* p = path;
    size_t dir_len = strlen(dir);
    while (*p) {
        const char* start = p;
        const char* end = strchr(start, ';');
        size_t len = end ? (size_t)(end - start) : strlen(start);

        if (len == dir_len && _strnicmp(start, dir, len) == 0) {
            return 1;
        }

        p = end ? (end + 1) : (start + len);
    }

    return 0;
}

int install_add_to_path(const char* dir) {
    HKEY hKey = NULL;
    LONG rc = RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0,
                            KEY_QUERY_VALUE | KEY_SET_VALUE, &hKey);
    if (rc != ERROR_SUCCESS) return 0;

    DWORD type = REG_EXPAND_SZ;
    DWORD cb = 0;
    rc = RegQueryValueExA(hKey, "Path", NULL, &type, NULL, &cb);
    if (rc == ERROR_FILE_NOT_FOUND) {
        rc = RegSetValueExA(hKey, "Path", 0, REG_EXPAND_SZ,
                            (const BYTE*)dir, (DWORD)(strlen(dir) + 1));
        RegCloseKey(hKey);
        if (rc != ERROR_SUCCESS) return 0;
        SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                            (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
        return 1;
    }
    if (rc != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 0;
    }

    char* buf = (char*)malloc(cb + 1);
    if (!buf) {
        RegCloseKey(hKey);
        return 0;
    }

    rc = RegQueryValueExA(hKey, "Path", NULL, &type, (BYTE*)buf, &cb);
    if (rc != ERROR_SUCCESS) {
        free(buf);
        RegCloseKey(hKey);
        return 0;
    }
    buf[cb] = '\0';

    if (path_contains_dir(buf, dir)) {
        free(buf);
        RegCloseKey(hKey);
        return 1;
    }

    size_t cur_len = strlen(buf);
    size_t dir_len = strlen(dir);
    size_t need_sep = (cur_len > 0 && buf[cur_len - 1] != ';') ? 1 : 0;
    size_t new_len = cur_len + need_sep + dir_len + 1;

    char* out = (char*)malloc(new_len);
    if (!out) {
        free(buf);
        RegCloseKey(hKey);
        return 0;
    }

    memcpy(out, buf, cur_len);
    if (need_sep) out[cur_len++] = ';';
    memcpy(out + cur_len, dir, dir_len);
    out[cur_len + dir_len] = '\0';

    free(buf);

    rc = RegSetValueExA(hKey, "Path", 0, type, (const BYTE*)out,
                        (DWORD)(strlen(out) + 1));
    free(out);
    RegCloseKey(hKey);
    if (rc != ERROR_SUCCESS) return 0;

    SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                        (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
    return 1;
}

int install_create_start_menu_shortcut(const char* exe_path) {
    char programs[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, programs) != S_OK) {
        return 0;
    }

    char link_path[MAX_PATH];
    snprintf(link_path, sizeof(link_path), "%s\\LiveBanner.lnk", programs);

    char exe_dir[MAX_PATH];
    strncpy(exe_dir, exe_path, sizeof(exe_dir) - 1);
    exe_dir[sizeof(exe_dir) - 1] = '\0';
    for (int i = (int)strlen(exe_dir) - 1; i >= 0; --i) {
        if (exe_dir[i] == '\\' || exe_dir[i] == '/') {
            exe_dir[i] = '\0';
            break;
        }
    }

    HRESULT hr = CoInitialize(NULL);
    int need_uninit = SUCCEEDED(hr);

    IShellLinkA* psl = NULL;
    hr = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IShellLinkA, (void**)&psl);
    if (FAILED(hr) || !psl) {
        if (need_uninit) CoUninitialize();
        return 0;
    }

    psl->lpVtbl->SetPath(psl, exe_path);
    psl->lpVtbl->SetWorkingDirectory(psl, exe_dir);
    psl->lpVtbl->SetDescription(psl, "LiveBanner");

    IPersistFile* ppf = NULL;
    hr = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void**)&ppf);
    if (SUCCEEDED(hr) && ppf) {
        wchar_t wpath[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, link_path, -1, wpath, MAX_PATH);
        hr = ppf->lpVtbl->Save(ppf, wpath, TRUE);
        ppf->lpVtbl->Release(ppf);
    }

    psl->lpVtbl->Release(psl);
    if (need_uninit) CoUninitialize();

    return SUCCEEDED(hr) ? 1 : 0;
}
