/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2015    WrinklyNinja

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
    <http://www.gnu.org/licenses/>.
    */

#include "api.h"
#include "../backend/game/game.h"
#include "../backend/globals.h"
#include "../backend/error.h"
#include "../backend/helpers/streams.h"
#include "../backend/plugin_sorter.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <clocale>
#include <list>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/core.hpp>

const unsigned int loot_ok = loot::error::ok;
const unsigned int loot_error_liblo_error = loot::error::liblo_error;
const unsigned int loot_error_file_write_fail = loot::error::path_write_fail;
const unsigned int loot_error_parse_fail = loot::error::path_read_fail;
const unsigned int loot_error_condition_eval_fail = loot::error::condition_eval_fail;
const unsigned int loot_error_regex_eval_fail = loot::error::regex_eval_fail;
const unsigned int loot_error_no_mem = loot::error::no_mem;
const unsigned int loot_error_invalid_args = loot::error::invalid_args;
const unsigned int loot_error_no_tag_map = loot::error::no_tag_map;
const unsigned int loot_error_path_not_found = loot::error::path_not_found;
const unsigned int loot_error_no_game_detected = loot::error::no_game_detected;
const unsigned int loot_error_git_error = loot::error::git_error;
const unsigned int loot_error_windows_error = loot::error::windows_error;
const unsigned int loot_error_sorting_error = loot::error::sorting_error;
const unsigned int loot_return_max = loot_error_sorting_error;

// The following are the games identifiers used by the API.
const unsigned int loot_game_tes4 = loot::Game::tes4;
const unsigned int loot_game_tes5 = loot::Game::tes5;
const unsigned int loot_game_fo3 = loot::Game::fo3;
const unsigned int loot_game_fonv = loot::Game::fonv;

// LOOT message types.
const unsigned int loot_message_say = loot::Message::say;
const unsigned int loot_message_warn = loot::Message::warn;
const unsigned int loot_message_error = loot::Message::error;

// LOOT message languages.
const unsigned int loot_lang_any = loot::Language::any;
const unsigned int loot_lang_english = loot::Language::english;
const unsigned int loot_lang_spanish = loot::Language::spanish;
const unsigned int loot_lang_russian = loot::Language::russian;
const unsigned int loot_lang_french = loot::Language::french;
const unsigned int loot_lang_chinese = loot::Language::chinese;
const unsigned int loot_lang_polish = loot::Language::polish;
const unsigned int loot_lang_brazilian_portuguese = loot::Language::brazilian_portuguese;
const unsigned int loot_lang_finnish = loot::Language::finnish;
const unsigned int loot_lang_german = loot::Language::german;
const unsigned int loot_lang_danish = loot::Language::danish;
const unsigned int loot_lang_korean = loot::Language::korean;

// LOOT cleanliness codes.
const unsigned int loot_needs_cleaning_no = 0;
const unsigned int loot_needs_cleaning_yes = 1;
const unsigned int loot_needs_cleaning_unknown = 2;

struct _loot_db_int : public loot::Game {
    _loot_db_int(const unsigned int clientGame, const std::string& gamePath, const boost::filesystem::path& gameLocalDataPath)
        : Game(clientGame),
        extTagMap(nullptr),
        extAddedTagIds(nullptr),
        extRemovedTagIds(nullptr),
        extMessageArray(nullptr),
        extMessageArraySize(0),
        extStringArray(nullptr),
        extStringArraySize(0),
        extRevisionID(nullptr),
        extRevisionDate(nullptr) {
        this->SetGamePath(gamePath);
        this->Init(false, gameLocalDataPath);
    }

