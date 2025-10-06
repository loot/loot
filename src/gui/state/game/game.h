/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2012 WrinklyNinja

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

#ifndef LOOT_GUI_STATE_GAME_GAME
#define LOOT_GUI_STATE_GAME_GAME

#ifndef _WIN32
#ifdef emit
// The TBB library is used on Linux to provide the implementation of the
// standard library parallel execution algorithms, but oneTBB 2021.8.0 defines
// an emit() function that clashes with Qt's emit macro, which may be defined
// when this header is included. As such, undefine the macro before including
// the standard library headers, and redefine it afterwards.
#undef emit
#define LOOT_SHOULD_REDEFINE_EMIT
#endif
#endif

#include <execution>
#include <filesystem>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <variant>

#ifdef LOOT_SHOULD_REDEFINE_EMIT
// Assume emit was Qt's emit macro, which is empty.
#define emit
#undef LOOT_SHOULD_REDEFINE_EMIT
#endif

#include "gui/sourced_message.h"
#include "gui/state/change_count.h"
#include "gui/state/game/game_settings.h"
#include "gui/state/game/load_order_backup.h"
#include "gui/state/logging.h"
#include "loot/api.h"

namespace loot {
std::filesystem::path getMasterlistPath(
    const std::filesystem::path& lootDataPath,
    const GameSettings& settings);

void initLootGameFolder(const std::filesystem::path& lootDataPath_,
                        const GameSettings& settings);

// The Creation Club has been replaced, but while it was active it was
// available for Skyrim SE and Fallout 4.
bool hadCreationClub(GameId gameId);

class CreationClubPlugins {
public:
  void load(GameId gameId, const std::filesystem::path& gamePath);

  bool isCreationClubPlugin(const std::string& name) const;

private:
  // Use Filename to benefit from libloot's case-insensitive comparisons.
  std::set<Filename> creationClubPlugins_;
};

namespace gui {
class Game {
public:
  Game(const GameSettings& gameSettings,
       const std::filesystem::path& lootDataPath,
       const std::filesystem::path& preludePath);
  Game(const Game& game) = delete;
  Game(Game&& game) noexcept;
  ~Game() = default;

  Game& operator=(const Game& game) = delete;
  Game& operator=(Game&& game) noexcept;

  const GameSettings& getSettings() const;
  GameSettings& getSettings();

  const CreationClubPlugins& getCreationClubPlugins() const;
  CreationClubPlugins& getCreationClubPlugins();

  void init();
  bool isInitialised() const;

  std::shared_ptr<const PluginInterface> getPlugin(
      const std::string& name) const;
  std::vector<std::shared_ptr<const PluginInterface>> getPlugins() const;
  std::vector<SourcedMessage> checkInstallValidity(
      const PluginInterface& plugin,
      const PluginMetadata& metadata,
      std::string_view language) const;

  void redatePlugins();  // Change timestamps to match load order (Skyrim only).

  void backUpCurrentLoadOrder(std::string_view name) const;
  std::vector<LoadOrderBackup> findLoadOrderBackups() const;

  void loadAllInstalledPlugins(
      bool headersOnly);  // Loads all installed plugins.
  bool arePluginsFullyLoaded()
      const;  // Checks if the game's plugins have already been loaded.
  bool supportsLightPlugins() const;
  bool supportsMediumPlugins() const;

  std::filesystem::path getMasterlistPath() const;
  std::filesystem::path getUserlistPath() const;
  std::filesystem::path getGroupNodePositionsPath() const;
  std::filesystem::path getActivePluginsFilePath() const;
  std::filesystem::path getOldMessagesPath() const;

  std::vector<std::string> getLoadOrder() const;
  void setLoadOrder(const std::vector<std::string>& loadOrder);

  bool isPluginActive(const std::string& pluginName) const;
  std::optional<short> getActiveLoadOrderIndex(
      const PluginInterface& plugin,
      const std::vector<std::string>& loadOrder) const;

  bool isLoadOrderAmbiguous() const;

  std::vector<std::string> sortPlugins();
  ChangeCount& getSortCount();

  std::vector<SourcedMessage> getMessages(std::string_view language,
                                          bool warnOnCaseSensitivePaths) const;
  void appendMessage(const SourcedMessage& message);
  void clearMessages();

