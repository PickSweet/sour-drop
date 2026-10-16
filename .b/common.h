#ifndef BUILDERS_COMMON_H
#define BUILDERS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#ifndef CC
#define BUILD_CC "gcc"
#else
#define BUILD_CC STRINGIFY(CC)
#endif

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

#define ARCHIVE_PATH(root, name) root name ARCHIVE_SUFFIX
#define EXE_PATH(root, name) root name EXE_SUFFIX
#define SHARED_PATH(root, name) root name SHARED_SUFFIX

#define TARGET_PAIR(OUT,SRC,name) {OUT #name ".o", SRC #name ".c"}

typedef struct {
    const char *bin_path;
    const char *src_path;
} Target;

#endif // BUILDERS_COMMON_H