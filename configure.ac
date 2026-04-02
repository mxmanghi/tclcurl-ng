#!/bin/bash -norc
dnl	This file is an input file used by the GNU "autoconf" program to
dnl	generate the file "configure", which is run during Tcl installation
dnl	to configure the system for the local environment.

#-----------------------------------------------------------------------
# Sample configure.in for Tcl Extensions.  The only places you should
# need to modify this file are marked by the string __CHANGE__
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# __CHANGE__
# Set your package name and version numbers here.
#
# This initializes the environment with PACKAGE_NAME and PACKAGE_VERSION
# set as provided.  These will also be added as -D defs in your Makefile
# so you can encode the package version directly into the source files.
#-----------------------------------------------------------------------

AC_INIT([TclCurl],[m4_esyscmd_s([cat VERSION])],[massimo.manghi@rivetweb.org],[tclcurl-ng])

#--------------------------------------------------------------------
# Call TEA_INIT as the first TEA_ macro to set up initial vars.
# This will define a ${TEA_PLATFORM} variable == "unix" or "windows"
# as well as PKG_LIB_FILE and PKG_STUB_LIB_FILE.
#--------------------------------------------------------------------

TEA_INIT([3.9])

AC_CONFIG_AUX_DIR(tclconfig)
AC_CONFIG_HEADERS([config.h])

#--------------------------------------------------------------------
# Load the tclConfig.sh file
#--------------------------------------------------------------------

TEA_PATH_TCLCONFIG
TEA_LOAD_TCLCONFIG

#--------------------------------------------------------------------
# Load the tkConfig.sh file if necessary (Tk extension)
#--------------------------------------------------------------------

#TEA_PATH_TKCONFIG
#TEA_LOAD_TKCONFIG

#-----------------------------------------------------------------------
# Handle the --prefix=... option by defaulting to what Tcl gave.
# Must be called after TEA_LOAD_TCLCONFIG and before TEA_SETUP_COMPILER.
#-----------------------------------------------------------------------

TEA_PREFIX

#-----------------------------------------------------------------------
# Standard compiler checks.
# This sets up CC by using the CC env var, or looks for gcc otherwise.
# This also calls AC_PROG_CC, AC_PROG_INSTALL and a few others to create
# the basic setup necessary to compile executables.
#-----------------------------------------------------------------------

TEA_SETUP_COMPILER

#-----------------------------------------------------------------------
# __CHANGE__
# Specify the C source files to compile in TEA_ADD_SOURCES,
# public headers that need to be installed in TEA_ADD_HEADERS,
# stub library C source files to compile in TEA_ADD_STUB_SOURCES,
# and runtime Tcl library files in TEA_ADD_TCL_SOURCES.
# This defines PKG(_STUB)_SOURCES, PKG(_STUB)_OBJECTS, PKG_HEADERS
# and PKG_TCL_SOURCES.
#-----------------------------------------------------------------------

TEA_ADD_SOURCES([tclcurl.c tcl_size.c curl_mime.c curl_setopts.c curl_getinfo.c ftp.c curl_utils.c multi.c])
TCLCURL_SCRIPTS=tclcurl.tcl
AC_SUBST(TCLCURL_SCRIPTS)

#--------------------------------------------------------------------
# Find libcurl, if it's not present, it makes no sense to compile
# this.
#--------------------------------------------------------------------

