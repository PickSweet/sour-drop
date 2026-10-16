#define NOB_IMPLEMENTATION
#include "3rdparty/nob/nob.h"
#include ".b/common.h"

#define SRC_FOLDER "src/"

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} SourceList;

int main(int argc, char **argv) {
    // ensure the cache directories exist
    if (!mkdir_if_not_exists(_CACHE)) return 1;
    if (!mkdir_if_not_exists(_DEP)) return 1;
    if (!mkdir_if_not_exists(_BIN)) return 1;
    if (!mkdir_if_not_exists(BUILDER_OUT_FOLDER)) return 1;

    GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};
    if (argv[1] && strcmp(argv[1], "tools") == 0){ // temporary hack to build tools only, for testing
        if (!file_exists(BUILDER_FOLDER "zopfli-builder.exe")) {
            cmd_append(&cmd, BUILD_CC ,"-DCC="BUILD_CC, "-Wall", "-Wextra", "-Wpedantic", "-o", EXE_PATH(BUILDER_OUT_FOLDER,"zopfli-builder"), BUILDER_FOLDER "zopfli.c");
            if (!cmd_run(&cmd)) return 1;
        }
        if (!file_exists(BUILDER_FOLDER "cc65-builder.exe")) {
            cmd_append(&cmd, BUILD_CC ,"-DCC="BUILD_CC, "-Wall", "-Wextra", "-Wpedantic", "-o", EXE_PATH(BUILDER_OUT_FOLDER,"cc65-builder"), BUILDER_FOLDER "cc65.c");
            if (!cmd_run(&cmd)) return 1;
        }
        cmd_append(&cmd, EXE_PATH(BUILDER_OUT_FOLDER, "zopfli-builder"));
        if (!cmd_run(&cmd)) return 1;
        cmd_append(&cmd, EXE_PATH(BUILDER_OUT_FOLDER, "cc65-builder"));
        if (!cmd_run(&cmd)) return 1;
    }
    // cmd_append(&cmd, BUILD_CC , "-Wall", "-Wextra", "-Wpedantic", "-std=c89", "-o", "bin/main", "src/main.c", NULL);
    // if (!cmd_run(&cmd)) return 1;

    return 0;
}