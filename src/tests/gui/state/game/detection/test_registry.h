/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014 WrinklyNinja

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
<https://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TESTS_GUI_STATE_GAME_DETECTION_TEST_REGISTRY
#define LOOT_TESTS_GUI_STATE_GAME_DETECTION_TEST_REGISTRY

#include "gui/state/game/detection/registry.h"

namespace loot::test {
class TestRegistry : public RegistryInterface {
public:
  std::optional<std::string> GetStringValue(
      const RegistryValue&) const override {
    return stringValue_;
  }

  std::vector<std::string> GetSubKeys(const std::string&,
                                      const std::string&) const override {
    return subKeys_;
  }

  void SetStringValue(const std::string& value) { stringValue_ = value; }

  void SetSubKeys(const std::vector<std::string> subKeys) {
    subKeys_ = subKeys;
  }

private:
  std::optional<std::string> stringValue_;
  std::vector<std::string> subKeys_;
};
}

#endif
