class MiniSynth
  attr_reader :inst, :porta, :envg, :key, :synth, :lfo, :distort, :fenvg, :feamp
  def synth=(synth)
    @synth.disconnect @synth.freq
    @synth.disconnect @synth.out
    @synth = synth
    @synth.freq = @key.freq
    @lfm.in1 = @synth.out
  end
  def initialize
    @inst = Controllers::Instrument.new
    @porta = Filters::Portamento.new
    @envg = EnvelopeGenerator.new
    @key = Key.new
    @envg.ctl = @inst.ctl
    @porta.in = @inst.out
    @key.note = @porta.out
    @synth = Synths::Cotangent.new
    @synth.freq = @key.freq
    @lfo = Synths::Sine.new
    @lfo.freq = 0.0
    @lfo.offset = 1.0
    @lfo.amp = 0.0
    @lfm = Modulator.new
    @lfm.in1 = @synth.out
    @lfm.in2 = @lfo.out
    @em = Modulator.new
    @em.in1 = @lfm.out
    @em.in2 = @envg.out
    @lowpass = Filters::Lowpass.new
    @lowpass.in = @em.out


    @fenvg = EnvelopeGenerator.new
    @fenvg.linear = true
    @fenvg.ctl = @inst.ctl
    @feamp = Modulator.new
    @feamp.in1 = @fenvg.out
    @feamp.in2 = 1.0
    @fem = Filters::Lin2Exp.new
    @fem.in = @feamp.out
    @fem.zero = @key.freq
    @lowpass.freq = @fem.out

    @distort = Filters::Distortion.new
    @distort.in = @lowpass.out
    @distort.sharpness = 4
    @distort.gain = 1.0
    @distort.connect @distort.out, "system:playback_1"
    @distort.connect @distort.out, "system:playback_2"
  end
end
