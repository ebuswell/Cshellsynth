lib_LTLIBRARIES = libcshellsynth.la
libcshellsynth_la_SOURCES = controller.c instrument.c jclient.c key.c sine.c synth.c
libcshellsynth_la_LDFLAGS = -ljack -module -export-dynamic $(LDFLAGS)
# include_HEADERS = controller.h instrument.h synth.h jclient.h
check_PROGRAMS = test
test_LDFLAGS = -ljack -lcshellsynth
