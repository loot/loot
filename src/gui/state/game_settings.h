/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2017    WrinklyNinja

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

#ifndef LOOT_GUI_STATE_GAME_SETTINGS
#define LOOT_GUI_STATE_GAME_SETTINGS

#include <string>
#include <list>

#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>

#include "loot/enum/game_type.h"

namespace loot {
class GameSettings {
public:
  GameSettings();
  explicit GameSettings(const GameType gameType, const std::string& lootFolder = "");

  bool IsRepoBranchOldDefault() const;

  bool operator == (const GameSettings& rhs) const;  //Compares names and folder names.

  GameType Type() const;
  std::string Name() const;  //Returns the game's name, eg. "TES IV: Oblivion".
  std::string FolderName() const;
  std::string Master() const;
  std::string RegistryKey() const;
  std::string RepoURL() const;
  std::string RepoBranch() const;
  boost::filesystem::path GamePath() const;

  GameSettings& SetName(const std::string& name);
  GameSettings& SetMaster(const std::string& masterFile);
  GameSettings& SetRegistryKey(const std::string& registry);
  GameSettings& SetRepoURL(const std::string& repositoryURL);
  GameSettings& SetRepoBranch(const std::string& repositoryBranch);
  GameSettings& SetGamePath(const boost::filesystem::path& path);

private:
  static const std::set<std::string> oldDefaultBranches;

  GameType type_;
  std::string name_;
  std::string masterFile_;

  std::string registryKey_;

  std::string lootFolderName_;
  std::string repositoryURL_;
  std::string repositoryBranch_;

  boost::filesystem::path gamePath_;  //Path to the game's folder.
};
}

namespace YAML {
template<>
struct convert<loot::GameSettings> {
  static Node encode(const loot::GameSettings& rhs) {
    Node node;

    node["type"] = loot::GameSettings(rhs.Type()).FolderName();
    node["name"] = rhs.Name();
    node["folder"] = rhs.FolderName();
    node["master"] = rhs.Master();
    node["repo"] = rhs.RepoURL();
    node["branch"] = rhs.RepoBranch();
    node["path"] = rhs.GamePath().string();
    node["registry"] = rhs.RegistryKey();

    return node;
  }

  static bool decode(const Node& node, loot::GameSettings& rhs) {
    using loot::GameSettings;
    using loot::GameType;

    if (!node.IsMap())
      throw RepresentationException(node.Mark(), "bad conversion: 'game settings' object must be a map");
    if (!node["folder"])
      throw RepresentationException(node.Mark(), "bad conversion: 'folder' key missing from 'game settings' object");
    if (!node["type"])
      throw RepresentationException(node.Mark(), "bad conversion: 'type' key missing from 'game settings' object");

    if (node["type"].as<std::string>() == GameSettings(GameType::tes4).FolderName())
      rhs = GameSettings(GameType::tes4, node["folder"].as<std::string>());
    else if (node["type"].as<std::string>() == GameSettings(GameType::tes5).FolderName())
      rhs = GameSettings(GameType::tes5, node["folder"].as<std::string>());
    else if (node["type"].as<std::string>() == GameSettings(GameType::tes5se).FolderName())
      rhs = GameSettings(GameType::tes5se, node["folder"].as<std::string>());
    else if (node["type"].as<std::string>() == GameSettings(GameType::fo3).FolderName())
      rhs = GameSettings(GameType::fo3, node["folder"].as<std::string>());
    else if (node["type"].as<std::string>() == GameSettings(GameType::fonv).FolderName())
      rhs = GameSettings(GameType::fonv, node["folder"].as<std::string>());
    else if (node["type"].as<std::string>() == GameSettings(GameType::fo4).FolderName())
      rhs = GameSettings(GameType::fo4, node["folder"].as<std::string>());
    else
      throw RepresentationException(node.Mark(), "bad conversion: invalid value for 'type' key in 'game settings' object");

    if (node["name"])
      rhs.SetName(node["name"].as<std::string>());
    if (node["master"])
      rhs.SetMaster(node["master"].as<std::string>());
    if (node["repo"])
      rhs.SetRepoURL(node["repo"].as<std::string>());
    if (node["branch"])
      rhs.SetRepoBranch(node["branch"].as<std::string>());
    if (node["path"])
      rhs.SetGamePath(node["path"].as<std::string>());
    if (node["registry"])
      rhs.SetRegistryKey(node["registry"].as<std::string>());

    return true;
  }
};

Emitter& operator << (Emitter& out, const loot::GameSettings& rhs);
}

#endif