    ~_loot_db_int() {
        delete[] extAddedTagIds;
        delete[] extRemovedTagIds;
        delete[] extRevisionID;
        delete[] extRevisionDate;

        if (extTagMap != nullptr) {
            for (size_t i = 0; i < bashTagMap.size(); i++)
                delete[] extTagMap[i];  //Gotta clear those allocated strings.
            delete[] extTagMap;
        }

        if (extMessageArray != nullptr) {
            for (size_t i = 0; i < extMessageArraySize; i++)
                delete[] extMessageArray[i].message;  //Gotta clear those allocated strings.
            delete[] extMessageArray;
        }

        if (extStringArray != nullptr) {
            for (size_t i = 0; i < extStringArraySize; i++)
                delete[] extStringArray[i];  //Gotta clear those allocated strings.
            delete[] extStringArray;
        }
    }

    loot::MetadataList rawUserMetadata;
    loot::Masterlist rawMetadata;

    std::unordered_map<std::string, unsigned int> bashTagMap;

    char ** extTagMap;

    char ** extStringArray;
    size_t extStringArraySize;

    char * extRevisionID;
    char * extRevisionDate;

    unsigned int * extAddedTagIds;
    unsigned int * extRemovedTagIds;

    loot_message * extMessageArray;
    size_t extMessageArraySize;
};

char * extMessageStr = nullptr;

// std::string to null-terminated char string converter.
char * ToNewCString(std::string str) {
    char * p = new char[str.length() + 1];
    return strcpy(p, str.c_str());
}

unsigned int c_error(const loot::error& e) {
    delete[] extMessageStr;
    try {
        extMessageStr = new char[strlen(e.what()) + 1];
        strcpy(extMessageStr, e.what());
    }
    catch (std::bad_alloc& /*e*/) {
        extMessageStr = nullptr;
    }
    return e.code();
}

unsigned int c_error(const unsigned int code, const std::string& what) {
    return c_error(loot::error(code, what.c_str()));
}

//////////////////////////////
// Error Handling Functions
//////////////////////////////

// Outputs a string giving the details of the last time an error or
// warning return code was returned by a function. The string exists
// until this function is called again or until CleanUpAPI is called.
LOOT_API unsigned int loot_get_error_message(const char ** const message) {
    if (message == nullptr)
        return c_error(loot_error_invalid_args, "Null message pointer passed.");

    *message = extMessageStr;

    return loot_ok;
}

// Frees memory allocated to error string.
LOOT_API void     loot_cleanup() {
    delete[] extMessageStr;
    extMessageStr = nullptr;
}

//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of LOOT supports the API from the given
// LOOT version. Abstracts LOOT API stability policy away from clients.
LOOT_API bool loot_is_compatible(const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch) {
    return versionMajor == loot::g_version_major;
}

