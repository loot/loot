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

#include "backend/metadata/plugin_cleaning_data.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "backend/game/game.h"
#include "backend/helpers/helpers.h"

namespace loot {
PluginCleaningData::PluginCleaningData() : crc_(0), itm_(0), ref_(0), nav_(0) {}

PluginCleaningData::PluginCleaningData(uint32_t crc, const std::string& utility) : crc_(crc), utility_(utility), itm_(0), ref_(0), nav_(0) {}

PluginCleaningData::PluginCleaningData(uint32_t crc, unsigned int itm, unsigned int ref, unsigned int nav, const std::string& utility) : crc_(crc), itm_(itm), ref_(ref), nav_(nav), utility_(utility) {}

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

Message PluginCleaningData::AsMessage() const {
  boost::format f;
  if (this->itm_ > 0 && this->ref_ > 0 && this->nav_ > 0)
    f = boost::format(boost::locale::translate("Contains %1% ITM records, %2% deleted references and %3% deleted navmeshes. Clean with %4%.")) % this->itm_ % this->ref_ % this->nav_ % this->utility_;
  else if (this->itm_ == 0 && this->ref_ == 0 && this->nav_ == 0)
    f = boost::format(boost::locale::translate("Clean with %1%.")) % this->utility_;

  else if (this->itm_ == 0 && this->ref_ > 0 && this->nav_ > 0)
    f = boost::format(boost::locale::translate("Contains %1% deleted references and %2% deleted navmeshes. Clean with %3%.")) % this->ref_ % this->nav_ % this->utility_;
  else if (this->itm_ == 0 && this->ref_ == 0 && this->nav_ > 0)
    f = boost::format(boost::locale::translate("Contains %1% deleted navmeshes. Clean with %2%.")) % this->nav_ % this->utility_;
  else if (this->itm_ == 0 && this->ref_ > 0 && this->nav_ == 0)
    f = boost::format(boost::locale::translate("Contains %1% deleted references. Clean with %2%.")) % this->ref_ % this->utility_;

  else if (this->itm_ > 0 && this->ref_ == 0 && this->nav_ > 0)
    f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% deleted navmeshes. Clean with %3%.")) % this->itm_ % this->nav_ % this->utility_;
  else if (this->itm_ > 0 && this->ref_ == 0 && this->nav_ == 0)
    f = boost::format(boost::locale::translate("Contains %1% ITM records. Clean with %2%.")) % this->itm_ % this->utility_;

  else if (this->itm_ > 0 && this->ref_ > 0 && this->nav_ == 0)
    f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% deleted references. Clean with %3%.")) % this->itm_ % this->ref_ % this->utility_;

  return Message(Message::Type::warn, f.str());
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

namespace YAML {
Emitter& operator << (Emitter& out, const loot::PluginCleaningData& rhs) {
  out << BeginMap
    << Key << "crc" << Value << Hex << rhs.CRC() << Dec
    << Key << "util" << Value << YAML::SingleQuoted << rhs.CleaningUtility();

  if (rhs.ITMs() > 0)
    out << Key << "itm" << Value << rhs.ITMs();
  if (rhs.DeletedRefs() > 0)
    out << Key << "udr" << Value << rhs.DeletedRefs();
  if (rhs.DeletedNavmeshes() > 0)
    out << Key << "nav" << Value << rhs.DeletedNavmeshes();

  out << EndMap;

  return out;
}
}
