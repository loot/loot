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
#ifdef _WIN32
HKEY GetRegistryRootKey(const std::string& rootKey) {
  if (rootKey == "HKEY_CLASSES_ROOT")
    return HKEY_CLASSES_ROOT;
  else if (rootKey == "HKEY_CURRENT_CONFIG")
    return HKEY_CURRENT_CONFIG;
  else if (rootKey == "HKEY_CURRENT_USER")
    return HKEY_CURRENT_USER;
  else if (rootKey == "HKEY_LOCAL_MACHINE")
    return HKEY_LOCAL_MACHINE;
  else if (rootKey == "HKEY_USERS")
    return HKEY_USERS;
  else
    throw std::invalid_argument("Invalid registry key given.");
}
#endif
}

namespace loot {
std::optional<std::string> Registry::GetStringValue(
    const RegistryValue& value) const {
#ifdef _WIN32
  HKEY hKey = GetRegistryRootKey(value.rootKey);
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
                         ToWinWide(value.subKey).c_str(),
                         ToWinWide(value.valueName).c_str(),
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
                      ToWinWide(value.subKey).c_str(),
                      ToWinWide(value.valueName).c_str(),
                      RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY,
                      NULL,
                      &wstr[0],
                      &len);
  }

  if (ret == ERROR_SUCCESS) {
    // Passing c_str() cuts off any unused buffer.
    std::string stringValue = FromWinWide(wstr.c_str());
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
#else
  return std::nullopt;
#endif
}

std::vector<std::string> Registry::GetSubKeys(const std::string& rootKey,
                                              const std::string& subKey) const {
#ifdef _WIN32
  const auto logger = getLogger();
  if (logger) {
    logger->trace("Getting subkey names for registry key and subkey: {}, {}",
                  rootKey,
                  subKey);
  }

  HKEY hKey;
  auto status = RegOpenKeyEx(GetRegistryRootKey(rootKey),
                             ToWinWide(subKey).c_str(),
                             0,
                             KEY_ENUMERATE_SUB_KEYS,
                             &hKey);

  if (status != ERROR_SUCCESS) {
    if (logger) {
      // system_error gets the details from Windows.
      const auto error =
          std::system_error(GetLastError(), std::system_category());

      logger->warn("Failed to open the Registry key \"{}\": {}",
                   rootKey + "\\" + subKey,
                   error.what());
    }

    // Don't throw because failure could be because the key simply does not
    // exist, which is an unexceptional failure state.
    return {};
  }

  std::vector<std::string> subKeyNames;
  DWORD subKeyIndex = 0;
  DWORD len = MAX_PATH;
  std::wstring subKeyName(MAX_PATH, 0);
  status = ERROR_SUCCESS;

  while (status == ERROR_SUCCESS) {
    status = RegEnumKeyEx(hKey,
                          subKeyIndex,
                          &subKeyName[0],
                          &len,
                          nullptr,
                          nullptr,
                          nullptr,
                          nullptr);

    if (status != ERROR_SUCCESS && status != ERROR_NO_MORE_ITEMS) {
      RegCloseKey(hKey);

      throw std::system_error(
          GetLastError(),
          std::system_category(),
          "Failed to get the subkeys of the Registry key: " + rootKey + "\\" +
              subKey);
    }

    subKeyNames.push_back(FromWinWide(subKeyName.c_str()));

    subKeyIndex += 1;
  }

  RegCloseKey(hKey);

  return subKeyNames;
#else
  return {};
#endif
}

std::optional<std::filesystem::path> ReadPathFromRegistry(
    const RegistryInterface& registry,
    const RegistryValue& value) {
  try {
    const auto installedPath = registry.GetStringValue(value);

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

std::vector<std::filesystem::path> FindGameInstallPathsInRegistry(
    const RegistryInterface& registry,
    const std::vector<RegistryValue>& registryValues) {
  std::vector<std::filesystem::path> installPaths;

  for (const auto& registryValue : registryValues) {
    const auto path = ReadPathFromRegistry(registry, registryValue);

    if (path.has_value()) {
      installPaths.push_back(path.value());
    }
  }

  return installPaths;
}
}