  void loadMetadata();
  std::vector<std::string> getKnownBashTags() const;

  std::vector<Group> getGroups() const;
  std::vector<Group> getMasterlistGroups() const;
  std::vector<Group> getUserGroups() const;

  std::optional<PluginMetadata> getMasterlistMetadata(
      const std::string& pluginName,
      bool evaluateConditions = false) const;
  std::optional<PluginMetadata> getUserMetadata(
      const std::string& pluginName,
      bool evaluateConditions = false) const;
  std::optional<PluginMetadata> getNonUserMetadata(
      const PluginInterface& plugin) const;

  bool evaluateConstraint(const File& file) const;

  void setUserGroups(const std::vector<Group>& groups);
  void addUserMetadata(const PluginMetadata& metadata);
  void clearUserMetadata(const std::string& pluginName);
  void clearAllUserMetadata();
  void saveUserMetadata();

  std::string getLoadOrderAsTextTable() const;

  bool fileExists(const std::string& file) const;

private:
  std::filesystem::path getLOOTGamePath() const;
  std::filesystem::path getBackupsPath() const;
  std::vector<std::filesystem::path> getInstalledPluginPaths() const;
  std::optional<std::filesystem::path> resolveGameFilePath(
      const std::string& pluginName) const;

  void appendMessages(std::vector<SourcedMessage> messages);

  void loadCurrentLoadOrderState();

  GameSettings settings_;
  CreationClubPlugins creationClubPlugins_;
  std::unique_ptr<GameInterface> gameHandle_;
  std::vector<SourcedMessage> messages_;
  std::filesystem::path lootDataPath_;
  std::filesystem::path preludePath_;
  ChangeCount sortCount_;
  bool pluginsFullyLoaded_{false};
  bool supportsLightPlugins_{false};
};
}

std::string getMetadataAsBBCodeYaml(const gui::Game& game,
                                    const std::string& pluginName);

typedef std::
    tuple<std::shared_ptr<const PluginInterface>, std::optional<short>, bool>
                                    LoadOrderTuple;

std::vector<LoadOrderTuple> mapToLoadOrderTuples(
    const gui::Game& game,
    const std::vector<std::string>& loadOrder);

template<typename T>
std::vector<T> mapFromLoadOrderData(
    const gui::Game& game,
    const std::vector<std::string>& loadOrder,
    const std::function<
        T(std::shared_ptr<const PluginInterface>, std::optional<short>, bool)>& mapper) {
  const auto data = mapToLoadOrderTuples(game, loadOrder);

  // Now perform the mapping in a second loop that can be parallelised
  // (because sometimes the mapper is slow).
  //
  // Store mapped data in an std::variant because if the transformation is
  // fallible then there needs to be some way of detecting that. The second
  // type in the variant holds the exception message string if an exception
  // is thrown by the mapper.
  typedef std::variant<T, std::string> MappedDataOrError;
  const auto transformer = [&mapper](const LoadOrderTuple& loadOrderTuple) {
    try {
      const auto& [plugin, activeLoadOrderIndex, isActive] = loadOrderTuple;

      const auto mappedData = mapper(plugin, activeLoadOrderIndex, isActive);

      return MappedDataOrError(mappedData);
    } catch (const std::exception& e) {
      const auto logger = getLogger();
      if (logger) {
        logger->error(
            "Failed to map load order data to output type, exception is: {}",
            e.what());
      }
      return MappedDataOrError(e.what());
    }
  };

  // Can't use std::back_inserter as the output iterator when running the
  // transform in parallel, so presize the vector.
  std::vector<MappedDataOrError> maybeMappedData(data.size());

  std::transform(std::execution::par_unseq,
                 data.cbegin(),
                 data.cend(),
                 maybeMappedData.begin(),
                 transformer);

  std::vector<T> mappedData;
  mappedData.reserve(maybeMappedData.size());

  for (const auto& mappedDataOrError : maybeMappedData) {
    if (std::holds_alternative<T>(mappedDataOrError)) {
      mappedData.push_back(std::get<T>(mappedDataOrError));
    } else {
      throw std::runtime_error(std::get<std::string>(mappedDataOrError));
    }
  }

  return mappedData;
}
}

#endif
