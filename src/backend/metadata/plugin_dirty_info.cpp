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
    <http://www.gnu.org/licenses/>.
    */

#include "plugin_dirty_info.h"

#include "../game/game.h"
#include "../helpers/helpers.h"

#include <boost/locale.hpp>
#include <boost/format.hpp>

using namespace std;

namespace loot {
    PluginDirtyInfo::PluginDirtyInfo() : _crc(0), _itm(0), _ref(0), _nav(0) {}

    PluginDirtyInfo::PluginDirtyInfo(uint32_t crc, unsigned int itm, unsigned int ref, unsigned int nav, const std::string& utility) : _crc(crc), _itm(itm), _ref(ref), _nav(nav), _utility(utility) {}

    bool PluginDirtyInfo::operator < (const PluginDirtyInfo& rhs) const {
        return _crc < rhs.CRC();
    }

    bool PluginDirtyInfo::operator == (const PluginDirtyInfo& rhs) const {
        return _crc == rhs.CRC();
    }

    uint32_t PluginDirtyInfo::CRC() const {
        return _crc;
    }

    unsigned int PluginDirtyInfo::ITMs() const {
        return _itm;
    }

    unsigned int PluginDirtyInfo::DeletedRefs() const {
        return _ref;
    }

    unsigned int PluginDirtyInfo::DeletedNavmeshes() const {
        return _nav;
    }

    std::string PluginDirtyInfo::CleaningUtility() const {
        return _utility;
    }

    Message PluginDirtyInfo::AsMessage() const {
        boost::format f;
        if (this->_itm > 0 && this->_ref > 0 && this->_nav > 0)
            f = boost::format(boost::locale::translate("Contains %1% ITM records, %2% deleted references and %3% deleted navmeshes. Clean with %4%.")) % this->_itm % this->_ref % this->_nav % this->_utility;
        else if (this->_itm == 0 && this->_ref == 0 && this->_nav == 0)
            f = boost::format(boost::locale::translate("Clean with %1%.")) % this->_utility;

        else if (this->_itm == 0 && this->_ref > 0 && this->_nav > 0)
            f = boost::format(boost::locale::translate("Contains %1% deleted references and %2% deleted navmeshes. Clean with %3%.")) % this->_ref % this->_nav % this->_utility;
        else if (this->_itm == 0 && this->_ref == 0 && this->_nav > 0)
            f = boost::format(boost::locale::translate("Contains %1% deleted navmeshes. Clean with %2%.")) % this->_nav % this->_utility;
        else if (this->_itm == 0 && this->_ref > 0 && this->_nav == 0)
            f = boost::format(boost::locale::translate("Contains %1% deleted references. Clean with %2%.")) % this->_ref % this->_utility;

        else if (this->_itm > 0 && this->_ref == 0 && this->_nav > 0)
            f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% deleted navmeshes. Clean with %3%.")) % this->_itm % this->_nav % this->_utility;
        else if (this->_itm > 0 && this->_ref == 0 && this->_nav == 0)
            f = boost::format(boost::locale::translate("Contains %1% ITM records. Clean with %2%.")) % this->_itm % this->_utility;

        else if (this->_itm > 0 && this->_ref > 0 && this->_nav == 0)
            f = boost::format(boost::locale::translate("Contains %1% ITM records and %2% deleted references. Clean with %3%.")) % this->_itm % this->_ref % this->_utility;

        return Message(Message::warn, f.str());
    }

    bool PluginDirtyInfo::EvalCondition(Game& game, const std::string& pluginName) const {
        if (pluginName.empty())
            return false;

        // First need to get plugin's CRC.
        uint32_t crc = 0;

        // Get the CRC from the game plugin cache if possible.
        try {
            crc = game.GetPlugin(pluginName).Crc();
        }
        catch (...) {}

        // Otherwise calculate it from the file.
        if (crc == 0) {
            if (boost::filesystem::exists(game.DataPath() / pluginName)) {
                crc = GetCrc32(game.DataPath() / pluginName);
            }
            else if (boost::filesystem::exists(game.DataPath() / (pluginName + ".ghost"))) {
                crc = GetCrc32(game.DataPath() / (pluginName + ".ghost"));
            }
        }

        return _crc == crc;
    }
}

namespace YAML {
    Emitter& operator << (Emitter& out, const loot::PluginDirtyInfo& rhs) {
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
