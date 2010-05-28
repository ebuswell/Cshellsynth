#ifndef CONTROLLER_H
#define CONTROLLER_H 1

#include <jack/jack.h>

typedef struct cs_ctlr_struct {
    jack_client_t *client;
    pthread_mutex_t lock;
    jack_port_t *ctl_port;
    jack_port_t *out_port;
} cs_ctlr_t;

#define cs_ctlr_destroy(cs_ctlr) jclient_locking_destroy((jclient_locking_t *) (cs_ctlr))
int cs_ctlr_init(cs_ctlr_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
