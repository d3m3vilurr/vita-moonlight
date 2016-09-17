/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2016 Ilya Zhuravlev, Sunguk Lee, Vasyl Horbachenko
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

enum {
  NO_TOUCH_ACTION = 0,
  ON_SCREEN_TOUCH,
  SCREEN_TAP,
  SWIPE_START,
  ON_SCREEN_SWIPE
} TouchScreenState;

enum {
  TOUCHSEC_NORTHWEST = 600,
  TOUCHSEC_NORTHEAST,
  TOUCHSEC_SOUTHWEST,
  TOUCHSEC_SOUTHEAST,
  TOUCHSEC_SPECIAL_NW = 700,
  TOUCHSEC_SPECIAL_NE,
  TOUCHSEC_SPECIAL_SW,
  TOUCHSEC_SPECIAL_SE
} TouchScreenSection;

enum {
  LEFTX,
  LEFTY,
  RIGHTX,
  RIGHTY
} PadSection;

#define INPUT_TYPE_MASK 0xffff0000
#define INPUT_VALUE_MASK 0x0000ffff

#define INPUT_TYPE_KEYBOARD 0x00000000
#define INPUT_TYPE_SPECIAL 0x00010000
#define INPUT_TYPE_MOUSE 0x00020000
#define INPUT_TYPE_GAMEPAD 0x00030000

enum {
  INPUT_SPECIAL_KEY_PAUSE
};

bool vitainput_init();
void vitainput_config(CONFIGURATION config);

void vitainput_start(void);
void vitainput_stop(void);
