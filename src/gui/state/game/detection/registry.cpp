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

#include "gui/state/game/detection/registry.h"

#include "gui/helpers.h"
#include "gui/state/game/detection/common.h"
#include "gui/state/game/game_settings.h"
#include "gui/state/logging.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {
using loot::RegistryRootKey;

#ifdef _WIN32
HKEY getRegistryRootKey(const RegistryRootKey rootKey) {
  if (rootKey == RegistryRootKey::CURRENT_USER) {
    return HKEY_CURRENT_USER;
  } else if (rootKey == RegistryRootKey::LOCAL_MACHINE) {
    return HKEY_LOCAL_MACHINE;
  } else {
    throw std::invalid_argument("Invalid registry key given.");
  }
}
#endif
}

namespace loot {
std::string format_as(RegistryRootKey rootKey) {
  if (rootKey == RegistryRootKey::CURRENT_USER) {
    return "HKEY_CURRENT_USER";
  } else if (rootKey == RegistryRootKey::LOCAL_MACHINE) {
    return "HKEY_LOCAL_MACHINE";
  } else {
    throw std::invalid_argument("Invalid registry key given.");
  }
}

#ifdef _WIN32
std::optional<std::string> Registry::getStringValue(
    const RegistryValue& value) const {
  HKEY hKey = getRegistryRootKey(value.rootKey);
  DWORD len = MAX_PATH;
  std::wstring wstr(MAX_PATH, 0);

  auto logger = getLogger();
  if (logger) {
    logger->trace(
        "Getting string for registry key, subkey and value: {}, {}, "
        "{}",
        value.rootKey,
        value.subKey,
        value.valueName);
  }

  LONG ret = RegGetValue(hKey,
                         toWinWide(value.subKey).c_str(),
                         toWinWide(value.valueName).c_str(),
                         RRF_RT_REG_SZ | RRF_SUBKEY_WOW6432KEY,
                         NULL,
                         &wstr[0],
                         &len);

  if (ret != ERROR_SUCCESS) {
    // Try again using the native registry view. On 32-bit Windows
    // this just does the same thing again. I don't think it's worth
    // trying to skip for the few 32-bit Windows users that remain.
    logger->debug(
        "Failed to get string value from 32-bit Registry view, trying 64-bit "
        "Registry view.");
    ret = RegGetValue(hKey,
                      toWinWide(value.subKey).c_str(),
                      toWinWide(value.valueName).c_str(),
                      RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY,
                      NULL,
                      &wstr[0],
                      &len);
  }

  if (ret == ERROR_SUCCESS) {
    // Passing c_str() cuts off any unused buffer.
    std::string stringValue = fromWinWide(wstr.c_str());
    if (logger) {
      logger->debug("Found string: {}", stringValue);
    }
    return stringValue;
  } else {
    if (logger) {
      logger->debug("Failed to get string value.");
    }
    return std::nullopt;
  }
}
#else
std::optional<std::string> Registry::getStringValue(
    [[maybe_unused]] const RegistryValue& value) const {
  return std::nullopt;
}
#endif

std::optional<std::filesystem::path> readPathFromRegistry(
    const RegistryInterface& registry,
    const RegistryValue& value) {
  try {
    const auto installedPath = registry.getStringValue(value);

    if (installedPath.has_value()) {
      return std::filesystem::u8path(installedPath.value());
    }
  } catch (const std::exception& e) {
    const auto logger = getLogger();
    if (logger) {
      logger->error(
          "Error while trying to look up the registry key \"{}\\{}\\{}\": {}",
          value.rootKey,
          value.subKey,
          value.valueName,
          e.what());
    }
  }

  return std::nullopt;
}

std::vector<std::filesystem::path> findGameInstallPathsInRegistry(
    const RegistryInterface& registry,
    const std::vector<RegistryValue>& registryValues) {
  std::vector<std::filesystem::path> installPaths;

  for (const auto& registryValue : registryValues) {
    const auto path = readPathFromRegistry(registry, registryValue);

    if (path.has_value()) {
      installPaths.push_back(path.value());
    }
  }

  return installPaths;
}
}
