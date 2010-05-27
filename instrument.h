#ifndef INSTRUMENT_H
#define INSTRUMENT_H 1

#include <jack/jack.h>
#include "controller.h"

typedef struct cs_inst_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
    jack_default_audio_sample_t value;
    jack_default_audio_sample_t last_value;
} cs_inst_t;

#define cs_inst_destroy(cs_inst) cs_ctlr_destroy((cs_ctlr_t *) (cs_inst))
int cs_inst_init(cs_inst_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
