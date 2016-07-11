/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2016    WrinklyNinja

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

#include "loot_settings.h"
#include "backend/app/loot_version.h"

#include <thread>

#include <boost/filesystem/fstream.hpp>

using namespace std;

namespace loot {
    LootSettings::WindowPosition::WindowPosition() : top(0), bottom(0), left(0), right(0) {}

    LootSettings::LootSettings() :
        gameSettings({
            GameSettings(GameSettings::tes4),
            GameSettings(GameSettings::tes5),
            GameSettings(GameSettings::fo3),
            GameSettings(GameSettings::fonv),
            GameSettings(GameSettings::fo4),
            GameSettings(GameSettings::tes4, "Nehrim")
                .SetName("Nehrim - At Fate's Edge")
                .SetMaster("Nehrim.esm")
                .SetRegistryKey("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Nehrim - At Fate's Edge_is1\\InstallLocation"),
    }),
        enableDebugLogging(false),
        updateMasterlist(true),
        game("auto"),
        language(Language(Language::Code::english)),
        lastGame("auto") {}

    void LootSettings::load(YAML::Node& settings) {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        upgradeYaml(settings);

        if (settings["enableDebugLogging"])
            enableDebugLogging = settings["enableDebugLogging"].as<bool>();
        if (settings["updateMasterlist"])
            updateMasterlist = settings["updateMasterlist"].as<bool>();
        if (settings["game"])
            game = settings["game"].as<string>();
        if (settings["language"])
            language = Language(settings["language"].as<string>());
        if (settings["lastGame"])
            lastGame = settings["lastGame"].as<string>();
        if (settings["lastVersion"])
            lastVersion = settings["lastVersion"].as<string>();

        if (settings["window"]
            && settings["window"]["top"] && settings["window"]["bottom"]
            && settings["window"]["left"] && settings["window"]["right"]) {
            windowPosition.top = settings["window"]["top"].as<long>();
            windowPosition.bottom = settings["window"]["bottom"].as<long>();
            windowPosition.left = settings["window"]["left"].as<long>();
            windowPosition.right = settings["window"]["right"].as<long>();
        }

        if (settings["games"]) {
            gameSettings = settings["games"].as<vector<GameSettings>>();

            // If a base game isn't in the settings, add it.
            if (find(begin(gameSettings), end(gameSettings), GameSettings(GameSettings::tes4)) == end(gameSettings))
                gameSettings.push_back(GameSettings(GameSettings::tes4));

            if (find(begin(gameSettings), end(gameSettings), GameSettings(GameSettings::tes5)) == end(gameSettings))
                gameSettings.push_back(GameSettings(GameSettings::tes5));

            if (find(begin(gameSettings), end(gameSettings), GameSettings(GameSettings::fo3)) == end(gameSettings))
                gameSettings.push_back(GameSettings(GameSettings::fo3));

            if (find(begin(gameSettings), end(gameSettings), GameSettings(GameSettings::fonv)) == end(gameSettings))
                gameSettings.push_back(GameSettings(GameSettings::fonv));

            if (find(begin(gameSettings), end(gameSettings), GameSettings(GameSettings::fo4)) == end(gameSettings))
                gameSettings.push_back(GameSettings(GameSettings::fo4));
        }

        if (settings["filters"])
            filters = settings["filters"].as<map<string, bool>>();
    }

    void LootSettings::load(const boost::filesystem::path& file) {
        boost::filesystem::ifstream in(file);
        YAML::Node content = YAML::Load(in);
        load(content);
    }

    void LootSettings::save(const boost::filesystem::path& file) {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << toYaml();

        boost::filesystem::ofstream out(file);
        out << yout.c_str();
    }

    bool LootSettings::isDebugLoggingEnabled() const {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        return enableDebugLogging;
    }

    bool LootSettings::isWindowPositionStored() const {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        return windowPosition.top != 0 || windowPosition.bottom != 0 || windowPosition.left != 0 || windowPosition.right != 0;
    }

    std::string LootSettings::getGame() const {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        return game;
    }

