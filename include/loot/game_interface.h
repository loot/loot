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
#ifndef LOOT_GAME_INTERFACE
#define LOOT_GAME_INTERFACE

#include "loot/database_interface.h"
#include "loot/plugin_interface.h"

namespace loot {
/** @brief The interface provided for accessing game-specific functionality. */
class GameInterface {
public:
  /**
   *  @name Metadata Access
   *  @{
   */

  /**
   * @brief Get the database interface used for accessing metadata-related
   *        functionality.
   * @returns A shared pointer to the game's DatabaseInterface
   */
  virtual std::shared_ptr<DatabaseInterface> GetDatabase() = 0;

  /**
   * @}
   * @name Plugin Data Access
   * @{
   */

  /**
   * @brief Check if a file is a valid plugin.
   * @details The validity check is not exhaustive: it checks that the file
   *          extension is ``.esm`` or ``.esp`` (after trimming any ``.ghost``
   *          extension), and that the ``TES4`` header can be parsed.
   * @param  plugin
   *         The filename of the file to check.
   * @returns True if the file is a valid plugin, false otherwise.
   */
  virtual bool IsValidPlugin(const std::string& plugin) = 0;

  /**
   * @brief Parses plugins and loads their data.
   * @details Any previously-loaded plugin data is discarded when this function
   *          is called.
   * @param plugins
   *        The filenames of the plugins to load.
   * @param loadHeadersOnly
   *        If true, only the plugins' ``TES4`` headers are loaded. If false,
   *        all records in the plugins are parsed, apart from the main master
   *        file if it has been identified by a previous call to
   *        ``IdentifyMainMasterFile()``.
   */
  virtual void LoadPlugins(const std::vector<std::string>& plugins, bool loadHeadersOnly) = 0;

  /**
   * @brief Get data for a loaded plugin.
   * @details Throws an exception if the given plugin has not been loaded.
   * @param  pluginName
   *         The filename of the plugin to get data for.
   * @returns A const PluginInterface reference. The reference remains valid
   *          until the ``LoadPlugins()`` or ``SortPlugins()`` functions are
   *          next called or this GameInterface is destroyed.
   */
  virtual std::shared_ptr<const PluginInterface> GetPlugin(const std::string& pluginName) = 0;

  /**
   * @brief Get a set of const references to all loaded plugins' PluginInterface
   *        objects.
   * @returns A set of const PluginInterface references. The references remain
   *          valid until the ``LoadPlugins()`` or ``SortPlugins()`` functions
   *          are next called or this GameInterface is destroyed.
   */
  virtual std::set<std::shared_ptr<const PluginInterface>> GetLoadedPlugins() = 0;

  /**
  *  @}
  *  @name Sorting
  *  @{
  */

  /**
  *  @brief Identify the game's main master file.
  *  @details When sorting, LOOT always only loads the headers of the game's
  *           main master file as a performance optimisation.
  */
  virtual void IdentifyMainMasterFile(const std::string& masterFile) = 0;

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
  *  @name Load Order Interaction
  *  @{
  */

  /**
   * @brief Check if a plugin is active.
   * @param  plugin
   *         The filename of the plugin for which to check the active state.
   * @returns True if the plugin is active, false otherwise.
   */
  virtual bool IsPluginActive(const std::string& plugin) = 0;

  /**
   * @brief Get the current load order.
   * @returns A vector of plugin filenames in their load order.
   */
  virtual std::vector<std::string> GetLoadOrder() = 0;

  /**
   * @brief Set the game's load order.
   * @param loadOrder
   *        A vector of plugin filenames sorted in the load order to set.
   */
  virtual void SetLoadOrder(const std::vector<std::string>& loadOrder) = 0;
};
}

#endif
