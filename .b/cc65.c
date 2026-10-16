#define NOB_IMPLEMENTATION
#include "../3rdparty/nob/nob.h"
#include "common.h"

#define CC65_DIR _BIN "cc65/"
#define OBJECT_DIR CC65_DIR "o/"
#define TEMP_DIR CC65_DIR "t/"
#define LIB_DIR CC65_DIR "l/"
#define SRC_DIR THIRD_PARTY "cc65/src/"

#define TARGET(name) TARGET_PAIR(OBJECT_DIR, SRC_DIR, name)

#define SHARE_DIR THIRD_PARTY "cc65/"

#define CA65_INC "\"" SHARE_DIR "asminc\""
#define CC65_INC "\"" SHARE_DIR "include\""
#define CL65_TGT "\"" SHARE_DIR "target\""
#define LD65_LIB "\"" SHARE_DIR "lib\""
#define LD65_OBJ "\"" SHARE_DIR "lib\""
#define LD65_CFG "\"" SHARE_DIR "cfg\""
#define BUILD_ID "\"Git-nob\""

#define CFLAGS "-O3", "-I"SRC_DIR "common/", \
         "-Wall", "-Wextra", "-Wno-char-subscripts", \
         "-DCA65_INC="CA65_INC, "-DCC65_INC="CC65_INC, "-DCL65_TGT="CL65_TGT, \
         "-DLD65_LIB="LD65_LIB, "-DLD65_OBJ="LD65_OBJ, "-DLD65_CFG="LD65_CFG, \
         "-DBUILD_ID="BUILD_ID

static const char* tool_targets[] = {
    "ar65",
    "ca65",
    "cc65",
    "chrcvt65",
    "cl65",
    "co65",
    "da65",
    "grc65",
    "ld65",
    "od65",
    "sim65",
    "sp65",
};

int build_common(int argc, char **argv);
int build_tools(int argc, char **argv);
int build_lib(int argc, char **argv);

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);
    if (get_file_type(SRC_DIR) != FILE_DIRECTORY) {
        nob_log(ERROR, "Source directory not found: %s\n", SRC_DIR);
        return 1;
    }
    if(!mkdir_if_not_exists(CC65_DIR)) return 1;
    if(!mkdir_if_not_exists(OBJECT_DIR)) return 1;
    if(!mkdir_if_not_exists(TEMP_DIR)) return 1;
    if(!mkdir_if_not_exists(LIB_DIR)) return 1;

    if (build_common(argc, argv) != 0) return 1;
    if (build_tools(argc, argv) != 0) return 1;
    if (build_lib(argc, argv) != 0) return 1;

    return 0;
}

int build_common(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    Cmd cmd = {0};
    Procs procs = {0};
    File_Paths o = {0};
    size_t current_mark = temp_save();
    
    if(!mkdir_if_not_exists(OBJECT_DIR "common")) return 1;
    
    File_Paths s = {0};
    if (!read_entire_dir(SRC_DIR "common", &s)) return 1;
    
    for (size_t i = 0; i < s.count; ++i) {
        const char *name = s.items[i];
        const char *ext = strrchr(name, '.');
        if (!ext || strcmp(ext, ".c") != 0) continue; // Check FIRST before using ext

        const char *src_path = temp_sprintf("%s%s", SRC_DIR "common/", name);
        if (get_file_type(src_path) != FILE_REGULAR) continue;

        size_t name_len = (size_t)(ext - name);
        const char *obj_path = temp_sprintf("%scommon/%.*s.o", OBJECT_DIR, (int)name_len, name);
        
        nob_log(INFO, "Building common source: %s", name);
        da_append(&o, obj_path);

        cmd_append(&cmd, BUILD_CC, CFLAGS, "-c", src_path, "-o", obj_path);
        if (!cmd_run(&cmd, .async = &procs)) return 1;
    }
    
    if(!procs_flush(&procs)) return 1;

    const char *common_lib = temp_sprintf("%scommon.a", TEMP_DIR);
    cmd_append(&cmd, AR, "rcs", common_lib);
    for(size_t i = 0; i < o.count; ++i){ // Fixed: loop through all items, not o.count - 1
        cmd_append(&cmd, o.items[i]);
    }
    if (!cmd_run(&cmd)) return 1;

    temp_rewind(current_mark);
    return 0;
}

