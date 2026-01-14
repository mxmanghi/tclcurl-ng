#ifndef __tclcompat_h__
#define __tclcompat_h__

#include <tcl.h>
#include <tclDecls.h>
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

#endif
