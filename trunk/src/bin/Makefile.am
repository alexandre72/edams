AUTOMAKE_OPTIONS     = 1.6 foreign
MAINTAINERCLEANFILES = Makefile.in Makefile

AM_CPPFLAGS = \
-W -Wall -Wextra \
-I$(top_srcdir) \
-I$(top_builddir) \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EDAMS_CFLAGS@ \
@EMOTION_CFLAGS@

bin_PROGRAMS = edams

edamsdir = $(libdir)

edams_SOURCES = \
about_dlg.c \
actions_editor.c \
edams.c \
init.c \
map.c \
devices_picker.c \
preferences_dlg.c \
shutdown.c \
widgets_picker.c

edams_LDADD = \
    $(top_builddir)/src/lib/libedams.la \
	@LTLIBINTL@ \
	@EDAMS_LIBS@ \
	@EMOTION_LIBS@
edams_LDFLAGS =
