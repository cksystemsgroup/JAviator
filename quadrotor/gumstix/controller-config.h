/* controller-config.h.  Generated from controller-config.h.in by configure.  */
/* controller-config.h.in.  Generated from configure.in by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* The version number of this release */
#define CONTROLLER_VERSION "0.0.1"

/* The Epoch of this release */
#define CONTROLLER_VERSION_MAJOR 0

/* The Minor version number of this release */
#define CONTROLLER_VERSION_MICRO 1

/* The Major version number of this release */
#define CONTROLLER_VERSION_MINOR 0

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Name of package */
#define PACKAGE "controller"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "controller"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "controller 0.0.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "controller"

/* Define to the home page for this package. */
/* #undef PACKAGE_URL */

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.0.1"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Target CPU arch */
#define TARGET_CPU_ARM 1

/* Target CPU arch */
/* #undef TARGET_CPU_POWERPC */

/* Target CPU arch */
/* #undef TARGET_CPU_X86 */

/* Target CPU arch */
/* #undef TARGET_CPU_X86_64 */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Version number of package */
#define VERSION "0.0.1"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif
