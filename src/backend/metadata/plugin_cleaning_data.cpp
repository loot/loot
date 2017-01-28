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

#include "loot/metadata/plugin_cleaning_data.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "backend/game/game.h"
#include "backend/helpers/helpers.h"

namespace loot {
PluginCleaningData::PluginCleaningData() : crc_(0), itm_(0), ref_(0), nav_(0) {}

PluginCleaningData::PluginCleaningData(uint32_t crc, const std::string& utility)
  : crc_(crc), utility_(utility), itm_(0), ref_(0), nav_(0) {}

PluginCleaningData::PluginCleaningData(uint32_t crc,
                                       const std::string& utility,
                                       const std::vector<MessageContent>& info,
                                       unsigned int itm,
                                       unsigned int ref,
                                       unsigned int nav)
  : crc_(crc), itm_(itm), ref_(ref), nav_(nav), utility_(utility), info_(info) {}

bool PluginCleaningData::operator < (const PluginCleaningData& rhs) const {
  return crc_ < rhs.CRC();
}

bool PluginCleaningData::operator == (const PluginCleaningData& rhs) const {
  return crc_ == rhs.CRC();
}

uint32_t PluginCleaningData::CRC() const {
  return crc_;
}

unsigned int PluginCleaningData::ITMs() const {
  return itm_;
}

unsigned int PluginCleaningData::DeletedRefs() const {
  return ref_;
}

unsigned int PluginCleaningData::DeletedNavmeshes() const {
  return nav_;
}

std::string PluginCleaningData::CleaningUtility() const {
  return utility_;
}

std::vector<MessageContent> PluginCleaningData::Info() const {
  return info_;
}

MessageContent PluginCleaningData::ChooseInfo(const LanguageCode language) const {
  BOOST_LOG_TRIVIAL(trace) << "Choosing dirty info content.";
  return MessageContent::Choose(info_, language);
}

Message PluginCleaningData::AsMessage() const {
  using boost::format;
  using boost::locale::translate;

  const std::string itmRecords = (format(translate("%1% ITM record", "%1% ITM records", itm_)) % itm_).str();
  const std::string deletedReferences = (format(translate("%1% deleted reference", "%1% deleted references", ref_)) % ref_).str();
  const std::string deletedNavmeshes = (format(translate("%1% deleted navmesh", "%1% deleted navmeshes", nav_)) % nav_).str();

  format f;
  if (itm_ > 0 && ref_ > 0 && nav_ > 0)
    f = format(translate("%1% found %2%, %3% and %4%.")) % utility_ % itmRecords % deletedReferences % deletedNavmeshes;
  else if (itm_ == 0 && ref_ == 0 && nav_ == 0)
    f = format(translate("%1% found dirty edits.")) % utility_;

  else if (itm_ == 0 && ref_ > 0 && nav_ > 0)
    f = format(translate("%1% found %2% and %3%.")) % utility_ % deletedReferences % deletedNavmeshes;
  else if (itm_ > 0 && ref_ == 0 && nav_ > 0)
    f = format(translate("%1% found %2% and %3%.")) % utility_ % itmRecords % deletedNavmeshes;
  else if (itm_ > 0 && ref_ > 0 && nav_ == 0)
    f = format(translate("%1% found %2% and %3%.")) % utility_ % itmRecords % deletedReferences;

  else if (itm_ > 0)
    f = format(translate("%1% found %2%.")) % utility_ % itmRecords;
  else if (ref_ > 0)
    f = format(translate("%1% found %2%.")) % utility_ % deletedReferences;
  else if (nav_ > 0)
    f = format(translate("%1% found %2%.")) % utility_ % deletedNavmeshes;

  std::string message = f.str();
  if (info_.empty()) {
    return Message(MessageType::warn, message);
  }

  auto info = info_;
  for (auto& content : info) {
    content = MessageContent(message + " " + content.GetText(), content.GetLanguage());
  }

  return Message(MessageType::warn, info);
}

bool PluginCleaningData::EvalCondition(Game& game, const std::string& pluginName) const {
  if (pluginName.empty())
    return false;

  // First need to get plugin's CRC.
  uint32_t crc = 0;

  // Get the CRC from the game plugin cache if possible.
  try {
    crc = game.GetPlugin(pluginName).Crc();
  } catch (...) {}

  // Otherwise calculate it from the file.
  if (crc == 0) {
    if (boost::filesystem::exists(game.DataPath() / pluginName)) {
      crc = GetCrc32(game.DataPath() / pluginName);
    } else if (boost::filesystem::exists(game.DataPath() / (pluginName + ".ghost"))) {
      crc = GetCrc32(game.DataPath() / (pluginName + ".ghost"));
    }
  }

  return crc_ == crc;
}
}
