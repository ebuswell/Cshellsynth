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
      if value.instance_of? Array
        value.each do |one|
          @mix[index].in1 = one
        end
      else
        @mix[index].in1 = value
      end
    end
  end
  class InAmpself
    include MixerHelp
    def []=(index, value)
      create_up_to(index)
      @mix[index].in1_amp = value
    end
  end
  attr_reader :in, :amp
  def initialize
    @mix = [LLMixer.new]
    @in = Inself.new(@mix)
    @amp = InAmpself.new(@mix)
  end
  def out
    @mix[0].out
  end
  def out=(out)
    @mix[0].out = out
  end
  def next_free
    @mix.each_index do |i|
      if ! @mix[i].in1.connected?
        return i
      end
    end
    return @mix.length
  end
end

class FullMixer
  module MixerHelp
    def initialize(mixL, mixR, pan_a)
      @mixL = mixL
      @mixR = mixR
      @pan_a = pan_a
    end
    def create_up_to(num)
      for i in 1..num do
        if @pan_a[i] == nil
          @mixL[i] = LLMixer.new
          @mixL[i - 1].in2 = @mixL[i].out
          @mixR[i] = LLMixer.new
          @mixR[i - 1].in2 = @mixR[i].out
          @pan_a[i] = LLPan.new
          @pan_a[i].outL = @mixL[i].in1
          @pan_a[i].outR = @mixR[i].in1
        end
      end
    end
  end
  class Inself
    include MixerHelp
    def [](index)
      create_up_to(index)
      @pan_a[index].in
    end
    def []=(index, value)
      create_up_to(index)
      if value.instance_of? Array
        value.each do |one|
          @pan_a[index].in = one
        end
      else
        @pan_a[index].in = value
      end
    end
  end
  class InAmpself
    include MixerHelp
    def []=(index, value)
      create_up_to(index)
      @mixL[index].in1_amp = value
      @mixR[index].in1_amp = value
    end
  end
  class Panself
    include MixerHelp
    def [](index)
      create_up_to(index)
      @pan_a[index].pan
    end
    def []=(index, value)
      create_up_to(index)
      if value.instance_of? Array
        value.each do |one|
          @pan_a[index].pan = one
        end
      else
        @pan_a[index].pan = value
      end
    end
  end
  attr_reader :in, :amp, :pan
  def initialize
    @mixL = [LLMixer.new]
    @mixR = [LLMixer.new]
    @pan_a = [LLPan.new]
    @pan_a[0].outL = @mixL[0].in1
    @pan_a[0].outR = @mixR[0].in1
    @in = Inself.new(@mixL, @mixR, @pan_a)
    @amp = InAmpself.new(@mixL, @mixR, @pan_a)
    @pan = Panself.new(@mixL, @mixR, @pan_a)
  end
  def outL
    @mixL[0].out
  end
  def outL=(out)
    @mixL[0].out = out
  end
  def outR
    @mixR[0].out
  end
  def outR=(out)
    @mixR[0].out = out
  end
  def next_free
    @pan_a.each_index do |i|
      if ! @pan_a[i].in.connected?
        return i
      end
    end
    return @pan_a.length
  end
end

$mixer ||= FullMixer.new
$mixer.outL = "system:playback_1"
$mixer.outR = "system:playback_2"

class PolyClient
  attr_reader :polyphony
  def initialize(klass, number, *args)
    @clients = []
    @cl_klass = klass
    @polyphony = number
    @cl_args = args
    number.times do
      @clients << klass.new(*args)
    end
  end
  def polyphony=(number)
    if number > @polyphony
      while @polyphony < number
        @clients << @cl_klass.new(*@cl_args)
        @polyphony += 1
      end
    else
      @clients = @clients[0...number]
      @polyphony = number
    end
  end
  def method_missing(symbol, *args)
    if @clients[0].respond_to? symbol
      ret = []
      @clients.each_index do |i|
        nargs = []
        args.each do |arg|
          if arg.instance_of? Array
            nargs << arg[i]
          else
            nargs << arg
          end
        end
        ret << @clients[i].send(symbol, *nargs)
      end
      return ret
    else
      super(symbol, *args)
    end
  end
end

