#define PS_BUILD_IMP
#include "internal/ps.h/ps_build.h"
#include ".b/common.h"

#define SRC_FOLDER "src/"
#define CFLAGS "-Wall", "-Wextra", "-Wpedantic"

int build(ps_builder_t *builder) {
    // ensure the cache directories exist
    _ps_build_rebuild_yourself(builder->args);
    if (!ps_dir_ensure(_CACHE)) return 1;
    if (!ps_dir_ensure(_DEP)) return 1;
    if (!ps_dir_ensure(_BIN)) return 1;
    if (!ps_dir_ensure(BUILDER_OUT_FOLDER)) return 1;

    if (!ps_file_exists(BUILDER_OUT_FOLDER "zopfli-builder.exe")) {
        ps_idx_t run_zopfli_builder = _ps_build_append_shell_cmd(builder, _exe_path(BUILDER_OUT_FOLDER, "zopfli-builder"));
        _ps_build_append_child_shell_cmd(builder, run_zopfli_builder, PS_BUILD_CC, CFLAGS, "-o", _exe_path(BUILDER_OUT_FOLDER,"zopfli-builder"), BUILDER_FOLDER "zopfli.c");
    } else {
        _ps_build_append_shell_cmd(builder, _exe_path(BUILDER_OUT_FOLDER, "zopfli-builder"));
        ps_log(PS_LOG_INFO, "zopfli-builder already exists\n");
    }
    if (!ps_file_exists(BUILDER_OUT_FOLDER "cc65-builder.exe")) {
        ps_idx_t run_cc65_builder = _ps_build_append_shell_cmd(builder, _exe_path(BUILDER_OUT_FOLDER, "cc65-builder"));
        _ps_build_append_child_shell_cmd(builder, run_cc65_builder, PS_BUILD_CC, CFLAGS, "-o", _exe_path(BUILDER_OUT_FOLDER,"cc65-builder"), BUILDER_FOLDER "cc65.c");
    } else {
        _ps_build_append_shell_cmd(builder, _exe_path(BUILDER_OUT_FOLDER, "cc65-builder"));
        ps_log(PS_LOG_INFO, "cc65-builder already exists\n");
    }
    return 0;
}