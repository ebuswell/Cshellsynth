# extra.rb
# 
# Copyright 2010 Evan Buswell
# 
# This file is part of Cshellsynth.
# 
# Cshellsynth is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
# 
# Cshellsynth is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.
class Mixer
  module MixerHelp
    def initialize(mix)
      @mix = mix
    end
    def create_up_to(num)
      for i in 1..num do
        if @mix[i] == nil
          @mix[i] = LLMixer.new
          @mix[i - 1].in2 = @mix[i].out
        end
      end
    end
    def to_a
      return @mix
    end
  end
  class Inself
    include MixerHelp
    def [](index)
      create_up_to(index)
      @mix[index].in1
    end
    def []=(index, value)
      create_up_to(index)
      @mix[index].in1 = value
    end
  end
  class InAmpself
    include MixerHelp
    def []=(index, value)
      create_up_to(index)
      @mix[index].in1_amp = value
    end
  end
  attr_reader :in, :in_amp
  def initialize
    @mix = [LLMixer.new]
    @in = Inself.new(@mix)
    @in_amp = InAmpself.new(@mix)
  end
  def out
    @mix[0].out
  end
  def out=(out)
    @mix[0].out = out
  end
end

class MiniSynth
  attr_reader :clock, :seq, :inst, :porta, :envg, :key, :synth, :lfo, :distort, :fenvg, :filter
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
    @synth = Synths::Square.new
    @synth.duty_cycle = 0.125
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
    @fem = Filters::Lin2Exp.new
    @fem.in = @fenvg.out
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
