/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2013-2014    WrinklyNinja

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
#include "../backend/game.h"
#include "../backend/metadata.h"
#include "../backend/parsers.h"
#include "../backend/generators.h"
#include "../backend/error.h"
#include "../backend/streams.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <clocale>
#include <list>
#include <vector>
#include <regex>
#include <unordered_set>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/filesystem.hpp>

const unsigned int loot_ok                          = loot::error::ok;
const unsigned int loot_error_liblo_error           = loot::error::liblo_error;
const unsigned int loot_error_file_write_fail       = loot::error::path_write_fail;
const unsigned int loot_error_parse_fail            = loot::error::path_read_fail;
const unsigned int loot_error_condition_eval_fail   = loot::error::condition_eval_fail;
const unsigned int loot_error_regex_eval_fail       = loot::error::regex_eval_fail;
const unsigned int loot_error_no_mem                = loot::error::no_mem;
const unsigned int loot_error_invalid_args          = loot::error::invalid_args;
const unsigned int loot_error_no_tag_map            = loot::error::no_tag_map;
const unsigned int loot_error_path_not_found        = loot::error::path_not_found;
const unsigned int loot_error_no_game_detected      = loot::error::no_game_detected;
const unsigned int loot_error_windows_error         = loot::error::windows_error;
const unsigned int loot_error_sorting_error         = loot::error::sorting_error;
const unsigned int loot_return_max                  = loot_error_sorting_error;

// The following are the games identifiers used by the API.
const unsigned int loot_game_tes4                   = loot::Game::tes4;
const unsigned int loot_game_tes5                   = loot::Game::tes5;
const unsigned int loot_game_fo3                    = loot::Game::fo3;
const unsigned int loot_game_fonv                   = loot::Game::fonv;

// LOOT message types.
const unsigned int loot_message_say                 = loot::Message::say;
const unsigned int loot_message_warn                = loot::Message::warn;
const unsigned int loot_message_error               = loot::Message::error;
const unsigned int loot_message_tag                 = loot::Message::tag;

// LOOT message languages.
const unsigned int loot_lang_any                    = loot::Language::any;
const unsigned int loot_lang_english                = loot::Language::english;
const unsigned int loot_lang_spanish                = loot::Language::spanish;
const unsigned int loot_lang_russian                = loot::Language::russian;
const unsigned int loot_lang_french                 = loot::Language::french;
const unsigned int loot_lang_chinese                = loot::Language::chinese;
const unsigned int loot_lang_polish                 = loot::Language::polish;
const unsigned int loot_lang_brazilian_portuguese   = loot::Language::brazilian_portuguese;
const unsigned int loot_lang_finnish                = loot::Language::finnish;
const unsigned int loot_lang_german                 = loot::Language::german;

// LOOT cleanliness codes.
const unsigned int loot_needs_cleaning_no       = 0;
const unsigned int loot_needs_cleaning_yes      = 1;
const unsigned int loot_needs_cleaning_unknown  = 2;


struct _loot_db_int {
    _loot_db_int()
        : extTagMap(nullptr),
        extAddedTagIds(nullptr),
        extRemovedTagIds(nullptr),
        extMessageArray(nullptr),
        extMessageArraySize(0) {

        extMessage.type = loot_message_say;
        extMessage.message = nullptr;
    }

    ~_loot_db_int() {
        delete [] extAddedTagIds;
        delete [] extRemovedTagIds;
        delete [] extMessage.message;

        if (extTagMap != nullptr) {
            for (size_t i=0; i < bashTagMap.size(); i++)
                delete [] extTagMap[i];  //Gotta clear those allocated strings.
            delete [] extTagMap;
        }

        if (extMessageArray != nullptr) {
            for (size_t i=0; i < extMessageArraySize; i++)
                delete [] extMessageArray[i].message;  //Gotta clear those allocated strings.
            delete [] extMessageArray;
        }
    }

    loot::Game game;
    std::list<loot::Plugin> metadata, rawMetadata, userMetadata, rawUserMetadata;

