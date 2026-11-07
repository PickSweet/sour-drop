#ifndef _H_BUILDERS_COMMON_H_
#define _H_BUILDERS_COMMON_H_


#ifndef AR
#define AR "ar"
#endif

#define _CACHE ".sour-drop/"
#define _DEP _CACHE "d/"
#define _BIN _CACHE "b/"
#define THIRD_PARTY "3rdparty/"


#define BUILDER_FOLDER ".b/"
#define BUILDER_OUT_FOLDER _BIN ".b/"


#define ARCHIVE_SUFFIX ".a"
#ifdef _WIN32
#define EXE_SUFFIX ".exe"
#define SHARED_SUFFIX ".dll"
#else
#define EXE_SUFFIX ""
#define SHARED_SUFFIX ".so"
#endif

#define _archive_path(root, name) root name ARCHIVE_SUFFIX
#define _exe_path(root, name) root name EXE_SUFFIX
#define _shared_path(root, name) root name SHARED_SUFFIX

typedef struct {
    const char *bin_path;
    const char *src_path;
} target_pair_t;

#define _target_pair(object_dir, src_dir, name) {object_dir _ps_stringify(name) ".o", src_dir _ps_stringify(name) ".c"}

#endif // _H_BUILDERS_COMMON_H_