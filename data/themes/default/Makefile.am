AUTOMAKE_OPTIONS     = 1.6 foreign
MAINTAINERCLEANFILES = Makefile.in Makefile

EDCS = edams.edc
EDJS = $(EDCS:%.edc=%.edj)
EXTRA_DIST= $(EDCS)
CLEANFILES= $(EDJS)
themedir = $(pkgdatadir)/themes/default
theme_DATA = $(EDJS)

all: $(EDJS)

edams.edj: edams.edc
SUFFIXES = .edc .edj
.edc.edj:
	$(edje_cc) -v -id '$(top_srcdir)/data/themes/default/images/' \
	-id '$(top_srcdir)/data/themes/default/images/about' \
	-id '$(top_srcdir)/data/themes/default/images/clockdigital' \
	-id '$(top_srcdir)/data/themes/default/images/clock' \
	-id '$(top_srcdir)/data/themes/default/images/clockbling' \
	-id '$(top_srcdir)/data/themes/default/images/counter' \
	-id '$(top_srcdir)/data/themes/default/images/door' \
	-id '$(top_srcdir)/data/themes/default/images/home' \
	-id '$(top_srcdir)/data/themes/default/images/jauge' \
	-id '$(top_srcdir)/data/themes/default/images/led' \
	-id '$(top_srcdir)/data/themes/default/images/lightbulb' \
	-id '$(top_srcdir)/data/themes/default/images/lock' \
	-id '$(top_srcdir)/data/themes/default/images/mail' \
	-id '$(top_srcdir)/data/themes/default/images/mailbox' \
	-id '$(top_srcdir)/data/themes/default/images/monitor' \
	-id '$(top_srcdir)/data/themes/default/images/mute' \
	-id '$(top_srcdir)/data/themes/default/images/sign' \
	-id '$(top_srcdir)/data/themes/default/images/slider' \
	-id '$(top_srcdir)/data/themes/default/images/switch' \
	-id '$(top_srcdir)/data/themes/default/images/switch2' \
	-id '$(top_srcdir)/data/themes/default/images/text' \
	-id '$(top_srcdir)/data/themes/default/images/thermometer' \
	-fd $(top_srcdir)/data/themes/default/fonts $< $(top_builddir)/data/themes/default/$(@F)
