#define PS_BUILD_IMP
#include "../internal/ps.h/ps_build.h"
#include "common.h"

#define ZOPFLI_DIR _BIN "zopfli/"
#define OBJECT_DIR ZOPFLI_DIR "o/"
#define SRC_DIR THIRD_PARTY "zopfli/src/zopfli/"

#define _target(name) _target_pair(OBJECT_DIR, SRC_DIR, name)

#define CFLAGS "-W", "-Wall", "-Wextra", "-ansi", "-pedantic", "-lm", "-O3", "-Wno-unused-function", "-fPIC", "-I" SRC_DIR
static const target_pair_t targets[] = {
    _target(blocksplitter),
    _target(cache),
    _target(deflate),
    _target(gzip_container),
    _target(hash),
    _target(katajainen),
    _target(lz77),
    _target(squeeze),
    _target(tree),
    _target(util),
    _target(zlib_container),
    _target(zopfli_lib),
    _target(zopfli_bin),
};

int build(ps_builder_t *builder) {

    _ps_build_rebuild_yourself(builder->args);
    if (!ps_dir_exists(SRC_DIR)){
        ps_log(PS_LOG_ERROR, "Source directory not found: %s\n", SRC_DIR);
        return 1;
    }

    // Cmd cmd = {0};
    // Procs procs = {0};
    const char ***o = (const char ***)&((ps_list_t){0});
    if (!ps_dir_ensure(ZOPFLI_DIR)) return 1;
    if (!ps_dir_ensure(OBJECT_DIR)) return 1;
    if (!ps_dir_ensure(_BIN "zopfli/")) return 1;

    ps_idx_t zopfli = _ps_build_append_group(builder);
    ps_idx_t zopfli_objects = _ps_build_append_child_group(builder, zopfli);
    for (size_t i = 0; i < _ps_array_size(targets); ++i){
        _ps_list_append_one(o, (targets[i].bin_path));
        if (ps_file_get_mod_time(targets[i].src_path) >=  ps_file_get_mod_time(targets[i].bin_path))
            _ps_build_append_child_shell_cmd(builder, zopfli_objects, PS_BUILD_CC, CFLAGS, "-c", targets[i].src_path, "-o", targets[i].bin_path);
    }

    // build static library
    if (ps_file_get_mod_time(_archive_path(ZOPFLI_DIR, "libzopfli")) <= ps_file_get_mod_time(targets[0].bin_path)) {
        const char ***static_cmd = (const char ***)&((ps_list_t){0});
        _ps_list_append(static_cmd, AR, "rcs", _archive_path(ZOPFLI_DIR, "libzopfli"));
        _ps_list_append_list(static_cmd, o);
        _ps_build_append_child_shell_cmd_list(builder, zopfli, static_cmd);
    }
    

    // build shared library
    
    if (ps_file_get_mod_time(_shared_path(ZOPFLI_DIR, "zopfli")) <= ps_file_get_mod_time(targets[0].bin_path)) {
        const char ***shared_cmd = (const char ***)&((ps_list_t){0});
        _ps_list_append(shared_cmd, PS_BUILD_CC);
        _ps_list_append_list(shared_cmd, o);
        _ps_list_append(shared_cmd, CFLAGS, "-shared", "-o", _shared_path(ZOPFLI_DIR, "zopfli"));
        _ps_build_append_child_shell_cmd_list(builder, zopfli, shared_cmd);
    }
    
    // build binary
    if (ps_file_get_mod_time(_exe_path(ZOPFLI_DIR, "zopfli")) <= ps_file_get_mod_time(targets[0].bin_path)) {
        const char ***bin_cmd = (const char ***)&((ps_list_t){0});
        _ps_list_append(bin_cmd, PS_BUILD_CC);
        _ps_list_append_list(bin_cmd, o);
        _ps_list_append(bin_cmd, CFLAGS, "-o", _exe_path(ZOPFLI_DIR, "zopfli"));
        _ps_build_append_child_shell_cmd_list(builder, zopfli, bin_cmd);
    }


    return 0;
}