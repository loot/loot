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

#ifndef __LOOT_GAME__
#define __LOOT_GAME__

#include "plugin.h"
#include "metadata_list.h"
#include "masterlist.h"

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

#include <api/libloadorder.h>
#include <src/libespm.h>
#include <yaml-cpp/yaml.h>

namespace loot {
    class Game {
    public:
        //Game functions.
        Game();  //Sets game to LOOT_Game::autodetect, with all other vars being empty.
        Game(const unsigned int baseGameCode, const std::string& lootFolder = "");
        ~Game();

        Game& SetDetails(const std::string& name, const std::string& masterFile,
                         const std::string& repositoryURL, const std::string& repositoryBranch,
                         const std::string& path, const std::string& registry);
        Game& Init(bool createFolder, const boost::filesystem::path& gameLocalAppData = "");

        bool IsInstalled() const;

        bool operator == (const Game& rhs) const;  //Compares names and folder names.
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

        //Game plugin functions.
        bool IsActive(const std::string& plugin) const;

        void GetLoadOrder(std::list<std::string>& loadOrder) const;
        void SetLoadOrder(const std::list<std::string>& loadOrder) const;  //Modifies game load order, even though const.
        void SetLoadOrder(const char * const * const loadOrder, const size_t numPlugins) const;  // For API.

        void RefreshActivePluginsList();
        void RedatePlugins();  //Change timestamps to match load order (Skyrim only).
        void LoadPlugins(bool headersOnly);  //Loads all installed plugins.
        bool HasBeenLoaded();  // Checks if the game's plugins have already been loaded.

        bool IsValidPlugin(const std::string& name) const;

        std::list<Plugin> Sort(const unsigned int language, std::function<void(const std::string&)> progressCallback);

        //Caches for condition results, active plugins and CRCs.
        std::unordered_map<std::string, bool> conditionCache;  //Holds lowercased strings.
        std::unordered_map<std::string, uint32_t> crcCache;  //Holds lowercased strings.

        //Plugin data and metadata lists.
        Masterlist masterlist;
        MetadataList userlist;
        std::unordered_map<std::string, Plugin> plugins;  //Map so that plugin data can be edited.

        espm::Settings espm_settings;

        static const unsigned int autodetect = 0;
        static const unsigned int tes4 = 1;
        static const unsigned int tes5 = 2;
        static const unsigned int fo3 = 3;
        static const unsigned int fonv = 4;
    private:
        unsigned int id;
        std::string _name;
        std::string _masterFile;

        std::string registryKey;

        std::string lootFolderName;
        std::string _repositoryURL;
        std::string _repositoryBranch;

        boost::filesystem::path gamePath;  //Path to the game's folder.
        boost::filesystem::path _gameLocalDataPath;  // Path to the game's folder in %LOCALAPPDATA%.

        std::unordered_set<std::string> activePlugins;  //Holds lowercased strings.

        lo_game_handle gh;

        //Creates directory in LOOT folder for LOOT's game-specific files.
        void CreateLOOTGameFolder();

        void InitLibloHandle();
    };

    std::list<Game> GetGames(YAML::Node& settings);
}

namespace YAML {
    template<>
    struct convert < loot::Game > {
        static Node encode(const loot::Game& rhs) {
            Node node;

            node["type"] = loot::Game(rhs.Id()).FolderName();
            node["name"] = rhs.Name();
            node["folder"] = rhs.FolderName();
            node["master"] = rhs.Master();
            node["repo"] = rhs.RepoURL();
            node["branch"] = rhs.RepoBranch();
            node["path"] = rhs.GamePath().string();
            node["registry"] = rhs.RegistryKey();

            return node;
        }

        static bool decode(const Node& node, loot::Game& rhs) {
            if (!node.IsMap() || !node["folder"] || !node["type"])
                return false;

            if (node["type"].as<std::string>() == loot::Game(loot::Game::tes4).FolderName())
                rhs = loot::Game(loot::Game::tes4, node["folder"].as<std::string>());
            else if (node["type"].as<std::string>() == loot::Game(loot::Game::tes5).FolderName())
                rhs = loot::Game(loot::Game::tes5, node["folder"].as<std::string>());
            else if (node["type"].as<std::string>() == loot::Game(loot::Game::fo3).FolderName())
                rhs = loot::Game(loot::Game::fo3, node["folder"].as<std::string>());
            else if (node["type"].as<std::string>() == loot::Game(loot::Game::fonv).FolderName())
                rhs = loot::Game(loot::Game::fonv, node["folder"].as<std::string>());
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

    Emitter& operator << (Emitter& out, const loot::Game& rhs);
}

#endif
