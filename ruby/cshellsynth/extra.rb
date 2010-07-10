class MiniSynth
  attr_reader :clock, :seq, :inst, :porta, :envg, :key, :synth, :lfo, :distort, :fenvg, :feamp, :filter
  def synth=(synth)
    @synth.disconnect @synth.freq
    @synth.disconnect @synth.out
    @synth = synth
    @synth.freq = @key.freq
    @filter.in = @synth.out
  end
  def filter=(filter)
    @filter.disconnect @filter.freq
    @filter.disconnect @filter.out
    @filter.disconnect @filter.in
    @filter = filter
    @filter.freq = @fem.out
    @filter.in = @synth.out
    @lfm.in1 = @filter.out
  end
  def initialize
    @inst = Controllers::Instrument.new
    @clock = Clock.new
    @seq = Controllers::Sequencer.new
    @seq.clock = @clock.clock
    @porta = Filters::Portamento.new
    @envg = EnvelopeGenerator.new
    @key = Key.new
    @envg.ctl = @inst.ctl
    @envg.ctl = @seq.ctl
    @porta.in = @inst.out
    @porta.in = @seq.out
    @key.note = @porta.out
    @synth = Synths::Cotangent.new
    @synth.freq = @key.freq
    @lfo = Synths::Sine.new
    @lfo.freq = 0.0
    @lfo.offset = 1.0
    @lfo.amp = 0.0
    @filter = Filters::Lowpass.new
    @filter.in = @synth.out
    @lfm = Modulator.new
    @lfm.in1 = @filter.out
    @lfm.in2 = @lfo.out
    @em = Modulator.new
    @em.in1 = @lfm.out
    @em.in2 = @envg.out


    @fenvg = EnvelopeGenerator.new
    @fenvg.linear = true
    @fenvg.ctl = @inst.ctl
    @fenvg.ctl = @seq.ctl
    @feamp = Modulator.new
    @feamp.in1 = @fenvg.out
    @feamp.in2 = 1.0
    @fem = Filters::Lin2Exp.new
    @fem.in = @feamp.out
    @fem.zero = @key.freq
    @filter.freq = @fem.out

    @distort = Filters::Distortion.new
    @distort.in = @em.out
    @distort.sharpness = 4
    @distort.gain = 1.0
    @distort.connect @distort.out, "system:playback_1"
    @distort.connect @distort.out, "system:playback_2"
  end
end
