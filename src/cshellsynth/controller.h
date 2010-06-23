#ifndef CSHELLSYNTH_CONTROLLER_H
#define CSHELLSYNTH_CONTROLLER_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

typedef struct cs_ctlr_struct {
    jack_client_t *client;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
} cs_ctlr_t;

#define cs_ctlr_destroy(cs_ctlr) jclient_destroy((jclient_t *) (cs_ctlr))
int cs_ctlr_init(cs_ctlr_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
