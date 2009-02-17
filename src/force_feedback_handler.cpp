/* 
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include "command_line_options.hpp"
#include "force_feedback_handler.hpp"

std::ostream& operator<<(std::ostream& out, const struct ff_envelope& envelope)
{
  out << "Envelope(attack_length:" << envelope.attack_length
      << ", attack_level:" << envelope.attack_level
      << ", fade_length:" << envelope.fade_length
      << ", fade_level:" << envelope.fade_level << ")";
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct ff_replay& replay)
{
  out << "Replay(length:" << replay.length << ", delay:" << replay.delay << ")";
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct ff_trigger& trigger)
{
  out << "Trigger(button:" << trigger.button << ", interval:" << trigger.interval << ")";
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct ff_effect& effect)
{
  std::cout << "Effect(";
  switch (effect.type)
    {
      case FF_CONSTANT:
        out << "FF_CONSTANT("
            << "level:" << effect.u.constant.level
            << ", envelope:" << effect.u.constant.envelope << ")";
        break;

      case FF_PERIODIC:
        out << "FF_PERIODIC("
            << ", waveform:" << effect.u.periodic.waveform
            << ", period:" << effect.u.periodic.period
            << ", magnitude:" << effect.u.periodic.magnitude
            << ", offset:" << effect.u.periodic.offset
            << ", phase:" << effect.u.periodic.phase
            << ", envelope:" << effect.u.periodic.envelope << ")";
        break;

      case FF_RAMP:
        out << "FF_RAMP(" 
            << "start_level:" << effect.u.ramp.start_level
            << ", end_level:" << effect.u.ramp.end_level
            << ", envelope:" <<  effect.u.ramp.envelope << ")";
        break;

      case FF_SPRING:
        out << "FF_SPRING()";
        break;

      case FF_FRICTION:
        out << "FF_FRICTION()";
        break;

      case FF_DAMPER:
        out << "FF_DAMPER()";
        break;

      case FF_RUMBLE:
        out << "FF_RUMBLE("
            << "strong_magnitude:" << effect.u.rumble.strong_magnitude
            << ", weak_magnitude:" << effect.u.rumble.weak_magnitude << ")";
        break;

      case FF_INERTIA:
        out << "FF_INERTIA()";
        break;

      case FF_CUSTOM:
        out << "FF_CUSTOM()";
        break;

      default:
        out << "FF_<unknown>()";
        break;
    }

  out << ", direction:" << effect.direction
      << ", replay:" << effect.replay
      << ", trigger:" << effect.trigger << ")";

  return out;
}

ForceFeedbackEffect::ForceFeedbackEffect()
  : playing(false),
    count(0),
    weak_magnitude(0),
    strong_magnitude(0)
{
}

static int clamp(int lhs, int rhs, int v)
{
  return std::max(lhs, std::min(v, rhs));
}

ForceFeedbackEffect::ForceFeedbackEffect(const struct ff_effect& effect)
  : playing(false),
    count(0),
    weak_magnitude(0),
    strong_magnitude(0)
{
  // Since we can't excute most effects directly, we have to emulate
  // them, for documentation on effects see:
  //
  // http://www.immersion.com/developer/downloads/ImmFundamentals/HTML/
  // http://msdn.microsoft.com/en-us/library/bb219655(VS.85).aspx
  // http://github.com/github/linux-2.6/blob/f3b8436ad9a8ad36b3c9fa1fe030c7f38e5d3d0b/Documentation/input/ff.txt
  // /usr/include/linux/input.h
  //
  // Since documentation is a little lacking, some of the emulation is
  // likely be wrong.

  delay  = effect.replay.delay;
  length = effect.replay.length;

  switch(effect.type)
    {
      case FF_CONSTANT:
        start_weak_magnitude   = clamp(0, 0x7fff, abs(effect.u.constant.level));
        start_strong_magnitude = clamp(0, 0x7fff, abs(effect.u.constant.level));
        end_weak_magnitude     = clamp(0, 0x7fff, abs(effect.u.constant.level));
        end_strong_magnitude   = clamp(0, 0x7fff, abs(effect.u.constant.level));

        envelope = effect.u.constant.envelope;
        break;

      case FF_PERIODIC:
        start_weak_magnitude   = clamp(0, 0x7fff, abs(effect.u.periodic.magnitude));
        start_strong_magnitude = clamp(0, 0x7fff, abs(effect.u.periodic.magnitude));
        end_weak_magnitude     = clamp(0, 0x7fff, abs(effect.u.periodic.magnitude));
        end_strong_magnitude   = clamp(0, 0x7fff, abs(effect.u.periodic.magnitude));

        envelope = effect.u.periodic.envelope;
        break;

      case FF_RAMP:
        start_weak_magnitude   = clamp(0, 0x7fff, abs(effect.u.ramp.start_level));
        start_strong_magnitude = clamp(0, 0x7fff, abs(effect.u.ramp.start_level));
        end_weak_magnitude     = clamp(0, 0x7fff, abs(effect.u.ramp.end_level));
        end_strong_magnitude   = clamp(0, 0x7fff, abs(effect.u.ramp.end_level));

        envelope = effect.u.ramp.envelope;
        break;

      case FF_RUMBLE:
        start_weak_magnitude   = clamp(0, 0x7fff, effect.u.rumble.weak_magnitude);
        start_strong_magnitude = clamp(0, 0x7fff, effect.u.rumble.strong_magnitude);
        end_weak_magnitude     = clamp(0, 0x7fff, effect.u.rumble.weak_magnitude);
        end_strong_magnitude   = clamp(0, 0x7fff, effect.u.rumble.strong_magnitude);
        break;
        
      default:
        // Unsupported effects
        // case FF_SPRING:
	// case FF_FRICTION:
        // case FF_DAMPER
        // case FF_INERTIA:
        std::cout << "Unsupported effect: " << effect << std::endl;
        start_weak_magnitude   = 0;
        start_strong_magnitude = 0;
        end_weak_magnitude     = 0;
        end_strong_magnitude   = 0;
        break;
    }
}

static int get_pos(int start, int end, int pos, int len)
{
  int rel = end - start;
  return start + (rel * pos / len);
}

void
ForceFeedbackEffect::update(int msec_delta)
{
  if (playing)
    {
      count += msec_delta;

      if (count > delay)
        {
          int t = count - delay;
          if (t < envelope.attack_length)
            { // attack
              strong_magnitude = get_pos(start_strong_magnitude, end_strong_magnitude, t, length);
              weak_magnitude   = get_pos(start_weak_magnitude,   end_weak_magnitude,   t, length);
              
              // apply envelope
              strong_magnitude = ((envelope.attack_level * t) + strong_magnitude * (envelope.attack_length - t)) / envelope.attack_length;
              weak_magnitude   = ((envelope.attack_level * t) + weak_magnitude   * (envelope.attack_length - t)) / envelope.attack_length;
            }
          else if  (t < length - envelope.fade_length)
            { // sustain
              strong_magnitude = get_pos(start_strong_magnitude, end_strong_magnitude, t, length);
              weak_magnitude   = get_pos(start_weak_magnitude,   end_weak_magnitude,   t, length);
            }
          else if (t < length)
            { // fade
              strong_magnitude = get_pos(start_strong_magnitude, end_strong_magnitude, t, length);
              weak_magnitude   = get_pos(start_weak_magnitude,   end_weak_magnitude,   t, length);

              // apply envelope
              int dt = t - (length - envelope.fade_length);
              strong_magnitude = ((envelope.fade_level * dt) + strong_magnitude * (envelope.fade_length - dt)) / envelope.fade_length;
              weak_magnitude   = ((envelope.fade_level * dt) + weak_magnitude   * (envelope.fade_length - dt)) / envelope.fade_length;
            }
          else
            { // effect ended
              stop();
            }
        }
    }
}

void
ForceFeedbackEffect::play()
{
  playing = true;
}

void
ForceFeedbackEffect::stop()
{
  playing = false;
  count = 0;
  weak_magnitude   = 0;
  strong_magnitude = 0;
}

ForceFeedbackHandler::ForceFeedbackHandler()
  : gain(0xFFFF),
    max_effects(16),
    weak_magnitude(0),
    strong_magnitude(0)
{
  
}

ForceFeedbackHandler::~ForceFeedbackHandler()
{
  
}

int
ForceFeedbackHandler::get_max_effects()
{
  return max_effects;
}

void
ForceFeedbackHandler::upload(const struct ff_effect& effect)
{
  if (command_line_options->verbose)
    std::cout << "FF_UPLOAD("
              << "effect_id:" << effect.id
              << ", effect_type:" << effect.type
              << ",\n          "  << effect 
              << ")" << std::endl;

  std::map<int, ForceFeedbackEffect>::iterator i = effects.find(effect.id);
  if (i == effects.end())
    {
      effects[effect.id] = ForceFeedbackEffect(effect);
    }
  else
    {
      ForceFeedbackEffect old_effect = i->second;
      ForceFeedbackEffect new_effect(effect);

      // We the copy state variables of the effect , so we can update
      // the effect while it is playing
      new_effect.playing          = old_effect.playing;
      new_effect.count            = old_effect.count;
      new_effect.weak_magnitude   = old_effect.weak_magnitude;
      new_effect.strong_magnitude = old_effect.strong_magnitude;

      effects[effect.id] = effect;
    }
}

void
ForceFeedbackHandler::erase(int id)
{
  if (command_line_options->verbose)
    std::cout << "FF_ERASE(effect_id:" << id << ")" << std::endl;

  std::map<int, ForceFeedbackEffect>::iterator i = effects.find(id);
  if (i != effects.end())
    effects.erase(i);
  else
    std::cout << "Error: ForceFeedbackHandler::erase: Unknown id " << id << std::endl;
}

void
ForceFeedbackHandler::play(int id)
{
  if (command_line_options->verbose)
    std::cout << "FFPlay(effect_id:" << id << ")" << std::endl;

  std::map<int, ForceFeedbackEffect>::iterator i = effects.find(id);
  if (i != effects.end())
    i->second.play();
  else
    std::cout << "Error: ForceFeedbackHandler::play: Unknown id " << id << std::endl;
}

void
ForceFeedbackHandler::stop(int id)
{
  if (command_line_options->verbose)
    std::cout << "FFStop(effect_id:" << id << ")" << std::endl;

  std::map<int, ForceFeedbackEffect>::iterator i = effects.find(id);
  if (i != effects.end())
    i->second.stop();
  else
    std::cout << "Error: ForceFeedbackHandler::play: Unknown id " << id << std::endl;
}

void
ForceFeedbackHandler::set_gain(int g)
{
  gain = g;
}

void
ForceFeedbackHandler::update(int msec_delta)
{
  weak_magnitude   = 0;
  strong_magnitude = 0;

  if (!effects.empty())
    {
      for(Effects::iterator i = effects.begin(); i != effects.end(); ++i)
        {
          i->second.update(msec_delta);

          weak_magnitude   += i->second.get_weak_magnitude();
          strong_magnitude += i->second.get_strong_magnitude();
        }

      weak_magnitude   = std::min(weak_magnitude,   0x7fff);
      strong_magnitude = std::min(strong_magnitude, 0x7fff);
    }
}

int
ForceFeedbackHandler::get_weak_magnitude() const
{
  return weak_magnitude * gain / 0xffff;
}

int
ForceFeedbackHandler::get_strong_magnitude() const
{
  return strong_magnitude * gain / 0xffff;
}

/* EOF */
