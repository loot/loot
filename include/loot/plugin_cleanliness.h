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
#ifndef LOOT_PLUGIN_CLEANLINESS
#define LOOT_PLUGIN_CLEANLINESS

/**
 * The namespace used by the LOOT API.
 */
namespace loot {
/**
 * @brief Codes used to indicate the cleanliness of a plugin according to the
 *        information contained within the loaded masterlist/userlist.
 */
enum struct PluginCleanliness : unsigned int {
  /** Indicates that the plugin is clean. */
  clean,
  /** Indicates that the plugin is dirty. */
  dirty,
  /**
   * Indicates that the plugin contains dirty edits, but that they are
   * part of the plugin's intended functionality and should not be removed.
   */
  do_not_clean,
  /**
   * Indicates that no data is available on whether the plugin is dirty or not.
   */
  unknown,
};
}

#endif
