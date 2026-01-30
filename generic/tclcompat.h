#ifndef __tclcompat_h__
#define __tclcompat_h__

#include <tcl.h>
#include <tclDecls.h>
#include <limits.h>  /* for INT_MAX etc. */
#include <sys/types.h>

#ifndef CONST
#  define CONST const
#endif
#ifndef CONST84
#  define CONST84 const
#endif
#ifndef CONST86
#  define CONST86 const
#endif
#ifndef CONST84_RETURN
#  define CONST84_RETURN const
#endif

#ifndef TCL_SIZE_MAX
# define Tcl_GetSizeIntFromObj Tcl_GetIntFromObj
# define TCL_SIZE_MAX      INT_MAX
# ifndef Tcl_Size
    typedef int Tcl_Size;
# endif
# define TCL_SIZE_MODIFIER ""
#endif

/* Definition suggested in
 *
 * https://www.gnu.org/software/autoconf/manual/autoconf-2.67/html_node/Particular-Headers.html
 *
 * in order to have a portable definition of the 'bool' data type
 */

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif

#endif /* __tclcompat_h */
