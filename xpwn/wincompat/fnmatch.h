/*
 * fnmatch.h - minimal Windows/MinGW compatibility shim.
 *
 * MinGW-w64 does not ship <fnmatch.h> / fnmatch(). xpwn's outputstate.c needs
 * it for wildcard removal of files, so we provide a POSIX-compatible subset.
 */

#ifndef WINCOMPAT_FNMATCH_H
#define WINCOMPAT_FNMATCH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Returned when the pattern does not match the string. */
#define FNM_NOMATCH 1

/* Flags (POSIX <fnmatch.h>). */
#define FNM_NOESCAPE    0x01 /* '\' is an ordinary character */
#define FNM_PATHNAME    0x02 /* '*' and '?' do not match '/' */
#define FNM_PERIOD      0x04 /* leading '.' must be matched explicitly */
#define FNM_LEADING_DIR 0x08 /* ignore a trailing "/..." in string */
#define FNM_CASEFOLD    0x10 /* case-insensitive (GNU extension) */

int fnmatch(const char *pattern, const char *string, int flags);

#ifdef __cplusplus
}
#endif

#endif /* WINCOMPAT_FNMATCH_H */
