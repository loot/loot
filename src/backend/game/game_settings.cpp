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

#include "game_settings.h"
#include "../app/loot_paths.h"
#include "../helpers/helpers.h"
#include "../error.h"

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {
    const unsigned int GameSettings::autodetect = 0;
    const unsigned int GameSettings::tes4 = 1;
    const unsigned int GameSettings::tes5 = 2;
    const unsigned int GameSettings::fo3 = 3;
    const unsigned int GameSettings::fonv = 4;
    const unsigned int GameSettings::fo4 = 5;

    GameSettings::GameSettings() : _id(GameSettings::autodetect) {}

    GameSettings::GameSettings(const unsigned int gameCode, const std::string& folder) : _id(gameCode) {
        if (Id() == GameSettings::tes4) {
            _name = "TES IV: Oblivion";
            _registryKey = "Software\\Bethesda Softworks\\Oblivion\\Installed Path";
            _lootFolderName = "Oblivion";
            _masterFile = "Oblivion.esm";
            _repositoryURL = "https://github.com/loot/oblivion.git";
            _repositoryBranch = "v0.8";

            bashTags = {
                "Actors.ACBS",
                "Actors.AIData",
                "Actors.AIPackages",
                "Actors.AIPackagesForceAdd",
                "Actors.Animations",
                "Actors.CombatStyle",
                "Actors.DeathItem",
                "Actors.Skeleton",
                "Actors.Spells",
                "Actors.SpellsForceAdd",
                "Actors.Stats",
                "Body-F",
                "Body-M",
                "Body-Size-F",
                "Body-Size-M",
                "C.Climate",
                "C.Light",
                "C.Music",
                "C.Name",
                "C.Owner",
                "C.RecordFlags",
                "C.Water",
                "Creatures.Blood",
                "Deactivate",
                "Delev",
                "Eyes",
                "Factions",
                "Filter",
                "Graphics",
                "Hair",
                "IIM",
                "Invent",
                "InventOnly",
                "Merge",
                "MustBeActiveIfImported",
                "Names",
                "NoMerge",
                "NpcFaces",
                "NpcFacesForceFullImport",
                "NPC.Class",
                "Npc.EyesOnly",
                "Npc.HairOnly",
                "NPC.Race",
                "Relations",
                "Relev",
                "Roads",
                "R.AddSpells",
                "R.Attributes-F",
                "R.Attributes-M",
                "R.ChangeSpells",
                "R.Description",
                "R.Ears",
                "R.Head",
                "R.Mouth",
                "R.Relations",
                "R.Skills",
                "R.Teeth",
                "ScriptContents",
                "Scripts",
                "Sound",
                "SpellStats",
                "Stats",
                "Voice-F",
                "Voice-M",
            };
        }
        else if (Id() == GameSettings::tes5) {
            _name = "TES V: Skyrim";
            _registryKey = "Software\\Bethesda Softworks\\Skyrim\\Installed Path";
            _lootFolderName = "Skyrim";
            _masterFile = "Skyrim.esm";
            _repositoryURL = "https://github.com/loot/skyrim.git";
            _repositoryBranch = "v0.8";

            bashTags = {
                "Actors.ACBS",
                "Actors.AIData",
                "Actors.AIPackages",
                "Actors.AIPackagesForceAdd",
                "Actors.Animations",
                "Actors.CombatStyle",
                "Actors.DeathItem",
                "Actors.Skeleton",
                "Actors.Spells",
                "Actors.SpellsForceAdd",
                "Actors.Stats",
                "Body-F",
                "Body-M",
                "Body-Size-F",
                "Body-Size-M",
                "C.Acoustic",
                "C.Climate",
                "C.ImageSpace",
                "C.Light",
                "C.Location",
                "C.Music",
                "C.Name",
                "C.Owner",
                "C.RecordFlags",
                "C.Regions",
                "C.SkyLighting",
                "C.Water",
                "Creatures.Blood",
                "Deactivate",
                "Delev",
                "Eyes",
                "Factions",
                "Filter",
                "Graphics",
                "Hair",
                "IIM",
                "Invent",
                "InventOnly",
                "Merge",
                "MustBeActiveIfImported",
                "Names",
                "NoMerge",
                "NpcFaces",
                "NpcFacesForceFullImport",
                "NPC.Class",
                "Npc.EyesOnly",
                "Npc.HairOnly",
                "NPC.Race",
                "Relations",
                "Relev",
                "Roads",
                "R.AddSpells",
                "R.Attributes-F",
                "R.Attributes-M",
                "R.ChangeSpells",
                "R.Description",
                "R.Ears",
                "R.Head",
                "R.Mouth",
                "R.Relations",
                "R.Skills",
                "R.Teeth",
                "ScriptContents",
                "Scripts",
                "Sound",
                "SpellStats",
                "Stats",
                "Voice-F",
                "Voice-M",
            };
        }
        else if (Id() == GameSettings::fo3) {
            _name = "Fallout 3";
            _registryKey = "Software\\Bethesda Softworks\\Fallout3\\Installed Path";
            _lootFolderName = "Fallout3";
            _masterFile = "Fallout3.esm";
            _repositoryURL = "https://github.com/loot/fallout3.git";
            _repositoryBranch = "v0.8";

            bashTags = {
                "Deflst",
                "Destructible",
            };
        }
        else if (Id() == GameSettings::fonv) {
            _name = "Fallout: New Vegas";
            _registryKey = "Software\\Bethesda Softworks\\FalloutNV\\Installed Path";
            _lootFolderName = "FalloutNV";
            _masterFile = "FalloutNV.esm";
            _repositoryURL = "https://github.com/loot/falloutnv.git";
            _repositoryBranch = "v0.8";

            bashTags = {
                "Deflst",
                "Destructible",
                "WeaponMods",
            };
        }
        else if (Id() == GameSettings::fo4) {
            _name = "Fallout 4";
            _registryKey = "Software\\Bethesda Softworks\\Fallout4\\Installed Path";
            _lootFolderName = "Fallout4";
            _masterFile = "Fallout4.esm";
            _repositoryURL = "https://github.com/loot/fallout4.git";
            _repositoryBranch = "v0.8";
        }

        if (!folder.empty())
            _lootFolderName = folder;
    }

    bool GameSettings::IsInstalled() {
        try {
            BOOST_LOG_TRIVIAL(trace) << "Checking if game \"" << _name << "\" is installed.";
            if (!_gamePath.empty() && fs::exists(_gamePath / "Data" / _masterFile))
                return true;

            if (fs::exists(fs::path("..") / "Data" / _masterFile)) {
                _gamePath = "..";
                return true;
            }

#ifdef _WIN32
            string path;
            string key_parent = fs::path(_registryKey).parent_path().string();
            string key_name = fs::path(_registryKey).filename().string();
            path = RegKeyStringValue("HKEY_LOCAL_MACHINE", key_parent, key_name);
            if (!path.empty() && fs::exists(fs::path(path) / "Data" / _masterFile)) {
                _gamePath = path;
                return true;
            }
#endif
        }
        catch (exception &e) {
            BOOST_LOG_TRIVIAL(error) << "Error while checking if game \"" << _name << "\" is installed: " << e.what();
        }

        return false;
    }

    bool GameSettings::operator == (const GameSettings& rhs) const {
        return (boost::iequals(_name, rhs.Name()) || boost::iequals(_lootFolderName, rhs.FolderName()));
    }

    unsigned int GameSettings::Id() const {
        return _id;
    }

    libespm::GameId GameSettings::LibespmId() const {
        if (_id == GameSettings::tes4)
            return libespm::GameId::OBLIVION;
        else if (_id == GameSettings::tes5)
            return libespm::GameId::SKYRIM;
        else if (_id == GameSettings::fo3)
            return libespm::GameId::FALLOUT3;
        else if (_id == GameSettings::fonv)
            return libespm::GameId::FALLOUTNV;
        else
            return libespm::GameId::FALLOUT4;
    }

    string GameSettings::Name() const {
        return _name;
    }

    string GameSettings::FolderName() const {
        return _lootFolderName;
    }

    std::string GameSettings::Master() const {
        return _masterFile;
    }

    std::string GameSettings::RegistryKey() const {
        return _registryKey;
    }

    std::string GameSettings::RepoURL() const {
        return _repositoryURL;
    }

    std::string GameSettings::RepoBranch() const {
        return _repositoryBranch;
    }

    fs::path GameSettings::GamePath() const {
        return _gamePath;
    }

    fs::path GameSettings::DataPath() const {
        if (_gamePath.empty())
            return "";
        else
            return _gamePath / "Data";
    }

    fs::path GameSettings::MasterlistPath() const {
        if (_lootFolderName.empty())
            return "";
        else
            return LootPaths::getLootDataPath() / _lootFolderName / "masterlist.yaml";
    }

    fs::path GameSettings::UserlistPath() const {
        if (_lootFolderName.empty())
            return "";
        else
            return LootPaths::getLootDataPath() / _lootFolderName / "userlist.yaml";
    }

    std::vector<std::string> GameSettings::getSupportedBashTags() const {
        return bashTags;
    }

    GameSettings& GameSettings::SetName(const std::string& name) {
        BOOST_LOG_TRIVIAL(trace) << "Setting \"" << _name << "\" name to: " << name;
        _name = name;
        return *this;
    }

    GameSettings& GameSettings::SetMaster(const std::string& masterFile) {
        BOOST_LOG_TRIVIAL(trace) << "Setting \"" << _name << "\" master file to: " << masterFile;
        _masterFile = masterFile;
        return *this;
    }

    GameSettings& GameSettings::SetRegistryKey(const std::string& registry) {
        BOOST_LOG_TRIVIAL(trace) << "Setting \"" << _name << "\" registry key to: " << registry;
        _registryKey = registry;
        return *this;
    }

    GameSettings& GameSettings::SetRepoURL(const std::string& repositoryURL) {
        BOOST_LOG_TRIVIAL(trace) << "Setting \"" << _name << "\" repo URL to: " << repositoryURL;
        _repositoryURL = repositoryURL;
        return *this;
    }

    GameSettings& GameSettings::SetRepoBranch(const std::string& repositoryBranch) {
        BOOST_LOG_TRIVIAL(trace) << "Setting \"" << _name << "\" repo branch to: " << repositoryBranch;
        _repositoryBranch = repositoryBranch;
        return *this;
    }

    GameSettings& GameSettings::SetGamePath(const boost::filesystem::path& path) {
        BOOST_LOG_TRIVIAL(trace) << "Setting \"" << _name << "\" game path to: " << path;
        _gamePath = path;
        return *this;
    }
}

namespace YAML {
    Emitter& operator << (Emitter& out, const loot::GameSettings& rhs) {
        out << BeginMap
            << Key << "type" << Value << YAML::SingleQuoted << loot::GameSettings(rhs.Id()).FolderName()
            << Key << "folder" << Value << YAML::SingleQuoted << rhs.FolderName()
            << Key << "name" << Value << YAML::SingleQuoted << rhs.Name()
            << Key << "master" << Value << YAML::SingleQuoted << rhs.Master()
            << Key << "repo" << Value << YAML::SingleQuoted << rhs.RepoURL()
            << Key << "branch" << Value << YAML::SingleQuoted << rhs.RepoBranch()
            << Key << "path" << Value << YAML::SingleQuoted << rhs.GamePath().string()
            << Key << "registry" << Value << YAML::SingleQuoted << rhs.RegistryKey()
            << EndMap;

        return out;
    }
}
