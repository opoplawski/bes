
# Automake file for fileout_json
#
# 06/23/14 ndp
#

AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I conf

# Set the module version here, in the spec file and in configure.ac
M_NAME=fileout_json
M_VER=1.0.9

AM_CPPFLAGS = -I$(top_srcdir)/dispatch -I$(top_srcdir)/dap $(DAP_CFLAGS)
LIBADD = $(DAP_SERVER_LIBS) $(DAP_CLIENT_LIBS)

AM_CPPFLAGS += -DMODULE_NAME=\"$(M_NAME)\" -DMODULE_VERSION=\"$(M_VER)\"

SUBDIRS = . unit-tests tests

lib_besdir=$(libdir)/bes
lib_bes_LTLIBRARIES = libfojson_module.la

libfojson_module_la_SOURCES = $(FOJSON_SRC) $(FOJSON_HDR)
libfojson_module_la_LDFLAGS = -avoid-version -module 
libfojson_module_la_LIBADD = $(LIBADD)

FOJSON_SRC = FoInstanceJsonTransform.cc FoInstanceJsonTransmitter.cc FoJsonRequestHandler.cc FoJsonModule.cc \
	FoDapJsonTransmitter.cc FoDapJsonTransform.cc StreamString.cc fojson_utils.cc

FOJSON_HDR = FoInstanceJsonTransform.h FoInstanceJsonTransmitter.h FoJsonRequestHandler.h FoJsonModule.h \
	FoDapJsonTransmitter.h FoDapJsonTransform.h StreamString.h fojson_utils.h

EXTRA_DIST = data fojson.conf.in

CLEANFILES = *~ fojson.conf

install-data-local: fojson.conf
	@currdate=`date +"%y%m%d%H%M"`; \
	test -d $(DESTDIR)$(sysconfdir)/bes || $(MKDIR_P) $(DESTDIR)$(sysconfdir)/bes; \
	test -d $(DESTDIR)$(sysconfdir)/bes/modules || $(MKDIR_P) $(DESTDIR)$(sysconfdir)/bes/modules; \
	test -f $(DESTDIR)$(sysconfdir)/bes/modules/fojson.conf && mv -f $(DESTDIR)$(sysconfdir)/bes/modules/fojson.conf $(DESTDIR)$(sysconfdir)/bes/modules/fojson.conf.$$currdate; \
	$(INSTALL_DATA) fojson.conf $(DESTDIR)$(sysconfdir)/bes/modules/fojson.conf

uninstall-local:
	rm -f $(DESTDIR)$(sysconfdir)/bes/modules/fojson.conf

fojson.conf: fojson.conf.in $(top_srcdir)/config.status
	sed -e "s%[@]bes_modules_dir[@]%${lib_besdir}%" $< > fojson.conf


C4_DIR=./cccc
.PHONY: cccc
cccc:	
	cccc --outdir=$(C4_DIR) $(FONC_SRC) $(FONC_HDR)


