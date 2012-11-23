###Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

m4_define([v_maj], [0])
m4_define([v_min], [0])
m4_define([v_mic], [2])
m4_define([v_ver], [v_maj.v_min.v_mic])

AC_INIT([edams], [v_ver], [alexandre.dussart@laposte.net])
AC_PREREQ([2.60])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_GNU_SOURCE
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_ISC_POSIX
AC_PROG_MKDIR_P


AM_INIT_AUTOMAKE([1.6 dist-bzip2])
AC_CONFIG_HEADERS(config.h)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])
define([AC_LIBTOOL_LANG_GCJ_CONFIG], [:])
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])
AC_PROG_LIBTOOL

AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST

case "$host_os" in
  mingw32ce* | cegcc*)
    MODULE_ARCH="$host_os-$host_cpu"
    MODULE_EDJE="$host_os-$host_cpu"
    ;;
  *)
    MODULE_ARCH="$host_os-$host_cpu-v_maj.v_min.v_mic"
    ;;
esac
AC_SUBST(release_info)
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")


# pkg-config
PKG_PROG_PKG_CONFIG

#Must-to-have libraries checks.
requirements="\
   eina >= 1.2.99 \
   eet >= 1.6.99 \
   evas >= 1.2.99 \
   ecore >= 1.2.99 \
   edje >= 1.2.99 \
   elementary >= 1.0.99 \
   "
PKG_CHECK_MODULES([EDAMS], [${requirements}])

PKG_CHECK_MODULES([EMOTION],
        [emotion >= 1.0.99 ],
        [have_emotion="yes"],
        [have_emotion="no"])
AM_CONDITIONAL([HAVE_EMOTION], [test "x${have_emotion}" = "xyes"])


#Check if cross-compilation.
lt_enable_auto_import=""
ELM_UNIX_DEF="#undef"
ELM_WIN32_DEF="#undef"
have_windows="no"
have_dlopen="no"
case "$host_os" in
   mingw*)
      PKG_CHECK_MODULES([EVIL], [evil])
      AC_DEFINE(HAVE_EVIL, 1, [Set to 1 if evil package is installed.])
      lt_enable_auto_import="-Wl,--enable-auto-import"
      ELM_WIN32_DEF="#define"
      have_windows="yes"
dnl managed by evil
      AC_DEFINE(HAVE_DLADDR)
      have_dlopen="yes"
      dlopen_libs=-ldl
      requirement_elm="evil"
      have_socket="no"
      ;;
   *)
      ELM_UNIX_DEF="#define"
      AC_CHECK_FUNCS(dlopen, res=yes, res=no)
      if test "x$res" = "xyes"; then
        AC_CHECK_FUNCS(dladdr, AC_DEFINE(HAVE_DLADDR))
        have_dlopen="yes"
      else
        AC_CHECK_LIB(dl, dlopen, res=yes, res=no)
        if test "x$res" = "xyes"; then
          AC_CHECK_LIB(dl, dladdr, AC_DEFINE(HAVE_DLADDR))
          dlopen_libs=-ldl
          have_dlopen="yes"
        fi
      fi
      have_socket="yes"
      ;;
esac
AC_SUBST(dlopen_libs)
AC_SUBST(lt_enable_auto_import)
AC_SUBST(ELM_UNIX_DEF)
AC_SUBST(ELM_WIN32_DEF)
AM_CONDITIONAL([ELEMENTARY_WINDOWS_BUILD], [test "x${have_windows}" = "xyes"])

have_elementary_win32="no"
want_elementary_win32="auto"
AC_ARG_ENABLE([ecore-win32],
   [AC_HELP_STRING([--disable-ecore-win32], [disable ecore-win32 support. @<:@default=detect@:>@])],
   [want_elementary_win32=$enableval], [])

if test "x$want_elementary_win32" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_WIN32],
      [ecore-win32 >= 1.2.0],
      [
       AC_DEFINE(HAVE_ELEMENTARY_WIN32, 1, [Windows XP support for Elementary])
       have_elementary_win32="yes"
       requirement_elm="ecore-win32 >= 1.2.0 ${requirement_elm}"
      ],
      [have_elementary_win32="no"]
   )
else
    have_elementary_win32="no"
fi
if test "x$want_elementary_win32" = "xyes" -a "x$have_elementary_win32" = "xno"; then
    AC_MSG_ERROR([ecore-win32 support requested, but not found by pkg-config.])
fi


have_elementary_x="no"
want_elementary_x="auto"
AC_ARG_ENABLE([ecore-x],
   [AC_HELP_STRING([--disable-ecore-x], [disable ecore-x support. @<:@default=detect@:>@])],
   [want_elementary_x=$enableval], [])

if test "x$want_elementary_x" != "xno"; then
   PKG_CHECK_MODULES([ELEMENTARY_X],
      [ecore-x >= 1.2.0],
      [
       AC_DEFINE(HAVE_ELEMENTARY_X, 1, [X11 support for Elementary])
       have_elementary_x="yes"
       requirement_elm="ecore-x >= 1.2.0 ${requirement_elm}"
      ],
      [have_elementary_x="no"]
   )
else
    have_elementary_x="no"
fi
if test "x$want_elementary_x" = "xyes" -a "x$have_elementary_x" = "xno"; then
    AC_MSG_ERROR([ecore-x support requested, but not found by pkg-config.])
fi


#Set edje_cc path.
AC_ARG_WITH([edje-cc],
   [AC_HELP_STRING([--with-edje-cc=PATH],
                   [specify a specific path to edje_cc])],
   [edje_cc=$withval;
    AC_MSG_NOTICE([edje_cc explicitly set to $edje_cc])
   ],[edje_cc=$(pkg-config --variable=prefix edje)/bin/edje_cc])
AC_SUBST(edje_cc)


m4_ifdef([AM_GNU_GETTEXT_VERSION], [
AM_GNU_GETTEXT_VERSION([0.17])
])

m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
have_po="yes"
],[
have_po="no"
])
AC_SUBST(LTLIBINTL)

AC_OUTPUT([
Makefile
edams.spec
edams.desktop
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/modules/Makefile
src/modules/skel/Makefile
data/Makefile
data/icons/Makefile
data/themes/Makefile
data/themes/default/Makefile
])


ifdef([AM_GNU_GETTEXT], [AC_CONFIG_FILES([po/Makefile.in])])


AC_OUTPUT
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "Engines"
echo "  X11..................: ${have_elementary_x}"
echo "  Windows XP...........: ${have_elementary_win32}"
echo
if test "${have_emotion}" = "yes"; then
echo "  Sound Support........: Emotion backend"
fi
if test "${have_windows}" = "yes"; then
echo
echo "Windows build"
echo "  Elementary Win32.....: ${have_elementary_win32}"
fi
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'su
do')"
echo "  Prefix...............: $prefix"
echo