module Synths
  class Triangle < PolyClient
    def initialize(*args)
      super(LLTriangle, 1, *args)
    end
  end
  class Square < PolyClient
    def initialize(*args)
      super(LLSquare, 1, *args)
    end
  end
  class Sine < PolyClient
    def initialize(*args)
      super(LLSine, 1, *args)
    end
  end
  class RisingSaw < PolyClient
    def initialize(*args)
      super(LLRisingSaw, 1, *args)
    end
  end
  class Noise < PolyClient
    White = LLNoise::White
    Red = LLNoise::Red
    Pink = LLNoise::Pink
    def initialize(*args)
      super(LLNoise, 1, *args)
    end
  end
  class FallingSaw < PolyClient
    def initialize(*args)
      super(LLFallingSaw, 1, *args)
    end
  end
  class Parabola < PolyClient
    def initialize(*args)
      super(LLParabola, 1, *args)
    end
  end
  class Dsf < PolyClient
    def initialize(*args)
      super(LLDsf, 1, *args)
    end
  end
  class Cotangent < PolyClient
    def initialize(*args)
      super(LLCotangent, 1, *args)
    end
  end
  class ImpulseTrain < PolyClient
    def initialize(*args)
      super(LLImpulseTrain, 1, *args)
    end
  end
end

module Filters
  class Portamento < PolyClient
    def initialize(*args)
      super(LLPortamento, 1, *args)
    end
  end
  class Lowpass < PolyClient
    def initialize(*args)
      super(LLLowpass, 1, *args)
    end
  end
  class Lin2Exp < PolyClient
    def initialize(*args)
      super(LLLin2Exp, 1, *args)
    end
  end
  class Lin2Exp < PolyClient
    def initialize(*args)
      super(LLLin2Exp, 1, *args)
    end
  end
  class Highpass < PolyClient
    def initialize(*args)
      super(LLHighpass, 1, *args)
    end
  end
  class Distortion < PolyClient
    Exponential = LLDistortion::Exponential
    Hyperbolic = LLDistortion::Hyperbolic
    Arctangent = LLDistortion::Arctangent
    Tube = LLDistortion::Tube
    def initialize(*args)
      super(LLDistortion, 1, *args)
    end
  end
  class Bandpass < PolyClient
    def initialize(*args)
      super(LLBandpass, 1, *args)
    end
  end
  class Notch < PolyClient
    def initialize(*args)
      super(LLNotch, 1, *args)
    end
  end
end

class Key < PolyClient
  Major = LLKey::Major
  Minor = LLKey::Minor
  Pythagorean = LLKey::Pythagorean
  Equal = LLKey::Equal

  A = LLKey::A
  A_Sharp = LLKey::A_Sharp
  B_Flat = LLKey::B_Flat
  B = LLKey::B
  C_Flat = LLKey::C_Flat
  C = LLKey::C
  B_Sharp = LLKey::B_Sharp
  C_Sharp = LLKey::C_Sharp
  D_Flat = LLKey::D_Flat
  D = LLKey::D
  D_Sharp = LLKey::D_Sharp
  E_Flat = LLKey::E_Flat
  E = LLKey::E
  F_Flat = LLKey::F_Flat
  F = LLKey::F
  E_Sharp = LLKey::E_Sharp
  F_Sharp = LLKey::F_Sharp
  G_Flat = LLKey::G_Flat
  G = LLKey::G
  G_Sharp = LLKey::G_Sharp
  A_Flat = LLKey::A_Flat
  
  def initialize(*args)
    super(LLKey, 1, *args)
  end
  def tuning=(tuning)
    @clients.each do |client|
      client.tuning=tuning
    end
  end
end
class Modulator < PolyClient
  def initialize(*args)
    super(LLModulator, 1, *args)
  end
end
class Pan < PolyClient
  def initialize(*args)
    super(LLPan, 1, *args)
  end
end
class EnvelopeGenerator < PolyClient
  def initialize(*args)
    super(LLEnvelopeGenerator, 1, *args)
  end
end

class Sampler < LLSampler
  def ctl=(ctl)
    if ctl.instance_of? Array
      ctl.each do |c|
        super(c)
      end
    else
      super(ctl)
    end
  end
  def outL=(outL)
    if outL.instance_of? Array
      outL.each do |c|
        super(c)
      end
    else
      super(outL)
    end
  end
  def outR=(outR)
    if outR.instance_of? Array
      outR.each do |c|
        super(c)
      end
    else
      super(outR)
    end
  end
