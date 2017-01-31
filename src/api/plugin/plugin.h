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
    <https://www.gnu.org/licenses/>.
    */
#ifndef LOOT_API_PLUGIN_PLUGIN
#define LOOT_API_PLUGIN_PLUGIN

#include <cstdint>
#include <list>
#include <set>
#include <string>
#include <vector>

#include <boost/locale.hpp>
#include <libespm/Plugin.h>

#include "loot/metadata/plugin_metadata.h"
#include "loot/enum/game_type.h"
#include "loot/plugin_interface.h"

namespace loot {
class Game;

class Plugin : public PluginInterface, private libespm::Plugin {
public:
  Plugin(const Game& game, const std::string& name, const bool headerOnly);

  std::string GetName() const;
  std::string GetLowercasedName() const;
  std::string GetVersion() const;
  std::vector<std::string> GetMasters() const;
  std::vector<Message> GetStatusMessages() const;
  std::set<Tag> GetBashTags() const;
  uint32_t GetCRC() const;

  bool IsMaster() const;
  bool IsEmpty() const;
  bool LoadsArchive() const;
  bool DoFormIDsOverlap(const PluginInterface& plugin) const;

  bool IsActive() const;

  //Load ordering functions.
  size_t NumOverrideFormIDs() const;
  std::set<libespm::FormId> OverlapFormIDs(const Plugin& plugin) const;

  // Validity checks.
  static bool IsValid(const std::string& filename, const Game& game);
  static uintmax_t GetFileSize(const std::string& filename, const Game& game);

  bool operator < (const Plugin& rhs) const;
private:
  static libespm::GameId GetLibespmGameId(GameType gameType);

  bool isEmpty_;  // Does the plugin contain any records other than the TES4 header?
  bool isActive_;
  bool loadsArchive_;
  const std::string name_;
  std::string version_;  //Obtained from description field.
  uint32_t crc_;
  std::set<Tag> tags_;
  std::vector<Message> messages_;

  //Useful caches.
  size_t numOverrideRecords_;
};
}

#endif