AC_ARG_WITH(curlprefix,
 [  --with-curlprefix       base directory for the cURL install '/usr', '/usr/local',...],
 [
    curlprefix=$withval
    curlpath=$curlprefix/bin
    AC_CHECK_PROG(curl,curl,yes,no,$curlpath)
    if test "x$curl" = xno ; then
       AC_MSG_ERROR([can not find cURL in $curlpath])
    fi
 ],[
    AC_CHECK_PROG(curl,curl-config,yes,no)
    if test "x$curl" = xno ; then
        AC_MSG_ERROR([can not find cURL or libcurl... go to http://curl.haxx.se/ to download and then install it first])
    else
        curlprefix=`curl-config --prefix`
        curlpath=$curlprefix/bin
    fi
])

AC_ARG_WITH(curlinclude,
 [  --with-curlinclude      directory containing the public libcurl header files],[
    TEA_ADD_INCLUDES([-I$withval])
    curlinclude=$withval
 ],
 [
    TEA_ADD_INCLUDES([-I$curlprefix/include])
     curlinclude=$curlprefix/include
 ])

AC_CHECK_HEADER(curl/curl.h,headercurl=yes, headercurl=no)
if test "x$headercurl" = xno ; then
    AC_MSG_CHECKING([checking for headers at $curlinclude])
    if test [ ! -r $curlinclude/curl/curl.h ] ; then
        AC_MSG_ERROR([cURL headers not found, you may need to install a curl-devel package])
    fi
    AC_MSG_RESULT([found])
fi

AC_ARG_WITH(libcurl,
 [  --with-libcurl          directory containing libcurl],[
    TEA_ADD_LIBS([-L$withval])
 ],
 [ 
    TEA_ADD_LIBS([-L$curlprefix/lib])
 ])

AC_MSG_CHECKING([if libcurl version is recent enough])
CURL_VERSION=`$curlpath/curl-config --checkfor 7.21.7`
if test "${CURL_VERSION}" != "" ; then
    echo ${CURL_VERSION}
    AC_MSG_ERROR([libcurl version too old, please upgrade])
fi
AC_MSG_RESULT(yes)

# We need to modify this original line to strip a few things
# See: http://lists.debian.org/debian-devel-announce/2006/09/msg00018.html

TEA_ADD_LIBS([`$curlpath/curl-config --libs|sed \
    -e 's/\-lkrb5\>//g' \
    -e 's/\-lgssapi_krb5//g' \
    -e 's/\-lk5crypto//g' \
    -e 's/\-lkrb5support//g' \
 `])


#--------------------------------------------------------------------
# __CHANGE__
# A few miscellaneous platform-specific items:
#
# Define a special symbol for Windows (BUILD_sample in this case) so
# that we create the export library with the dll.
#
# Windows creates a few extra files that need to be cleaned up.
# You can add more files to clean if your extension creates any extra
# files.
#
# TEA_ADD_* any platform specific compiler/build info here.
#--------------------------------------------------------------------

# Add pkgIndex.tcl if it is generated in the Makefile instead of ./configure
# and change Makefile.in to move it from CONFIG_CLEAN_FILES to BINARIES var.
#CLEANFILES="pkgIndex.tcl"
if test "${TEA_PLATFORM}" = "windows" ; then
    AC_DEFINE(BUILD_sample, 1, [Build windows export dll])
    CLEANFILES="$CLEANFILES *.lib *.dll *.exp *.ilk *.pdb vc*.pch"
    #TEA_ADD_SOURCES([win/winFile.c])
    #TEA_ADD_INCLUDES([-I\"$(${CYGPATH} ${srcdir}/win)\"])
else
    # Ensure no empty else clauses
    :
    #TEA_ADD_SOURCES([unix/unixFile.c])
    #TEA_ADD_LIBS([-lsuperfly])
fi
AC_SUBST(CLEANFILES)

#--------------------------------------------------------------------
# __CHANGE__
# Choose which headers you need.  Extension authors should try very
# hard to only rely on the Tcl public header files.  Internal headers
# contain private data structures and are subject to change without
# notice.
# This MUST be called after TEA_LOAD_TCLCONFIG / TEA_LOAD_TKCONFIG
#--------------------------------------------------------------------

TEA_PUBLIC_TCL_HEADERS
#TEA_PRIVATE_TCL_HEADERS

#TEA_PUBLIC_TK_HEADERS
#TEA_PRIVATE_TK_HEADERS
#TEA_PATH_X

#--------------------------------------------------------------------
# Check whether --enable-threads or --disable-threads was given.
# This auto-enables if Tcl was compiled threaded.
#--------------------------------------------------------------------

TEA_ENABLE_THREADS

#--------------------------------------------------------------------
# The statement below defines a collection of symbols related to
# building as a shared library instead of a static library.
#--------------------------------------------------------------------

TEA_ENABLE_SHARED

#--------------------------------------------------------------------
# This macro figures out what flags to use with the compiler/linker
# when building shared/static debug/optimized objects.  This information
# can be taken from the tclConfig.sh file, but this figures it all out.
#--------------------------------------------------------------------

TEA_CONFIG_CFLAGS

#--------------------------------------------------------------------
# Set the default compiler switches based on the --enable-symbols option.
#--------------------------------------------------------------------

TEA_ENABLE_SYMBOLS

#--------------------------------------------------------------------
# Everyone should be linking against the Tcl stub library.  If you
# can't for some reason, remove this definition.  If you aren't using
# stubs, you also need to modify the SHLIB_LD_LIBS setting below to
# link against the non-stubbed Tcl library.  Add Tk too if necessary.
#--------------------------------------------------------------------

AC_DEFINE(USE_TCL_STUBS, 1, [Use Tcl stubs])
#AC_DEFINE(USE_TK_STUBS, 1, [Use Tk stubs])

#--------------------------------------------------------------------
# This macro generates a line to use when building a library.  It
# depends on values set by the TEA_ENABLE_SHARED, TEA_ENABLE_SYMBOLS,
# and TEA_LOAD_TCLCONFIG macros above.
#--------------------------------------------------------------------

TEA_MAKE_LIB

#--------------------------------------------------------------------
# Determine the name of the tclsh and/or wish executables in the
# Tcl and Tk build directories or the location they were installed
# into. These paths are used to support running test cases only,
# the Makefile should not be making use of these paths to generate
# a pkgIndex.tcl file or anything else at extension build time.
#--------------------------------------------------------------------

TEA_PROG_TCLSH
#TEA_PROG_WISH

#--------------------------------------------------------------------
# Temporary support for old code that still uses deprecated options
# and mechanisms
#--------------------------------------------------------------------

dnl ----------------------------------------------------------------------
dnl CURL_ENABLE_DEPRECATED_CODE
dnl
dnl Usage (in configure.ac):
dnl   AC_CONFIG_HEADERS([config.h])
dnl   CURL_ENABLE_DEPRECATED_CODE
dnl ----------------------------------------------------------------------
AC_DEFUN([CURL_ENABLE_DEPRECATED_CODE], [
  dnl Make sure symbols have descriptions even if conditionally defined
  AH_TEMPLATE([CURL_PRE_7_56_DEPR],
    [Define to 1 to enable deprecated code for CURL versions prior to 7.56.])
  AH_TEMPLATE([CURL_PRE_7_84_DEPR],
    [Define to 1 to enable deprecated code for CURL versions prior to 7.84.])
  AH_TEMPLATE([CURL_PRE_7_11_1_DEPR],
    [Define to 1 to enable deprecated code for CURL versions prior to 7.11.1.])
  AH_TEMPLATE([CURL_PRE_8_17_0_DEPR],
    [Define to 1 to enable deprecated code for CURL versions prior to 8.17.0.])

  AC_ARG_ENABLE([deprecated-code],
    [AS_HELP_STRING(
      [--enable-deprecated-code=LIST],
      [Enable deprecated code for selected CURL versions. LIST is a comma-separated list of: v7.56,v7.84,v7.11.1,v8.17.0. Use --enable-deprecated-code (or =all) to enable all; default is none.]
    )],
    [curl_depr_list=$enableval],
    [curl_depr_list=no]
  )

  curl_pre_7_56_depr=no
  curl_pre_7_84_depr=no
  curl_pre_7_11_1_depr=no
  curl_pre_8_17_0_depr=no

  AS_CASE([$curl_depr_list],
    [no|false|none], [:],
    [yes|true|all], [
      curl_pre_7_56_depr=yes
      curl_pre_7_84_depr=yes
      curl_pre_7_11_1_depr=yes
      curl_pre_8_17_0_depr=yes
    ],
    [*], [
      dnl Remove spaces/tabs around commas etc. (portable enough for configure)
      curl_depr_list=`AS_ECHO(["$curl_depr_list"]) | tr -d '	 '`

      dnl Empty list means "enable none"
      if test "x$curl_depr_list" != x; then
        curl_depr_words=`AS_ECHO(["$curl_depr_list"]) | tr ',' ' '`
        for curl_ver in $curl_depr_words; do
          dnl Accept optional leading "v"
          curl_ver=${curl_ver#v}

          AS_CASE([$curl_ver],
            [7.56],   [curl_pre_7_56_depr=yes],
            [7.84],   [curl_pre_7_84_depr=yes],
            [7.11.1], [curl_pre_7_11_1_depr=yes],
            [8.17.0], [curl_pre_8_17_0_depr=yes],
            [*],      [AC_MSG_ERROR(
                         [Unsupported value '$curl_ver' for --enable-deprecated-code. Allowed: v7.56,v7.84,v7.11.1,v8.17.0]
                       )]
          )
        done
      fi
    ]
  )

  if test "x$curl_pre_7_56_depr" = xyes; then
    AC_DEFINE([CURL_PRE_7_56_DEPR], [1])
  fi
  if test "x$curl_pre_7_84_depr" = xyes; then
    AC_DEFINE([CURL_PRE_7_84_DEPR], [1])
  fi
  if test "x$curl_pre_7_11_1_depr" = xyes; then
    AC_DEFINE([CURL_PRE_7_11_1_DEPR], [1])
  fi
  if test "x$curl_pre_8_17_0_depr" = xyes; then
    AC_DEFINE([CURL_PRE_8_17_0_DEPR], [1])
  fi

  dnl Optional: show what was enabled
  curl_depr_enabled=
  test "x$curl_pre_7_56_depr" = xyes   && curl_depr_enabled="$curl_depr_enabled v7.56"
  test "x$curl_pre_7_84_depr" = xyes   && curl_depr_enabled="$curl_depr_enabled v7.84"
  test "x$curl_pre_7_11_1_depr" = xyes && curl_depr_enabled="$curl_depr_enabled v7.11.1"
  test "x$curl_pre_8_17_0_depr" = xyes && curl_depr_enabled="$curl_depr_enabled v8.17.0"
  test "x$curl_depr_enabled" = x && curl_depr_enabled="none"

  AC_MSG_NOTICE([deprecated curl code enabled for: $curl_depr_enabled])
])

# Tcl9 defines MODULE_SCOPE, but autotools override it, so we try
# to prevent to have warnings for preprocessor symbols being redefined

AH_BOTTOM([#ifdef MODULE_SCOPE
# undef MODULE_SCOPE
#endif
])

CURL_ENABLE_DEPRECATED_CODE

#--------------------------------------------------------------------
# Finally, substitute all of the various values into the Makefile.
# You may alternatively have a special pkgIndex.tcl.in or other files
# which require substituting th AC variables in.  Include these here.
#--------------------------------------------------------------------

AC_CONFIG_FILES([Makefile pkgIndex.tcl doc/tclcurl.n doc/tclcurl_share.n])
AC_OUTPUT
