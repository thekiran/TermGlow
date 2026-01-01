#pragma once

typedef struct BannerConfig {
    char banner_path[512];
    int  fps_delay_ms;   // e.g. 60ms
} BannerConfig;

int  banner_run_once(const BannerConfig* cfg);
int  banner_run_loop(const BannerConfig* cfg);
