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

/**
 *  @mainpage
 *  @author WrinklyNinja
 *  @version 0.9.2
 *  @copyright
 *      The LOOT API is distributed under the GNU General Public License v3.0.
 *      For the full text of the license, see the "GNU GPL v3.txt" file
 *      included in the source archive.
 *
 *  @section intro_sec Introduction
 *      LOOT is a utility that helps users avoid serious conflicts between
 *      their mods by setting their plugins in an optimal load order. It also
 *      provides tens of thousands of plugin-specific messages, including usage
 *      notes, requirements, incompatibilities, bug warnings and installation
 *      mistake notifications, and thousands of Bash Tag suggestions.
 *
 *      This metadata that LOOT supplies is stored in its masterlist, which is
 *      maintained by the LOOT team using information provided by mod authors
 *      and users. Users can also add to and modify the metadata used by LOOT
 *      through the use of userlist files. The LOOT API provides a way for
 *      third-party developers to access this metadata for use in their own
 *      programs.
 *
 *      All further API documentation is contained within the documentation for
 *      api.h.
 *
 * @section credit_sec Credits
 *      The LOOT API is written by [WrinklyNinja]
 *      (https://github.com/WrinklyNinja) in C/C++ and makes use of the
 *      [Boost](http://www.boost.org/),
 *      [libespm](https://github.com/WrinklyNinja/libespm),
 *      [libgit2](https://github.com/libgit2/libgit2),
 *      [libloadorder](https://github.com/WrinklyNinja/libloadorder/),
 *      [Pseudosem](https://github.com/WrinklyNinja/pseudosem) and
 *      [yaml-cpp](https://github.com/WrinklyNinja/yaml-cpp) libraries. Copyright
 *      license information for all these may be found in the
 *      "docs/licenses/Licenses.txt" file.
 *
 * @section history_sec Version History
 *      ## 0.9.2 - *3 August 2016*
 *
 *        * Fixed the wrong API binary being packaged. This caused the v0.9.0 and v0.9.1 API releases to actually be re-releases of a snapshot build made at some point between v0.8.1 and v0.9.0: the affected API releases were taken offline once this was discovered.
 *        * Fixed `loot_get_plugin_tags()`` remembering results and including them in the results of subsequent calls.
 *        * Fixed an error occurring when the user's temporary files directory didn't exist and updating the masterlist tried to create a directory there.
 *        * Fixed errors when reading some Oblivion plugins during sorting, including the official DLC.
 *        * libespm (2.5.5) and Pseudosem (1.1.0) dependencies have been updated to the versions given in brackets.
 *
 *      ## 0.9.1 - *23 June 2016*
 *
 *        * No API changes.
 *
 *      ## 0.9.0 - *21 May 2016*
 *
 *        * Moved API header location to the more standard `include/loot/api.h`.
 *        * Documented LOOT's masterlist versioning system.
 *        * Made all API outputs fully const to make it clear they should not
 *          be modified and to avoid internal const casting.
 *        * Removed the `loot_cleanup()` function, as the one string it used to
 *          destroy is now stored on the stack and so destroyed when the API is
 *          unloaded.
 *        * The `loot_db` type is now an opaque struct, and functions that used
 *          to take it as a value now take a pointer to it.
 *        * Removed the `loot_lang_any` constant. The `loot_lang_english`
 *          constant should be used instead.
 *
 *      ## 0.8.1 - *27 September 2015*
 *
 *        * Fixed crash when loading plugins due to lack of thread safety.
 *        * Fixed masterlist updater and validator not checking for valid
 *          condition and regex syntax.
 *        * Check for safe file paths when parsing conditions.
 *        * Updated Boost (1.59.0), libgit2 (0.23.2) and CEF (branch 2454)
 *          dependencies. This fixes the masterlist updater not working
 *          correctly for Windows Vista users.
 *
 *      ## 0.8.0 - *22 July 2015*
 *
 *        * Fixed many miscellaneous bugs, including initialisation crashes and
 *          incorrect metadata input/output handling.
 *        * Fixed LOOT silently discarding some non-unique metadata: an error
 *          will now occur when loading or attempting to apply such metadata.
 *        * Fixed and improved LOOT's version comparison behaviour for a wide
 *          variety of version string formats. This involved removing LOOT's
 *          usage of the Alphanum code library.
 *        * Improved plugin loading performance for computers with weaker
 *          multithreading capabilities (eg. non-hyperthreaded dual-core or
 *          single-core CPUs).
 *        * LOOT no longer outputs validity warnings for inactive plugins.
 *        * Metadata syntax support changes, see the metadata syntax document
 *          for details.
 *        * Updated libgit2 to v0.23.0.
 *
 *      ## 0.7.1 - *22 June 2015*
 *
 *        * Fixed "No existing load order position" errors when sorting.
 *        * Fixed output of Bash Tag removal suggestions in
 *          `loot_write_minimal_list()`.
 *
 *      ## 0.7.0 - *20 May 2015*
 *
 *        * Initial API release.
 */

