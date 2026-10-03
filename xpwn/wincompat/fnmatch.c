/*
 * fnmatch.c - minimal Windows/MinGW compatibility implementation of fnmatch().
 *
 * Supports the POSIX pattern language minus a few GNU-only extras:
 *   *        any sequence of characters
 *   ?        any single character
 *   [...]    bracket expression, including ranges and [!...] / [^...] negation
 *   \x       escaped literal (unless FNM_NOESCAPE)
 *
 * Flags honoured: FNM_NOESCAPE, FNM_PATHNAME, FNM_PERIOD, FNM_LEADING_DIR,
 * FNM_CASEFOLD. Returns 0 on match, FNM_NOMATCH otherwise.
 */

#include "fnmatch.h"

#include <stddef.h>
#include <string.h>

/* Case folding helper; only folds when FNM_CASEFOLD was requested. */
static int fn_fold(int c, int flags)
{
    if((flags & FNM_CASEFOLD) && c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    return c;
}

/* True when s begins a new path component (or the whole string). */
static int fn_comp_start(const char *s, const char *start, int flags)
{
    if(s == start)
        return 1;
    if((flags & FNM_PATHNAME) && s[-1] == '/')
        return 1;
    return 0;
}

/*
 * Match a bracket expression. *pp points just past the '['.
 * Returns 1 on match, 0 on no match, -1 if the expression is unterminated.
 * On a terminated expression *pp is advanced past the closing ']'.
 */
static int fn_bracket(const char **pp, int c, int flags)
{
    const char *p = *pp;
    int negate = 0;
    int matched = 0;
    int any = 0;

    if(*p == '!' || *p == '^') {
        negate = 1;
        p++;
    }

    for(;;) {
        int lo, hi, cf;

        if(*p == '\0')
            return -1;                  /* unterminated: caller treats '[' literally */

        if(*p == ']' && any)
            break;

        lo = (unsigned char)*p;
        if(lo == '\\' && !(flags & FNM_NOESCAPE) && p[1] != '\0') {
            p++;
            lo = (unsigned char)*p;
        }
        p++;
        hi = lo;

        /* Range such as a-z; a '-' right before ']' is a literal '-'. */
        if(*p == '-' && p[1] != '\0' && p[1] != ']') {
            p++;
            hi = (unsigned char)*p;
            if(hi == '\\' && !(flags & FNM_NOESCAPE) && p[1] != '\0') {
                p++;
                hi = (unsigned char)*p;
            }
            p++;
        }

        if(lo > hi) {
            int t = lo;
            lo = hi;
            hi = t;
        }

        cf = fn_fold((unsigned char)c, flags);
        if(cf >= fn_fold(lo, flags) && cf <= fn_fold(hi, flags))
            matched = 1;

        any = 1;
    }

    *pp = p + 1;                        /* skip the closing ']' */
    return negate ? !matched : matched;
}

int fnmatch(const char *pattern, const char *string, int flags)
{
    const char *const start = string;
    const char *star_p = NULL;          /* pattern position just after the last '*' */
    const char *star_s = NULL;          /* string position the last '*' started at */

    for(;;) {
        int c = (unsigned char)*pattern;

        if(c == '\0') {
            if(*string == '\0')
                return 0;
            if((flags & FNM_LEADING_DIR) && *string == '/')
                return 0;
            goto backtrack;
        }

        if(c == '*') {
            while(*pattern == '*')
                pattern++;

            if(*pattern == '\0') {
                /* Trailing '*': it consumes the rest of the string. */
                if((flags & FNM_PATHNAME) && strchr(string, '/') != NULL)
                    goto backtrack;
                if((flags & FNM_PERIOD) && *string == '.' &&
                   fn_comp_start(string, start, flags))
                    goto backtrack;
                return 0;
            }

            star_p = pattern;
            star_s = string;
            continue;
        }

        if(*string == '\0')
            goto backtrack;

        /* A leading period must be matched by a literal period. */
        if((flags & FNM_PERIOD) && *string == '.' &&
           fn_comp_start(string, start, flags) &&
           !(c == '.' ||
             (c == '\\' && !(flags & FNM_NOESCAPE) && pattern[1] == '.')))
            goto backtrack;

        if(c == '?') {
            if((flags & FNM_PATHNAME) && *string == '/')
                goto backtrack;
            pattern++;
            string++;
            continue;
        }

        if(c == '[') {
            const char *q = pattern + 1;
            int r = fn_bracket(&q, (unsigned char)*string, flags);

            if(r == 1) {
                pattern = q;
                string++;
                continue;
            }
            if(r == -1 && *string == '[') {
                pattern++;
                string++;
                continue;
            }
            goto backtrack;
        }

        if(c == '\\' && !(flags & FNM_NOESCAPE) && pattern[1] != '\0') {
            pattern++;
            c = (unsigned char)*pattern;
        }

        if(fn_fold(c, flags) != fn_fold((unsigned char)*string, flags))
            goto backtrack;

        pattern++;
        string++;
        continue;

backtrack:
        if(star_p == NULL)
            return FNM_NOMATCH;
        if(*star_s == '\0')
            return FNM_NOMATCH;         /* nothing left for the star to consume */
        if((flags & FNM_PATHNAME) && *star_s == '/')
            return FNM_NOMATCH;
        star_s++;
        string = star_s;
        pattern = star_p;
    }
}
