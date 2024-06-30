/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2021    Oliver Hamlet

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

#ifndef LOOT_GUI_QT_PLUGIN_ITEM_MODEL
#define LOOT_GUI_QT_PLUGIN_ITEM_MODEL

#include <QtCore/QAbstractListModel>

#include "gui/plugin_item.h"
#include "gui/qt/counters.h"
#include "gui/qt/filters_states.h"
#include "gui/qt/general_info.h"
#include "gui/qt/helpers.h"

Q_DECLARE_METATYPE(loot::PluginItem);

namespace loot {
static constexpr int RawDataRole = Qt::UserRole + 1;
static constexpr int EditorStateRole = Qt::UserRole + 2;
static constexpr int CountersRole = Qt::UserRole + 4;
static constexpr int CardContentFiltersRole = Qt::UserRole + 5;
static constexpr int ContentSearchRole = Qt::UserRole + 6;
static constexpr int DragRole = Qt::UserRole + 7;
static constexpr int SearchResultRole = Qt::UserRole + 8;

struct SearchResultData {
  SearchResultData() = default;
  SearchResultData(bool isResult, bool isCurrentResult);

  bool isResult{false};
  bool isCurrentResult{false};
};

class PluginItemModel : public QAbstractListModel {
  Q_OBJECT
public:
  static constexpr int SIDEBAR_POSITION_COLUMN = 0;
  static constexpr int SIDEBAR_INDEX_COLUMN = 1;
  static constexpr int SIDEBAR_NAME_COLUMN = 2;
  static constexpr int SIDEBAR_STATE_COLUMN = 3;
  static constexpr int CARDS_COLUMN = 4;

  explicit PluginItemModel(QObject* parent);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;

  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  QStringList mimeTypes() const override;

  QMimeData* mimeData(const QModelIndexList& indexes) const override;

  bool setData(const QModelIndex& index,
               const QVariant& value,
               int role) override;

  const std::vector<PluginItem>& getPluginItems() const;

  std::vector<std::string> getPluginNames() const;

  std::unordered_map<std::string, int> getPluginNameToRowMap() const;

  void setPluginItems(std::vector<PluginItem>&& items);

  void setEditorPluginName(const std::optional<std::string>& editorPluginName);

  void setGeneralInformation(bool gameSupportsLightPlugins,
                             bool gameSupportsMediumPlugins,
                             const FileRevisionSummary& masterlistRevision,
                             const FileRevisionSummary& preludeRevision,
                             const std::vector<SourcedMessage>& messages);

  void setPreludeRevision(const FileRevisionSummary& preludeRevision);

  void setGeneralMessages(std::vector<SourcedMessage>&& messages);

  const std::vector<SourcedMessage>& getGeneralMessages() const;

  const GeneralInformation& getGeneralInfo() const;

  void setCardContentFiltersState(CardContentFiltersState&& state);

  QModelIndex setCurrentSearchResult(size_t resultIndex);

private:
  GeneralInformation generalInformation;
  std::vector<PluginItem> items;
  std::vector<bool> searchResults;
  std::optional<int> currentSearchResultIndex;

  std::optional<std::string> currentEditorPluginName;
  CardContentFiltersState cardContentFiltersState;
};
}

Q_DECLARE_METATYPE(loot::SearchResultData);

#endif
