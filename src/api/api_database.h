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
                         const bool overwrite);

  void WriteMinimalList(const std::string& outputFile,
                        const bool overwrite);

  void IdentifyMainMasterFile(const std::string& masterFile);

  std::vector<std::string> SortPlugins(const std::vector<std::string>& plugins);

  bool UpdateMasterlist(const std::string& masterlist_path,
                        const std::string& remote_url,
                        const std::string& remote_branch);

  MasterlistInfo GetMasterlistRevision(const std::string& masterlist_path,
                                       const bool get_short_id);

  std::set<std::string> GetKnownBashTags();

  std::vector<Message> GetGeneralMessages();

  PluginMetadata GetPluginMetadata(const std::string& plugin,
                                   bool includeUserMetadata = true);

  PluginMetadata GetPluginUserMetadata(const std::string& plugin);

  void SetPluginUserMetadata(const PluginMetadata& pluginMetadata);

  void DiscardPluginUserMetadata(const std::string& plugin);

  void DiscardAllUserMetadata();

  PluginTags GetPluginTags(const std::string& plugin);

  std::vector<SimpleMessage> GetPluginMessages(const std::string& plugin,
                                               const LanguageCode language);

  PluginCleanliness GetPluginCleanliness(const std::string& plugin);
private:
  Game& game_;

  Masterlist unevaluatedMasterlist_;
  MetadataList unevaluatedUserlist_;
};
}

#endif
