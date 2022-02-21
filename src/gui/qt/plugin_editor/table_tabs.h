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

#ifndef LOOT_GUI_QT_PLUGIN_EDITOR_TABLE_TABS
#define LOOT_GUI_QT_PLUGIN_EDITOR_TABLE_TABS

#include <loot/metadata/file.h>
#include <loot/metadata/location.h>
#include <loot/metadata/message.h>
#include <loot/metadata/plugin_cleaning_data.h>
#include <loot/metadata/tag.h>

#include <QtCore/QAbstractTableModel>
#include <QtGui/QShowEvent>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

#include "gui/state/loot_settings.h"

namespace loot {
class BaseTableTab : public QWidget {
  Q_OBJECT
public:
  explicit BaseTableTab(QWidget* parent);

signals:
  void tableRowCountChanged(bool hasUserMetadata);

protected:
  QAbstractItemModel* getTableModel() const;
  void setTableModel(QAbstractItemModel* model);
  void setColumnHidden(int column, bool hide);
  void setItemDelegateForColumn(int column, QStyledItemDelegate* delegate);
  void setColumnFixedWidth(int column, int width);
  void configureAsDropTarget();

  virtual bool hasUserMetadata() const = 0;

  void resizeEvent(QResizeEvent* event) override;

private:
  QTableView* tableView{new QTableView(this)};
  QPushButton* addNewRowButton{new QPushButton(this)};
  QPushButton* deleteRowButton{new QPushButton(this)};

  void setupUi();
  void translateUi();

private slots:
  void on_addNewRowButton_clicked();
  void on_deleteRowButton_clicked();

  void onSelectionModelSelectionChanged();

  void onModelRowsInserted();
  void onModelRowsRemoved();
};

template<typename T>
class MetadataTableTab : public BaseTableTab {
public:
  using BaseTableTab::BaseTableTab;

  virtual void initialiseInputs(const std::vector<T>& nonUserMetadata,
                                const std::vector<T>& userMetadata) = 0;

  virtual std::vector<T> getUserMetadata() const = 0;
};

class FileTableTab : public MetadataTableTab<File> {
  Q_OBJECT
public:
  FileTableTab(QWidget* parent,
               const std::vector<LootSettings::Language>& languages,
               const std::string& language,
               const QStringList& completions);

  void initialiseInputs(const std::vector<File>& nonUserMetadata,
                        const std::vector<File>& userMetadata) override;

  std::vector<File> getUserMetadata() const override;

  bool hasUserMetadata() const override;

private:
  const std::vector<LootSettings::Language>& languages;
  const std::string& language;
  const QStringList& completions;
};

class LoadAfterFileTableTab : public FileTableTab {
public:
  using FileTableTab::FileTableTab;

  void initialiseInputs(const std::vector<File>& nonUserMetadata,
                        const std::vector<File>& userMetadata) override;
};

class MessageContentTableWidget : public MetadataTableTab<MessageContent> {
  Q_OBJECT
public:
  MessageContentTableWidget(
      QWidget* parent,
      const std::vector<LootSettings::Language>& languages);

  void initialiseInputs(
      const std::vector<MessageContent>& nonUserMetadata,
      const std::vector<MessageContent>& userMetadata) override;

  std::vector<MessageContent> getUserMetadata() const override;

  bool hasUserMetadata() const override;

private:
  std::vector<std::pair<QString, QVariant>> languages;
  std::map<std::string, QVariant> languageMap;
};

class MessageTableTab : public MetadataTableTab<Message> {
  Q_OBJECT
public:
  MessageTableTab(QWidget* parent,
                  const std::vector<LootSettings::Language>& languages,
                  const std::string& language);

  void initialiseInputs(const std::vector<Message>& nonUserMetadata,
                        const std::vector<Message>& userMetadata) override;

  std::vector<Message> getUserMetadata() const override;

  bool hasUserMetadata() const override;

private:
  const std::vector<LootSettings::Language>& languages;
  const std::string& language;
};

class LocationTableTab : public MetadataTableTab<Location> {
  Q_OBJECT
public:
  using MetadataTableTab::MetadataTableTab;

  void initialiseInputs(const std::vector<Location>& nonUserMetadata,
                        const std::vector<Location>& userMetadata) override;

  std::vector<Location> getUserMetadata() const override;

  bool hasUserMetadata() const override;
};

class CleaningDataTableTab : public MetadataTableTab<PluginCleaningData> {
  Q_OBJECT
public:
  CleaningDataTableTab(QWidget* parent,
                       const std::vector<LootSettings::Language>& languages,
                       const std::string& language);

  void initialiseInputs(
      const std::vector<PluginCleaningData>& nonUserMetadata,
      const std::vector<PluginCleaningData>& userMetadata) override;

  void hideCounts(bool hide);

  std::vector<PluginCleaningData> getUserMetadata() const override;

  bool hasUserMetadata() const override;

private:
  const std::vector<LootSettings::Language>& languages;
  const std::string& language;
};

class TagTableTab : public MetadataTableTab<Tag> {
  Q_OBJECT
public:
  TagTableTab(QWidget* parent, const QStringList& completions);

  void initialiseInputs(const std::vector<Tag>& nonUserMetadata,
                        const std::vector<Tag>& userMetadata) override;

  std::vector<Tag> getUserMetadata() const override;

  bool hasUserMetadata() const override;

private:
  const QStringList& completions;
};
}

#endif
