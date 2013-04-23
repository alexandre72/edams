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
device.c
mbox.c \
myfileselector.c \
path.c \
location.c \
settings.c \
sound.c \
cosm.c \
thingspeak.c \
utils.c \
voicerss.c \
widget.c

libedams_la_LIBADD = \
@EDAMS_LIBS@ \
@LTLIBINTL@ \
@EMOTION_LIBS@

libedams_la_LDFLAGS = -no-undefined -avoid-version
