    #define PS_BUILD_IMP
    #include "../internal/ps.h/ps_build.h"
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
    #define BUILD_ID _ps_stringify("\"pick_sweet\"")

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

    int build_common(ps_builder_t *builder, ps_idx_t common_group);
    int build_tools(ps_builder_t *builder, ps_idx_t tool_group);
    int build_lib(ps_builder_t *builder, ps_idx_t lib_group);

    int build(ps_builder_t *builder) {
        _ps_build_rebuild_yourself(builder->args);
        if (!ps_dir_exists(SRC_DIR)) {
            ps_log(PS_LOG_ERROR, "Source directory not found: %s\n", SRC_DIR);
            return 1;
        }
        if(!ps_dir_ensure(CC65_DIR)) return 1;
        if(!ps_dir_ensure(OBJECT_DIR)) return 1;
        if(!ps_dir_ensure(TEMP_DIR)) return 1;
        if(!ps_dir_ensure(LIB_DIR)) return 1;
        ps_idx_t lib_group = _ps_build_append_group(builder);
        ps_idx_t tool_group = _ps_build_append_child_group(builder, lib_group);
        ps_idx_t common_group = _ps_build_append_child_group(builder, tool_group);
        if (build_common(builder, common_group) != 0) return 1;
        if (build_tools(builder, tool_group) != 0) return 1;
        if (build_lib(builder, lib_group) != 0) return 1;

        return 0;
    }

    int build_common(ps_builder_t *builder, ps_idx_t common_group) {
        const char ***o = (const char ***)&((ps_list_t){0});
        
        if(!ps_dir_ensure(OBJECT_DIR "common")) return 1;
        ps_path_list_t c_files = ps_dir_find_files(SRC_DIR "common/", "*.c",0);
        _ps_list_foreach(&c_files.items, f){
            const char* file = f->items;
            const char *filename = strrchr(file, PS_PATH_SEP);
            filename = filename ? filename + 1 : file;
            char **file_copy = (char **)&((ps_path_t){0});
            ps_str_append_fmt(file_copy, "%scommon/%.*s.o", OBJECT_DIR, (int)(strlen(filename) - 2), filename);
            _ps_list_append_one(o, *file_copy);
            if (ps_file_get_mod_time(file) >= ps_file_get_mod_time(*file_copy))
                _ps_build_append_child_shell_cmd(builder, common_group, PS_BUILD_CC, CFLAGS, "-c", file, "-o", *file_copy);
        }

        const char *common_lib = TEMP_DIR "common.a";
        if (ps_file_get_mod_time(common_lib) <= ps_file_get_mod_time((c_files.items)->items)) {
            const char ***ar_cmd = (const char ***)&((ps_list_t){0});
            _ps_list_append(ar_cmd, AR, "rcs", common_lib);
            _ps_list_foreach(o, obj){
                _ps_list_append_one(ar_cmd, *obj);
            }
            _ps_build_append_child_shell_cmd_list(builder,common_group, ar_cmd);
        }

        return 0;
    }

    int build_tools(ps_builder_t *builder, ps_idx_t tool_group) {
        const char *common_lib = TEMP_DIR "common.a";

        for (size_t t = 0; t < _ps_array_size(tool_targets); ++t) {
            const char *tool = tool_targets[t];

            char **tool_obj_dir = (char **)&((ps_path_t){0});
            ps_str_append_fmt(tool_obj_dir, "%s%s/", OBJECT_DIR, tool);
            
            if (!ps_dir_ensure(*tool_obj_dir)) return 1;

            char **tool_src_dir = (char **)&((ps_path_t){0});
            ps_str_append_fmt(tool_src_dir, "%s%s/", SRC_DIR, tool);
            if (!ps_dir_exists(*tool_src_dir)) return 1;
            ps_path_list_t c_files = ps_dir_find_files(*tool_src_dir, "*.c", 0);
            const char ***tool_objs = (const char ***)&((ps_list_t){0});

            _ps_list_foreach(&c_files.items, f){
                const char *file = f->items;
                
                const char *filename = strrchr(file, PS_PATH_SEP);
                filename = filename ? filename + 1 : file;

                char **file_copy = (char **)&((ps_path_t){0});
                ps_str_append_fmt(file_copy, "%s%s/%.*s.o", OBJECT_DIR, tool, (int)(strlen(filename) - 2), filename);

                _ps_list_append_one(tool_objs, *file_copy);
                
                if (ps_file_get_mod_time(file) >= ps_file_get_mod_time(*file_copy))
                    _ps_build_append_child_shell_cmd(builder, tool_group, PS_BUILD_CC, CFLAGS, "-c", file, "-o", *file_copy);
            }

            // Link tool binary against common.a using start-group/end-group
            char **bin_path = (char**)(&(ps_path_t){0});

            ps_str_append_fmt(bin_path, "%s%s%s", CC65_DIR, tool, EXE_SUFFIX);
            if (ps_file_get_mod_time(*bin_path) <= ps_file_get_mod_time((c_files.items)->items)) {
                const char ***cmd = (const char ***)&((ps_list_t){0});
                _ps_list_append(cmd, PS_BUILD_CC, "-Wl,--start-group", common_lib);
                _ps_list_foreach(tool_objs,obj){
                    _ps_list_append_one(cmd, *obj);
                }
                _ps_list_append(cmd, "-Wl,--end-group", "-o", *bin_path, "-lm");
                _ps_build_append_child_shell_cmd_list(builder, tool_group, cmd);
            }
        }
        return 0;
    }

    #define AR65 CC65_DIR "ar65" EXE_SUFFIX
    #define CA65 CC65_DIR "ca65" EXE_SUFFIX

    #define NONE_DIR THIRD_PARTY "cc65/libsrc/none/"

    int build_lib(ps_builder_t *builder, ps_idx_t lib_group) {
        if (!ps_dir_exists(NONE_DIR)) return 1;
        if (!ps_dir_ensure(OBJECT_DIR "none/")) return 1;
        ps_path_list_t s_files = ps_dir_find_files(NONE_DIR, "*.s", 0);
        const char ***o = (const char ***)&((ps_list_t){0});
        
        _ps_list_foreach(&s_files.items, f){
            const char *file = f->items;
            
            // Extract just the filename (e.g., "zeropage.s" -> "zeropage")
            const char *filename = strrchr(file, PS_PATH_SEP);
            filename = filename ? filename + 1 : file;

        char **file_copy = (char **)&((ps_path_t){0});
        // Subtract 2 to drop the ".s" extension
        ps_str_append_fmt(file_copy, "%snone/%.*s.o", OBJECT_DIR, (int)(strlen(filename) - 2), filename);

        _ps_list_append_one(o, *file_copy);
        if (ps_file_get_mod_time(file) >= ps_file_get_mod_time(*file_copy))
            _ps_build_append_child_shell_cmd(builder, lib_group, CA65, "-I" THIRD_PARTY "cc65/asminc", "-I" NONE_DIR, "-o", *file_copy, file);
    }
        if (ps_file_get_mod_time(LIB_DIR "none.lib") <= ps_file_get_mod_time((s_files.items)->items)) {
            const char ***cmd = (const char ***)&((ps_list_t){0});
            _ps_list_append(cmd, AR65, "a", LIB_DIR "none.lib");
            _ps_list_foreach(o, obj){
                _ps_list_append_one(cmd, *obj);
            }
            _ps_build_append_child_shell_cmd_list(builder, lib_group, cmd);
        }
        return 0;
    }