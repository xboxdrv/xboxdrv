// uinpp - Linux uinput library for C++
// Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "force_feedback_handler.hpp"

#include <algorithm>
#include <cmath>

#include <logmich/log.hpp>

#include <format>
#include <string>

// Assuming the structs and enums (FF_CONSTANT, etc.) are already defined.
template <>
struct std::formatter<ff_envelope> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const ff_envelope& envelope, std::format_context& ctx) const {
    return std::format_to(ctx.out(),
      "Envelope(attack_length:{}, attack_level:{}, fade_length:{}, fade_level:{})",
      envelope.attack_length,
      envelope.attack_level,
      envelope.fade_length,
      envelope.fade_level);
  }
};

template <>
struct std::formatter<ff_replay> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const ff_replay& replay, std::format_context& ctx) const {
    return std::format_to(ctx.out(),
      "Replay(length:{}, delay:{})",
      replay.length,
      replay.delay);
  }
};

template <>
struct std::formatter<ff_trigger> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const ff_trigger& trigger, std::format_context& ctx) const {
    return std::format_to(ctx.out(),
      "Trigger(button:{}, interval:{})",
      trigger.button,
      trigger.interval);
  }
};

template <>
struct std::formatter<ff_effect> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const ff_effect& effect, std::format_context& ctx) const {
    std::string type_str;

    switch (effect.type) {
      case FF_CONSTANT:
        type_str = std::format("FF_CONSTANT(level:{}, envelope:{})",
                               effect.u.constant.level,
                               effect.u.constant.envelope);
        break;
      case FF_PERIODIC:
        type_str = std::format(
          "FF_PERIODIC(waveform:{}, period:{}, magnitude:{}, offset:{}, phase:{}, envelope:{})",
          effect.u.periodic.waveform,
          effect.u.periodic.period,
          effect.u.periodic.magnitude,
          effect.u.periodic.offset,
          effect.u.periodic.phase,
          effect.u.periodic.envelope);
        break;
      case FF_RAMP:
        type_str = std::format("FF_RAMP(start_level:{}, end_level:{}, envelope:{})",
                               effect.u.ramp.start_level,
                               effect.u.ramp.end_level,
                               effect.u.ramp.envelope);
        break;
      case FF_SPRING:
        type_str = "FF_SPRING()";
        break;
      case FF_FRICTION:
        type_str = "FF_FRICTION()";
        break;
      case FF_DAMPER:
        type_str = "FF_DAMPER()";
        break;
      case FF_RUMBLE:
        type_str = std::format("FF_RUMBLE(strong_magnitude:{}, weak_magnitude:{})",
                               effect.u.rumble.strong_magnitude,
                               effect.u.rumble.weak_magnitude);
        break;
      case FF_INERTIA:
        type_str = "FF_INERTIA()";
        break;
      case FF_CUSTOM:
        type_str = "FF_CUSTOM()";
        break;
      default:
        type_str = "FF_<unknown>()";
        break;
    }

    return std::format_to(ctx.out(),
      "Effect({}, direction:{}, replay:{}, trigger:{})",
      type_str,
      effect.direction,
      effect.replay,
      effect.trigger);
  }
};

namespace uinpp {

namespace {

int get_pos(int start, int end, int pos, int len)
{
  int rel = end - start;
  return start + (rel * pos / len);
}

} // namespace

ForceFeedbackEffect::ForceFeedbackEffect() :
  delay(),
  length(),
  start_strong_magnitude(),
  start_weak_magnitude(),
  end_strong_magnitude(),
  end_weak_magnitude(),
  envelope(),
  playing(false),
  count(0),
  weak_magnitude(0),
  strong_magnitude(0)
{
}

ForceFeedbackEffect::ForceFeedbackEffect(const struct ff_effect& effect) :
  delay(),
  length(),
  start_strong_magnitude(),
  start_weak_magnitude(),
  end_strong_magnitude(),
  end_weak_magnitude(),
  envelope(),
  playing(false),
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
      start_weak_magnitude   = std::clamp(std::abs(effect.u.constant.level), 0, 0x7fff);
      start_strong_magnitude = std::clamp(std::abs(effect.u.constant.level), 0, 0x7fff);
      end_weak_magnitude     = std::clamp(std::abs(effect.u.constant.level), 0, 0x7fff);
      end_strong_magnitude   = std::clamp(std::abs(effect.u.constant.level), 0, 0x7fff);

      envelope = effect.u.constant.envelope;
      break;

    case FF_PERIODIC:
      start_weak_magnitude   = std::clamp(std::abs(effect.u.periodic.magnitude), 0, 0x7fff);
      start_strong_magnitude = std::clamp(std::abs(effect.u.periodic.magnitude), 0, 0x7fff);
      end_weak_magnitude     = std::clamp(std::abs(effect.u.periodic.magnitude), 0, 0x7fff);
      end_strong_magnitude   = std::clamp(std::abs(effect.u.periodic.magnitude), 0, 0x7fff);

