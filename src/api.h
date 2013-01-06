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

#ifndef __BOSS_API_H__
#define __BOSS_API_H__

#include <stdint.h>
#include <stddef.h>

#if defined(_MSC_VER)
//MSVC doesn't support C99, so do the stdbool.h definitions ourselves.
//START OF stdbool.h DEFINITIONS.
#   ifndef __cplusplus
#       define bool _Bool
#       define true 1
#       define false   0
#   endif
#   define __bool_true_false_are_defined   1
//END OF stdbool.h DEFINITIONS.
#else
#   include <stdbool.h>
#endif

// set up dll import/export decorators
// when compiling the dll on windows, ensure BOSS_API_EXPORT is defined. clients
// that use this header do not need to define anything to import the symbols
// properly.
#if defined(_WIN32) || defined(_WIN64)
#   ifdef BOSS_API_EXPORT
#       define BOSS_API __declspec(dllexport)
#   else
#       define BOSS_API __declspec(dllimport)
#   endif
#else
#   define BOSS_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif


////////////////////////
// Types
////////////////////////

// All API strings are uint8_t* strings encoded in UTF-8. Strings returned
// by the API should not have their memory freed by the client: the API will
// clean up after itself.
// All API numbers and error codes are uint32_t integers.

// Abstracts the definition of BOSS's internal state while still providing
// type safety across the API.
typedef struct _boss_db_int * boss_db;

// boss_tag structure gives the Unique ID number (UID) for each Bash Tag and
// the corresponding Tag name string.
typedef struct {
    uint32_t id;
    const uint8_t * name;  // don't use char for utf-8 since char can be signed
} boss_tag;

// boss_message structure gives the type of message and it contents.
typedef struct {
    uint32_t type;
    const uint8_t * message;
} boss_message;


// The following are the possible codes that the API can return.
BOSS_API extern const uint32_t BOSS_API_OK;
BOSS_API extern const uint32_t BOSS_API_OK_NO_UPDATE_NECESSARY;
BOSS_API extern const uint32_t BOSS_API_WARN_BAD_FILENAME;
BOSS_API extern const uint32_t BOSS_API_WARN_LO_MISMATCH;
BOSS_API extern const uint32_t BOSS_API_ERROR_FILE_WRITE_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_FILE_DELETE_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_FILE_NOT_UTF8;
BOSS_API extern const uint32_t BOSS_API_ERROR_FILE_NOT_FOUND;
BOSS_API extern const uint32_t BOSS_API_ERROR_FILE_RENAME_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_TIMESTAMP_READ_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_TIMESTAMP_WRITE_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_PARSE_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_CONDITION_EVAL_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_REGEX_EVAL_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_NO_MEM;
BOSS_API extern const uint32_t BOSS_API_ERROR_INVALID_ARGS;
BOSS_API extern const uint32_t BOSS_API_ERROR_NETWORK_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_NO_INTERNET_CONNECTION;
BOSS_API extern const uint32_t BOSS_API_ERROR_NO_TAG_MAP;
BOSS_API extern const uint32_t BOSS_API_ERROR_PLUGINS_FULL;
BOSS_API extern const uint32_t BOSS_API_ERROR_GAME_NOT_FOUND;
BOSS_API extern const uint32_t BOSS_API_ERROR_PLUGIN_BEFORE_MASTER;
BOSS_API extern const uint32_t BOSS_API_RETURN_MAX;

// The following are the games identifiers used by the API.
BOSS_API extern const uint32_t BOSS_API_GAME_OBLIVION;
BOSS_API extern const uint32_t BOSS_API_GAME_FALLOUT3;
BOSS_API extern const uint32_t BOSS_API_GAME_FALLOUTNV;
BOSS_API extern const uint32_t BOSS_API_GAME_NEHRIM;
BOSS_API extern const uint32_t BOSS_API_GAME_SKYRIM;
BOSS_API extern const uint32_t BOSS_API_GAME_MORROWIND;

// BOSS message types.
BOSS_API extern const uint32_t BOSS_API_MESSAGE_SAY;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_TAG;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_REQUIREMENT;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_INCOMPATIBILITY;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_DIRTY;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_WARN;
BOSS_API extern const uint32_t BOSS_API_MESSAGE_ERROR;



//////////////////////////////
// Error Handling Functions
//////////////////////////////

// Outputs a string giving the details of the last time an error or
// warning return code was returned by a function. The string exists
// until this function is called again or until CleanUpAPI is called.
BOSS_API uint32_t boss_get_error_message (uint8_t ** message);


//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of BOSS supports the API from the given
// BOSS version. Abstracts BOSS API stability policy away from clients.
BOSS_API bool boss_is_compatible (const uint32_t versionMajor, const uint32_t versionMinor, const uint32_t versionPatch);

// Returns the version string for this version of BOSS.
// The string exists until this function is called again or until
// CleanUpAPI is called.
BOSS_API uint32_t boss_get_version (uint32_t * versionMajor, uint32_t * versionMinor, uint32_t * versionPatch);


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
BOSS_API uint32_t boss_create_db (boss_db * db, const uint32_t clientGame, const uint8_t * gamePath);

// Destroys the given DB, freeing any memory allocated as part of its use.
BOSS_API void     boss_destroy_db (boss_db db);

// Frees memory allocated to version and error strings.
BOSS_API void     boss_cleanup ();


///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

// Loads the masterlist and userlist from the paths specified.
// Can be called multiple times. On error, the database is unchanged.
// Paths are case-sensitive if the underlying filesystem is case-sensitive.
// masterlistPath and userlistPath are files.
BOSS_API uint32_t boss_load_lists (boss_db db, const uint8_t * masterlistPath,
                                    const uint8_t * userlistPath);

// Evaluates all conditional lines and regex mods the loaded masterlist.
// This exists so that Load() doesn't need to be called whenever the mods
// installed are changed. Evaluation does not take place unless this function
// is called. Repeated calls re-evaluate the masterlist from scratch each time,
// ignoring the results of any previous evaluations. Paths are case-sensitive
// if the underlying filesystem is case-sensitive.
BOSS_API uint32_t boss_eval_lists (boss_db db);


//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
BOSS_API uint32_t boss_get_tag_map (boss_db db, boss_tag ** tagMap, size_t * numTags);

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
                                    bool * userlistModified);

// Returns the messages attached to the given plugin. Messages are valid until Load,
// DestroyBossDb or GetPluginMessages are next called. plugin is case-insensitive.
// If no messages are attached, *messages will be NULL and numMessages will equal 0.
BOSS_API uint32_t boss_get_plugin_messages (boss_db db, const uint8_t * plugin,
                                            boss_message ** messages,
                                            size_t * numMessages);

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions,
// and/or dirty messages, plus the Tag suggestions and/or messages themselves and their
// conditions, in order to create the Wrye Bash taglist. outputFile is the path to use
// for output. If outputFile already exists, it will only be overwritten if overwrite is true.
BOSS_API uint32_t boss_write_minimal_list (boss_db db, const uint8_t * outputFile, const bool overwrite);


#ifdef __cplusplus
}
#endif

#endif
