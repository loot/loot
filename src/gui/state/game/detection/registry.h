/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2012 WrinklyNinja

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

#ifndef LOOT_GUI_STATE_GAME_DETECTION_REGISTRY
#define LOOT_GUI_STATE_GAME_DETECTION_REGISTRY

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace loot {
struct RegistryValue {
  std::string rootKey;
  std::string subKey;
  std::string valueName;
};

class RegistryInterface {
public:
  virtual ~RegistryInterface() = default;

  virtual std::optional<std::string> GetStringValue(
      const RegistryValue& value) const = 0;

  virtual std::vector<std::string> GetSubKeys(
      const std::string& rootKey,
      const std::string& subKey) const = 0;
};

class Registry : public RegistryInterface {
  std::optional<std::string> GetStringValue(
      const RegistryValue& value) const override;

  std::vector<std::string> GetSubKeys(const std::string& rootKey,
                                      const std::string& subKey) const override;
};

std::optional<std::filesystem::path> ReadPathFromRegistry(
    const RegistryInterface& registry,
    const RegistryValue& value);

std::vector<std::filesystem::path> FindGameInstallPathsInRegistry(
    const RegistryInterface& registry,
    const std::vector<RegistryValue>& registryValues);
}

#endif
