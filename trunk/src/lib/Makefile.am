AUTOMAKE_OPTIONS     = 1.6 foreign
MAINTAINERCLEANFILES = Makefile.in Makefile

AM_CPPFLAGS = \
-W -Wall -Wextra \
-I$(top_srcdir) \
-I$(top_srcdir)/src/bin \
-DPACKAGE_DIR=\"$(prefix)\" \
-g -DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
@EDAMS_CFLAGS@ \
@EMOTION_CFLAGS@

lib_LTLIBRARIES = libedams.la

libedams_la_SOURCES = \
action.c \
cJSON.c \
crontab.c \
gnuplot.c \
mbox.c \
myfileselector.c \
path.c \
location.c \
settings.c \
cosm.c \
utils.c \
widget.c \
xpl.c \
xpl-config.c \
xpl-io.c \
xpl-listeners.c \
xpl-message.c \
xpl-service.c \
xpl-store.c \
xpl-utils.c

libedams_la_LIBADD = \
@EDAMS_LIBS@ \
@LTLIBINTL@ \
@EMOTION_LIBS@

libedams_la_LDFLAGS = -no-undefined -avoid-version