    std::unordered_map<std::string, unsigned int> bashTagMap;

    char ** extTagMap;

    unsigned int * extAddedTagIds;
    unsigned int * extRemovedTagIds;

    loot_message extMessage;
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
LOOT_API unsigned int loot_get_error_message (const char ** const message) {
    if (message == nullptr)
        return c_error(loot_error_invalid_args, "Null message pointer passed.");

    *message = extMessageStr;

    return loot_ok;
}

// Frees memory allocated to error string.
LOOT_API void     loot_cleanup () {
    delete [] extMessageStr;
    extMessageStr = nullptr;
}


//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of LOOT supports the API from the given
// LOOT version. Abstracts LOOT API stability policy away from clients.
LOOT_API bool loot_is_compatible (const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch) {
    return versionMajor == loot::g_version_major && versionMinor == loot::g_version_minor;
}

// Returns the version string for this version of LOOT.
// The string exists until this function is called again or until
// CleanUpAPI is called.
LOOT_API unsigned int loot_get_version (unsigned int * const versionMajor, unsigned int * const versionMinor, unsigned int * const versionPatch) {
    if (versionMajor == nullptr || versionMinor == nullptr || versionPatch == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    *versionMajor = loot::g_version_major;
    *versionMinor = loot::g_version_minor;
    *versionPatch = loot::g_version_patch;

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
LOOT_API unsigned int loot_create_db (loot_db * const db, const unsigned int clientGame, const char * const gamePath) {
    if (db == nullptr || (clientGame != loot_game_tes4 && clientGame != loot_game_tes5 && clientGame != loot_game_fo3 && clientGame != loot_game_fonv))
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    //Set the locale to get encoding conversions working correctly.
    std::setlocale(LC_CTYPE, "");
    std::locale global_loc = std::locale();
    std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
    boost::filesystem::path::imbue(loc);

    //Disable logging or else stdout will get overrun.
    boost::log::core::get()->set_logging_enabled(false);

    std::string game_path = "";
    if (gamePath != nullptr)
        game_path = gamePath;

    loot::Game game;
    try {
        game = loot::Game(clientGame).SetPath(game_path).Init();  //This also checks to see if the game is installed if game_path is empty and throws an exception if it is not detected. It also creates a folder in %LOCALAPPDATA% and reads the active plugins list, but that shouldn't be an issue.
    } catch (loot::error& e) {
        return c_error(e);
    }

    loot_db retVal;
    try {
        retVal = new _loot_db_int;
    } catch (std::bad_alloc& e) {
        return c_error(loot_error_no_mem, e.what());
    }
    retVal->game = game;
    *db = retVal;

    return loot_ok;
}

// Destroys the given DB, freeing any memory allocated as part of its use.
LOOT_API void     loot_destroy_db (loot_db db) {
    delete db;
}


///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

// Loads the masterlist and userlist from the paths specified.
// Can be called multiple times. On error, the database is unchanged.
// Paths are case-sensitive if the underlying filesystem is case-sensitive.
// masterlistPath and userlistPath are files.
LOOT_API unsigned int loot_load_lists (loot_db db, const char * const masterlistPath,
                                    const char * const userlistPath) {
    if (db == nullptr || masterlistPath == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    std::list<loot::Plugin> temp;
    std::list<loot::Plugin> userTemp;

    try {
        if (boost::filesystem::exists(masterlistPath)) {
            loot::ifstream in(masterlistPath);
            YAML::Node tempNode = YAML::Load(in);
            in.close();
            temp = tempNode["plugins"].as< std::list<loot::Plugin> >();
        }
    } catch (std::exception& e) {
        return c_error(loot_error_parse_fail, e.what());
    }

    try {
        if (userlistPath != nullptr) {
            if (boost::filesystem::exists(userlistPath)) {
                if (boost::algorithm::iends_with(userlistPath, ".yaml")) {
                    loot::ifstream in(userlistPath);
                    YAML::Node tempNode = YAML::Load(in);
                    in.close();
                    userTemp = tempNode["plugins"].as< std::list<loot::Plugin> >();
                }
            }
        }
    } catch (YAML::Exception& e) {
        return c_error(loot_error_parse_fail, e.what());
    }

    //Also free memory.
    db->bashTagMap.clear();
    delete [] db->extAddedTagIds;
    delete [] db->extRemovedTagIds;

    if (db->extTagMap != nullptr) {
        for (size_t i=0; i < db->bashTagMap.size(); i++)
            delete [] db->extTagMap[i];  //Gotta clear those allocated strings.
        delete [] db->extTagMap;
    }

    if (db->extMessageArray != nullptr) {
        for (size_t i=0; i < db->extMessageArraySize; i++)
            delete [] db->extMessageArray[i].message;  //Gotta clear those allocated strings.
        delete [] db->extMessageArray;
    }

    db->extAddedTagIds = nullptr;
    db->extRemovedTagIds = nullptr;
    db->extTagMap = nullptr;
    db->extMessageArray = nullptr;

    db->rawMetadata = temp;
    db->metadata = temp;
    db->userMetadata = userTemp;
    db->rawUserMetadata = userTemp;

    return loot_ok;
}

// Evaluates all conditional lines and regex mods the loaded masterlist.
// This exists so that Load() doesn't need to be called whenever the mods
// installed are changed. Evaluation does not take place unless this function
// is called. Repeated calls re-evaluate the masterlist from scratch each time,
// ignoring the results of any previous evaluations. Paths are case-sensitive
// if the underlying filesystem is case-sensitive.
LOOT_API unsigned int loot_eval_lists (loot_db db, const unsigned int language) {
    if (db == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    std::list<loot::Plugin> temp = db->rawMetadata;
    try {
        db->game.RefreshActivePluginsList();
        for (auto it=temp.begin(); it != temp.end();) {
            it->EvalAllConditions(db->game, language);
            if (it->IsRegexPlugin()) {
                boost::regex regex;
                try {
                    regex = boost::regex(it->Name(), boost::regex::perl|boost::regex::icase);
                } catch (boost::regex_error& e) {
                    return c_error(loot_error_regex_eval_fail, e.what());
                }

                for (boost::filesystem::directory_iterator itr(db->game.DataPath()); itr != boost::filesystem::directory_iterator(); ++itr) {
                    const std::string filename = itr->path().filename().string();
                    if (boost::regex_match(filename, regex)) {
                        loot::Plugin p = *it;
                        p.Name(filename);
                        temp.push_back(p);
                    }
                }
                it = temp.erase(it);
            } else {
                ++it;
            }
        }
    } catch (loot::error& e) {
        return c_error(e);
    }
    db->metadata = temp;

    temp = db->rawUserMetadata;
    try {
        for (auto it=temp.begin(); it != temp.end();) {
            it->EvalAllConditions(db->game, language);
            if (it->IsRegexPlugin()) {
                boost::regex regex;
                try {
                    regex = boost::regex(it->Name(), boost::regex::perl|boost::regex::icase);
                } catch (boost::regex_error& e) {
                    return c_error(loot_error_regex_eval_fail, e.what());
                }

                for (boost::filesystem::directory_iterator itr(db->game.DataPath()); itr != boost::filesystem::directory_iterator(); ++itr) {
                    const std::string filename = itr->path().filename().string();
                    if (boost::regex_match(filename, regex)) {
                        loot::Plugin p = *it;
                        p.Name(filename);
                        temp.push_back(p);
                    }
                }
                it = temp.erase(it);
            } else {
                ++it;
            }
        }
    } catch (loot::error& e) {
        return c_error(e);
    }
    db->userMetadata = temp;

    return loot_ok;
}


//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
LOOT_API unsigned int loot_get_tag_map (loot_db db, char *** const tagMap, size_t * const numTags) {
    if (db == nullptr || tagMap == nullptr || numTags == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    //Clear existing array allocation.
    if (db->extTagMap != nullptr) {
        for (size_t i=0, max=db->bashTagMap.size(); i < max; ++i) {
            delete [] db->extTagMap[i];
        }
        delete [] db->extTagMap;
        db->extTagMap = nullptr;
    }

    //Initialise output.
    *tagMap = nullptr;
    *numTags = 0;

    std::unordered_set<std::string> allTags;

    for (const auto &plugin: db->metadata) {
        std::set<loot::Tag> tags(plugin.Tags());
        for (const auto &tag: tags) {
            allTags.insert(tag.Name());
        }
    }
    for (const auto &plugin : db->userMetadata) {
        std::set<loot::Tag> tags(plugin.Tags());
        for (const auto &tag : tags) {
            allTags.insert(tag.Name());
        }
    }

    if (allTags.empty())
        return loot_ok;

    try {
        db->extTagMap = new char*[allTags.size()];
    } catch (std::bad_alloc& e) {
        return c_error(loot_error_no_mem, e.what());
    }

    unsigned int UID = 0;
    try {
        for (const auto &tag: allTags) {
            db->bashTagMap.emplace(tag, UID);
            //Also allocate memory.
            db->extTagMap[UID] = ToNewCString(tag);
            UID++;
        }
    } catch (std::bad_alloc& e) {
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
LOOT_API unsigned int loot_get_plugin_tags (loot_db db, const char * const plugin,
                                            unsigned int ** const tagIds_added,
                                            size_t * const numTags_added,
                                            unsigned int ** const tagIds_removed,
                                            size_t * const numTags_removed,
                                            bool * const userlistModified) {
    if (db == nullptr || plugin == nullptr || tagIds_added == nullptr || numTags_added == nullptr || tagIds_removed == nullptr || numTags_removed == nullptr || userlistModified == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");


    //Clear existing array allocations.
    delete [] db->extAddedTagIds;
    delete [] db->extRemovedTagIds;
    db->extAddedTagIds = nullptr;
    db->extRemovedTagIds = nullptr;

    //Initialise output.
    *tagIds_added = nullptr;
    *tagIds_removed = nullptr;
    *userlistModified = false;
    *numTags_added = 0;
    *numTags_removed = 0;

    std::unordered_set<std::string> tagsAdded, tagsRemoved;
    std::list<loot::Plugin>::iterator pluginIt = std::find(db->metadata.begin(), db->metadata.end(), loot::Plugin(plugin));
    if (pluginIt != db->metadata.end()) {
        std::set<loot::Tag> tags(pluginIt->Tags());
        for (const auto &tag: tags) {
            if (tag.IsAddition())
                tagsAdded.insert(tag.Name());
            else
                tagsRemoved.insert(tag.Name());
        }
    }

    pluginIt = std::find(db->userMetadata.begin(), db->userMetadata.end(), loot::Plugin(plugin));
    if (pluginIt != db->userMetadata.end()) {
        *userlistModified = true;
        std::set<loot::Tag> tags(pluginIt->Tags());
        for (const auto &tag : tags) {
            if (tag.IsAddition())
                tagsAdded.insert(tag.Name());
            else
                tagsRemoved.insert(tag.Name());
        }
    }

    if ((!tagsAdded.empty() || !tagsRemoved.empty()) && db->bashTagMap.empty()) {
        return c_error(loot_error_no_tag_map, "No Bash Tag map has been previously generated.");
    }

    std::vector<unsigned int> tagsAddedIDs, tagsRemovedIDs;
    for (const auto &tagNames: tagsAdded) {
        const auto mapIter(db->bashTagMap.find(tagNames));
        if (mapIter != db->bashTagMap.end())
            tagsAddedIDs.push_back(mapIter->second);
    }
    for (const auto &tagNames : tagsRemoved) {
        const auto mapIter(db->bashTagMap.find(tagNames));
        if (mapIter != db->bashTagMap.end())
            tagsAddedIDs.push_back(mapIter->second);
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
    } catch (std::bad_alloc& e) {
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
LOOT_API unsigned int loot_get_plugin_messages (loot_db db, const char * const plugin,
                                                loot_message ** const messages,
                                                size_t * const numMessages) {
    if (db == nullptr || plugin == nullptr || messages == nullptr || numMessages == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    //Clear existing array allocation.
    if (db->extMessageArray != nullptr) {
        for (size_t i=0; i < db->extMessageArraySize; ++i) {
            delete [] db->extMessageArray[i].message;
        }
        delete [] db->extMessageArray;
        db->extMessageArray = nullptr;
    }

    //Initialise output.
    *messages = nullptr;
    *numMessages = 0;

    std::list<loot::Message> pluginMessages;
    std::list<loot::Plugin>::iterator pluginIt = std::find(db->metadata.begin(), db->metadata.end(), loot::Plugin(plugin));
    if (pluginIt != db->metadata.end()) {
        pluginMessages = pluginIt->Messages();
    }

    pluginIt = std::find(db->userMetadata.begin(), db->userMetadata.end(), loot::Plugin(plugin));
    if (pluginIt != db->userMetadata.end()) {
        std::list<loot::Message> temp = pluginIt->Messages();
        pluginMessages.insert(pluginMessages.end(), temp.begin(), temp.end());
    }

    db->extMessageArraySize = pluginMessages.size();
    try {
        db->extMessageArray = new loot_message[db->extMessageArraySize];
        int i = 0;
        for (const auto &message: pluginMessages) {
            db->extMessageArray[i].type = message.Type();
            db->extMessageArray[i].message = ToNewCString(message.ChooseContent(loot::Language::any).Str());
        }
    } catch (std::bad_alloc& e) {
        return c_error(loot_error_no_mem, e.what());
    }

    *messages = db->extMessageArray;
    *numMessages = db->extMessageArraySize;

    return loot_ok;
}

LOOT_API unsigned int loot_get_dirty_info(loot_db db, const char * const plugin, unsigned int * const needsCleaning) {
    if (db == nullptr || plugin == nullptr || needsCleaning == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    *needsCleaning = loot_needs_cleaning_unknown;

    //Get all dirty info.
    std::set<loot::PluginDirtyInfo> dirtyInfo;
    std::list<loot::Plugin>::iterator pluginIt = std::find(db->metadata.begin(), db->metadata.end(), loot::Plugin(plugin));
    if (pluginIt != db->metadata.end()) {
        dirtyInfo = pluginIt->DirtyInfo();
    }
    pluginIt = std::find(db->userMetadata.begin(), db->userMetadata.end(), loot::Plugin(plugin));
    if (pluginIt != db->userMetadata.end()) {
        std::set<loot::PluginDirtyInfo> temp = pluginIt->DirtyInfo();
        dirtyInfo.insert(temp.begin(), temp.end());
    }

    if (!dirtyInfo.empty()) {
        *needsCleaning = loot_needs_cleaning_yes;
    }

    return loot_ok;
}


// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions,
// and/or dirty messages, plus the Tag suggestions and/or messages themselves and their
// conditions, in order to create the Wrye Bash taglist. outputFile is the path to use
// for output. If outputFile already exists, it will only be overwritten if overwrite is true.
LOOT_API unsigned int loot_write_minimal_list (loot_db db, const char * const outputFile, const bool overwrite) {
    if (db == nullptr || outputFile == nullptr)
        return c_error(loot_error_invalid_args, "Null pointer passed.");

    if (boost::filesystem::exists(outputFile) && !overwrite)
        return c_error(loot_error_invalid_args, "Output file exists but overwrite is not set to true.");

    std::list<loot::Plugin> temp = db->metadata;
    for (auto &plugin: temp) {
        loot::Plugin p(plugin.Name());
        p.Tags(plugin.Tags());
        p.DirtyInfo(plugin.DirtyInfo());

        plugin = p;
    }

    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << YAML::BeginMap
         << YAML::Key << "plugins" << YAML::Value << temp
         << YAML::EndMap;

    boost::filesystem::path p(outputFile);
    loot::ofstream out(p);
    if (out.fail())
        return c_error(loot_error_invalid_args, "Couldn't open output file");
    out << yout.c_str();
    out.close();

    return loot_ok;
}
