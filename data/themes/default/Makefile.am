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
	$(edje_cc) -v -id $(top_srcdir)/data/themes/default/images -id $(top_srcdir)/data/themes/default/images/clockbling -fd $(top_srcdir)/data/themes/default/fonts $< $(top_builddir)/data/themes/default/$(@F)
