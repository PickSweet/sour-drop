#define NOB_IMPLEMENTATION
#include "../3rdparty/nob/nob.h"
#include "common.h"

#define ZOPFLI_DIR _BIN "zopfli/"
#define OBJECT_DIR ZOPFLI_DIR "o/"
#define SRC_DIR THIRD_PARTY "zopfli/src/zopfli/"

#define TARGET(name) TARGET_PAIR(OBJECT_DIR, SRC_DIR, name)

#define CFLAGS "-W", "-Wall", "-Wextra", "-ansi", "-pedantic", "-lm", "-O3", "-Wno-unused-function", "-fPIC", "-I" SRC_DIR
static const Target targets[] = {
    TARGET(blocksplitter),
    TARGET(cache),
    TARGET(deflate),
    TARGET(gzip_container),
    TARGET(hash),
    TARGET(katajainen),
    TARGET(lz77),
    TARGET(squeeze),
    TARGET(tree),
    TARGET(util),
    TARGET(zlib_container),
    TARGET(zopfli_lib),
    TARGET(zopfli_bin),
};

int main(int argc, char **argv) {

    GO_REBUILD_URSELF(argc, argv);

    if (get_file_type(SRC_DIR) != FILE_DIRECTORY) {
        nob_log(ERROR, "Source directory not found: %s\n", SRC_DIR);
        return 1;
    }

    Cmd cmd = {0};
    Procs procs = {0};
    File_Paths o = {0};
    if(!mkdir_if_not_exists(ZOPFLI_DIR)) return 1;
    if (!mkdir_if_not_exists(OBJECT_DIR)) return 1;
    if (!mkdir_if_not_exists(_BIN "zopfli/")) return 1;

    for (size_t i = 0; i < ARRAY_LEN(targets); ++i){
        da_append(&o, targets[i].bin_path);
        cmd_append(&cmd, BUILD_CC);
        cmd_append(&cmd, CFLAGS);
        cmd_append(&cmd, "-c", targets[i].src_path);
        cmd_append(&cmd, "-o", targets[i].bin_path);
        if (!cmd_run(&cmd,.async = &procs)) return 1;
    }

    if(!procs_flush(&procs)) return 1;
    // build static library
    cmd_append(&cmd, AR, "rcs", ARCHIVE_PATH(ZOPFLI_DIR, "libzopfli"));
    for(size_t i = 0; i < o.count - 1; ++i){
        cmd_append(&cmd, o.items[i]);
    }
    if (!cmd_run(&cmd)) return 1;
    // build shared library
    cmd_append(&cmd, BUILD_CC);
    for(size_t i = 0; i < o.count - 1; ++i){
        cmd_append(&cmd, o.items[i]);
    }
    cmd_append(&cmd, CFLAGS, "-shared", "-o", SHARED_PATH(ZOPFLI_DIR, "zopfli"));
    if (!cmd_run(&cmd)) return 1;
    
    // build binary
    cmd_append(&cmd, BUILD_CC);
    for(size_t i = 0; i < o.count; ++i){
        cmd_append(&cmd, o.items[i]);
    }
    cmd_append(&cmd, CFLAGS, "-o", EXE_PATH(ZOPFLI_DIR, "zopfli"));
    if (!cmd_run(&cmd)) return 1;


    return 0;
}