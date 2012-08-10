/*
    Little Smalltalk, version two
    Written by Tim Budd, Oregon State University, July 1987

    environmental factors

    This include file gathers together environmental factors that
    are likely to change from one C compiler to another, or from
    one system to another.  Please refer to the installation 
    notes for more information.

    for systems using the Make utility, the system name is set
    by the make script.
    other systems (such as the Mac) should put a define statement
    at the beginning of the file, as shown below
*/

#ifndef ENV_H_INCLUDED
#define ENV_H_INCLUDED

#include <limits.h>

typedef unsigned char byte;

# define byteToInt(b) (b)

/* this is a bit sloppy - but it works */
# define longCanBeInt(l) ((l >= -16383) && (l <= 16383))

/* ======== various defines that should work on all systems ==== */

# define streq(a,b) (strcmp(a,b) == 0)

/* prototypes are another problem.  If they are available, they should be
used; but if they are not available their use will cause compiler errors.
To get around this we define a lot of symbols which become nothing if
prototypes aren't available */

# define X ,
# define OBJ object
# define OBJP object *
# define INT int
# define BOOL bool
# define STR char *
# define CSTR const char *
# define FLOAT double
# define NOARGS void
# define FILEP FILE *
# define FUNC ()


#if defined(__APPLE__)
#define SO_EXT  "so"
#define MAX_PATH  PATH_MAX
#elif defined(__CYGWIN__) || defined(__WIN32__)
#define SO_EXT  "dll"
#define MAX_PATH  PATH_MAX
#endif

#endif

void sysWarn(const char* s1, const char* s2);
void compilWarn(const char* selector, const char* str1, const char* str2);
void compilError(const char* selector, const char* str1, const char* str2);
