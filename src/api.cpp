/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012    WrinklyNinja

    This file is part of BOSS.

    BOSS is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#include "api.h"

//////////////////////////////
// Error Handling Functions
//////////////////////////////

// Outputs a string giving the details of the last time an error or
// warning return code was returned by a function. The string exists
// until this function is called again or until CleanUpAPI is called.
BOSS_API uint32_t boss_get_error_message (uint8_t ** message) {

}


//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of BOSS supports the API from the given
// BOSS version. Abstracts BOSS API stability policy away from clients.
BOSS_API bool boss_is_compatible (const uint32_t versionMajor, const uint32_t versionMinor, const uint32_t versionPatch) {

}

// Returns the version string for this version of BOSS.
// The string exists until this function is called again or until
// CleanUpAPI is called.
BOSS_API uint32_t boss_get_version (uint32_t * versionMajor, uint32_t * versionMinor, uint32_t * versionPatch) {

}


////////////////////////////////////
// Lifecycle Management Functions
////////////////////////////////////

// Explicitly manage database lifetime. Allows clients to free memory when
// they want/need to. clientGame sets the game the DB is for, and dataPath
// is the path to that game's Data folder, and is case-sensitive if the
// underlying filesystem is case-sensitive. This function also checks that
// plugins.txt and loadorder.txt (if they both exist) are in sync. If
// dataPath == NULL then the API will attempt to detect the data path of
// the specified game.
BOSS_API uint32_t boss_create_db (boss_db * db, const uint32_t clientGame, const uint8_t * gamePath) {

}

// Destroys the given DB, freeing any memory allocated as part of its use.
BOSS_API void     boss_destroy_db (boss_db db) {

}

// Frees memory allocated to version and error strings.
BOSS_API void     boss_cleanup () {

}


///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

// Loads the masterlist and userlist from the paths specified.
// Can be called multiple times. On error, the database is unchanged.
// Paths are case-sensitive if the underlying filesystem is case-sensitive.
// masterlistPath and userlistPath are files.
BOSS_API uint32_t boss_load_lists (boss_db db, const uint8_t * masterlistPath,
                                    const uint8_t * userlistPath) {

}

// Evaluates all conditional lines and regex mods the loaded masterlist.
// This exists so that Load() doesn't need to be called whenever the mods
// installed are changed. Evaluation does not take place unless this function
// is called. Repeated calls re-evaluate the masterlist from scratch each time,
// ignoring the results of any previous evaluations. Paths are case-sensitive
// if the underlying filesystem is case-sensitive.
BOSS_API uint32_t boss_eval_lists (boss_db db) {

}


//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
BOSS_API uint32_t boss_get_tag_map (boss_db db, boss_tag ** tagMap, size_t * numTags) {

}

// Returns arrays of Bash Tag UIDs for Bash Tags suggested for addition and removal
// by BOSS's masterlist and userlist, and the number of tags in each array.
// The returned arrays are valid until the db is destroyed or until the Load
// function is called.  The arrays should not be freed by the client. modName is
// case-insensitive. If no Tags are found for an array, the array pointer (*tagIds)
// will be NULL. The userlistModified bool is true if the userlist contains Bash Tag
// suggestion message additions.
BOSS_API uint32_t boss_get_plugin_tags (boss_db db, const uint8_t * plugin,
                                    uint32_t ** tagIds_added,
                                    size_t * numTags_added,
                                    uint32_t **tagIds_removed,
                                    size_t *numTags_removed,
                                    bool * userlistModified) {

}

// Returns the messages attached to the given plugin. Messages are valid until Load,
// DestroyBossDb or GetPluginMessages are next called. plugin is case-insensitive.
// If no messages are attached, *messages will be NULL and numMessages will equal 0.
BOSS_API uint32_t boss_get_plugin_messages (boss_db db, const uint8_t * plugin,
                                            boss_message ** messages,
                                            size_t * numMessages) {

}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions,
// and/or dirty messages, plus the Tag suggestions and/or messages themselves and their
// conditions, in order to create the Wrye Bash taglist. outputFile is the path to use
// for output. If outputFile already exists, it will only be overwritten if overwrite is true.
BOSS_API uint32_t boss_write_minimal_list (boss_db db, const uint8_t * outputFile, const bool overwrite) {

}