/**
 *  @file api.h
 *  @brief This file contains the API frontend.
 *
 *  @note The LOOT API is *not* thread safe. Thread safety is a goal, but one
 *        that has not yet been achieved. Bear this in mind if using it in a
 *        multi-threaded client.
 *
 *  @section var_sec Variable Types
 *
 *  The LOOT API uses character strings and integers for information
 *  input/output.
 *  - All strings are null-terminated byte character strings encoded in UTF-8.
 *  - All codes are unsigned integers at least 16 bits in size.
 *  - All array sizes are unsigned integers at least 16 bits in size.
 *  - File paths are case-sensitive if and only if the underlying file system
 *    is case-sensitive.
 *
 *  @section memory_sec Memory Management
 *
 *  The LOOT API manages the memory of strings and arrays it returns, so such
 *  strings and arrays should not be deallocated by the client.
 *
 *  Data returned by a function lasts until a function is called which returns
 *  data of the same type (eg. a string is stored until the client calls
 *  another function which returns a string, an integer array lasts until
 *  another integer array is returned, etc.).
 *
 *  All allocated memory is freed when loot_destroy_db() is called, except the
 *  string allocated by loot_get_error_message(), which must be freed by
 *  calling loot_cleanup().
 */

#ifndef LOOT_API_H
#define LOOT_API_H

#include <string>
#include <memory>

#include "loot/database_interface.h"
#include "loot/game_type.h"
#include "loot/loot_version.h"

/* set up dll import/export decorators
   when compiling the dll on windows, ensure LOOT_EXPORT is defined. clients
   that use this header do not need to define anything to import the symbols
   properly. */
#if defined(_WIN32)
#   ifdef LOOT_STATIC
#       define LOOT_API
#   elif defined LOOT_EXPORT
#       define LOOT_API __declspec(dllexport)
#   else
#       define LOOT_API __declspec(dllimport)
#   endif
#else
#   define LOOT_API
#endif

namespace loot {
/**@}*/
/**********************************************************************//**
 *  @name Version Functions
 *************************************************************************/
/**@{*/

/**
 *  @brief Checks for API compatibility.
 *  @details Checks whether the loaded API is compatible with the given
 *           version of the API, abstracting API stability policy away from
 *           clients. The version numbering used is major.minor.patch.
 *  @param versionMajor
 *      The major version number to check.
 *  @param versionMinor
 *      The minor version number to check.
 *  @param versionPatch
 *      The patch version number to check.
 *  @returns True if the API versions are compatible, false otherwise.
 */
LOOT_API bool IsCompatible(const unsigned int versionMajor,
                           const unsigned int versionMinor,
                           const unsigned int versionPatch);

/**@}*/
/**********************************************************************//**
 *  @name Lifecycle Management Functions
 *************************************************************************/
/**@{*/

/**
 *  @brief Initialise a new database handle.
 *  @details Creates a handle for a database, which is then used by all
 *           database functions.
 *  @param db
 *      A pointer to the handle that is created by the function.
 *  @param clientGame
 *      A game code for which to create the handle.
 *  @param gamePath
 *      The relative or absolute path to the game folder, or `NULL`.
 *      If `NULL`, the API will attempt to detect the data path of the
 *      specified game.
 *  @param gameLocalPath
 *      The relative or absolute path to the game's folder in
 *      `%LOCALAPPDATA%`, or `NULL`. If `NULL`, the API will attempt to
 *      look up the path that `%LOCALAPPDATA%` corresponds to. This
 *      parameter is provided so that systems lacking that environmental
 *      variable (eg. Linux) can still use the API.
 *  @returns A return code.
 */
LOOT_API std::shared_ptr<DatabaseInterface> CreateDatabase(const GameType game,
                                                           const std::string& game_path,
                                                           const std::string& game_local_path);
}

#endif
