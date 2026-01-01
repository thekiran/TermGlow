#pragma once

int autorun_disable_cmd(void);             // HKCU\Command Processor\AutoRun sil
int autorun_is_enabled_cmd(void);          // var mı?
int autorun_enable_cmd_runner(const char* exeFullPath); // AutoRun set (opsiyonel)
