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

#include "backend/plugin/plugin.h"

#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "backend/game/game.h"
#include "backend/helpers/helpers.h"

using libespm::FormId;
using std::set;
using std::string;

namespace loot {
Plugin::Plugin(const Game& game, const std::string& name, const bool headerOnly) :
  PluginMetadata(name),
  libespm::Plugin(Plugin::GetLibespmGameId(game.Type())),
  isEmpty_(true),
  isActive_(false),
  loadsArchive_(false),
  crc_(0),
  numOverrideRecords_(0) {
  try {
    boost::filesystem::path filepath = game.DataPath() / Name();

    // In case the plugin is ghosted.
    if (!boost::filesystem::exists(filepath) && boost::filesystem::exists(filepath.string() + ".ghost"))
      filepath += ".ghost";

    load(filepath, headerOnly);

    isEmpty_ = getRecordAndGroupCount() == 0;

    if (!headerOnly) {
      BOOST_LOG_TRIVIAL(trace) << Name() << ": Caching CRC value.";
      crc_ = GetCrc32(filepath);
    }

    BOOST_LOG_TRIVIAL(trace) << Name() << ": Counting override FormIDs.";
    for (const auto& formID : getFormIds()) {
      if (!boost::iequals(formID.getPluginName(), Name()))
        ++numOverrideRecords_;
    }

    //Also read Bash Tags applied and version string in description.
    string text = getDescription();
    BOOST_LOG_TRIVIAL(trace) << Name() << ": " << "Attempting to extract Bash Tags from the description.";
    size_t pos1 = text.find("{{BASH:");
    if (pos1 != string::npos && pos1 + 7 != text.length()) {
      pos1 += 7;

      size_t pos2 = text.find("}}", pos1);
      if (pos2 != string::npos && pos1 != pos2) {
        text = text.substr(pos1, pos2 - pos1);

        std::vector<string> bashTags;
        boost::split(bashTags, text, [](char c) { return c == ','; });

        for (auto &tag : bashTags) {
          boost::trim(tag);
          BOOST_LOG_TRIVIAL(trace) << Name() << ": " << "Extracted Bash Tag: " << tag;
          tags_.insert(Tag(tag));
        }
      }
    }
    // Get whether the plugin is active or not.
    isActive_ = game.IsPluginActive(Name());

    // Get whether the plugin loads an archive (BSA/BA2) or not.
    const string archiveExtension = game.GetArchiveFileExtension();

    if (game.Type() == GameType::tes5) {
        // Skyrim plugins only load BSAs that exactly match their basename.
      loadsArchive_ = boost::filesystem::exists(game.DataPath() / (Name().substr(0, Name().length() - 4) + archiveExtension));
    } else if (game.Type() != GameType::tes4 || boost::iends_with(Name(), ".esp")) {
        //Oblivion .esp files and FO3, FNV, FO4 plugins can load archives which begin with the plugin basename.
      string basename = Name().substr(0, Name().length() - 4);
      for (boost::filesystem::directory_iterator it(game.DataPath()); it != boost::filesystem::directory_iterator(); ++it) {
        if (boost::iequals(it->path().extension().string(), archiveExtension) && boost::istarts_with(it->path().filename().string(), basename)) {
          loadsArchive_ = true;
          break;
        }
      }
    }
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Cannot read plugin file \"" << name << "\". Details: " << e.what();
    messages_.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("Cannot read \"%1%\". Details: %2%")) % name % e.what()).str()));
  }

  BOOST_LOG_TRIVIAL(trace) << Name() << ": " << "Plugin loading complete.";
}

bool Plugin::DoFormIDsOverlap(const Plugin& plugin) const {
    //Basically std::set_intersection except with an early exit instead of an append to results.
  set<FormId> formIds(getFormIds());
  set<FormId> otherFormIds(plugin.getFormIds());
  auto i = begin(formIds);
  auto j = begin(otherFormIds);
  auto iend = end(formIds);
  auto jend = end(otherFormIds);

  while (i != iend && j != jend) {
    if (*i < *j)
      ++i;
    else if (*j < *i)
      ++j;
    else
      return true;
  }

  return false;
}

size_t Plugin::NumOverrideFormIDs() const {
  return numOverrideRecords_;
}

std::set<FormId> Plugin::OverlapFormIDs(const Plugin& plugin) const {
  set<FormId> formIds(getFormIds());
  set<FormId> otherFormIds(plugin.getFormIds());
  set<FormId> overlap;

  set_intersection(begin(formIds),
                   end(formIds),
                   begin(otherFormIds),
                   end(otherFormIds),
                   inserter(overlap, end(overlap)));

  return overlap;
}