// Returns the version string for this version of LOOT.
// The string exists until this function is called again or until
// CleanUpAPI is called.
LOOT_API unsigned int loot_get_version(unsigned int * const versionMajor, unsigned int * const versionMinor, unsigned int * const versionPatch) {
    if (versionMajor == nullptr || versionMinor == nullptr || versionPatch == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    *versionMajor = loot::g_version_major;
    *versionMinor = loot::g_version_minor;
    *versionPatch = loot::g_version_patch;

    return loot_ok;
}

LOOT_API unsigned int loot_get_build_id(const char ** const revision) {
    if (revision == nullptr)
        return c_error(loot_error_invalid_args, "Null message pointer passed.");

    *revision = loot::g_build_revision;

    return loot_ok;
}

////////////////////////////////////
// Lifecycle Management Functions
////////////////////////////////////

// Explicitly manage database lifetime. Allows clients to free memory when
// they want/need to. clientGame sets the game the DB is for, and dataPath
// is the path to that game's Data folder, and is case-sensitive if the
// underlying filesystem is case-sensitive. This function also checks that
// plugins.txt and loadorder.txt (if they both exist) are in sync. If
// dataPath == nullptr then the API will attempt to detect the data path of
// the specified game.
LOOT_API unsigned int loot_create_db(loot_db * const db,
                                     const unsigned int clientGame,
                                     const char * const gamePath,
                                     const char * const gameLocalPath) {
    if (db == nullptr || (clientGame != loot_game_tes4 && clientGame != loot_game_tes5 && clientGame != loot_game_fo3 && clientGame != loot_game_fonv))
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    //Set the locale to get encoding conversions working correctly.
    std::locale::global(boost::locale::generator().generate(""));
    boost::filesystem::path::imbue(std::locale());

    //Disable logging or else stdout will get overrun.
    boost::log::core::get()->set_logging_enabled(false);

    std::string game_path = "";
    if (gamePath != nullptr)
        game_path = gamePath;

    boost::filesystem::path game_local_path = "";
    if (gameLocalPath != nullptr)
        game_local_path = gameLocalPath;
#ifndef _WIN32
    else
        return c_error(loot_error_invalid_args, "A local data path must be supplied on non-Windows platforms.");
#endif

    try {
        // Check for valid paths.
        if (gamePath != nullptr && !boost::filesystem::is_directory(gamePath))
            return c_error(loot_error_invalid_args, "Given game path \"" + std::string(gamePath) + "\" is not a valid directory.");

        if (gameLocalPath != nullptr && !boost::filesystem::is_directory(gameLocalPath))
            return c_error(loot_error_invalid_args, "Given local data path \"" + std::string(gameLocalPath) + "\" is not a valid directory.");

        *db = new _loot_db_int(clientGame, game_path, game_local_path);
    }
    catch (loot::error& e) {
        return c_error(e);
    }
    catch (std::bad_alloc& e) {
        return c_error(loot_error_no_mem, e.what());
    }
    catch (std::exception& e) {
        return c_error(loot_error_invalid_args, e.what());
    }

    return loot_ok;
}

// Destroys the given DB, freeing any memory allocated as part of its use.
LOOT_API void     loot_destroy_db(loot_db db) {
    delete db;
}

///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

// Loads the masterlist and userlist from the paths specified.
// Can be called multiple times. On error, the database is unchanged.
// Paths are case-sensitive if the underlying filesystem is case-sensitive.
// masterlistPath and userlistPath are files.
LOOT_API unsigned int loot_load_lists(loot_db db, const char * const masterlistPath,
                                      const char * const userlistPath) {
    if (db == nullptr || masterlistPath == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    loot::Masterlist temp;
    loot::MetadataList userTemp;

    try {
        if (boost::filesystem::exists(masterlistPath)) {
            // We don't want to update the masterlist too.
            temp.MetadataList::Load(masterlistPath);
        }
        else {
            return c_error(loot_error_path_not_found, std::string("The given masterlist path does not exist: ") + masterlistPath);
        }
    }
    catch (std::exception& e) {
        return c_error(loot_error_parse_fail, e.what());
    }

    try {
        if (userlistPath != nullptr) {
            if (boost::filesystem::exists(userlistPath)) {
                userTemp.Load(userlistPath);
            }
            else {
                return c_error(loot_error_path_not_found, std::string("The given userlist path does not exist: ") + userlistPath);
            }
        }
    }
    catch (YAML::Exception& e) {
        return c_error(loot_error_parse_fail, e.what());
    }

    //Also free memory.
    db->bashTagMap.clear();
    delete[] db->extAddedTagIds;
    delete[] db->extRemovedTagIds;

    if (db->extTagMap != nullptr) {
        for (size_t i = 0; i < db->bashTagMap.size(); i++)
            delete[] db->extTagMap[i];  //Gotta clear those allocated strings.
        delete[] db->extTagMap;
    }

    if (db->extMessageArray != nullptr) {
        for (size_t i = 0; i < db->extMessageArraySize; i++)
            delete[] db->extMessageArray[i].message;  //Gotta clear those allocated strings.
        delete[] db->extMessageArray;
    }

    db->extAddedTagIds = nullptr;
    db->extRemovedTagIds = nullptr;
    db->extTagMap = nullptr;
    db->extMessageArray = nullptr;

    db->masterlist = temp;
    db->rawMetadata = temp;
    db->userlist = userTemp;
    db->rawUserMetadata = userTemp;

    return loot_ok;
}

// Evaluates all conditional lines and regex mods the loaded masterlist.
// This exists so that Load() doesn't need to be called whenever the mods
// installed are changed. Evaluation does not take place unless this function
// is called. Repeated calls re-evaluate the masterlist from scratch each time,
// ignoring the results of any previous evaluations. Paths are case-sensitive
// if the underlying filesystem is case-sensitive.
LOOT_API unsigned int loot_eval_lists(loot_db db, const unsigned int language) {
    if (db == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");
    if (language != loot_lang_any
        && language != loot_lang_english
        && language != loot_lang_spanish
        && language != loot_lang_russian
        && language != loot_lang_french
        && language != loot_lang_chinese
        && language != loot_lang_polish
        && language != loot_lang_brazilian_portuguese
        && language != loot_lang_finnish
        && language != loot_lang_german
        && language != loot_lang_danish)
        return c_error(loot_error_invalid_args, "Invalid language code given.");

    // Clear caches before evaluating conditions.
    db->conditionCache.clear();
    db->crcCache.clear();

    loot::Masterlist temp = db->rawMetadata;
    loot::MetadataList userTemp = db->rawUserMetadata;
    try {
        // Refresh active plugins before evaluating conditions.
        db->RefreshActivePluginsList();
        temp.EvalAllConditions(*db, language);
        userTemp.EvalAllConditions(*db, language);
    }
    catch (loot::error& e) {
        return c_error(e);
    }
    db->masterlist = temp;
    db->userlist = userTemp;

    return loot_ok;
}

////////////////////////////////////
// LOOT Functionality Functions
////////////////////////////////////

LOOT_API unsigned int loot_sort_plugins(loot_db db,
                                        char *** const sortedPlugins,
                                        size_t * const numPlugins) {
    if (db == nullptr || sortedPlugins == nullptr || numPlugins == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    //Clear existing array allocation.
    if (db->extStringArray != nullptr) {
        for (size_t i = 0; i < db->extStringArraySize; ++i) {
            delete[] db->extStringArray[i];
        }
        delete[] db->extStringArray;
        db->extStringArray = nullptr;
    }

    //Initialise output.
    *numPlugins = 0;
    *sortedPlugins = nullptr;

    try {
        // Always reload all the plugins.
        db->LoadPlugins(false);

        //Sort plugins into their load order.
        loot::PluginSorter sorter;
        std::list<loot::Plugin> plugins = sorter.Sort(*db, loot_lang_any, [](const std::string& message) {});

        db->extStringArraySize = plugins.size();
        db->extStringArray = new char*[db->extStringArraySize];

        size_t i = 0;
        for (const auto &plugin : plugins) {
            db->extStringArray[i] = ToNewCString(plugin.Name());
            ++i;
        }
    }
    catch (loot::error &e) {
        return c_error(e);
    }
    catch (std::bad_alloc& e) {
        return c_error(loot_error_no_mem, e.what());
    }

    *numPlugins = db->extStringArraySize;
    *sortedPlugins = db->extStringArray;

    return loot_ok;
}

LOOT_API unsigned int loot_apply_load_order(loot_db db,
                                            const char * const * const loadOrder,
                                            const size_t numPlugins) {
    if (db == nullptr || loadOrder == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    try {
        db->SetLoadOrder(loadOrder, numPlugins);
    }
    catch (loot::error &e) {
        return c_error(e);
    }

    return loot_ok;
}

LOOT_API unsigned int loot_update_masterlist(loot_db db,
                                             const char * const masterlistPath,
                                             const char * const remoteURL,
                                             const char * const remoteBranch,
                                             bool * const updated) {
    if (db == nullptr || masterlistPath == nullptr || remoteURL == nullptr || remoteBranch == nullptr || updated == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");
    if (!boost::filesystem::is_directory(boost::filesystem::path(masterlistPath).parent_path()))
        return c_error(loot_error_invalid_args, "Given masterlist path \"" + std::string(masterlistPath) + "\" does not have a valid parent directory.");

    *updated = false;

    try {
        loot::Masterlist masterlist;
        *updated = masterlist.Update(masterlistPath, remoteURL, remoteBranch);
    }
    catch (loot::error &e) {
        return c_error(e);
    }

    return loot_ok;
}

LOOT_API unsigned int loot_get_masterlist_revision(loot_db db,
                                                   const char * const masterlistPath,
                                                   const bool getShortID,
                                                   char ** const revisionID,
                                                   char ** const revisionDate,
                                                   bool * const isModified) {
    if (db == nullptr || masterlistPath == nullptr || revisionID == nullptr || revisionDate == nullptr || isModified == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    *revisionID = nullptr;
    *revisionDate = nullptr;
    *isModified = false;

    bool edited = false;
    try {
        loot::Masterlist masterlist;
        std::string id = masterlist.GetRevision(masterlistPath, getShortID);
        std::string date = masterlist.GetDate(masterlistPath);

        if (boost::ends_with(id, " (edited)")) {
            id = id.substr(0, id.length() - 9);
            date = date.substr(0, date.length() - 9);
            edited = true;
        }

        db->extRevisionID = ToNewCString(id);
        db->extRevisionDate = ToNewCString(date);
    }
    catch (loot::error &e) {
        if (e.code() == loot_ok)
            return loot_ok;
        else
            return c_error(e);
    }
    catch (std::bad_alloc& e) {
        return c_error(loot_error_no_mem, e.what());
    }

    *revisionID = db->extRevisionID;
    *revisionDate = db->extRevisionDate;
    *isModified = edited;

    return loot_ok;
}

//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
LOOT_API unsigned int loot_get_tag_map(loot_db db, char *** const tagMap, size_t * const numTags) {
    if (db == nullptr || tagMap == nullptr || numTags == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    //Clear existing array allocation.
    if (db->extTagMap != nullptr) {
        for (size_t i = 0, max = db->bashTagMap.size(); i < max; ++i) {
            delete[] db->extTagMap[i];
        }
        delete[] db->extTagMap;
        db->extTagMap = nullptr;
    }

    //Initialise output.
    *tagMap = nullptr;
    *numTags = 0;

    std::set<std::string> allTags;

    for (const auto &plugin : db->masterlist.Plugins()) {
        for (const auto &tag : plugin.Tags()) {
            allTags.insert(tag.Name());
        }
    }
    for (const auto &plugin : db->userlist.Plugins()) {
        for (const auto &tag : plugin.Tags()) {
            allTags.insert(tag.Name());
        }
    }

    if (allTags.empty())
        return loot_ok;

    try {
        db->extTagMap = new char*[allTags.size()];
    }
    catch (std::bad_alloc& e) {
        return c_error(loot_error_no_mem, e.what());
    }

    try {
        unsigned int UID = 0;
        for (const auto &tag : allTags) {
            db->bashTagMap.insert(std::pair<std::string, unsigned int>(tag, UID));
            //Also allocate memory.
            db->extTagMap[UID] = ToNewCString(tag);
            UID++;
        }
    }
    catch (std::bad_alloc& e) {
        return c_error(loot_error_no_mem, e.what());
    }

    *tagMap = db->extTagMap;
    *numTags = allTags.size();

    return loot_ok;
}

// Returns arrays of Bash Tag UIDs for Bash Tags suggested for addition and removal
// by LOOT's masterlist and userlist, and the number of tags in each array.
// The returned arrays are valid until the db is destroyed or until the Load
// function is called.  The arrays should not be freed by the client. modName is
// case-insensitive. If no Tags are found for an array, the array pointer (*tagIds)
// will be nullptr. The userlistModified bool is true if the userlist contains Bash Tag
// suggestion message additions.
LOOT_API unsigned int loot_get_plugin_tags(loot_db db, const char * const plugin,
                                           unsigned int ** const tagIds_added,
                                           size_t * const numTags_added,
                                           unsigned int ** const tagIds_removed,
                                           size_t * const numTags_removed,
                                           bool * const userlistModified) {
    if (db == nullptr || plugin == nullptr || tagIds_added == nullptr || numTags_added == nullptr || tagIds_removed == nullptr || numTags_removed == nullptr || userlistModified == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    if (db->bashTagMap.empty()) {
        return c_error(loot_error_no_tag_map, "No Bash Tag map has been previously generated.");
    }

    //Clear existing array allocations.
    delete[] db->extAddedTagIds;
    delete[] db->extRemovedTagIds;
    db->extAddedTagIds = nullptr;
    db->extRemovedTagIds = nullptr;

    //Initialise output.
    *tagIds_added = nullptr;
    *tagIds_removed = nullptr;
    *userlistModified = false;
    *numTags_added = 0;
    *numTags_removed = 0;

    std::set<std::string> tagsAdded, tagsRemoved;
    loot::PluginMetadata p = db->masterlist.FindPlugin(loot::PluginMetadata(plugin));
    for (const auto &tag : p.Tags()) {
        if (tag.IsAddition())
            tagsAdded.insert(tag.Name());
        else
            tagsRemoved.insert(tag.Name());
    }

    p = db->userlist.FindPlugin(loot::PluginMetadata(plugin));
    *userlistModified = !p.Tags().empty();
    for (const auto &tag : p.Tags()) {
        *userlistModified = true;
        if (tag.IsAddition())
            tagsAdded.insert(tag.Name());
        else
            tagsRemoved.insert(tag.Name());
    }

    std::vector<unsigned int> tagsAddedIDs, tagsRemovedIDs;
    for (const auto &tagName : tagsAdded) {
        const auto mapIter(db->bashTagMap.find(tagName));
        if (mapIter != db->bashTagMap.end())
            tagsAddedIDs.push_back(mapIter->second);
    }
    for (const auto &tagName : tagsRemoved) {
        const auto mapIter(db->bashTagMap.find(tagName));
        if (mapIter != db->bashTagMap.end())
            tagsRemovedIDs.push_back(mapIter->second);
    }

    //Allocate memory.
    size_t numAdded = tagsAddedIDs.size();
    size_t numRemoved = tagsRemovedIDs.size();
    try {
        if (numAdded != 0) {
            db->extAddedTagIds = new uint32_t[numAdded];
            for (size_t i = 0; i < numAdded; i++)
                db->extAddedTagIds[i] = tagsAddedIDs[i];
        }
        if (numRemoved != 0) {
            db->extRemovedTagIds = new uint32_t[numRemoved];
            for (size_t i = 0; i < numRemoved; i++)
                db->extRemovedTagIds[i] = tagsRemovedIDs[i];
        }
    }
    catch (std::bad_alloc& e) {
        return c_error(loot_error_no_mem, e.what());
    }

    //Set outputs.
    *tagIds_added = db->extAddedTagIds;
    *tagIds_removed = db->extRemovedTagIds;
    *numTags_added = numAdded;
    *numTags_removed = numRemoved;

    return loot_ok;
}

// Returns the messages attached to the given plugin. Messages are valid until Load,
// loot_destroy_db or loot_get_plugin_messages are next called. plugin is case-insensitive.
// If no messages are attached, *messages will be nullptr and numMessages will equal 0.
LOOT_API unsigned int loot_get_plugin_messages(loot_db db, const char * const plugin,
                                               loot_message ** const messages,
                                               size_t * const numMessages) {
    if (db == nullptr || plugin == nullptr || messages == nullptr || numMessages == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    //Clear existing array allocation.
    if (db->extMessageArray != nullptr) {
        for (size_t i = 0; i < db->extMessageArraySize; ++i) {
            delete[] db->extMessageArray[i].message;
        }
        delete[] db->extMessageArray;
        db->extMessageArray = nullptr;
        db->extMessageArraySize = 0;
    }

    //Initialise output.
    *messages = nullptr;
    *numMessages = 0;

    loot::PluginMetadata p = db->masterlist.FindPlugin(loot::PluginMetadata(plugin));
    std::list<loot::Message> pluginMessages(p.Messages());

    p = db->userlist.FindPlugin(loot::PluginMetadata(plugin));
    std::list<loot::Message> temp(p.Messages());
    pluginMessages.insert(pluginMessages.end(), temp.begin(), temp.end());

    if (!pluginMessages.empty()) {
        db->extMessageArraySize = pluginMessages.size();
        try {
            db->extMessageArray = new loot_message[db->extMessageArraySize];
            int i = 0;
            for (const auto &message : pluginMessages) {
                db->extMessageArray[i].type = message.Type();
                db->extMessageArray[i].message = ToNewCString(message.ChooseContent(loot::Language::any).Str());
                ++i;
            }
        }
        catch (std::bad_alloc& e) {
            return c_error(loot_error_no_mem, e.what());
        }
    }

    *messages = db->extMessageArray;
    *numMessages = db->extMessageArraySize;

    return loot_ok;
}

LOOT_API unsigned int loot_get_dirty_info(loot_db db, const char * const plugin, unsigned int * const needsCleaning) {
    if (db == nullptr || plugin == nullptr || needsCleaning == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    *needsCleaning = loot_needs_cleaning_unknown;

    // Is there any dirty info? Testing for applicability happens in loot_eval_lists().
    if (!db->masterlist.FindPlugin(loot::Plugin(plugin)).DirtyInfo().empty()
        || !db->userlist.FindPlugin(loot::Plugin(plugin)).DirtyInfo().empty()) {
        *needsCleaning = loot_needs_cleaning_yes;
    }

    // Is there a message beginning with the substring "Do not clean."?
    // This isn't a very reliable system, because if the lists have been evaluated in some language
    // other than English, the strings will be in different languages (and the API can't tell what they'd be)
    // and the strings may be non-standard and begin with something other than "Do not clean." anyway.
    std::list<loot::Message> messages(db->masterlist.FindPlugin(loot::Plugin(plugin)).Messages());

    std::list<loot::Message> temp(db->userlist.FindPlugin(loot::Plugin(plugin)).Messages());
    messages.insert(messages.end(), temp.begin(), temp.end());

    for (const auto& message : messages) {
        if (boost::starts_with(message.ChooseContent(loot::Language::english).Str(), "Do not clean")) {
            *needsCleaning = loot_needs_cleaning_no;
            break;
        }
    }

    return loot_ok;
}

// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions,
// and/or dirty messages, plus the Tag suggestions and/or messages themselves and their
// conditions, in order to create the Wrye Bash taglist. outputFile is the path to use
// for output. If outputFile already exists, it will only be overwritten if overwrite is true.
LOOT_API unsigned int loot_write_minimal_list(loot_db db, const char * const outputFile, const bool overwrite) {
    if (db == nullptr || outputFile == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    if (!boost::filesystem::exists(boost::filesystem::path(outputFile).parent_path()))
        return c_error(loot_error_invalid_args, "Output directory does not exist.");

    if (boost::filesystem::exists(outputFile) && !overwrite)
        return c_error(loot_error_file_write_fail, "Output file exists but overwrite is not set to true.");

    loot::Masterlist temp = db->masterlist;
    std::unordered_set<loot::Plugin> minimalPlugins;
    for (const auto &plugin : temp.Plugins()) {
        loot::Plugin p(plugin.Name());
        p.Tags(plugin.Tags());
        p.DirtyInfo(plugin.DirtyInfo());
        minimalPlugins.insert(p);
    }

    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << YAML::BeginMap
        << YAML::Key << "plugins" << YAML::Value << minimalPlugins
        << YAML::EndMap;

    boost::filesystem::path p(outputFile);
    try {
        loot::ofstream out(p);
        if (out.fail())
            return c_error(loot_error_invalid_args, "Couldn't open output file.");
        out << yout.c_str();
        out.close();
    }
    catch (std::exception& e) {
        return c_error(loot_error_file_write_fail, e.what());
    }

    return loot_ok;
}