      envelope = effect.u.periodic.envelope;
      break;

    case FF_RAMP:
      start_weak_magnitude   = std::clamp(std::abs(effect.u.ramp.start_level), 0, 0x7fff);
      start_strong_magnitude = std::clamp(std::abs(effect.u.ramp.start_level), 0, 0x7fff);
      end_weak_magnitude     = std::clamp(std::abs(effect.u.ramp.end_level), 0, 0x7fff);
      end_strong_magnitude   = std::clamp(std::abs(effect.u.ramp.end_level), 0, 0x7fff);

      envelope = effect.u.ramp.envelope;
      break;

    case FF_RUMBLE:
      start_weak_magnitude   = std::clamp(static_cast<int>(effect.u.rumble.weak_magnitude), 0, 0x7fff);
      start_strong_magnitude = std::clamp(static_cast<int>(effect.u.rumble.strong_magnitude), 0, 0x7fff);
      end_weak_magnitude     = std::clamp(static_cast<int>(effect.u.rumble.weak_magnitude), 0, 0x7fff);
      end_strong_magnitude   = std::clamp(static_cast<int>(effect.u.rumble.strong_magnitude), 0, 0x7fff);
      break;

    default:
      // Unsupported effects
      // case FF_SPRING:
      // case FF_FRICTION:
      // case FF_DAMPER
      // case FF_INERTIA:
      log_info("unsupported effect: {}", std::format("{}", effect));
      start_weak_magnitude   = 0;
      start_strong_magnitude = 0;
      end_weak_magnitude     = 0;
      end_strong_magnitude   = 0;
      break;
  }
}

void
ForceFeedbackEffect::update(int msec_delta)
{
  if (!playing)
  {
    return;
  }

  count += msec_delta;
  if (count <= delay)
  {
    return;
  }

  int t = count - delay;

  // Linux FF: replay.length == 0 means infinite (until FF_STOP / effect removal).
  // Timed path would treat t < 0 / t < length as finished and stop() immediately.
  if (length == 0)
  {
    if (envelope.attack_length > 0 && t < envelope.attack_length)
    {
      // One-shot attack into the steady level, then hold forever.
      strong_magnitude = get_pos(start_strong_magnitude, end_strong_magnitude, t, envelope.attack_length);
      weak_magnitude   = get_pos(start_weak_magnitude,   end_weak_magnitude,   t, envelope.attack_length);
      strong_magnitude = ((envelope.attack_level * t) + strong_magnitude * (envelope.attack_length - t)) / envelope.attack_length;
      weak_magnitude   = ((envelope.attack_level * t) + weak_magnitude   * (envelope.attack_length - t)) / envelope.attack_length;
    }
    else
    {
      strong_magnitude = end_strong_magnitude;
      weak_magnitude   = end_weak_magnitude;
    }
    return;
  }

  if (t < envelope.attack_length)
  { // attack
    strong_magnitude = get_pos(start_strong_magnitude, end_strong_magnitude, t, length);
    weak_magnitude   = get_pos(start_weak_magnitude,   end_weak_magnitude,   t, length);

    // apply envelope
    strong_magnitude = ((envelope.attack_level * t) + strong_magnitude * (envelope.attack_length - t)) / envelope.attack_length;
    weak_magnitude   = ((envelope.attack_level * t) + weak_magnitude   * (envelope.attack_length - t)) / envelope.attack_length;
  }
  else if (t < length - envelope.fade_length)
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

ForceFeedbackHandler::ForceFeedbackHandler() :
  gain(0xFFFF),
  max_effects(16),
  effects(),
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
  log_debug("FF_UPLOAD(effect_id: {}, effect_type: {}, effect: {})",
            effect.id, effect.type, std::format("{}", effect));

  auto const i = effects.find(effect.id);
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
  log_debug("FF_ERASE(effect_id: {})", id);

  auto const i = effects.find(id);
  if (i != effects.end())
  {
    effects.erase(i);
  }
  else
  {
    log_warn("unknown id {}", id);
  }
}

void
ForceFeedbackHandler::play(int id)
{
  log_debug("FFPlay(effect_id: })", id);

  auto const i = effects.find(id);
  if (i != effects.end())
  {
    i->second.play();
  }
  else
  {
    log_warn("unknown id {}", id);
  }
}

void
ForceFeedbackHandler::stop(int id)
{
  log_debug("FFStop(effect_id:{})", id);

  auto const i = effects.find(id);
  if (i != effects.end())
  {
    i->second.stop();
  }
  else
  {
    log_warn("unknown id {}", id);
  }
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
    for(auto i = effects.begin(); i != effects.end(); ++i)
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

} // namespace uinpp

/* EOF */
