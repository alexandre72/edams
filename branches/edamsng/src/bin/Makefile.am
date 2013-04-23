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
cmnd_editor.c \
debug_editor.c \
edams.c \
exec_editor.c \
global_view.c \
init.c \
locations_creator.c \
mail_editor.c \
osd_editor.c \
preferences_dlg.c \
scheduler_editor.c \
shutdown.c \
widget_editor.c \
voice_editor.c

edams_LDADD = \
    $(top_builddir)/src/lib/libedams.la \
	@LTLIBINTL@ \
	@EDAMS_LIBS@ \
	@EMOTION_LIBS@
edams_LDFLAGS =
