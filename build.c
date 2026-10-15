#define NOB_IMPLEMENTATION
#include "3rdparty/nob/nob.h"
#include "builders/common.h"

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

    GO_REBUILD_URSELF_PLUS(argc, argv, "builders/zopfli.c");

    Cmd cmd = {0};
    //test
    if (!file_exists("builders/zopfli-builder.exe")) {
        cmd_append(&cmd, "gcc" , "-Wall", "-Wextra", "-Wpedantic", "-o", "builders/zopfli-builder.exe", "builders/zopfli.c");
        if (!cmd_run(&cmd)) return 1;
    }

    cmd.count = 0;
    cmd_append(&cmd, "builders/zopfli-builder.exe");
    if (!cmd_run(&cmd)) return 1;

    cmd_append(&cmd, "gcc" , "-Wall", "-Wextra", "-Wpedantic", "-std=c89", "-o", "bin/main", "src/main.c", NULL);

    return 0;
}