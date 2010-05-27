#include <stdio.h>
#include <stdlib.h>
#include "instrument.h"
#include "key.h"
#include "sine.h"

int main(int argc, char **argv) {
    cs_inst_t inst;
    cs_key_t key;
    cs_sine_t sine;
    int r = cs_inst_init(&inst, "inst", 0, NULL);
    if(r != 0) {
	perror("Could not initialize instrument");
	exit(r);
    }
    jack_activate(inst.client);
    r = cs_key_init(&key, "key", 0, NULL);
    if(r != 0) {
	perror("Could not initialize key");
	exit(r);
    }
    jack_activate(key.client);
    r = cs_sine_init(&sine, "sine", 0, NULL);
    if(r != 0) {
	perror("Could not initialize sine");
	exit(r);
    }
    jack_activate(sine.client);
    jack_connect(inst.client, "inst:out", "key:note");
    jack_connect(key.client, "key:freq", "sine:freq");
    jack_connect(sine.client, "sine:out", "system:playback_1");
    jack_connect(sine.client, "sine:out", "system:playback_2");
    r = cs_inst_play(&inst, 0);
    if(r != 0) {
	perror("Could not stop instrument");
	exit(r);
    }
    sleep(4);
    r = cs_inst_stop(&inst);
    if(r != 0) {
	perror("Could not stop instrument");
	exit(r);
    }
    r = cs_inst_destroy(&inst);
    if(r != 0) {
	perror("Could not destroy instrument");
	exit(r);
    }
    r = cs_key_destroy(&key);
    if(r != 0) {
	perror("Could not destroy key");
	exit(r);
    }
    r = cs_sine_destroy(&sine);
    if(r != 0) {
	perror("Could not destroy sine");
	exit(r);
    }
    return 0;
}
