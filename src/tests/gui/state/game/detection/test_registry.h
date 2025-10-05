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

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "gui/state/game/detection/registry.h"

namespace loot::test {
class TestRegistry : public RegistryInterface {
public:
  std::optional<std::string> getStringValue(
      const RegistryValue& value) const override {
    const auto it = stringValues_.find(value.subKey);
    if (it != stringValues_.end()) {
      return it->second;
    }

    return std::nullopt;
  }

  void SetStringValue(const std::string& subKey, const std::string& value) {
    stringValues_[subKey] = value;
  }

  void SetSubKeys(const std::string& subKey,
                  const std::vector<std::string>& subKeys) {
    subKeys_[subKey] = subKeys;
  }

private:
  std::map<std::string, std::string> stringValues_;
  std::map<std::string, std::vector<std::string>> subKeys_;
};
}

#endif
