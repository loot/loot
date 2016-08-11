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

#include "loot/api.h"

#include <algorithm>
#include <clocale>
#include <list>
#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/log/core.hpp>
#include <yaml-cpp/yaml.h>

#include "api/loot_db.h"
#include "backend/error.h"
#include "backend/app/loot_paths.h"
#include "backend/app/loot_version.h"
#include "backend/plugin/plugin_sorter.h"

using loot::Error;
using loot::GameType;
using loot::MessageType;
using loot::Language;

const unsigned int loot_ok = Error::asUnsignedInt(Error::Code::ok);
const unsigned int loot_error_liblo_error = Error::asUnsignedInt(Error::Code::liblo_error);
const unsigned int loot_error_file_write_fail = Error::asUnsignedInt(Error::Code::path_write_fail);
const unsigned int loot_error_parse_fail = Error::asUnsignedInt(Error::Code::path_read_fail);
const unsigned int loot_error_condition_eval_fail = Error::asUnsignedInt(Error::Code::condition_eval_fail);
const unsigned int loot_error_regex_eval_fail = Error::asUnsignedInt(Error::Code::regex_eval_fail);
const unsigned int loot_error_no_mem = Error::asUnsignedInt(Error::Code::no_mem);
const unsigned int loot_error_invalid_args = Error::asUnsignedInt(Error::Code::invalid_args);
const unsigned int loot_error_no_tag_map = Error::asUnsignedInt(Error::Code::no_tag_map);
const unsigned int loot_error_path_not_found = Error::asUnsignedInt(Error::Code::path_not_found);
const unsigned int loot_error_no_game_detected = Error::asUnsignedInt(Error::Code::no_game_detected);
const unsigned int loot_error_git_error = Error::asUnsignedInt(Error::Code::git_error);
const unsigned int loot_error_windows_error = Error::asUnsignedInt(Error::Code::windows_error);
const unsigned int loot_error_sorting_error = Error::asUnsignedInt(Error::Code::sorting_error);
const unsigned int loot_return_max = loot_error_sorting_error;

// The following are the games identifiers used by the API.
const unsigned int loot_game_tes4 = static_cast<unsigned int>(GameType::tes4);
const unsigned int loot_game_tes5 = static_cast<unsigned int>(GameType::tes5);
const unsigned int loot_game_fo3 = static_cast<unsigned int>(GameType::fo3);
const unsigned int loot_game_fonv = static_cast<unsigned int>(GameType::fonv);
const unsigned int loot_game_fo4 = static_cast<unsigned int>(GameType::fo4);

const unsigned int loot_message_say = static_cast<unsigned int>(MessageType::say);
const unsigned int loot_message_warn = static_cast<unsigned int>(MessageType::warn);
const unsigned int loot_message_error = static_cast<unsigned int>(MessageType::error);

// LOOT message languages.
const unsigned int loot_lang_english = static_cast<unsigned int>(Language::Code::english);
const unsigned int loot_lang_spanish = static_cast<unsigned int>(Language::Code::spanish);
const unsigned int loot_lang_russian = static_cast<unsigned int>(Language::Code::russian);
const unsigned int loot_lang_french = static_cast<unsigned int>(Language::Code::french);
const unsigned int loot_lang_chinese = static_cast<unsigned int>(Language::Code::chinese);
const unsigned int loot_lang_polish = static_cast<unsigned int>(Language::Code::polish);
const unsigned int loot_lang_brazilian_portuguese = static_cast<unsigned int>(Language::Code::brazilian_portuguese);
const unsigned int loot_lang_finnish = static_cast<unsigned int>(Language::Code::finnish);
const unsigned int loot_lang_german = static_cast<unsigned int>(Language::Code::german);
const unsigned int loot_lang_danish = static_cast<unsigned int>(Language::Code::danish);
const unsigned int loot_lang_korean = static_cast<unsigned int>(Language::Code::korean);

