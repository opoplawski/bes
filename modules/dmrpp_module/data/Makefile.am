
AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = $(DAP_CFLAGS) $(BES_CPPFLAGS) -I$(top_srcdir)/dap -I$(top_srcdir)/dispatch
AM_CXXFLAGS = 

bin_PROGRAMS = build_dmrpp
noinst_PROGRAMS = endian_convert multiball keepalive2

build_dmrpp_CPPFLAGS = $(H5_CPPFLAGS) -I$(top_srcdir)/modules/dmrpp_module \
-I$(top_srcdir)/dispatch -I$(top_srcdir)/dap $(DAP_CFLAGS)

build_dmrpp_SOURCES = build_dmrpp.cc

# I tried linking with the HDF5 module, but that didn't work. The error was
# "... file was built for archive which is not the architecture being linked."
# jhrg 5/11/18
#
# HDF5_MODULE = $(H5_LDFLAGS) $(H5_LIBS) $(GCTP_LIBS) -L$(top_builddir)/modules/hdf5_handler -lhdf5_module

# Explictily linking with the .a archive removes the warnings about the .so
# being a module and not a real library. jhrg 5/11/18
#
# DMRPP_MODULE = -L$(top_builddir)/modules/dmrpp_module -ldmrpp_module

DMRPP_MODULE = $(top_builddir)/modules/dmrpp_module/.libs/libdmrpp_module.a
BESDAP_MODULE = $(top_builddir)/dap/.libs/libdap_module.a
build_dmrpp_LDADD = $(HDF5_MODULE) $(DMRPP_MODULE) $(BES_DISPATCH_LIB) $(BESDAP_MODULE) $(BES_EXTRA_LIBS) \
$(H5_LDFLAGS) $(H5_LIBS) $(DAP_SERVER_LIBS) $(DAP_CLIENT_LIBS) -lz

endian_convert_SOURCES = endian_convert.cc
endian_convert_LDADD = $(DAP_SERVER_LIBS)

multiball_SOURCES = multiball.cc 
multiball_LDADD = $(DAP_SERVER_LIBS) $(CURL_LIBS)

keepalive2_SOURCES = keepalive2.cc 
keepalive2_LDADD = $(DAP_SERVER_LIBS) $(CURL_LIBS) $(PTHREAD_LIBS)

noinst_SCRIPTS = keepalive_stats.sh mtest.sh shreddit.sh test_reuse.sh \
get_dmrpp.sh

# Build the bes.conf used for testing so that the value substituted for
# @abs_top_srcdir@ does not contain '../'. This happens when using 
# configure's value for the parameter when running the distcheck target.
%.conf : %.conf.in
	@clean_abs_top_srcdir=`echo ${abs_top_srcdir} | sed 's/\(.*\)\/\(.[^\/]*\)\/\.\./\1/g'`; \
	sed -e "s%[@]abs_top_srcdir[@]%$$clean_abs_top_srcdir%" \
		-e "s%[@]abs_top_builddir[@]%${abs_top_builddir}%" \
		-e "s%[@]modulesdir[@]%${modulesdir}%" $< > $@

bes.hdf5.cf.template.conf: $(srcdir)/bes.hdf5.cf.template.conf.in $(top_srcdir)/configure.ac

EXTRA_PROGRAMS = 

noinst_DATA = mds_for_tests

EXTRA_DIST = keepalive_stats.sh mtest.sh shreddit.sh test_reuse.sh \
mkChunkedTestData.py mkChunkedNewTypes.py get_dmrpp.sh README \
chunked_compund.bescmd chunked_enum.bescmd chunked_opaque.bescmd \
chunked_scalar_string.bescmd chunked_string_array.bescmd \
nc4_group_atomic.bescmd ntypes.bescmd dmrpp mds_for_tests

CLEANFILES = *~

# These are defined in the top-level Makefile.am
sample_datadir = $(datadir)/hyrax/data/dmrpp
sample_data_DATA = $(SAMPLE_DATA_FILES)

