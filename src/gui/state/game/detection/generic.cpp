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

#include "gui/state/game/detection/generic.h"

#include <loot/enum/game_type.h>

#include "gui/state/game/detection/common.h"
#include "gui/state/game/detection/game_install.h"
#include "gui/state/game/detection/gog.h"
#include "gui/state/game/detection/registry.h"

namespace {
using loot::GameId;
using loot::GameInstall;
using loot::InstallSource;

loot::RegistryValue GetRegistryValue(const GameId gameId) {
  switch (gameId) {
    case GameId::tes3:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Bethesda Softworks\\Morrowind",
              "Installed Path"};
    case GameId::tes4:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Bethesda Softworks\\Oblivion",
              "Installed Path"};
    case GameId::nehrim:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim"
              " - At Fate's Edge_is1",
              "InstallLocation"};
    case GameId::tes5:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Bethesda Softworks\\Skyrim",
              "Installed Path"};
    case GameId::enderal:
      return {"HKEY_CURRENT_USER", "SOFTWARE\\SureAI\\Enderal", "Install_Path"};
    case GameId::tes5se:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Bethesda Softworks\\Skyrim Special Edition",
              "Installed "
              "Path"};
    case GameId::enderalse:
      return {
          "HKEY_CURRENT_USER", "SOFTWARE\\SureAI\\EnderalSE", "Install_Path"};
    case GameId::tes5vr:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Bethesda Softworks\\Skyrim VR",
              "Installed Path"};
    case GameId::fo3:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Bethesda Softworks\\Fallout3",
              "Installed Path"};
    case GameId::fonv:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Bethesda Softworks\\FalloutNV",
              "Installed Path"};
    case GameId::fo4:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Bethesda Softworks\\Fallout4",
              "Installed Path"};
    case GameId::fo4vr:
      return {"HKEY_LOCAL_MACHINE",
              "Software\\Bethesda Softworks\\Fallout 4 VR",
              "Installed Path"};
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

bool IsSteamInstall(const GameId gameId,
                    const std::filesystem::path& installPath) {
  switch (gameId) {
    case GameId::tes3:
      return std::filesystem::exists(installPath / "steam_autocloud.vdf");
    case GameId::nehrim:
      return std::filesystem::exists(installPath / "steam_api.dll");
    case GameId::tes5:
    case GameId::tes5vr:
    case GameId::fo4vr:
      // Only released on Steam.
      return true;
    case GameId::tes4:
    case GameId::tes5se:
    case GameId::enderal:
    case GameId::enderalse:
    case GameId::fo3:
    case GameId::fonv:
    case GameId::fo4:
      // Most games have an installscript.vdf file in their Steam install.
      return std::filesystem::exists(installPath / "installscript.vdf");
    default:
      throw std::logic_error("Unrecognised game ID");
  }
}

bool IsGogInstall(const GameId gameId,
                  const std::filesystem::path& installPath) {
  const auto gogGameIds = loot::gog::GetGogGameIds(gameId);

  for (const auto& gogGameId : gogGameIds) {
    const auto iconPath =
        installPath / std::filesystem::u8path("goggame-" + gogGameId + ".ico");

    if (std::filesystem::exists(iconPath)) {
      return true;
    }
  }

  return false;
}

bool IsEpicInstall(const GameId gameId,
                   const std::filesystem::path& installPath) {
  switch (gameId) {
    case GameId::tes5se:
      return std::filesystem::exists(installPath / "EOSSDK-Win64-Shipping.dll");
    case GameId::fo3:
      return std::filesystem::exists(installPath / "FalloutLauncherEpic.exe");
    case GameId::fonv:
      return std::filesystem::exists(installPath / "EOSSDK-Win32-Shipping.dll");
    default:
      return false;
  }
}

bool IsMicrosoftInstall(const GameId gameId,
                        const std::filesystem::path& installPath) {
  switch (gameId) {
    case GameId::tes3:
    case GameId::tes4:
    case GameId::fo3:
    case GameId::fonv:
      // tes3, tes4, fo3 and fonv install paths are localised, with the
      // appxmanifest.xml file sitting in the parent directory.
      return std::filesystem::exists(installPath.parent_path() /
                                     "appxmanifest.xml");
    case GameId::tes5se:
    case GameId::fo4:
      return std::filesystem::exists(installPath / "appxmanifest.xml");
    default:
      return false;
  }
}

