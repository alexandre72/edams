###Process this file with autoconf to produce a configure script.


define([v_rev], esyscmd([sh -c "svnversion|tr -d '\n'"]))dnl

m4_define([v_maj], [0])
m4_define([v_min], [9])
m4_define([v_mic], [0])
m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])

AC_INIT([edams],[v_ver],[alexandre.dussart@laposte.net])
AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_USE_SYSTEM_EXTENSIONS
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_MKDIR_P
AC_GNU_SOURCE

AM_INIT_AUTOMAKE([1.11 dist-bzip2])
AC_CONFIG_HEADERS(config.h)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])
define([AC_LIBTOOL_LANG_GCJ_CONFIG], [:])
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])
LT_INIT

AC_C_BIGENDIAN
AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST


# pkg-config
PKG_PROG_PKG_CONFIG

#Must-to-have libraries checks.
requirements="\
   eina >= 1.7 \
   eet >= 1.7 \
   evas >= 1.7 \
   ecore >= 1.7 \
   edje >= 1.7 \
   elementary >= 1.7 \
   "
PKG_CHECK_MODULES([EDAMS], [${requirements}])

PKG_CHECK_MODULES([EMOTION], [emotion >= 1.0.99 ], [have_emotion="yes"],[have_emotion="no"])
have_emotion="no"
AM_CONDITIONAL([HAVE_EMOTION], [test "x${have_emotion}" = "xyes"])
#AM_COND_IF([HAVE_EMOTION], [AC_DEFINE(HAVE_EMOTION, 1, [Set to 1 if emotion package is installed.])])


#Set edje_cc path.
AC_ARG_WITH([edje-cc],
   [AS_HELP_STRING([--with-edje-cc=PATH],[specify a specific path to edje_cc])],
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

AC_CONFIG_FILES([
Makefile
edams.desktop
src/Makefile
src/bin/Makefile
src/lib/Makefile
data/Makefile
data/themes/Makefile
data/themes/default/Makefile
])
AC_OUTPUT

ifdef([AM_GNU_GETTEXT], [AC_CONFIG_FILES([po/Makefile.in])])

AC_OUTPUT
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
if test "${have_emotion}" = "yes"; then
echo "  Sound Support........: Yes(Emotion backend)"
fi
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo "  edje_cc..............: $edje_cc"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'su
do')"
echo "  Prefix...............: $prefix"
echo
