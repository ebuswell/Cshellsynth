#ifndef CSHELLSYNTH_INSTRUMENT_H
#define CSHELLSYNTH_INSTRUMENT_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/controller.h>

typedef struct cs_inst_struct {
    jack_client_t *client;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    atomic_float_t value;
    atomic_float_t ctl;
} cs_inst_t;

#define cs_inst_destroy(cs_inst) cs_ctlr_destroy((cs_ctlr_t *) (cs_inst))
int cs_inst_init(cs_inst_t *self, const char *client_name, jack_options_t flags, char *server_name);
void cs_inst_play(cs_inst_t *self, float value);
void cs_inst_stop(cs_inst_t *self);

#endif