    std::string LootSettings::getLastGame() const {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        return lastGame;
    }

    std::string LootSettings::getLastVersion() const {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        return lastVersion;
    }

    const Language& LootSettings::getLanguage() const {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        return language;
    }

    const LootSettings::WindowPosition& LootSettings::getWindowPosition() const {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        return windowPosition;
    }

    std::vector<GameSettings> LootSettings::getGameSettings() const {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        return gameSettings;
    }

    void LootSettings::storeLastGame(const std::string& lastGame) {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        this->lastGame = lastGame;
    }

    void LootSettings::storeWindowPosition(const WindowPosition& position) {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        windowPosition = position;
    }

    void LootSettings::storeGameSettings(const std::vector<GameSettings>& gameSettings) {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        this->gameSettings = gameSettings;
    }

    void LootSettings::storeFilterState(const std::string& filterId, bool enabled) {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        filters[filterId] = enabled;
    }

    void LootSettings::updateLastVersion() {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        lastVersion = LootVersion::string();
    }

    YAML::Node LootSettings::toYaml() const {
        std::lock_guard<std::recursive_mutex> guard(mutex);

        YAML::Node node;

        node["enableDebugLogging"] = enableDebugLogging;
        node["updateMasterlist"] = updateMasterlist;
        node["game"] = game;
        node["language"] = language.GetLocale();
        node["lastGame"] = lastGame;
        node["lastVersion"] = lastVersion;

        if (isWindowPositionStored()) {
            node["window"]["top"] = windowPosition.top;
            node["window"]["bottom"] = windowPosition.bottom;
            node["window"]["left"] = windowPosition.left;
            node["window"]["right"] = windowPosition.right;
        }

        node["games"] = gameSettings;

        if (!filters.empty())
            node["filters"] = filters;

        return node;
    }

    void LootSettings::upgradeYaml(YAML::Node& yaml) {
        // Upgrade YAML settings' keys and values from those used in earlier
        // versions of LOOT.

        if (yaml["Debug Verbosity"] && !yaml["enableDebugLogging"])
            yaml["enableDebugLogging"] = yaml["Debug Verbosity"].as<unsigned int>() > 0;

        if (yaml["Update Masterlist"] && !yaml["updateMasterlist"])
            yaml["updateMasterlist"] = yaml["Update Masterlist"];

        if (yaml["Game"] && !yaml["game"])
            yaml["game"] = yaml["Game"];

        if (yaml["Language"] && !yaml["language"])
            yaml["language"] = yaml["Language"];

        if (yaml["Last Game"] && !yaml["lastGame"])
            yaml["lastGame"] = yaml["Last Game"];

        if (yaml["Games"] && !yaml["games"]) {
            yaml["games"] = yaml["Games"];

            for (auto node : yaml["games"]) {
                if (node["url"]) {
                    node["repo"] = node["url"];
                    node["branch"] = "v0.8";
                }
            }
        }

        if (yaml["games"]) {
            const set<string> oldDefaultBranches({
                "master",
                "v0.7",
            });

            // Handle exception if YAML is invalid, eg. if an unrecognised
            // game type is used (which can happen if downgrading from a
            // later version of LOOT that supports more game types).
            // However, can't remove elements from a sequence Node, so have to
            // copy the valid elements into a new node then overwrite the
            // original.
            YAML::Node validGames;
            for (auto node : yaml["games"]) {
                try {
                    GameSettings settings(node.as<GameSettings>());

                    if (!yaml["Games"]) {
                        // Update existing default branch, if the default
                        // repositories are used.
                        if (settings.RepoURL() == GameSettings(settings.Id()).RepoURL()
                            && oldDefaultBranches.count(settings.RepoBranch()) == 1) {
                            settings.SetRepoBranch("v0.8");
                        }
                    }

                    validGames.push_back(settings);
                }
                catch (...) {}
            }
            yaml["games"] = validGames;
        }

        if (yaml["filters"])
            yaml["filters"].remove("contentFilter");
    }
}
