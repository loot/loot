/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2015    WrinklyNinja

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

#ifndef LOOT_GUI_LOOT_SETTINGS
#define LOOT_GUI_LOOT_SETTINGS

#include "backend/game/game_settings.h"
#include "backend/helpers/language.h"

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>

namespace loot {
    class LootSettings {
    public:
        struct WindowPosition {
            WindowPosition();

            long top;
            long bottom;
            long left;
            long right;
        };

        LootSettings();

        void load(YAML::Node& settings);
        void load(const boost::filesystem::path& file);
        void save(const boost::filesystem::path& file);

        bool isDebugLoggingEnabled() const;
        bool isWindowPositionStored() const;
        std::string getGame() const;
        std::string getLastGame() const;
        std::string getLastVersion() const;
        const Language& getLanguage() const;
        const WindowPosition& getWindowPosition() const;
        std::vector<GameSettings> getGameSettings() const;

        void storeLastGame(const std::string& lastGame);
        void storeWindowPosition(const WindowPosition& position);
        void storeGameSettings(const std::vector<GameSettings>& gameSettings);
        void storeFilterState(const std::string& filterId, bool enabled);
        void updateLastVersion();

        YAML::Node toYaml() const;
    private:
        bool enableDebugLogging;
        bool updateMasterlist;
        std::string game;
        std::string lastGame;
        std::string lastVersion;
        Language language;
        WindowPosition windowPosition;
        std::vector<GameSettings> gameSettings;
        std::map<std::string, bool> filters;

        mutable std::recursive_mutex mutex;

        static void upgradeYaml(YAML::Node& yaml);
    };
}

#endif
