/*
 * win32_compat.h - Windows/MinGW compatibility shims for xpwn.
 *
 * This header is force-included into every translation unit when building with
 * MinGW (see the top-level CMakeLists.txt), so that the upstream sources keep
 * compiling unmodified on a toolchain that does not implement the GNU/glibc
 * extensions they assume.
 */

#ifndef WINCOMPAT_WIN32_COMPAT_H
#define WINCOMPAT_WIN32_COMPAT_H

#if defined(_WIN32) && !defined(__CYGWIN__)

#include <stddef.h>

/*
 * include/common.h shadows a few CRT names with macros (mkdir, fseeko, ftello,
 * off_t). Because this header is force-included ahead of everything else, we
 * pull the affected system headers in here first so that the CRT's own
 * declarations are parsed before those macros exist -- otherwise e.g.
 * <unistd.h> -> <io.h>'s `int mkdir(const char *)` gets rewritten into a
 * two-argument macro call and fails to compile.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <io.h>

/*
 * mingw-w64 has no memmem(). Declared here; implemented in win32_compat.c
 * (which is compiled into libcommon). Used by common/patchfinder.c and
 * kernel/kernel.c.
 */
#ifndef WINCOMPAT_HAVE_MEMMEM
#define WINCOMPAT_HAVE_MEMMEM
void *memmem(const void *haystack, size_t haystacklen,
             const void *needle, size_t needlelen);
#endif

#endif /* _WIN32 && !__CYGWIN__ */

#endif /* WINCOMPAT_WIN32_COMPAT_H */
