#ifndef BUILDERS_COMMON_H
#define BUILDERS_COMMON_H

#define _CACHE ".sour-drop/"
#define _DEP _CACHE "d/"
#define _BIN _CACHE "b/"
#define THIRD_PARTY "3rdparty/"

#define TARGET_PAIR(OUT,SRC,name) {OUT #name ".o", SRC #name ".c"}

typedef struct {
    const char *bin_path;
    const char *src_path;
} Target;

#endif // BUILDERS_COMMON_H