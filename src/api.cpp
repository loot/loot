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
#include "globals.h"
#include "game.h"
#include "metadata.h"
#include "parsers.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <fstream>
#include <clocale>
#include <list>
#include <vector>

#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

const unsigned int BOSS_API_OK                          = 0;
const unsigned int BOSS_API_ERROR_LIBLO_ERROR           = 1;
const unsigned int BOSS_API_ERROR_PARSE_FAIL            = 2;
const unsigned int BOSS_API_ERROR_CONDITION_EVAL_FAIL   = 3;
const unsigned int BOSS_API_ERROR_REGEX_EVAL_FAIL       = 4;
const unsigned int BOSS_API_ERROR_NO_MEM                = 5;
const unsigned int BOSS_API_ERROR_INVALID_ARGS          = 6;
const unsigned int BOSS_API_ERROR_NO_TAG_MAP            = 7;
const unsigned int BOSS_API_RETURN_MAX                  = BOSS_API_ERROR_NO_TAG_MAP;

// The following are the games identifiers used by the API.
const unsigned int BOSS_API_GAME_TES4                   = BOSS_GAME_TES4;
const unsigned int BOSS_API_GAME_TES5                   = BOSS_GAME_TES5;
const unsigned int BOSS_API_GAME_FO3                    = BOSS_GAME_FO3;
const unsigned int BOSS_API_GAME_FONV                   = BOSS_GAME_FONV;

// BOSS message types.
const unsigned int BOSS_API_MESSAGE_SAY                 = 1;
const unsigned int BOSS_API_MESSAGE_WARN                = 2;
const unsigned int BOSS_API_MESSAGE_ERROR               = 3;

struct _boss_db_int {
    _boss_db_int()
        : extTagMap(NULL),
        extAddedTagIds(NULL),
        extRemovedTagIds(NULL),
        extMessageArray(NULL),
        extMessageArraySize(0) {
    }

    ~_boss_db_int() {
        delete [] extAddedTagIds;
        delete [] extRemovedTagIds;

        if (extTagMap != NULL) {
            for (size_t i=0; i < bashTagMap.size(); i++)
                delete [] extTagMap[i];  //Gotta clear those allocated strings.
            delete [] extTagMap;
        }

        if (extMessageArray != NULL) {
            for (size_t i=0; i < extMessageArraySize; i++)
                delete [] extMessageArray[i].message;  //Gotta clear those allocated strings.
            delete [] extMessageArray;
        }
    }

    boss::Game game;
    std::list<boss::Plugin> metadata;
    std::list<boss::Plugin> userMetadata;

    boost::unordered_map<std::string, unsigned int> bashTagMap;

    char ** extTagMap;

    unsigned int * extAddedTagIds;
    unsigned int * extRemovedTagIds;

    boss_message * extMessageArray;
    size_t extMessageArraySize;
};

char * extMessageStr;

// std::string to null-terminated char string converter.
char * ToNewCString(std::string str) {
    char * p = new char[str.length() + 1];
    return strcpy(p, str.c_str());
}

//////////////////////////////
// Error Handling Functions
//////////////////////////////

// Outputs a string giving the details of the last time an error or
// warning return code was returned by a function. The string exists
// until this function is called again or until CleanUpAPI is called.
BOSS_API unsigned int boss_get_error_message (char ** message) {
    if (message == NULL)
        return BOSS_API_ERROR_INVALID_ARGS;

    *message = "Something went wrong.";

    return BOSS_API_OK;
}

// Frees memory allocated to error string.
BOSS_API void     boss_cleanup () {
    delete[] extMessageStr;
}


//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of BOSS supports the API from the given
// BOSS version. Abstracts BOSS API stability policy away from clients.
BOSS_API bool boss_is_compatible (const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch) {
    return versionMajor == BOSS_VERSION_MAJOR && versionMinor == BOSS_VERSION_MINOR;
}

