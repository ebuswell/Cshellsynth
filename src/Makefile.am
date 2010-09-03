# Makefile.am
# 
# Copyright 2010 Evan Buswell
# 
# This file is part of Cshellsynth.
# 
# Cshellsynth is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
# 
# Cshellsynth is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.

lib_LTLIBRARIES = libcshellsynth.la
libcshellsynth_la_SOURCES = bandpass.c clock.c controller.c distortion.c dsf.c envelope_generator.c falling_saw.c filter.c cot.c highpass.c impulse_train.c instrument.c jclient.c key.c lin2exp.c lowpass.c mixer.c modulator.c noise.c notch.c pan.c parabola.c portamento.c rising_saw.c sampler.c sequencer.c sine.c square.c synth.c triangle.c
libcshellsynth_la_LDFLAGS = $(JACK_LIBS) $(SNDFILE_LIBS) -module -export-dynamic $(LDFLAGS)
libcshellsynth_la_CFLAGS = $(JACK_CFLAGS) $(SNDFILE_CFLAGS) $(CFLAGS) -Wall -Wextra
# libcshellsynth_la_CXXFLAGS = $(JACK_CFLAGS) $(SNDFILE_CFLAGS) $(CXXFLAGS) -Wall -Wextra
nobase_include_HEADERS = cshellsynth/atomic-types.h cshellsynth/bandpass.h cshellsynth/clock.h cshellsynth/controller.h cshellsynth/distortion.h cshellsynth/dsf.h cshellsynth/envelope_generator.h cshellsynth/falling_saw.h cshellsynth/filter.h cshellsynth/cot.h cshellsynth/highpass.h cshellsynth/impulse_train.h cshellsynth/instrument.h cshellsynth/jclient.h cshellsynth/key.h cshellsynth/lin2exp.h cshellsynth/lowpass.h cshellsynth/mixer.h cshellsynth/modulator.h cshellsynth/noise.h cshellsynth/notch.h cshellsynth/pan.h cshellsynth/parabola.h cshellsynth/portamento.h cshellsynth/rising_saw.h cshellsynth/sampler.h cshellsynth/sequencer.h cshellsynth/sine.h cshellsynth/square.h cshellsynth/synth.h cshellsynth/triangle.h cshellsynth.h
check_PROGRAMS = test
test_LDFLAGS = $(JACK_LIBS) $(SNDFILE_LIBS) -lcshellsynth
test_CFLAGS = $(JACK_CFLAGS) $(SNDFILE_CFLAGS) $(CFLAGS) -Wall -Wextra
TESTS = test
MAINTAINERCLEANFILES = Makefile.in config.h.in config.h.in~

EXTRA_DIST = atomic-float.h atomic.h atomic-long.h atomic-ptr.h falling_saw.h parabola.h util.h cshellsynth/main.dxg
