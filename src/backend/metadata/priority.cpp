/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

    This file is part of LOOT.

    LOOT is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.  If not, see
    <http://www.gnu.org/licenses/>.
    */

#include "backend/metadata/priority.h"

namespace loot {
Priority::Priority() : value_(0), isExplicitZeroValue_(false) {}

Priority::Priority(const int value)
  : isExplicitZeroValue_(true) {
  if (value > 127) {
    value_ = 127;
  } else if (value < -127) {
    value_ = -127;
  } else {
    value_ = value;
  }
}

short Priority::getValue() const {
  return value_;
}

bool Priority::isExplicit() const {
  return value_ != 0 || isExplicitZeroValue_;
}

bool Priority::operator < (const Priority& rhs) const {
  return value_ < rhs.value_;
}

bool Priority::operator > (const Priority& rhs) const {
  return value_ > rhs.value_;
}

bool Priority::operator >= (const Priority& rhs) const {
  return value_ >= rhs.value_;
}

bool Priority::operator == (const Priority& rhs) const {
  return value_ == rhs.value_;
}

bool Priority::operator > (const uint8_t rhs) const {
  return value_ > rhs;
}
}
