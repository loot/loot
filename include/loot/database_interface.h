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
#ifndef LOOT_DATABASE_INTERFACE
#define LOOT_DATABASE_INTERFACE

#include <string>
#include <vector>

#include "loot/language_code.h"
#include "loot/masterlist_info.h"
#include "loot/plugin_cleanliness.h"
#include "loot/plugin_tags.h"
#include "loot/simple_message.h"

namespace loot {
/** @brief The interface provided by API's database handle. */
class DatabaseInterface {
public:
  /**
   *  @name Data Loading
   *  @{
   */

  /**
   *  @brief Loads the masterlist and userlist from the paths specified.
   *  @details Can be called multiple times, each time replacing the
   *           previously-loaded data.
   *  @param masterlist_path
   *         A string containing the relative or absolute path to the masterlist
   *         file that should be loaded.
   *  @param userlist_path
   *         A string containing the relative or absolute path to the userlist
   *         file that should be loaded, or an empty string. If an empty string,
   *         no userlist will be loaded.
   */
  virtual void LoadLists(const std::string& masterlist_path,
                         const std::string& userlist_path = "") = 0;

  /**
   *  @brief Evaluates all conditions and regular expression metadata entries.
   *  @details Repeated calls re-evaluate the metadata from scratch. This
   *           function affects the output of all the database access functions.
   */
  virtual void EvalLists() = 0;

  /**
   *  @}
   *  @name Sorting
   *  @{
   */

  /**
   *  @brief Calculates a new load order for the game's installed plugins
   *         (including inactive plugins) and outputs the sorted order.
   *  @details Pulls metadata from the masterlist and userlist if they are
   *           loaded, and reads the contents of each plugin. No changes are
   *           applied to the load order used by the game. This function does
   *           not load or evaluate the masterlist or userlist.
   *  @param plugins
   *         A vector of filenames of the plugins to sort.
   *  @returns A vector of the given plugin filenames in their sorted load
   *           order.
   */
  virtual std::vector<std::string> SortPlugins(const std::vector<std::string>& plugins) = 0;

  /**
   *  @}
   *  @name Masterlist Update
   *  @{
   */

  /**
   *  @brief Update the given masterlist.
   *  @details Uses Git to update the given masterlist to a given remote.
   *           If the masterlist doesn't exist, this will create it. This
   *           function also initialises a Git repository in the given
   *           masterlist's parent folder. If the masterlist was not already
   *           up-to-date, it will be re-loaded, but not re-evaluated.
   *
   *           If a Git repository is already present, it will be used to
   *           perform a diff-only update, but if for any reason a
   *           fast-forward merge update is not possible, the existing
   *           repository will be deleted and a new repository cloned from
   *           the given remote.
   *  @param masterlist_path
   *         A string containing the relative or absolute path to the masterlist
   *         file that should be updated. The filename must match the filename
   *         of the masterlist file in the given remote repository, otherwise it
   *         will not be updated correctly. Although LOOT itself expects this
   *         filename to be "masterlist.yaml", the API does not check for any
   *         specific filename.
   *  @param remote_url
   *         The URL of the remote from which to fetch updates. This can also be
   *         a relative or absolute path to a local repository.
   *  @param remote_branch
   *         The branch of the remote from which to apply updates. LOOT's
   *         official masterlists are versioned using separate branches for each
   *         new version of the masterlist syntax, so if you're using them,
   *         check their repositories to see which is the latest release branch.
   *  @returns `true` if the masterlist was updated. `false` if no update was
   *           necessary, ie. it was already up-to-date. If `true`, the
   *           masterlist will have been re-loaded, but will need to be
   *           re-evaluated separately.
   */
  virtual bool UpdateMasterlist(const std::string& masterlist_path,
                                const std::string& remote_url,
                                const std::string& remote_branch) = 0;

  /**
   *  @brief Get the given masterlist's revision.
   *  @details Getting a masterlist's revision is only possible if it is found
   *           inside a local Git repository.
   *  @param masterlist_path
   *         A string containing the relative or absolute path to the masterlist
   *         file that should be queried.
   *  @param get_short_id
   *         If `true`, the shortest unique hexadecimal revision hash that is at
   *         least 7 characters long will be outputted. Otherwise, the full 40
   *         character hash will be outputted.
   *  @returns The revision data.
   */
  virtual MasterlistInfo GetMasterlistRevision(const std::string& masterlist_path,
                                               const bool get_short_id) = 0;

  /**
   *  @}
   *  @name Plugin Data Access
   *  @{
   */

  /**
   *  @brief Outputs the Bash Tags suggested for addition and removal by the
   *         database for the given plugin.
   *  @param plugin
   *         The filename of the plugin to look up Bash Tag suggestions for.
   *  @returns Bash Tag data for the plugin.
  */
  virtual PluginTags GetPluginTags(const std::string& plugin) = 0;

  /**
   *  @brief Outputs the messages associated with the given plugin in the
   *         database.
   *  @param plugin
   *         The filename of the plugin to look up messages for.
   *  @param language
   *         The language to use when choosing which message content strings
   *         to return.
   *  @returns A vector of messages associated with the specified plugin. Empty
   *           if the plugin has no messages associated with it.
   */
  virtual std::vector<SimpleMessage> GetPluginMessages(const std::string& plugin,
                                                       const LanguageCode language) = 0;

  /**
   *  @brief Determines the database's knowledge of a plugin's cleanliness.
   *  @details Outputs whether the plugin should be cleaned or not, or if
   *           no data is available. The mechanism used to determine that
   *           a plugin should not be cleaned is not very reliable, and is
   *           likely to fail if `EvalLists()` was called with a
   *           language other than English. As such, some plugins that should
   *           not be cleaned may have the `PluginCleanliness::unknown`
   *           code outputted.
   *  @param plugin
   *      The plugin to look up the cleanliness state for.
   *  @returns A plugin cleanliness code.
   */
  virtual PluginCleanliness GetPluginCleanliness(const std::string& plugin) = 0;

  /**
   *  @}
   *  @name Miscellaneous
   *  @{
   */

  /**
  *  @brief Writes a minimal metadata file that only contains plugins with
  *         Bash Tag suggestions and/or dirty info, plus the suggestions and
  *         info themselves.
  *  @param outputFile
  *         The path to which the file shall be written.
  *  @param overwrite
  *         If `false` and `outputFile` already exists, no data will be
  *         written. Otherwise, data will be written.
  */
  virtual void WriteMinimalList(const std::string& outputFile,
                                const bool overwrite) = 0;

  /** @} */
};
}

#endif
