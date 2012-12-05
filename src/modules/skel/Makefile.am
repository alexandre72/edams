AUTOMAKE_OPTIONS     = 1.6 foreign
MAINTAINERCLEANFILES = Makefile.in Makefile

AM_CPPFLAGS = \
-W -Wall -Wextra \
-I$(top_srcdir) \
-I$(top_builddir) \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
@EDAMS_CFLAGS@

pkgdir = $(libdir)/edams/modules/$(MODULE_ARCH)
pkg_LTLIBRARIES = skel.la

skel_la_SOURCES = module.c module.h

skel_la_LIBADD = $(top_srcdir)/src/lib/libedams.la  @edams_LIBS@ @EMOTION_LIBS@ @LTLIBINTL@
skel_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -module -avoid-version
skel_la_LIBTOOLFLAGS = --tag=disable-static

themedir = $(pkgdatadir)/themes/default
theme_DATA = skel.edj

EXTRA_DIST = module.edc

%.edj:  module.edc
	$(edje_cc) -v -id ./images  $< $@