// Returns the version string for this version of BOSS.
// The string exists until this function is called again or until
// CleanUpAPI is called.
BOSS_API unsigned int boss_get_version (unsigned int * versionMajor, unsigned int * versionMinor, unsigned int * versionPatch) {
    if (versionMajor == NULL || versionMinor == NULL || versionPatch == NULL)
        return BOSS_API_ERROR_INVALID_ARGS;

    *versionMajor = BOSS_VERSION_MAJOR;
    *versionMinor = BOSS_VERSION_MINOR;
    *versionPatch = BOSS_VERSION_PATCH;

    return BOSS_API_OK;
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
BOSS_API unsigned int boss_create_db (boss_db * db, const unsigned int clientGame, const char * gamePath) {
    if (db == NULL || (clientGame != BOSS_API_GAME_TES4 && clientGame != BOSS_API_GAME_TES5 && clientGame != BOSS_API_GAME_FO3 && clientGame != BOSS_API_GAME_FONV))
        return BOSS_API_ERROR_INVALID_ARGS;

    //Set the locale to get encoding conversions working correctly.
    std::setlocale(LC_CTYPE, "");
    std::locale global_loc = std::locale();
    std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
    boost::filesystem::path::imbue(loc);

    std::string game_path = "";
    if (gamePath != NULL)
        game_path = gamePath;

    boss::Game game;
    try {
        game = boss::Game(clientGame, game_path);  //This also checks to see if the game is installed if game_path is empty and throws an exception if it is not detected.
    } catch (std::runtime_error& e) {
        return BOSS_API_ERROR_INVALID_ARGS;
    }

    boss_db retVal;
    try {
        retVal = new _boss_db_int;
    } catch (std::bad_alloc /*&e*/) {
        return BOSS_API_ERROR_INVALID_ARGS;
    }
    retVal->game = game;
    *db = retVal;

    return BOSS_API_OK;
}

// Destroys the given DB, freeing any memory allocated as part of its use.
BOSS_API void     boss_destroy_db (boss_db db) {
    delete db;
}


///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

// Loads the masterlist and userlist from the paths specified.
// Can be called multiple times. On error, the database is unchanged.
// Paths are case-sensitive if the underlying filesystem is case-sensitive.
// masterlistPath and userlistPath are files.
BOSS_API unsigned int boss_load_lists (boss_db db, const char * masterlistPath,
                                    const char * userlistPath) {
    if (db == NULL || masterlistPath == NULL)
        return BOSS_API_ERROR_INVALID_ARGS;

    std::list<boss::Plugin> temp;
    std::list<boss::Plugin> userTemp;
    try {
        YAML::Node tempNode = YAML::LoadFile(masterlistPath);
        temp = tempNode["plugins"].as< std::list<boss::Plugin> >();

        tempNode = YAML::LoadFile(userlistPath);
        userTemp = tempNode["plugins"].as< std::list<boss::Plugin> >();
    } catch (YAML::Exception &e) {
        return BOSS_API_ERROR_INVALID_ARGS;
    }

    db->metadata = temp;
    db->userMetadata = userTemp;

    return BOSS_API_OK;
}

// Evaluates all conditional lines and regex mods the loaded masterlist.
// This exists so that Load() doesn't need to be called whenever the mods
// installed are changed. Evaluation does not take place unless this function
// is called. Repeated calls re-evaluate the masterlist from scratch each time,
// ignoring the results of any previous evaluations. Paths are case-sensitive
// if the underlying filesystem is case-sensitive.
BOSS_API unsigned int boss_eval_lists (boss_db db) {

    std::list<boss::Plugin> temp = db->metadata;
    try {
        for (std::list<boss::Plugin>::iterator it=temp.begin(), endIt=temp.end(); it != endIt; ++it) {
            it->EvalAllConditions(db->game);
        }
    } catch (std::runtime_error& e) {
        return BOSS_API_ERROR_INVALID_ARGS;
    }
    db->metadata = temp;

    temp = db->userMetadata;
    try {
        for (std::list<boss::Plugin>::iterator it=temp.begin(), endIt=temp.end(); it != endIt; ++it) {
            it->EvalAllConditions(db->game);
        }
    } catch (std::runtime_error& e) {
        return BOSS_API_ERROR_INVALID_ARGS;
    }
    db->userMetadata = temp;

    return BOSS_API_OK;
}


//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
BOSS_API unsigned int boss_get_tag_map (boss_db db, char *** tagMap, size_t * numTags) {
    if (db == NULL || tagMap == NULL || numTags == NULL)
        return BOSS_API_ERROR_INVALID_ARGS;

    //Clear existing array allocation.
    if (db->extTagMap != NULL) {
        for (size_t i=0, max=db->bashTagMap.size(); i < max; ++i) {
            delete [] db->extTagMap[i];
        }
        delete [] db->extTagMap;
    }

    //Initialise output.
    *tagMap = NULL;
    *numTags = 0;

    boost::unordered_set<std::string> allTags;

    for (std::list<boss::Plugin>::iterator it=db->metadata.begin(), endIt=db->metadata.end(); it != endIt; ++it) {
        std::list<boss::Tag> tags = it->Tags();
        for (std::list<boss::Tag>::const_iterator jt=tags.begin(), endJt=tags.end(); jt != endJt; ++jt) {
            allTags.insert(jt->Name());
        }
    }
    for (std::list<boss::Plugin>::iterator it=db->userMetadata.begin(), endIt=db->userMetadata.end(); it != endIt; ++it) {
        std::list<boss::Tag> tags = it->Tags();
        for (std::list<boss::Tag>::const_iterator jt=tags.begin(), endJt=tags.end(); jt != endJt; ++jt) {
            allTags.insert(jt->Name());
        }
    }

    if (allTags.empty())
        return BOSS_API_OK;

    try {
        db->extTagMap = new char*[allTags.size()];
    } catch (std::bad_alloc /*&e*/) {
        return BOSS_API_ERROR_INVALID_ARGS;
    }

    unsigned int UID = 0;
    try {
        for (boost::unordered_set<std::string>::const_iterator it=allTags.begin(), endIt=allTags.end(); it != endIt; ++it) {
            db->bashTagMap.emplace(*it, UID);
            //Also allocate memory.
            db->extTagMap[UID] = ToNewCString(*it);
            UID++;
        }
    } catch (std::bad_alloc /*&e*/) {
        return BOSS_API_ERROR_INVALID_ARGS;
    }

    *tagMap = db->extTagMap;
    *numTags = allTags.size();

    return BOSS_API_OK;
}

// Returns arrays of Bash Tag UIDs for Bash Tags suggested for addition and removal
// by BOSS's masterlist and userlist, and the number of tags in each array.
// The returned arrays are valid until the db is destroyed or until the Load
// function is called.  The arrays should not be freed by the client. modName is
// case-insensitive. If no Tags are found for an array, the array pointer (*tagIds)
// will be NULL. The userlistModified bool is true if the userlist contains Bash Tag
// suggestion message additions.
BOSS_API unsigned int boss_get_plugin_tags (boss_db db, const char * plugin,
                                            unsigned int ** tagIds_added,
                                            size_t * numTags_added,
                                            unsigned int **tagIds_removed,
                                            size_t * numTags_removed,
                                            bool * userlistModified) {
    if (db == NULL || plugin == NULL || tagIds_added == NULL || numTags_added == NULL || tagIds_removed == NULL || numTags_removed == NULL || userlistModified == NULL)
        return BOSS_API_ERROR_INVALID_ARGS;


    //Clear existing array allocations.
    delete [] db->extAddedTagIds;
    delete [] db->extRemovedTagIds;

    //Initialise output.
    *tagIds_added = NULL;
    *tagIds_removed = NULL;
    *userlistModified = false;
    *numTags_added = 0;
    *numTags_removed = 0;

    boost::unordered_set<std::string> tagsAdded, tagsRemoved;
    std::list<boss::Plugin>::iterator it = std::find(db->metadata.begin(), db->metadata.end(), boss::Plugin(plugin));
    if (it != db->metadata.end()) {
        std::list<boss::Tag> tags = it->Tags();
        for (std::list<boss::Tag>::const_iterator it=tags.begin(), endIt=tags.end(); it != endIt; ++it) {
            if (it->IsAddition())
                tagsAdded.insert(it->Name());
            else
                tagsRemoved.insert(it->Name());
        }
    }

    it = std::find(db->userMetadata.begin(), db->userMetadata.end(), boss::Plugin(plugin));
    if (it != db->userMetadata.end()) {
        *userlistModified = true;
    }

    std::vector<unsigned int> tagsAddedIDs, tagsRemovedIDs;
    for (boost::unordered_set<std::string>::const_iterator it=tagsAdded.begin(), endIt=tagsAdded.end(); it != endIt; ++it) {
        boost::unordered_map<std::string, unsigned int>::const_iterator mapIter = db->bashTagMap.find(*it);
        if (mapIter != db->bashTagMap.end())
            tagsAddedIDs.push_back(mapIter->second);
    }
    for (boost::unordered_set<std::string>::const_iterator it=tagsRemoved.begin(), endIt=tagsRemoved.end(); it != endIt; ++it) {
        boost::unordered_map<std::string, unsigned int>::const_iterator mapIter = db->bashTagMap.find(*it);
        if (mapIter != db->bashTagMap.end())
            tagsRemovedIDs.push_back(mapIter->second);
    }

    //Allocate memory.
    size_t numAdded = tagsAddedIDs.size();
    size_t numRemoved = tagsRemovedIDs.size();
    try {
        if (numAdded != 0) {
            db->extAddedTagIds = new uint32_t[numAdded];
            for (size_t i=0; i < numAdded; i++)
                db->extAddedTagIds[i] = tagsAddedIDs[i];
        }
        if (numRemoved != 0) {
            db->extRemovedTagIds = new uint32_t[numRemoved];
            for (size_t i=0; i < numRemoved; i++)
                db->extRemovedTagIds[i] = tagsRemovedIDs[i];
        }
    } catch (std::bad_alloc /*&e*/) {
        return BOSS_API_ERROR_INVALID_ARGS;
    }

    //Set outputs.
    *tagIds_added = db->extAddedTagIds;
    *tagIds_removed = db->extRemovedTagIds;
    *numTags_added = numAdded;
    *numTags_removed = numRemoved;

    return BOSS_API_OK;
}

// Returns the messages attached to the given plugin. Messages are valid until Load,
// DestroyBossDb or GetPluginMessages are next called. plugin is case-insensitive.
// If no messages are attached, *messages will be NULL and numMessages will equal 0.
BOSS_API unsigned int boss_get_plugin_messages (boss_db db, const char * plugin,
                                                boss_message ** messages,
                                                size_t * numMessages) {
    if (db == NULL || plugin == NULL || messages == NULL || numMessages == NULL)
        return BOSS_API_ERROR_INVALID_ARGS;

    //Clear existing array allocation.
    if (db->extMessageArray != NULL) {
        for (size_t i=0; i < db->extMessageArraySize; ++i) {
            delete [] db->extMessageArray[i].message;
        }
        delete [] db->extMessageArray;
    }

    //Initialise output.
    *messages = NULL;
    *numMessages = 0;

    std::list<boss::Plugin>::iterator it = std::find(db->metadata.begin(), db->metadata.end(), boss::Plugin(plugin));
    if (it == db->metadata.end()) {
        it = std::find(db->userMetadata.begin(), db->userMetadata.end(), boss::Plugin(plugin));
        if (it == db->userMetadata.end())
            return BOSS_API_OK;
    }

    return BOSS_API_OK;
}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions,
// and/or dirty messages, plus the Tag suggestions and/or messages themselves and their
// conditions, in order to create the Wrye Bash taglist. outputFile is the path to use
// for output. If outputFile already exists, it will only be overwritten if overwrite is true.
BOSS_API unsigned int boss_write_minimal_list (boss_db db, const char * outputFile, const bool overwrite) {
    if (db == NULL || outputFile == NULL)
        return BOSS_API_ERROR_INVALID_ARGS;

    if (boost::filesystem::exists(outputFile) && !overwrite)
        return BOSS_API_ERROR_INVALID_ARGS;

    std::list<boss::Plugin> temp = db->metadata;
    for (std::list<boss::Plugin>::iterator it=temp.begin(), endIt=temp.end(); it != endIt; ++it) {
        boss::Plugin p(it->Name());
        p.Tags(it->Tags());
        *it = p;
    }

    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << YAML::BeginMap
         << YAML::Key << "plugins" << YAML::Value << temp
         << YAML::EndMap;

    std::ofstream out(outputFile);
    if (out.fail())
        return BOSS_API_ERROR_INVALID_ARGS;
    out << yout.c_str();
    out.close();

    return BOSS_API_OK;
}
