/*
 * instrument.c
 * 
 * Copyright 2010 Evan Buswell
 * 
 * This file is part of Cshellsynth.
 * 
 * Cshellsynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Cshellsynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <jack/jack.h>
#include <math.h>
#include <string.h>
#include "cshellsynth/instrument.h"
#include "cshellsynth/controller.h"
#include "atomic-float.h"

static int cs_inst_process(jack_nframes_t nframes, void *arg) {
    cs_inst_t *self = (cs_inst_t *) arg;
    float *out_buffer = (float *)jack_port_get_buffer(self->out_port, nframes);
    if(out_buffer == NULL) {
	return -1;
    }
    float *ctl_buffer = (float *)jack_port_get_buffer(self->ctl_port, nframes);
    if(ctl_buffer == NULL) {
	return -1;
    }

    memset(ctl_buffer, 0, nframes * sizeof(float));
    float ctl = atomic_float_xchg(&self->ctl, 0.0f);
    if(ctl != 0.0) {
	ctl_buffer[0] = ctl;
    }
    float value = atomic_float_read(&self->value);
    int i;
    for(i = 0; i < nframes; i++) {
	out_buffer[i] = value;
    }
    return 0;
}

int cs_inst_init(cs_inst_t *self, const char *client_name, jack_options_t flags, char *server_name) {
    int r = cs_ctlr_init((cs_ctlr_t *) self, client_name, flags, server_name);
    if(r != 0) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return r;
    }

    atomic_float_set(&self->value, 0.0f);
    atomic_float_set(&self->ctl, 0.0f);

    r = jack_set_process_callback(self->client, cs_inst_process, self);
    if(r != 0) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return r;
    }

    r = jack_activate(self->client);
    if(r != 0) {
	cs_ctlr_destroy((cs_ctlr_t *) self);
	return r;
    }

    return 0;
}

void cs_inst_play(cs_inst_t *self, float value) {
    atomic_float_set(&self->value, value);
    atomic_float_set(&self->ctl, 1.0f);
}

void cs_inst_stop(cs_inst_t *self) {
    atomic_float_set(&self->ctl, -1.0f);
}
