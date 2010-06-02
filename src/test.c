#include <stdio.h>
#include <stdlib.h>
#include "cshellsynth.h"
#include <time.h>
#include <math.h>

int main(int argc, char **argv) {
    cs_inst_t inst;
    cs_key_t key;
    cs_sine_t sine;
    cs_sine_t vib;
    cs_modu_t viba;
    cs_mix_t vibm;
    cs_rsaw_t rsawrm1;
    cs_sine_t sinerm2;
    cs_modu_t modu;
    cs_triangle_t triangle;
    cs_rsaw_t rsaw;
    cs_fsaw_t fsaw;
    cs_square_t square;
    cs_infh_t infh;
    cs_envg_t envg;
    cs_modu_t envm;
    cs_seq_t seq;
    cs_clock_t clock;
    int r = cs_inst_init(&inst, "inst", 0, NULL);
    if(r != 0) {
	perror("Could not initialize instrument");
	exit(r);
    }

    r = cs_key_init(&key, "key", 0, NULL);
    if(r != 0) {
	perror("Could not initialize key");
	exit(r);
    }
    r = cs_key_set_tuning(&key, CS_MAJOR_TUNING, CS_MAJOR_TUNING_SIZE);
    if(r != 0) {
	perror("Could not set tuning");
    }
    r = cs_key_set_root(&key, CS_G);
    if(r != 0) {
	perror("Could not set root");
    }

    r = cs_sine_init(&sine, "sine", 0, NULL);
    if(r != 0) {
	perror("Could not initialize sine");
	exit(r);
    }

    r = cs_sine_init(&vib, "vib", 0, NULL);
    if(r != 0) {
	perror("Could not initialize vib");
	exit(r);
    }
    r = cs_sine_set_freq(&vib, 8.0f);
    if(r != 0) {
	perror("Could not set freq");
    }

    r = cs_modu_init(&viba, "viba", 0, NULL);
    if(r != 0) {
	perror("Could not initialize viba");
	exit(r);
    }

    r = cs_modu_set_in2(&viba, 0.125);
    if(r != 0) {
	perror("Could not set in2");
	exit(r);
    }

    r = cs_mix_init(&vibm, "vibm", 0, NULL);
    if(r != 0) {
	perror("Could not initialize vibm");
	exit(r);
    }

    r = cs_rsaw_init(&rsawrm1, "rsawrm1", 0, NULL);
    if(r != 0) {
	perror("Could not initialize rsawrm1");
	exit(r);
    }

    r = cs_sine_init(&sinerm2, "sinerm2", 0, NULL);
    if(r != 0) {
	perror("Could not initialize sinerm2");
	exit(r);
    }
    r = cs_sine_set_freq(&sinerm2, 2500);

    r = cs_modu_init(&modu, "modu", 0, NULL);
    if(r != 0) {
	perror("Could not initialize modu");
	exit(r);
    }

    r = cs_triangle_init(&triangle, "triangle", 0, NULL);
    if(r != 0) {
	perror("Could not initialize triangle");
	exit(r);
    }

    r = cs_rsaw_init(&rsaw, "rsaw", 0, NULL);
    if(r != 0) {
	perror("Could not initialize rsaw");
	exit(r);
    }

    r = cs_fsaw_init(&fsaw, "fsaw", 0, NULL);
    if(r != 0) {
	perror("Could not initialize fsaw");
	exit(r);
    }

    r = cs_square_init(&square, "square", 0, NULL);
    if(r != 0) {
	perror("Could not initialize square");
	exit(r);
    }

    r = cs_infh_init(&infh, "infh", 0, NULL);
    if(r != 0) {
	perror("Could not initialize infh");
	exit(r);
    }

    r = cs_envg_init(&envg, "envg", 0, NULL);
    if(r != 0) {
	perror("Could not initialize envg");
	exit(r);
    }

    r = cs_envg_set_attack_t(&envg, 0.25);
    if(r != 0) {
	perror("Could not set attack_t");
	exit(r);
    }

    r = cs_envg_set_decay_t(&envg, 0.75);
    if(r != 0) {
	perror("Could not set decay_t");
	exit(r);
    }

    r = cs_envg_set_sustain_a(&envg, 0.0f);
    if(r != 0) {
	perror("Could not set sustain_a");
	exit(r);
    }

    r = cs_envg_set_release_t(&envg, 0.0);
    if(r != 0) {
	perror("Could not set release_t");
	exit(r);
    }

    r = cs_modu_init(&envm, "envm", 0, NULL);
    if(r != 0) {
	perror("Could not initialize envm");
	exit(r);
    }

    r = cs_seq_init(&seq, "seq", 0, NULL);
    if(r != 0) {
	perror("Could not initialize seq");
	exit(r);
    }

    r = cs_clock_init(&clock, "clock", 0, NULL);
    if(r != 0) {
	perror("Could not initialize clock");
	exit(r);
    }
    r = cs_clock_set_bpm(&clock, 240.0);
    if(r != 0) {
	perror("Could not initialize clock");
	exit(r);
    }
    r = cs_clock_set_meter(&clock, 3.0);
    if(r != 0) {
	perror("Could not initialize clock");
	exit(r);
    }

    jack_connect(vib.client, "vib:out", "viba:in1");
    jack_connect(viba.client, "viba:out", "vibm:in1");
    jack_connect(inst.client, "inst:out", "vibm:in2");
    jack_connect(vibm.client, "vibm:out", "key:note");
    jack_connect(inst.client, "inst:ctl", "envg:ctl");
    jack_connect(envg.client, "envg:out", "envm:in2");
    jack_connect(rsawrm1.client, "rsawrm1:out", "modu:in1");
    jack_connect(rsawrm1.client, "sinerm2:out", "modu:in2");
    jack_connect(sinerm2.client, "modu:out", "envm:in1");
    jack_connect(sine.client, "sine:out", "envm:in1");
    jack_connect(triangle.client, "triangle:out", "envm:in1");
    jack_connect(rsaw.client, "rsaw:out", "envm:in1");
    jack_connect(fsaw.client, "fsaw:out", "envm:in1");
    jack_connect(square.client, "square:out", "envm:in1");
    jack_connect(infh.client, "infh:out", "envm:in1");
    jack_connect(envm.client, "envm:out", "system:playback_1");
    jack_connect(envm.client, "envm:out", "system:playback_2");

    jack_connect(key.client, "key:freq", "sine:freq");
    r = cs_inst_play(&inst, 4.0f);
    if(r != 0) {
    	perror("Could not play instrument");
    	exit(r);
    }
    usleep(750000);
    jack_connect(key.client, "key:freq", "triangle:freq");
    jack_disconnect(key.client, "key:freq", "sine:freq");
    r = cs_inst_play(&inst, 2.0f);
    usleep(750000);
    jack_connect(key.client, "key:freq", "rsaw:freq");
    jack_disconnect(key.client, "key:freq", "triangle:freq");
    r = cs_inst_play(&inst, 0.0f);
    usleep(750000);
    jack_connect(key.client, "key:freq", "fsaw:freq");
    jack_disconnect(key.client, "key:freq", "rsaw:freq");
    r = cs_inst_play(&inst, -3.0f);
    usleep(750000);
    jack_connect(key.client, "key:freq", "square:freq");
    jack_disconnect(key.client, "key:freq", "fsaw:freq");
    r = cs_inst_play(&inst, -2);
    usleep(250000);
    jack_connect(key.client, "key:freq", "rsawrm1:freq");
    jack_disconnect(key.client, "key:freq", "square:freq");
    r = cs_inst_play(&inst, -1);
    usleep(250000);
    jack_connect(key.client, "key:freq", "sine:freq");
    jack_disconnect(key.client, "key:freq", "rsawrm1:freq");
    r = cs_inst_play(&inst, 0);
    usleep(250000);
    jack_connect(key.client, "key:freq", "triangle:freq");
    jack_disconnect(key.client, "key:freq", "sine:freq");
    r = cs_inst_play(&inst, -2);
    usleep(500000);
    jack_connect(key.client, "key:freq", "rsaw:freq");
    jack_disconnect(key.client, "key:freq", "triangle:freq");
    r = cs_inst_play(&inst, 0);
    usleep(250000);
    jack_connect(key.client, "key:freq", "fsaw:freq");
    jack_disconnect(key.client, "key:freq", "rsaw:freq");
    r = cs_inst_play(&inst, -3);
    usleep(1250000);
    r = cs_inst_stop(&inst);
    if(r != 0) {
    	perror("Could not stop instrument");
    	exit(r);
    }
    jack_connect(key.client, "key:freq", "infh:freq");
    jack_disconnect(key.client, "key:freq", "fsaw:freq");
    jack_connect(clock.client, "clock:clock", "seq:clock");
    jack_disconnect(inst.client, "inst:out", "vibm:in2");
    jack_disconnect(inst.client, "inst:ctl", "envg:ctl");
    jack_connect(seq.client, "seq:out", "vibm:in2");
    jack_connect(seq.client, "seq:ctl", "envg:ctl");
    jack_default_audio_sample_t **second_verse = alloca(11 * sizeof(jack_default_audio_sample_t *));
    jack_default_audio_sample_t **ptr = second_verse;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 0.0f;
    (*ptr)[1] = 2.75f;
    (*ptr)[2] = 1.0f;
    ptr++;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 3.0f;
    (*ptr)[1] = 5.75f;
    (*ptr)[2] = 4.0f;
    ptr++;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 6.0f;
    (*ptr)[1] = 8.75f;
    (*ptr)[2] = 2.0f;
    ptr++;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 9.0f;
    (*ptr)[1] = 11.75f;
    (*ptr)[2] = 0.0f;
    ptr++;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 12.0f;
    (*ptr)[1] = 12.95f;
    (*ptr)[2] = -2.0;
    ptr++;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 13.0f;
    (*ptr)[1] = 13.95f;
    (*ptr)[2] = -1.0;
    ptr++;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 14.0f;
    (*ptr)[1] = 14.95f;
    (*ptr)[2] = 0.0f;
    ptr++;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 15.0f;
    (*ptr)[1] = 16.95f;
    (*ptr)[2] = 1.0f;
    ptr++;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 17.0f;
    (*ptr)[1] = 17.95f;
    (*ptr)[2] = 2.0f;
    ptr++;
    *ptr = alloca(3 * sizeof(jack_default_audio_sample_t));
    (*ptr)[0] = 18.0f;
    (*ptr)[1] = 20.75f;
    (*ptr)[2] = 1.0f;
    ptr++;
    *ptr = NULL;
    cs_seq_sequence_once(&seq, 0.0f, 21.0f, second_verse);
    if(r != 0) {
    	perror("Could not create sequence");
    	exit(r);
    }
    sleep(9);
    r = cs_clock_destroy(&clock);
    if(r != 0) {
	perror("Could not destroy clock");
	exit(r);
    }
    r = cs_seq_destroy(&seq);
    if(r != 0) {
	perror("Could not destroy sequencer");
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
    r = cs_sine_destroy(&rsawrm1);
    if(r != 0) {
	perror("Could not destroy rsawrm1");
	exit(r);
    }
    r = cs_sine_destroy(&sinerm2);
    if(r != 0) {
	perror("Could not destroy sinerm2");
	exit(r);
    }
    r = cs_modu_destroy(&modu);
    if(r != 0) {
	perror("Could not destroy modu");
	exit(r);
    }
    r = cs_triangle_destroy(&triangle);
    if(r != 0) {
	perror("Could not destroy triangle");
	exit(r);
    }
    r = cs_rsaw_destroy(&rsaw);
    if(r != 0) {
	perror("Could not destroy rsaw");
	exit(r);
    }
    r = cs_fsaw_destroy(&fsaw);
    if(r != 0) {
	perror("Could not destroy fsaw");
	exit(r);
    }
    r = cs_square_destroy(&square);
    if(r != 0) {
	perror("Could not destroy square");
	exit(r);
    }
    r = cs_infh_destroy(&infh);
    if(r != 0) {
	perror("Could not destroy infh");
	exit(r);
    }
    r = cs_envg_destroy(&envg);
    if(r != 0) {
	perror("Could not destroy square");
	exit(r);
    }
    r = cs_modu_destroy(&envm);
    if(r != 0) {
	perror("Could not destroy envm");
	exit(r);
    }
    r = cs_sine_destroy(&vib);
    if(r != 0) {
	perror("Could not destroy vib");
	exit(r);
    }
    r = cs_modu_destroy(&viba);
    if(r != 0) {
	perror("Could not destroy viba");
	exit(r);
    }
    r = cs_mix_destroy(&vibm);
    if(r != 0) {
	perror("Could not destroy vibm");
	exit(r);
    }
    return 0;
}
