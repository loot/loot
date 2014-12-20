/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2014    WrinklyNinja

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

#include "metadata.h"

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include <boost/filesystem.hpp>

#include <api/libloadorder.h>
#include <src/libespm.h>
#include <yaml-cpp/yaml.h>

namespace loot {
    
    class Game;

    /* Each Game object should store the config details specific to that game.
       It should also store the plugin and masterlist data for that game.
       Plugin data should be stored as an unordered hashset, the elements of which are
       referenced by ordered lists and other structures.
       Masterlist / userlist data should be stored as structures which hold plugin and
       global message lists.
       Each game should have functions to load this plugin and masterlist / userlist
       data. Plugin data should be loaded as header-only and as full data.
    */

    class MetadataList {
    public:
        void Load(boost::filesystem::path& filepath);
        void Save(boost::filesystem::path& filepath);

        std::list<Plugin> plugins;
        std::list<Message> messages;
    };

    class Masterlist : public MetadataList {
    public:

        void Load(Game& game, const unsigned int language);  //Handles update with load fallback.
        void Update(Game& game, const unsigned int language);
        
        std::string GetRevision(boost::filesystem::path& path);
        std::string GetDate(boost::filesystem::path& path);

    private:
        void GetGitInfo(boost::filesystem::path& path);

        std::string revision;
        std::string date;
    };

    class Game {
    public:
        //Game functions.
        Game();  //Sets game to LOOT_Game::autodetect, with all other vars being empty.
        Game(const unsigned int baseGameCode, const std::string& lootFolder = "");

        Game& SetDetails(const std::string& name, const std::string& masterFile,
                        const std::string& repositoryURL, const std::string& repositoryBranch, 
                        const std::string& path, const std::string& registry);
        Game& SetPath(const std::string& path);  //Used by API.
        Game& Init();

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
        boost::filesystem::path ReportDataPath() const;

        //Game plugin functions.
        bool IsActive(const std::string& plugin) const;

        void GetLoadOrder(std::list<std::string>& loadOrder) const;
        void SetLoadOrder(const std::list<Plugin>& loadOrder) const;  //Modifies game load order, even though const.

        void RefreshActivePluginsList();
        void RedatePlugins();  //Change timestamps to match load order (Skyrim only).
        void LoadPlugins(bool headersOnly);  //Loads all installed plugins.

        void SortPrep(const unsigned int language, std::list<Message>& messages, std::function<void(const std::string&)> progressCallback);
        std::list<Plugin> Sort(const unsigned int language, std::list<Message>& messages, std::function<void(const std::string&)> progressCallback);

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

        std::unordered_set<std::string> activePlugins;  //Holds lowercased strings.

        //Creates directory in LOOT folder for LOOT's game-specific files.
        void CreateLOOTGameFolder();
    };

    std::vector<Game> GetGames(const YAML::Node& settings);

    size_t SelectGame(const YAML::Node& settings, const std::vector<Game>& games, const std::string& cmdLineGame);
}

#endif