SAMPLE_DATA_FILES = dmrpp/README \
dmrpp/a2_local_twoD.h5.dmrpp \
dmrpp/a3_local_twoD.h5.dmrpp \
dmrpp/bstream_3980492644c726c40d98833e277e02da.bs \
dmrpp/bstream_52310773dfe8f711ca3ad4a95cbd65d8.bs \
dmrpp/bstream_63004b2080d84a81469dc0f113a3c843.bs \
dmrpp/bstream_a6fee7e3efc727e4b43ae9daf881cb58.bs \
dmrpp/chunked_fourD.h5 \
dmrpp/chunked_fourD.h5.dmrpp \
dmrpp/chunked_gzipped_fourD.h5 \
dmrpp/chunked_gzipped_fourD.h5.dmrpp \
dmrpp/chunked_gzipped_oneD.h5 \
dmrpp/chunked_gzipped_oneD.h5.dmrpp \
dmrpp/chunked_gzipped_threeD.h5 \
dmrpp/chunked_gzipped_threeD.h5.dmrpp \
dmrpp/chunked_gzipped_twoD.h5 \
dmrpp/chunked_gzipped_twoD.h5.dmrpp \
dmrpp/chunked_oneD.h5 \
dmrpp/chunked_oneD.h5.dmrpp \
dmrpp/chunked_oneD_uneven.h5 \
dmrpp/chunked_oneD_uneven.h5.dmrpp \
dmrpp/chunked_shuffled_fourD.h5 \
dmrpp/chunked_shuffled_fourD.h5.dmrpp \
dmrpp/chunked_shuffled_oneD.h5 \
dmrpp/chunked_shuffled_oneD.h5.dmrpp \
dmrpp/chunked_shuffled_threeD.h5 \
dmrpp/chunked_shuffled_threeD.h5.dmrpp \
dmrpp/chunked_shuffled_twoD.h5 \
dmrpp/chunked_shuffled_twoD.h5.dmrpp \
dmrpp/chunked_shufzip_fourD.h5 \
dmrpp/chunked_shufzip_fourD.h5.dmrpp \
dmrpp/chunked_shufzip_oneD.h5 \
dmrpp/chunked_shufzip_oneD.h5.dmrpp \
dmrpp/chunked_shufzip_threeD.h5 \
dmrpp/chunked_shufzip_threeD.h5.dmrpp \
dmrpp/chunked_shufzip_twoD.h5 \
dmrpp/chunked_shufzip_twoD.h5.dmrpp \
dmrpp/chunked_threeD.h5 \
dmrpp/chunked_threeD.h5.dmrpp \
dmrpp/chunked_threeD_asymmetric.h5 \
dmrpp/chunked_threeD_asymmetric.h5.dmrpp \
dmrpp/chunked_threeD_asymmetric_uneven.h5 \
dmrpp/chunked_threeD_asymmetric_uneven.h5.dmrpp \
dmrpp/chunked_twoD.h5 \
dmrpp/chunked_twoD.h5.dmrpp \
dmrpp/chunked_twoD_asymmetric.h5 \
dmrpp/chunked_twoD_asymmetric.h5.dmrpp \
dmrpp/chunked_twoD_uneven.h5 \
dmrpp/chunked_twoD_uneven.h5.dmrpp \
dmrpp/coads_climatology.bin \
dmrpp/coads_climatology.dmrpp \
dmrpp/d_int.h5 \
dmrpp/d_int.h5.dmrpp \
dmrpp/grid_1_2d.h5 \
dmrpp/grid_1_2d.h5.dmrpp \
dmrpp/grid_2_2d.h5 \
dmrpp/grid_2_2d.h5.dmrpp \
dmrpp/http_d_int.h5.dmrpp \
dmrpp/http_t_int_scalar.h5.dmrpp \
dmrpp/nc4_group_atomic.h5 \
dmrpp/nc4_group_atomic.h5.dmrpp \
dmrpp/t_float.h5 \
dmrpp/t_float.h5.dmrpp \
dmrpp/t_int_scalar.h5 \
dmrpp/t_int_scalar.h5.dmrpp
