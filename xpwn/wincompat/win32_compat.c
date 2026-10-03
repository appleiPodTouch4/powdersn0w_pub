/*
 * win32_compat.c - implementations for the Windows/MinGW compatibility shims
 * declared in win32_compat.h.
 *
 * Only compiled on Windows (added to libcommon by common/CMakeLists.txt).
 */

#include "win32_compat.h"

#if defined(_WIN32) && !defined(__CYGWIN__)

#include <string.h>

/*
 * memmem(): find the first occurrence of the byte sequence `needle` inside
 * `haystack`. Semantics follow the glibc/GNU extension:
 *   - a zero-length needle matches at the start of the haystack
 *   - returns NULL when there is no match
 */
void *memmem(const void *haystack, size_t haystacklen,
             const void *needle, size_t needlelen)
{
    const unsigned char *h = (const unsigned char *)haystack;
    const unsigned char *n = (const unsigned char *)needle;

    if(haystack == NULL || needle == NULL)
        return NULL;

    if(needlelen == 0)
        return (void *)haystack;

    if(needlelen > haystacklen)
        return NULL;

    /* memchr() skips ahead to the next candidate first byte. */
    while(haystacklen >= needlelen) {
        const unsigned char *p = (const unsigned char *)
            memchr(h, n[0], haystacklen - needlelen + 1);

        if(p == NULL)
            return NULL;

        if(memcmp(p, n, needlelen) == 0)
            return (void *)p;

        haystacklen -= (size_t)(p - h) + 1;
        h = p + 1;
    }

    return NULL;
}

#endif /* _WIN32 && !__CYGWIN__ */
