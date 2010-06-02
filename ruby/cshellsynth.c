#include <ruby.h>

void Init_jackruby();
void Init_synths();
void Init_synth();
void Init_controllers();
void Init_controller();
void Init_mixer();
void Init_clock();
void Init_envelope_generator();
void Init_falling_saw();
void Init_infh();
void Init_instrument();
void Init_key();
void Init_modulator();
void Init_rising_saw();
void Init_sequencer();
void Init_sine();
void Init_square();
void Init_triangle();

void Init_cshellsynth() {
    Init_jackruby();
    Init_synths();
    Init_synth();
    Init_controllers();
    Init_controller();
    Init_mixer();
    Init_clock();
    Init_envelope_generator();
    Init_falling_saw();
    Init_infh();
    Init_instrument();
    Init_key();
    Init_modulator();
    Init_rising_saw();
    Init_sequencer();
    Init_sine();
    Init_square();
    Init_triangle();
}
