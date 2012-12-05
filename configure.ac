###Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

m4_define([v_maj], [0])
m4_define([v_min], [5])
m4_define([v_mic], [0])
m4_define([v_ver], [v_maj.v_min.v_mic])

AC_INIT([edams],[v_ver],[alexandre.dussart@laposte.net])
AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_USE_SYSTEM_EXTENSIONS
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_MKDIR_P


AM_INIT_AUTOMAKE([1.6 dist-bzip2])
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

#Set edje_cc path.
AC_ARG_WITH([edje-cc],
   [AS_HELP_STRING([--with-edje-cc=PATH],[specify a specific path to edje_cc])],
   [edje_cc=$withval;
    AC_MSG_NOTICE([edje_cc explicitly set to $edje_cc])
   ],[edje_cc=$(pkg-config --variable=prefix edje)/bin/edje_cc])
AC_SUBST(edje_cc)

#Set elm_prefs_cc path.
AC_ARG_WITH([elm_prefs_cc],
   [AS_HELP_STRING([--with-elm-prefs-cc=PATH],[specify a specific path to elm_prefs_cc])],
   [elm_prefs_cc=$withval;
    AC_MSG_NOTICE([elm_prefs_cc explicitly set to $elm_prefs_cc])
   ],[elm_prefs_cc=$(pkg-config --variable=prefix edje)/bin/elm_prefs_cc])
AC_SUBST(elm_prefs_cc)


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
edams.spec
edams.desktop
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/modules/Makefile
src/modules/skel/Makefile
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
echo "  elm_prefs_cc.........: $elm_prefs_cc"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'su
do')"
echo "  Prefix...............: $prefix"
echo