std::optional<GameInstall> FindGameInstallInRegistry(
    const loot::RegistryInterface& registry,
    const GameId gameId) {
  const auto path =
      loot::ReadPathFromRegistry(registry, GetRegistryValue(gameId));

  if (path.has_value() &&
      IsValidGamePath(gameId, GetMasterFilename(gameId), path.value())) {
    // Need to check what source the game is from.
    // The generic registry keys are not written by EGS or the MS Store,
    // so treat anything other than Steam and GOG as unknown.

    if (IsSteamInstall(gameId, path.value())) {
      return GameInstall{
          gameId, InstallSource::steam, path.value(), std::filesystem::path()};
    }

    if (IsGogInstall(gameId, path.value())) {
      return GameInstall{
          gameId, InstallSource::gog, path.value(), std::filesystem::path()};
    }

    return GameInstall{
        gameId, InstallSource::unknown, path.value(), std::filesystem::path()};
  }

  return std::nullopt;
}

std::optional<GameInstall> FindSiblingGameInstall(const GameId gameId) {
  const auto path = std::filesystem::current_path().parent_path();

  if (!IsValidGamePath(gameId, GetMasterFilename(gameId), path)) {
    return std::nullopt;
  }

  if (IsSteamInstall(gameId, path)) {
    return GameInstall{
        gameId, InstallSource::steam, path, std::filesystem::path()};
  }

  if (IsGogInstall(gameId, path)) {
    return GameInstall{
        gameId, InstallSource::gog, path, std::filesystem::path()};
  }

  if (IsEpicInstall(gameId, path)) {
    return GameInstall{
        gameId, InstallSource::epic, path, std::filesystem::path()};
  }

  if (IsMicrosoftInstall(gameId, path)) {
    return GameInstall{
        gameId, InstallSource::microsoft, path, std::filesystem::path()};
  }

  return GameInstall{
      gameId, InstallSource::unknown, path, std::filesystem::path()};
}
}

namespace loot::generic {
bool IsMicrosoftInstall(const GameId gameId,
                        const std::filesystem::path& installPath) {
  return ::IsMicrosoftInstall(gameId, installPath);
}

std::vector<GameInstall> FindGameInstalls(const RegistryInterface& registry,
                                          const GameId gameId) {
  std::vector<GameInstall> installs;

  const auto sibling = FindSiblingGameInstall(gameId);
  if (sibling.has_value()) {
    installs.push_back(sibling.value());
  }

  const auto registryInstall = FindGameInstallInRegistry(registry, gameId);
  if (registryInstall.has_value()) {
    installs.push_back(registryInstall.value());
  }

  return installs;
}

// Check if the given game settings resolve to an installed game, and
// detect its ID and install source.
std::optional<GameInstall> DetectGameInstall(const GameSettings& settings) {
  if (!IsValidGamePath(settings.Id(), settings.Master(), settings.GamePath())) {
    return std::nullopt;
  }

  const auto gameId = settings.Id();
  const auto installPath = settings.GamePath();

  if (IsSteamInstall(gameId, installPath)) {
    return GameInstall{
        gameId, InstallSource::steam, installPath, settings.GameLocalPath()};
  }

  if (IsGogInstall(gameId, installPath)) {
    return GameInstall{
        gameId, InstallSource::gog, installPath, settings.GameLocalPath()};
  }

  if (IsEpicInstall(gameId, installPath)) {
    return GameInstall{
        gameId, InstallSource::epic, installPath, settings.GameLocalPath()};
  }

  if (::IsMicrosoftInstall(gameId, installPath)) {
    return GameInstall{gameId,
                       InstallSource::microsoft,
                       installPath,
                       settings.GameLocalPath()};
  }

  return GameInstall{
      gameId, InstallSource::unknown, installPath, settings.GameLocalPath()};
}
}
