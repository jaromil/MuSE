/* include/cdk_config.h.  Generated automatically by configure.  */
/* 
 * $Id$
 */

#ifndef CDK_CONFIG_H
#define CDK_CONFIG_H 1


#define HAVE_CURSES_H 1
#define HAVE_DIRENT_H 1
#define HAVE_GETBEGX 1
#define HAVE_GETBEGY 1
#define HAVE_GETCWD 1
#define HAVE_GETMAXX 1
#define HAVE_GETMAXY 1
#define HAVE_GETOPT_H 1
#define HAVE_GETOPT_HEADER 1
#define HAVE_LIMITS_H 1
#define HAVE_LSTAT 1
#define HAVE_MKTIME 1
#define HAVE_MVWIN 1
#define HAVE_START_COLOR 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_TYPE_CHTYPE 1
#define HAVE_UNISTD_H 1
#define RETSIGTYPE void
#define STDC_HEADERS 1
#define SYSTEM_NAME "linux-gnu"
#define TYPE_CHTYPE_IS_SCALAR 1

#if !defined(HAVE_LSTAT) && !defined(lstat)
#define lstat(f,b) stat(f,b)
#endif

#endif /* CDK_CONFIG_H */
