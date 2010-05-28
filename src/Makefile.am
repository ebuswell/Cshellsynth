lib_LTLIBRARIES = libcshellsynth.la
libcshellsynth_la_SOURCES = controller.c falling_saw.c instrument.c jclient.c key.c modulator.c rising_saw.c sine.c square.c synth.c triangle.c
libcshellsynth_la_LDFLAGS = -ljack -module -export-dynamic $(LDFLAGS)
# include_HEADERS = controller.h instrument.h synth.h jclient.h
check_PROGRAMS = test
test_LDFLAGS = -ljack -lcshellsynth
TESTS = test