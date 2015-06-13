/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2015    WrinklyNinja

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

#include "plugin_loader.h"
#include "../game/game.h"
#include "../helpers/helpers.h"

#include <src/libespm.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/locale.hpp>

using namespace std;

namespace loot {
    PluginLoader::PluginLoader() : _isMaster(false), _crc(0) {}

    bool PluginLoader::Load(const Game& game, const std::string& name, const bool headerOnly, const bool checkValidityOnly) {
        espm::File * file = nullptr;
        espm::Settings espmSettings;

        // Get data from file contents using libespm.
        BOOST_LOG_TRIVIAL(trace) << name << ": " << "Opening with libespm...";
        boost::filesystem::path filepath = game.DataPath() / name;

        //In case the plugin is ghosted.
        if (!boost::filesystem::exists(filepath) && boost::filesystem::exists(filepath.string() + ".ghost"))
            filepath += ".ghost";

        try {
            if (game.Id() == Game::tes4) {
                espmSettings = espm::Settings("tes4");
                file = new espm::tes4::File(filepath, espmSettings, false, headerOnly);
            }
            else if (game.Id() == Game::tes5) {
                espmSettings = espm::Settings("tes5");
                file = new espm::tes5::File(filepath, espmSettings, false, headerOnly);
            }
            else if (game.Id() == Game::fo3) {
                espmSettings = espm::Settings("fo3");
                file = new espm::fo3::File(filepath, espmSettings, false, headerOnly);
            }
            else {
                espmSettings = espm::Settings("fonv");
                file = new espm::fonv::File(filepath, espmSettings, false, headerOnly);
            }

            // If only wanting to test for valid parsing, quit now.
            if (checkValidityOnly) {
                delete file;
                return true;
            }

            BOOST_LOG_TRIVIAL(trace) << filepath.filename() << ": " << "Checking master flag.";
            _isMaster = file->isMaster(espmSettings);

            BOOST_LOG_TRIVIAL(trace) << filepath.filename() << ": " << "Getting masters.";
            for (const auto& master : file->getMasters()) {
                _masters.push_back(boost::locale::conv::to_utf<char>(master, "Windows-1252", boost::locale::conv::stop));
            }

            BOOST_LOG_TRIVIAL(trace) << filepath.filename() << ": " << "Number of masters: " << _masters.size();

            if (!headerOnly) {
                BOOST_LOG_TRIVIAL(trace) << filepath.filename() << ": " << "Getting CRC.";
                _crc = file->crc;
            }

            BOOST_LOG_TRIVIAL(trace) << name << ": " << "Getting the FormIDs.";
            vector<uint32_t> records = file->getFormIDs();
            vector<string> plugins = _masters;
            plugins.push_back(name);
            for (const auto &record : records) {
                _formIDs.insert(FormID(plugins, record));
            }

            BOOST_LOG_TRIVIAL(trace) << name << ": " << "Checking if plugin is empty.";
            if (headerOnly) {
                // Check the header records count.
                _isEmpty = file->getNumRecords() == 0;
            }
            else {
                _isEmpty = _formIDs.empty();
            }

            //Also read Bash Tags applied and version string in description.
            BOOST_LOG_TRIVIAL(trace) << name << ": " << "Reading the description.";
            _description = boost::locale::conv::to_utf<char>(file->getDescription(), "Windows-1252", boost::locale::conv::stop);
        }
        catch (std::exception&) {
            delete file;
            throw;
        }

        BOOST_LOG_TRIVIAL(trace) << name << ": " << "Plugin loading complete.";
        return true;
    }

    bool PluginLoader::IsEmpty() const {
        return _isEmpty;
    }

    bool PluginLoader::IsMaster() const {
        return _isMaster;
    }

    const std::set<FormID>& PluginLoader::FormIDs() const {
        return _formIDs;
    }

    std::vector<std::string> PluginLoader::Masters() const {
        return _masters;
    }

    std::string PluginLoader::Description() const {
        return _description;
    }

    uint32_t PluginLoader::Crc() const {
        return _crc;
    }
}
