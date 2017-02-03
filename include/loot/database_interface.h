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

#include "loot/enum/language_code.h"
#include "loot/enum/plugin_cleanliness.h"
#include "loot/metadata/message.h"
#include "loot/metadata/plugin_metadata.h"
#include "loot/struct/masterlist_info.h"
#include "loot/struct/simple_message.h"

namespace loot {
/** @brief The interface provided by API's database handle. */
class DatabaseInterface {
public:
  /**
   *  @name Data Reading & Writing
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
   * Writes a metadata file containing all loaded user-added metadata.
   * @param outputFile
   *         The path to which the file shall be written.
   * @param overwrite
   *         If `false` and `outputFile` already exists, no data will be
   *         written. Otherwise, data will be written.
   */
  virtual void WriteUserMetadata(const std::string& outputFile,
                                const bool overwrite) const = 0;

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
                                 const bool overwrite) const = 0;

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
                                               const bool get_short_id) const = 0;

  /**
   *  @}
   *  @name Non-plugin Data Access
   *  @{
  */

  /**
   *  @brief Gets the Bash Tags that are listed in the loaded metadata lists.
   *  @details Bash Tag suggestions can include plugins not in this list.
   *  @returns A set of Bash Tag names.
  */
  virtual std::set<std::string> GetKnownBashTags() const = 0;

  /**
   *  @brief Get all general messages listen in the loaded metadata lists.
   *  @returns A vector of messages supplied in the metadata lists but not
   *           attached to any particular plugin.
   */
  virtual std::vector<Message> GetGeneralMessages() const = 0;

  /**
   *  @}
   *  @name Plugin Data Access
   *  @{
   */

  /**
   *  @brief Get all a plugin's loaded metadata.
   *  @param plugin
   *         The filename of the plugin to look up metadata for.
   *  @param includeUserMetadata
   *         If true, any user metadata the plugin has is included in the
   *         returned metadata, otherwise the metadata returned only includes
   *         metadata from the masterlist.
   *  @returns A PluginMetadata object containing all the plugin's metadata.
   *           If the plugin has no metadata, PluginMetadata.IsNameOnly()
   *           will return true.
   */
  virtual PluginMetadata GetPluginMetadata(const std::string& plugin,
                                           bool includeUserMetadata = true) const = 0;

  /**
  *  @brief Get a plugin's metadata loaded from the given userlist.
  *  @param plugin
  *         The filename of the plugin to look up user-added metadata for.
  *  @returns A PluginMetadata object containing the plugin's user-added
  *           metadata. If the plugin has no metadata,
  *           PluginMetadata.IsNameOnly() will return true.
  */
  virtual PluginMetadata GetPluginUserMetadata(const std::string& plugin) const = 0;

  /**
  *  @brief Sets a plugin's user metadata, overwriting any existing user
  *         metadata.
  *  @param pluginMetadata
  *         The user metadata you want to set, with plugin.Name() being the
  *         filename of the plugin the metadata is for.
  */
  virtual void SetPluginUserMetadata(const PluginMetadata& pluginMetadata) = 0;

  /**
   * @brief Discards all loaded user metadata for the plugin with the given
   *        filename.
   * @param plugin
   *        The filename of the plugin for which all user-added metadata
   *        should be deleted.
   */
  virtual void DiscardPluginUserMetadata(const std::string& plugin) = 0;

  /**
   * @brief Discards all loaded user metadata for all plugins, and any user-added
   *        general messages and known bash tags.
   */
  virtual void DiscardAllUserMetadata() = 0;

  /** @} */
};
}

#endif
