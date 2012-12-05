AUTOMAKE_OPTIONS     = 1.6 foreign
MAINTAINERCLEANFILES = Makefile.in Makefile

EDCS = edams.edc
EDJS = $(EDCS:%.edc=%.edj)
EPCS = edams.epc
EPBS = edams.epb
EXTRA_DIST= $(EDCS) $(EPCS)
CLEANFILES= $(EDJS) $(EPBS)
themedir = $(pkgdatadir)/themes/default
theme_DATA = $(EDJS) $(EPBS)

all: $(EDJS) $(EBPS)

edams.edj: edams.edc 
SUFFIXES = .edc .edj
.edc.edj:
	$(edje_cc) -v -id $(top_srcdir)/data/themes/default/images -fd $(top_srcdir)/data/themes/default/fonts $< $(top_builddir)/data/themes/default/$(@F)

edams.epb: edams.epc 
SUFFIXES = .epc .epb 
.epc.epb:
	$(elm_prefs_cc) $< $@
