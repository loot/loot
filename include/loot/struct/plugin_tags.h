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
#ifndef LOOT_PLUGIN_TAGS
#define LOOT_PLUGIN_TAGS

#include <set>
#include <string>

namespace loot {
/**
 * @brief A structure that holds data about the Bash Tag suggestions made by
 *        LOOT for a plugin.
 */
struct PluginTags {
  inline PluginTags() : userlist_modified(false) {}

  /**
   * @brief A set of Bash Tag names suggested for addition to the specified
   *        plugin. Empty if no Bash Tag additions are suggested.
   */
  std::set<std::string> added;

  /**
   * @brief A set of Bash Tag names suggested for removal from the specified
   *        plugin. Empty if no Bash Tag removals are suggested.
   */
  std::set<std::string> removed;

  /**
  * @brief `true` if the Bash Tag suggestions were modified by data in the
  *        userlist, `false` otherwise.
  */
  bool userlist_modified;
};
}

#endif