int build_tools(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    Cmd cmd = {0};
    Procs procs = {0};
    size_t current_mark = temp_save();

    const char *common_lib = temp_sprintf("%scommon.a", TEMP_DIR);

    for (size_t t = 0; t < ARRAY_LEN(tool_targets); ++t) {
        const char *tool = tool_targets[t];
        nob_log(INFO, "Building tool: %s", tool);

        const char *tool_obj_dir = temp_sprintf("%s%s/", OBJECT_DIR, tool);
        if (!mkdir_if_not_exists(tool_obj_dir)) return 1;

        const char *tool_src_dir = temp_sprintf("%s%s/", SRC_DIR, tool);
        File_Paths s = {0};
        if (!read_entire_dir(tool_src_dir, &s)) return 1;

        File_Paths tool_objs = {0};
        for (size_t i = 0; i < s.count; ++i) {
            const char *filename = s.items[i];
            const char *ext = strrchr(filename, '.');
            if (!ext || strcmp(ext, ".c") != 0) continue;

            const char *src_path = temp_sprintf("%s%s", tool_src_dir, filename);
            if (get_file_type(src_path) != FILE_REGULAR) continue;

            size_t name_len = (size_t)(ext - filename);
            const char *obj_path = temp_sprintf("%s%.*s.o", tool_obj_dir, (int)name_len, filename);
            da_append(&tool_objs, obj_path);

            cmd_append(&cmd, BUILD_CC, CFLAGS, "-c", src_path, "-o", obj_path);
            if (!cmd_run(&cmd, .async = &procs)) return 1;
        }

        if (!procs_flush(&procs)) return 1;

        // Link tool binary against common.a using start-group/end-group
        const char *bin_path = temp_sprintf("%s%s", CC65_DIR, tool);
        cmd_append(&cmd, BUILD_CC);
        cmd_append(&cmd, "-Wl,--start-group");
        cmd_append(&cmd, common_lib);
        for (size_t i = 0; i < tool_objs.count; ++i) {
            cmd_append(&cmd, tool_objs.items[i]);
        }
        cmd_append(&cmd, "-Wl,--end-group", "-o", bin_path, "-lm");
        if (!cmd_run(&cmd)) return 1;
    }

    temp_rewind(current_mark);
    return 0;
}

#define AR65 CC65_DIR "ar65" EXE_SUFFIX
#define CA65 CC65_DIR "ca65" EXE_SUFFIX

#define NONE_DIR THIRD_PARTY "cc65/libsrc/none/"

int build_lib(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    Cmd cmd = {0};
    Procs procs = {0};
    File_Paths o = {0};
    size_t current_mark = temp_save();

    File_Paths s = {0};
    if (!read_entire_dir(NONE_DIR, &s)) return 1;
    for (size_t i = 0; i < s.count; ++i) {
        const char *name = s.items[i];
        const char *ext = strrchr(name, '.');
        if (!ext || strcmp(ext, ".s") != 0) continue;

        const char *src_path = temp_sprintf("%s%s", NONE_DIR, name);
        if (get_file_type(src_path) != FILE_REGULAR) continue;

        size_t name_len = (size_t)(ext - name);
        const char *obj_path = temp_sprintf("%s%.*s.o", OBJECT_DIR, (int)name_len, name);
        da_append(&o, obj_path);

        cmd_append(&cmd, CA65,"-I" THIRD_PARTY "cc65/asminc", "-I" NONE_DIR, "-o", obj_path, src_path);
        if (!cmd_run(&cmd, .async = &procs)) return 1;
    }

    if(!procs_flush(&procs)) return 1;

    cmd_append(&cmd, AR65, "a", LIB_DIR "none.lib");
    for (size_t i = 0; i < o.count; ++i) {
        cmd_append(&cmd, o.items[i]);
    }
    if (!cmd_run(&cmd)) return 1;

    temp_rewind(current_mark);
    return 0;
}