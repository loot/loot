/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2016    WrinklyNinja

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

#ifndef LOOT_API_LOOT_DB
#define LOOT_API_LOOT_DB

#include <list>
#include <string>
#include <vector>

#include "api/game/game.h"
#include "loot/database_interface.h"

namespace loot {
struct ApiDatabase : public DatabaseInterface {
  ApiDatabase(Game& game);

  void LoadLists(const std::string& masterlist_path,
                 const std::string& userlist_path = "");

  void EvalLists();

  void WriteUserMetadata(const std::string& outputFile,
                         const bool overwrite) const;

  void WriteMinimalList(const std::string& outputFile,
                        const bool overwrite) const;

  bool UpdateMasterlist(const std::string& masterlist_path,
                        const std::string& remote_url,
                        const std::string& remote_branch);

  MasterlistInfo GetMasterlistRevision(const std::string& masterlist_path,
                                       const bool get_short_id) const;

  std::set<std::string> GetKnownBashTags() const;

  std::vector<Message> GetGeneralMessages(bool evaluateConditions = false) const;

  PluginMetadata GetPluginMetadata(const std::string& plugin,
                                   bool includeUserMetadata = true,
                                   bool evaluateConditions = false) const;

  PluginMetadata GetPluginUserMetadata(const std::string& plugin,
                                       bool evaluateConditions = false) const;

  void SetPluginUserMetadata(const PluginMetadata& pluginMetadata);

  void DiscardPluginUserMetadata(const std::string& plugin);

  void DiscardAllUserMetadata();
private:
  Game& game_;
};
}

#endif
