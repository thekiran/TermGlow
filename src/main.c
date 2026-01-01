#include "app.h"
#include <string.h>

int main(int argc, char** argv) {
    // Runner: LiveBanner.exe run
    if (argc >= 2 && strcmp(argv[1], "run") == 0) {
        return app_run_banner_runner() ? 0 : 1;
    }
    if (argc >= 2 && strcmp(argv[1], "run-loop") == 0) {
        return app_run_banner_loop() ? 0 : 1;
    }

    // Manager (default)
    return app_run_manager() ? 0 : 1;
}
