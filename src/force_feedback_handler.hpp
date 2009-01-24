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

#ifndef HEADER_FF_HANDLER_HPP
#define HEADER_FF_HANDLER_HPP

#include <linux/input.h>
#include <map>

class ForceFeedbackEffect
{
public:
  ForceFeedbackEffect();
  ForceFeedbackEffect(const struct ff_effect& e);

  // Delay before the effect start
  int delay;

  // Length of the effect
  int length;

  // Rumble motor strength
  int start_strong_magnitude;
  int start_weak_magnitude;

  // Rumble motor strength
  int end_strong_magnitude;
  int end_weak_magnitude;
 
  // Envelope
  struct Envelope 
  {
    Envelope()
      : attack_length(0),
        attack_level(0),
        fade_length(0),
        fade_level(0)
    {}

    Envelope(const struct ff_envelope& e) 
      : attack_length(e.attack_length),
        attack_level(e.attack_level),
        fade_length(e.fade_length),
        fade_level(e.fade_level)
    {
    }

    int attack_length;
    int attack_level;

    int fade_length;
    int fade_level;
  } envelope;

  bool playing;
  int  count;
  int  weak_magnitude;
  int  strong_magnitude;

  int  get_weak_magnitude()   const { return weak_magnitude; }
  int  get_strong_magnitude() const { return strong_magnitude; }

  void update(int msec_delta);
  void play();
  void stop();
};

/** */
class ForceFeedbackHandler
{
private:
  int max_effects;
  typedef std::map<int, ForceFeedbackEffect> Effects;
  Effects effects;

  int weak_magnitude;
  int strong_magnitude;

public:
  ForceFeedbackHandler();
  ~ForceFeedbackHandler();

  int get_max_effects();

  void upload(const struct ff_effect& effect);
  void erase(int id);
  
  void play(int id);
  void stop(int id);

  void update(int msec_delta);

  int get_weak_magnitude() const;
  int get_strong_magnitude() const;
};

#endif

/* EOF */