end
class Clock < LLClock
  def meter=(meter)
    if meter.instance_of? Array
      meter.each do |c|
        super(c)
      end
    else
      super(meter)
    end
  end
  def rate=(rate)
    if rate.instance_of? Array
      rate.each do |c|
        super(c)
      end
    else
      super(rate)
    end
  end
  def clock=(clock)
    if clock.instance_of? Array
      clock.each do |c|
        super(c)
      end
    else
      super(clock)
    end
  end
end

$clock = Clock.new

module Controllers
  class Instrument < LLInstrument
    def out=(out)
      if out.instance_of? Array
        out.each do |c|
          super(c)
        end
      else
        super(out)
      end
    end
    def ctl=(ctl)
      if ctl.instance_of? Array
        ctl.each do |c|
          super(c)
        end
      else
        super(ctl)
      end
    end
  end

  class Sequencer < PolyClient
    attr_accessor :granularity
    def initialize(*args)
      super(LLSequencer, 1, *args)
      @granularity = 0.5
    end
    def <<(seq_string)
      make_sequence_str(seq_string, true)
    end
    def >>(seq_string)
      make_sequence_str(seq_string, false)
    end
    def make_sequence_str(seq_string, repeat)
      seq = []
      counter = 0.0
      number = nil
      sustain = nil
      seq_string.each_char do |c|
        if ((c[0] >= 48 && c[0] <= 57) || c == "." || c == "-") # numerical
          if ! sustain.nil?
            seq << [sustain, sustain + (counter - sustain) * 0.92, number.to_f]
            sustain = nil
            number = nil
          end
          if number.nil?
            number = c
          else
            number += c
          end
        elsif c[0] <= 32 # Whitespace
          if ! sustain.nil?
            seq << [sustain, sustain + (counter - sustain) * 0.92, number.to_f]
            sustain = nil
            number = nil
          elsif ! number.nil?
            seq << [counter, counter + @granularity * 0.92, number.to_f]
            number = nil
            counter += @granularity # for the number
          end
          counter += @granularity # for the whitespace
        elsif c == "~"
          if number.nil?
            raise "invalid syntax"
          end
          if sustain.nil?
            sustain = counter
            counter += @granularity
          end
          counter += @granularity
        else
          raise "invalid syntax"
        end
      end
      if ! sustain.nil?
        seq << [sustain, sustain + (counter - sustain) * 0.92, number.to_f]
        sustain = nil
        number = nil
      elsif ! number.nil?
        seq << [counter, counter + @granularity * 0.92, number.to_f]
        counter += @granularity
      end
      puts "calling make_sequence(0.0, #{counter.inspect}, #{seq.inspect}, #{repeat.inspect})"
      make_sequence(0.0, counter, seq, repeat)
    end
    def sequence(offset, length, *sequence)
      make_sequence(offset, length, sequence, true)
    end
    def sequence_once(offset, length, *sequence)
      make_sequence(offset, length, sequence, false)
    end
    def make_sequence(offset, length, sequence, repeat)
      sequences = []
      @polyphony.times do
        sequences << []
      end
      sequence.each do |s|
        idx = 0
        lowest = 1.0/0.0
        sequences.each_index do |i|
          if sequences[i].last == nil
            if lowest != -1.0/0.0
              idx = i
              lowest = -1.0/0.0
            end
          elsif sequences[i].last[1] < lowest
            idx = i
            lowest = sequences[i].last[1]
          end
        end
        sequences[idx] << s
      end
      sequences.each_index do |i|
        @clients[i].make_sequence(offset, length, sequences[i], repeat)
      end
    end
  end
end

class MiniSynth
  attr_reader :clock, :seq, :inst, :porta, :envg, :key, :synth, :lfo, :distort, :fenvg, :filter, :out, :mixnum
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
    @seq = Controllers::Sequencer.new
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
    @out = @distort.out
    @mixnum = $mixer.next_free
    $mixer.in[@mixnum] = @out
    @seq.clock = $clock.clock
  end
  def amp=(arg)
    $mixer.amp[@mixnum] = arg
  end
  def pan=(arg)
    $mixer.pan[@mixnum] = arg
  end
  def out=(arg)
    @distort.out = arg
  end
end