bool Plugin::IsEmpty() const {
  return isEmpty_;
}

bool Plugin::IsValid(const std::string& filename, const Game& game) {
  BOOST_LOG_TRIVIAL(trace) << "Checking to see if \"" << filename << "\" is a valid plugin.";

  //If the filename passed ends in '.ghost', that should be trimmed.
  std::string name;
  if (boost::iends_with(filename, ".ghost"))
    name = filename.substr(0, filename.length() - 6);
  else
    name = filename;

  // Check that the file has a valid extension.
  if (!boost::iends_with(name, ".esm") && !boost::iends_with(name, ".esp"))
    return false;

  // Add the ".ghost" file extension if the plugin is ghosted.
  boost::filesystem::path filepath = game.DataPath() / name;
  if (!boost::filesystem::exists(filepath) && boost::filesystem::exists(filepath.string() + ".ghost"))
    filepath += ".ghost";

  if (libespm::Plugin::isValid(filepath, GetLibespmGameId(game.Type()), true))
    return true;

  BOOST_LOG_TRIVIAL(warning) << "The .es(p|m) file \"" << filename << "\" is not a valid plugin.";
  return false;
}

uintmax_t Plugin::GetFileSize(const std::string & filename, const Game & game) {
  boost::filesystem::path realPath = game.DataPath() / filename;
  if (!boost::filesystem::exists(realPath))
    realPath += ".ghost";

  return boost::filesystem::file_size(realPath);
}

bool Plugin::operator < (const Plugin & rhs) const {
  return boost::ilexicographical_compare(Name(), rhs.Name());;
}

bool Plugin::IsActive() const {
  return isActive_;
}

uint32_t Plugin::Crc() const {
  return crc_;
}

void Plugin::CheckInstallValidity(const Game& game) {
  BOOST_LOG_TRIVIAL(trace) << "Checking that the current install is valid according to " << Name() << "'s data.";
  if (IsActive()) {
    auto pluginExists = [](const Game& game, const std::string& file) {
      return boost::filesystem::exists(game.DataPath() / file)
        || ((boost::iends_with(file, ".esp") || boost::iends_with(file, ".esm")) && boost::filesystem::exists(game.DataPath() / (file + ".ghost")));
    };
    if (tags_.find(Tag("Filter")) == tags_.end()) {
      for (const auto &master : getMasters()) {
        if (!pluginExists(game, master)) {
          BOOST_LOG_TRIVIAL(error) << "\"" << Name() << "\" requires \"" << master << "\", but it is missing.";
          messages_.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("This plugin requires \"%1%\" to be installed, but it is missing.")) % master).str()));
        } else if (!game.IsPluginActive(master)) {
          BOOST_LOG_TRIVIAL(error) << "\"" << Name() << "\" requires \"" << master << "\", but it is inactive.";
          messages_.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("This plugin requires \"%1%\" to be active, but it is inactive.")) % master).str()));
        }
      }
    }

    for (const auto &req : Reqs()) {
      if (!pluginExists(game, req.Name())) {
        BOOST_LOG_TRIVIAL(error) << "\"" << Name() << "\" requires \"" << req.Name() << "\", but it is missing.";
        messages_.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("This plugin requires \"%1%\" to be installed, but it is missing.")) % req.Name()).str()));
      }
    }
    for (const auto &inc : Incs()) {
      if (pluginExists(game, inc.Name()) && game.IsPluginActive(inc.Name())) {
        BOOST_LOG_TRIVIAL(error) << "\"" << Name() << "\" is incompatible with \"" << inc.Name() << "\", but both are present.";
        messages_.push_back(Message(MessageType::error, (boost::format(boost::locale::translate("This plugin is incompatible with \"%1%\", but both are present.")) % inc.Name()).str()));
      }
    }
  }

  // Also generate dirty messages.
  for (const auto &element : DirtyInfo()) {
    messages_.push_back(element.AsMessage());
  }
}

bool Plugin::LoadsArchive() const {
  return loadsArchive_;
}

libespm::GameId Plugin::GetLibespmGameId(GameType gameType) {
  if (gameType == GameType::tes4)
    return libespm::GameId::OBLIVION;
  else if (gameType == GameType::tes5 || gameType == GameType::tes5se)
    return libespm::GameId::SKYRIM;
  else if (gameType == GameType::fo3)
    return libespm::GameId::FALLOUT3;
  else if (gameType == GameType::fonv)
    return libespm::GameId::FALLOUTNV;
  else
    return libespm::GameId::FALLOUT4;
}
}