// LOOT cleanliness codes.
const unsigned int loot_needs_cleaning_no = 0;
const unsigned int loot_needs_cleaning_yes = 1;
const unsigned int loot_needs_cleaning_unknown = 2;

std::string extMessageStr;

unsigned int c_error(const Error& e) {
  extMessageStr = e.what();
  return e.codeAsUnsignedInt();
}

unsigned int c_error(const unsigned int code, const std::string& what) {
  return c_error(Error(Error::Code(code), what.c_str()));
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

  *message = extMessageStr.c_str();

  return loot_ok;
}

//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of LOOT supports the API from the given
// LOOT version. Abstracts LOOT API stability policy away from clients.
LOOT_API bool loot_is_compatible(const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch) {
  if (versionMajor > 0)
    return versionMajor == loot::LootVersion::major;
  else
    return versionMinor == loot::LootVersion::minor;
}

// Returns the version string for this version of LOOT.
// The string exists until this function is called again or until
// CleanUpAPI is called.
LOOT_API unsigned int loot_get_version(unsigned int * const versionMajor, unsigned int * const versionMinor, unsigned int * const versionPatch) {
  if (versionMajor == nullptr || versionMinor == nullptr || versionPatch == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

  *versionMajor = loot::LootVersion::major;
  *versionMinor = loot::LootVersion::minor;
  *versionPatch = loot::LootVersion::patch;

  return loot_ok;
}

LOOT_API unsigned int loot_get_build_id(const char ** const revision) {
  if (revision == nullptr)
    return c_error(loot_error_invalid_args, "Null message pointer passed.");

  *revision = loot::LootVersion::revision.c_str();

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
LOOT_API unsigned int loot_create_db(loot_db ** const db,
                                     const unsigned int clientGame,
                                     const char * const gamePath,
                                     const char * const gameLocalPath) {
  if (db == nullptr
      || (clientGame != loot_game_tes4
          && clientGame != loot_game_tes5
          && clientGame != loot_game_fo3
          && clientGame != loot_game_fonv
          && clientGame != loot_game_fo4))
    return c_error(loot_error_invalid_args, "Null pointer passed.");

  loot::LootPaths::initialise();

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

    *db = new loot_db(clientGame, game_path, game_local_path);
  } catch (Error& e) {
    return c_error(e);
  } catch (std::bad_alloc& e) {
    return c_error(loot_error_no_mem, e.what());
  } catch (std::exception& e) {
    return c_error(loot_error_invalid_args, e.what());
  }

  return loot_ok;
}

// Destroys the given DB, freeing any memory allocated as part of its use.
LOOT_API void     loot_destroy_db(loot_db * const db) {
  delete db;
}

///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

// Loads the masterlist and userlist from the paths specified.
// Can be called multiple times. On error, the database is unchanged.
// Paths are case-sensitive if the underlying filesystem is case-sensitive.
// masterlistPath and userlistPath are files.
LOOT_API unsigned int loot_load_lists(loot_db * const db, const char * const masterlistPath,
                                      const char * const userlistPath) {
  if (db == nullptr || masterlistPath == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

  loot::Masterlist temp;
  loot::MetadataList userTemp;

  try {
    if (boost::filesystem::exists(masterlistPath)) {
      temp.Load(masterlistPath);
    } else {
      return c_error(loot_error_path_not_found, std::string("The given masterlist path does not exist: ") + masterlistPath);
    }
  } catch (std::exception& e) {
    return c_error(loot_error_parse_fail, e.what());
  }

  try {
    if (userlistPath != nullptr) {
      if (boost::filesystem::exists(userlistPath)) {
        userTemp.Load(userlistPath);
      } else {
        return c_error(loot_error_path_not_found, std::string("The given userlist path does not exist: ") + userlistPath);
      }
    }
  } catch (YAML::Exception& e) {
    return c_error(loot_error_parse_fail, e.what());
  }

  //Also free memory.
  db->clearBashTagMap();
  db->clearArrays();

  db->GetMasterlist() = temp;
  db->getUnevaluatedMasterlist() = temp;
  db->GetUserlist() = userTemp;
  db->getUnevaluatedUserlist() = userTemp;

  return loot_ok;
}

// Evaluates all conditional lines and regex mods the loaded masterlist.
// This exists so that Load() doesn't need to be called whenever the mods
// installed are changed. Evaluation does not take place unless this function
// is called. Repeated calls re-evaluate the masterlist from scratch each time,
// ignoring the results of any previous evaluations. Paths are case-sensitive
// if the underlying filesystem is case-sensitive.
LOOT_API unsigned int loot_eval_lists(loot_db * const db, const unsigned int language) {
  if (db == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");
  if (language != loot_lang_english
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
  db->ClearCachedConditions();

  loot::Masterlist temp = db->getUnevaluatedMasterlist();
  loot::MetadataList userTemp = db->getUnevaluatedUserlist();
  try {
      // Refresh active plugins before evaluating conditions.
    temp.EvalAllConditions(*db, Language(Language::Code(language)).GetCode());
    userTemp.EvalAllConditions(*db, Language(Language::Code(language)).GetCode());
  } catch (loot::Error& e) {
    return c_error(e);
  }
  db->GetMasterlist() = temp;
  db->GetUserlist() = userTemp;

  return loot_ok;
}

////////////////////////////////////
// LOOT Functionality Functions
////////////////////////////////////

LOOT_API unsigned int loot_sort_plugins(loot_db * const db,
                                        const char * const ** const sortedPlugins,
                                        size_t * const numPlugins) {
  if (db == nullptr || sortedPlugins == nullptr || numPlugins == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

//Initialise output.
  *numPlugins = 0;
  *sortedPlugins = nullptr;

  try {
      // Always reload all the plugins.
    db->LoadPlugins(false);

    //Sort plugins into their load order.
    loot::PluginSorter sorter;

    db->setPluginNames(sorter.Sort(*db, loot::Language::Code::english));
  } catch (Error &e) {
    return c_error(e);
  } catch (std::bad_alloc& e) {
    return c_error(loot_error_no_mem, e.what());
  }

  if (db->getPluginNames().empty())
    return loot_ok;

  *numPlugins = db->getPluginNames().size();
  *sortedPlugins = &db->getPluginNames()[0];

  return loot_ok;
}

LOOT_API unsigned int loot_apply_load_order(loot_db * const db,
                                            const char * const * const loadOrder,
                                            const size_t numPlugins) {
  if (db == nullptr || loadOrder == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

  try {
    db->SetLoadOrder(loadOrder, numPlugins);
  } catch (Error &e) {
    return c_error(e);
  }

  return loot_ok;
}

LOOT_API unsigned int loot_update_masterlist(loot_db * const db,
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
  } catch (Error &e) {
    return c_error(e);
  }

  return loot_ok;
}

LOOT_API unsigned int loot_get_masterlist_revision(loot_db * const db,
                                                   const char * const masterlistPath,
                                                   const bool getShortID,
                                                   const char ** const revisionID,
                                                   const char ** const revisionDate,
                                                   bool * const isModified) {
  if (db == nullptr || masterlistPath == nullptr || revisionID == nullptr || revisionDate == nullptr || isModified == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

  *revisionID = nullptr;
  *revisionDate = nullptr;
  *isModified = false;

  bool edited = false;
  try {
    loot::Masterlist::Info info = loot::Masterlist::GetInfo(masterlistPath, getShortID);
    std::string id = info.revision;
    std::string date = info.date;

    if (boost::ends_with(id, " (edited)")) {
      id = id.substr(0, id.length() - 9);
      date = date.substr(0, date.length() - 9);
      edited = true;
    }

    db->setRevisionIdString(id);
    db->setRevisionDateString(date);
  } catch (Error &e) {
    if (e.code() == Error::Code::ok)
      return loot_ok;
    else
      return c_error(e);
  } catch (std::bad_alloc& e) {
    return c_error(loot_error_no_mem, e.what());
  }

  *revisionID = db->getRevisionIdString();
  *revisionDate = db->getRevisionDateString();
  *isModified = edited;

  return loot_ok;
}

//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
LOOT_API unsigned int loot_get_tag_map(loot_db * const db, const char * const ** const tagMap, size_t * const numTags) {
  if (db == nullptr || tagMap == nullptr || numTags == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

//Clear existing array allocation.
  db->clearBashTagMap();

  //Initialise output.
  *tagMap = nullptr;
  *numTags = 0;

  std::set<std::string> allTags;

  for (const auto &plugin : db->GetMasterlist().Plugins()) {
    for (const auto &tag : plugin.Tags()) {
      allTags.insert(tag.Name());
    }
  }
  for (const auto &plugin : db->GetUserlist().Plugins()) {
    for (const auto &tag : plugin.Tags()) {
      allTags.insert(tag.Name());
    }
  }

  if (allTags.empty())
    return loot_ok;

  try {
    db->addBashTagsToMap(allTags);
  } catch (std::bad_alloc& e) {
    return c_error(loot_error_no_mem, e.what());
  }

  *tagMap = &db->getBashTagMap()[0];
  *numTags = db->getBashTagMap().size();

  return loot_ok;
}

// Returns arrays of Bash Tag UIDs for Bash Tags suggested for addition and removal
// by LOOT's masterlist and userlist, and the number of tags in each array.
// The returned arrays are valid until the db is destroyed or until the Load
// function is called.  The arrays should not be freed by the client. modName is
// case-insensitive. If no Tags are found for an array, the array pointer (*tagIds)
// will be nullptr. The userlistModified bool is true if the userlist contains Bash Tag
// suggestion message additions.
LOOT_API unsigned int loot_get_plugin_tags(loot_db * const db, const char * const plugin,
                                           const unsigned int ** const tagIds_added,
                                           size_t * const numTags_added,
                                           const unsigned int ** const tagIds_removed,
                                           size_t * const numTags_removed,
                                           bool * const userlistModified) {
  if (db == nullptr || plugin == nullptr || tagIds_added == nullptr || numTags_added == nullptr || tagIds_removed == nullptr || numTags_removed == nullptr || userlistModified == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

  if (db->getBashTagMap().empty()) {
    return c_error(loot_error_no_tag_map, "No Bash Tag map has been previously generated.");
  }

  //Initialise output.
  *tagIds_added = nullptr;
  *tagIds_removed = nullptr;
  *userlistModified = false;
  *numTags_added = 0;
  *numTags_removed = 0;

  std::set<std::string> tagsAdded, tagsRemoved;
  loot::PluginMetadata p = db->GetMasterlist().FindPlugin(loot::PluginMetadata(plugin));
  for (const auto &tag : p.Tags()) {
    if (tag.IsAddition())
      tagsAdded.insert(tag.Name());
    else
      tagsRemoved.insert(tag.Name());
  }

  p = db->GetUserlist().FindPlugin(loot::PluginMetadata(plugin));
  *userlistModified = !p.Tags().empty();
  for (const auto &tag : p.Tags()) {
    *userlistModified = true;
    if (tag.IsAddition())
      tagsAdded.insert(tag.Name());
    else
      tagsRemoved.insert(tag.Name());
  }

  try {
    db->setAddedTags(tagsAdded);
    db->setRemovedTags(tagsRemoved);
  } catch (Error& e) {
    return c_error(e);
  }

  //Set outputs.
  *numTags_added = db->getAddedTagIds().size();
  *numTags_removed = db->getRemovedTagIds().size();

  if (db->getAddedTagIds().empty())
    *tagIds_added = nullptr;
  else
    *tagIds_added = reinterpret_cast<const unsigned int*>(&db->getAddedTagIds()[0]);

  if (db->getRemovedTagIds().empty())
    *tagIds_removed = nullptr;
  else
    *tagIds_removed = reinterpret_cast<const unsigned int*>(&db->getRemovedTagIds()[0]);

  return loot_ok;
}

// Returns the messages attached to the given plugin. Messages are valid until Load,
// loot_destroy_db or loot_get_plugin_messages are next called. plugin is case-insensitive.
// If no messages are attached, *messages will be nullptr and numMessages will equal 0.
LOOT_API unsigned int loot_get_plugin_messages(loot_db * const db, const char * const plugin,
                                               const loot_message ** const messages,
                                               size_t * const numMessages) {
  if (db == nullptr || plugin == nullptr || messages == nullptr || numMessages == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

//Initialise output.
  *messages = nullptr;
  *numMessages = 0;

  loot::PluginMetadata p = db->GetMasterlist().FindPlugin(loot::PluginMetadata(plugin));
  std::list<loot::Message> pluginMessages(p.Messages());

  p = db->GetUserlist().FindPlugin(loot::PluginMetadata(plugin));
  std::list<loot::Message> temp(p.Messages());
  pluginMessages.insert(pluginMessages.end(), temp.begin(), temp.end());

  if (pluginMessages.empty())
    return loot_ok;

  db->setPluginMessages(pluginMessages);

  *messages = &db->getPluginMessages()[0];
  *numMessages = db->getPluginMessages().size();

  return loot_ok;
}

LOOT_API unsigned int loot_get_dirty_info(loot_db * const db, const char * const plugin, unsigned int * const needsCleaning) {
  if (db == nullptr || plugin == nullptr || needsCleaning == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

  *needsCleaning = loot_needs_cleaning_unknown;

  // Is there any dirty info? Testing for applicability happens in loot_eval_lists().
  if (!db->GetMasterlist().FindPlugin(loot::PluginMetadata(plugin)).DirtyInfo().empty()
      || !db->GetUserlist().FindPlugin(loot::PluginMetadata(plugin)).DirtyInfo().empty()) {
    *needsCleaning = loot_needs_cleaning_yes;
  }

  // Is there a message beginning with the substring "Do not clean."?
  // This isn't a very reliable system, because if the lists have been evaluated in some language
  // other than English, the strings will be in different languages (and the API can't tell what they'd be)
  // and the strings may be non-standard and begin with something other than "Do not clean." anyway.
  std::list<loot::Message> messages(db->GetMasterlist().FindPlugin(loot::PluginMetadata(plugin)).Messages());

  std::list<loot::Message> temp(db->GetUserlist().FindPlugin(loot::PluginMetadata(plugin)).Messages());
  messages.insert(messages.end(), temp.begin(), temp.end());

  for (const auto& message : messages) {
    if (boost::starts_with(message.GetText(), "Do not clean")) {
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
LOOT_API unsigned int loot_write_minimal_list(loot_db * const db, const char * const outputFile, const bool overwrite) {
  if (db == nullptr || outputFile == nullptr)
    return c_error(loot_error_invalid_args, "Null pointer passed.");

  if (!boost::filesystem::exists(boost::filesystem::path(outputFile).parent_path()))
    return c_error(loot_error_invalid_args, "Output directory does not exist.");

  if (boost::filesystem::exists(outputFile) && !overwrite)
    return c_error(loot_error_file_write_fail, "Output file exists but overwrite is not set to true.");

  loot::Masterlist temp = db->GetMasterlist();
  std::unordered_set<loot::PluginMetadata> minimalPlugins;
  for (const auto &plugin : temp.Plugins()) {
    loot::PluginMetadata p(plugin.Name());
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
    boost::filesystem::ofstream out(p);
    if (out.fail())
      return c_error(loot_error_file_write_fail, "Couldn't open output file.");
    out << yout.c_str();
    out.close();
  } catch (std::exception& e) {
    return c_error(loot_error_file_write_fail, e.what());
  }

  return loot_ok;
}
