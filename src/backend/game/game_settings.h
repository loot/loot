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

#ifndef __LOOT_GAME_SETTINGS__
#define __LOOT_GAME_SETTINGS__

#include <string>
#include <list>

#include <boost/filesystem.hpp>

#include <yaml-cpp/yaml.h>

namespace loot {
    class GameSettings {
    public:
        //Game functions.
        GameSettings();  //Sets game to LOOT_Game::autodetect, with all other vars being empty.
        GameSettings(const unsigned int baseGameCode, const std::string& lootFolder = "");

        GameSettings& SetDetails(const std::string& name, const std::string& masterFile,
                                 const std::string& repositoryURL, const std::string& repositoryBranch,
                                 const std::string& path, const std::string& registry);

        bool IsInstalled();  //Sets gamePath if the current value is not valid and a valid path is found.

        bool operator == (const GameSettings& rhs) const;  //Compares names and folder names.
        bool operator == (const std::string& nameOrFolderName) const;

        unsigned int Id() const;
        std::string Name() const;  //Returns the game's name, eg. "TES IV: Oblivion".
        std::string FolderName() const;
        std::string Master() const;
        std::string RegistryKey() const;
        std::string RepoURL() const;
        std::string RepoBranch() const;

        boost::filesystem::path GamePath() const;
        boost::filesystem::path DataPath() const;
        boost::filesystem::path MasterlistPath() const;
        boost::filesystem::path UserlistPath() const;

        static const unsigned int autodetect;
        static const unsigned int tes4;
        static const unsigned int tes5;
        static const unsigned int fo3;
        static const unsigned int fonv;
    protected:
        //Creates directory in LOOT folder for LOOT's game-specific files.
        void CreateLOOTGameFolder();
    private:
        unsigned int _id;
        std::string _name;
        std::string _masterFile;

        std::string _registryKey;

        std::string _lootFolderName;
        std::string _repositoryURL;
        std::string _repositoryBranch;

        boost::filesystem::path _gamePath;  //Path to the game's folder.
    };

    std::list<GameSettings> GetGameSettings(YAML::Node& settings);
}

namespace YAML {
    template<>
    struct convert < loot::GameSettings > {
        static Node encode(const loot::GameSettings& rhs) {
            Node node;

            node["type"] = loot::GameSettings(rhs.Id()).FolderName();
            node["name"] = rhs.Name();
            node["folder"] = rhs.FolderName();
            node["master"] = rhs.Master();
            node["repo"] = rhs.RepoURL();
            node["branch"] = rhs.RepoBranch();
            node["path"] = rhs.GamePath().string();
            node["registry"] = rhs.RegistryKey();

            return node;
        }

        static bool decode(const Node& node, loot::GameSettings& rhs) {
            if (!node.IsMap() || !node["folder"] || !node["type"])
                return false;

            if (node["type"].as<std::string>() == loot::GameSettings(loot::GameSettings::tes4).FolderName())
                rhs = loot::GameSettings(loot::GameSettings::tes4, node["folder"].as<std::string>());
            else if (node["type"].as<std::string>() == loot::GameSettings(loot::GameSettings::tes5).FolderName())
                rhs = loot::GameSettings(loot::GameSettings::tes5, node["folder"].as<std::string>());
            else if (node["type"].as<std::string>() == loot::GameSettings(loot::GameSettings::fo3).FolderName())
                rhs = loot::GameSettings(loot::GameSettings::fo3, node["folder"].as<std::string>());
            else if (node["type"].as<std::string>() == loot::GameSettings(loot::GameSettings::fonv).FolderName())
                rhs = loot::GameSettings(loot::GameSettings::fonv, node["folder"].as<std::string>());
            else
                return false;

            std::string name, master, repo, branch, path, registry;
            if (node["name"])
                name = node["name"].as<std::string>();
            if (node["master"])
                master = node["master"].as<std::string>();
            if (node["repo"])
                repo = node["repo"].as<std::string>();
            if (node["branch"])
                branch = node["branch"].as<std::string>();
            if (node["path"])
                path = node["path"].as<std::string>();
            if (node["registry"])
                registry = node["registry"].as<std::string>();

            rhs.SetDetails(name, master, repo, branch, path, registry);

            return true;
        }
    };

    Emitter& operator << (Emitter& out, const loot::GameSettings& rhs);
}

#endif
