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

#ifndef LOOT_GUI_QT_MAIN_WINDOW
#define LOOT_GUI_QT_MAIN_WINDOW

#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtCore/QtGlobal>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "gui/qt/card_delegate.h"
#include "gui/qt/filters_widget.h"
#include "gui/qt/groups_editor/groups_editor_dialog.h"
#include "gui/qt/plugin_editor/plugin_editor_widget.h"
#include "gui/qt/plugin_item_filter_model.h"
#include "gui/qt/plugin_item_model.h"
#include "gui/qt/search_dialog.h"
#include "gui/qt/settings/settings_dialog.h"
#include "gui/qt/tasks/tasks.h"
#include "gui/query/query.h"
#include "gui/state/loot_state.h"

namespace loot {
class MainWindow : public QMainWindow {
  Q_OBJECT
  Q_PROPERTY(QColor normalIconColor MEMBER normalIconColor NOTIFY
                 normalIconColorChanged)
  Q_PROPERTY(QColor disabledIconColor MEMBER disabledIconColor NOTIFY
                 disabledIconColorChanged)
  Q_PROPERTY(QColor selectedIconColor MEMBER selectedIconColor NOTIFY
                 selectedIconColorChanged)
  Q_PROPERTY(QColor selectedSidebarPluginTextColor MEMBER
                 selectedSidebarPluginTextColor NOTIFY
                     selectedSidebarPluginTextColorChanged)
  Q_PROPERTY(QColor unselectedSidebarPluginGroupColor MEMBER
                 unselectedSidebarPluginGroupColor NOTIFY
                     unselectedSidebarPluginGroupColorChanged)
  Q_PROPERTY(QColor linkColor MEMBER linkColor NOTIFY linkColorChanged)

public:
  explicit MainWindow(LootState &state, QWidget *parent = nullptr);

  void initialise();
  void applyTheme();

signals:
  void normalIconColorChanged();
  void disabledIconColorChanged();
  void selectedIconColorChanged();

  void selectedSidebarPluginTextColorChanged();
  void unselectedSidebarPluginGroupColorChanged();

  void linkColorChanged();

private:
  LootState &state;

  QAction *actionSort{new QAction(this)};
  QAction *actionApplySort{new QAction(this)};
  QAction *actionDiscardSort{new QAction(this)};
  QAction *actionUpdateMasterlist{new QAction(this)};
  QAction *actionSearch{new QAction(this)};
  QAction *actionViewDocs{new QAction(this)};
  QAction *actionOpenFAQs{new QAction(this)};
  QAction *actionOpenLOOTDataFolder{new QAction(this)};
  QAction *actionJoinDiscordServer{new QAction(this)};
  QAction *actionAbout{new QAction(this)};
  QAction *actionQuit{new QAction(this)};
  QAction *actionOpenGroupsEditor{new QAction(this)};
  QAction *actionCopyLoadOrder{new QAction(this)};
  QAction *actionCopyContent{new QAction(this)};
  QAction *actionRefreshContent{new QAction(this)};
  QAction *actionRedatePlugins{new QAction(this)};
  QAction *actionFixAmbiguousLoadOrder{new QAction(this)};
  QAction *actionClearAllUserMetadata{new QAction(this)};
  QAction *actionCopyPluginName{new QAction(this)};
  QAction *actionCopyCardContent{new QAction(this)};
  QAction *actionCopyMetadata{new QAction(this)};
  QAction *actionEditMetadata{new QAction(this)};
  QAction *actionClearMetadata{new QAction(this)};
  QAction *actionSettings{new QAction(this)};
  QAction *actionUpdateMasterlists{new QAction(this)};
  QAction *actionBackupData{new QAction(this)};

  QMenuBar *menubar{new QMenuBar(this)};
  QMenu *menuFile{new QMenu(menubar)};
  QMenu *menuHelp{new QMenu(menubar)};
  QMenu *menuGame{new QMenu(menubar)};
  QMenu *menuPlugin{new QMenu(menubar)};
  QStatusBar *statusbar{new QStatusBar(this)};
  QToolBar *toolBar{new QToolBar(this)};
  QComboBox *gameComboBox{new QComboBox(toolBar)};
  QProgressDialog *progressDialog{new QProgressDialog(this)};

  QSplitter *sidebarSplitter{new QSplitter(this)};
  QToolBox *toolBox{new QToolBox(sidebarSplitter)};
  FiltersWidget *filtersWidget{new FiltersWidget(toolBox)};
  QTableView *sidebarPluginsView{new QTableView(toolBox)};

  QSplitter *editorSplitter{
      new QSplitter(Qt::Orientation::Vertical, sidebarSplitter)};
  QListView *pluginCardsView{new QListView(editorSplitter)};
  PluginEditorWidget *pluginEditorWidget{
      new PluginEditorWidget(editorSplitter,
                             state.getSettings().getLanguages(),
                             state.getSettings().getLanguage())};

  SettingsDialog *settingsDialog{new SettingsDialog(this)};
  SearchDialog *searchDialog{new SearchDialog(this)};

  PluginItemModel *pluginItemModel{new PluginItemModel(this)};
  PluginItemFilterModel *proxyModel{new PluginItemFilterModel(this)};
  CardSizingCache cardSizingCache{pluginCardsView->viewport()};

  GroupsEditorDialog *groupsEditor{
      new GroupsEditorDialog(this, pluginItemModel)};

  std::optional<QPersistentModelIndex> lastEnteredCardIndex;

  QColor normalIconColor;
  QColor disabledIconColor;
  QColor selectedIconColor;

  QColor selectedSidebarPluginTextColor;
  QColor unselectedSidebarPluginGroupColor;

  QColor linkColor;

  std::vector<std::string> themes;

  void setupUi();
  void setupMenuBar();
  void setupToolBar();
  void setupViews();

  void translateUi();
  void setIcons();

  void enableGameActions();
  void disableGameActions();
  void enablePluginActions();
  void disablePluginActions();

  void enterEditingState();
  void exitEditingState();

  void enterSortingState();
  void exitSortingState();

  void loadGame(bool isOnLOOTStartup);
  void updateCounts(const std::vector<SourcedMessage> &generalMessages,
                    const std::vector<PluginItem> &plugins);
  void updateGeneralInformation();
  void updateGeneralMessages();
  void updateSidebarColumnWidths();
  void setFiltersState(PluginFiltersState &&state);
  void setFiltersState(PluginFiltersState &&state,
                       std::vector<std::string> &&overlappingPluginNames);
  void refreshSearch();
  void refreshPluginRawData(const std::string &pluginName);

  bool hasErrorMessages() const;

  void sortPlugins(bool isAutoSort);

  void showFirstRunDialog();
  void showNotification(const QString &message);

  QModelIndex getSelectedPluginIndex() const;
  PluginItem getSelectedPlugin() const;

  void closeEvent(QCloseEvent *event) override;

  void executeBackgroundQuery(std::unique_ptr<Query> query,
                              void (MainWindow::*onComplete)(QueryResult),
                              ProgressUpdater *progressUpdater);

  void handleError(const std::string &message);
  void handleException(const std::exception &exception);
  void handleQueryException(const Query &query,
                            const std::exception &exception);

  void handleGameDataLoaded(QueryResult result);
  bool handlePluginsSorted(QueryResult result);

  QMenu *createPopupMenu() override;

  std::optional<std::filesystem::path> createBackup();

  void checkForAmbiguousLoadOrder();

  void refreshGamesDropdown();

private slots:
  void on_actionSettings_triggered();
  void on_actionUpdateMasterlists_triggered();
  void on_actionBackupData_triggered();
  void on_actionQuit_triggered();
  void on_actionOpenGroupsEditor_triggered();
  void on_actionSearch_triggered();
  void on_actionCopyLoadOrder_triggered();
  void on_actionCopyContent_triggered();
  void on_actionFixAmbiguousLoadOrder_triggered();
  void on_actionRefreshContent_triggered();
  void on_actionRedatePlugins_triggered();
  void on_actionClearAllUserMetadata_triggered();
  void on_actionEditMetadata_triggered();
  void on_actionCopyPluginName_triggered();
  void on_actionCopyCardContent_triggered();
  void on_actionCopyMetadata_triggered();
  void on_actionClearMetadata_triggered();
  void on_actionViewDocs_triggered();
  void on_actionOpenFAQs_triggered();
  void on_actionOpenLOOTDataFolder_triggered();
  void on_actionJoinDiscordServer_triggered();
  void on_actionAbout_triggered();

  void on_gameComboBox_activated(int index);
  void on_actionSort_triggered();
  void on_actionApplySort_triggered();
  void on_actionDiscardSort_triggered();
  void on_actionUpdateMasterlist_triggered();

  void on_sidebarPluginsView_doubleClicked(const QModelIndex &index);
  void on_sidebarPluginsView_customContextMenuRequested(const QPoint &position);
  void handleSidebarPluginsSelectionChanged(const QItemSelection &selected,
                                            const QItemSelection &deselected);

  void on_pluginCardsView_entered(const QModelIndex &index);
  void on_pluginCardsView_pressed(const QModelIndex &index);
  void on_pluginCardsView_customContextMenuRequested(const QPoint &position);

  void on_pluginItemModel_dataChanged(const QModelIndex &topLeft,
                                      const QModelIndex &bottomRight,
                                      const QList<int> &roles);
  void on_pluginItemModel_rowsInserted(const QModelIndex &,
                                       int first,
                                       int last);

  void on_pluginEditorWidget_accepted(PluginMetadata userMetadata);
  void on_pluginEditorWidget_rejected();

  void on_filtersWidget_pluginFilterChanged(PluginFiltersState state);
  void on_filtersWidget_overlapFilterChanged(
      std::optional<std::string> targetPluginName);
  void on_filtersWidget_cardContentFilterChanged(CardContentFiltersState state);

  void on_settingsDialog_accepted();

  void on_groupsEditor_accepted();

  void on_searchDialog_finished();
  void on_searchDialog_textChanged(const QVariant &text);
  void on_searchDialog_currentResultChanged(size_t resultIndex);

  void handleGameChanged(QueryResult result);
  void handleRefreshGameDataLoaded(QueryResult result);
  void handleStartupGameDataLoaded(QueryResult result);
  void handlePluginsManualSorted(QueryResult result);
  void handlePluginsAutoSorted(QueryResult result);
  void handleMasterlistUpdated(std::vector<QueryResult> results);
  void handleMasterlistsUpdated(std::vector<QueryResult> results);
  void handleOverlapFilterChecked(QueryResult result);
  void handleProgressUpdate(const QString &message);
  void handleUpdateCheckFinished(QueryResult result);
  void handleUpdateCheckError(const std::string &);

  void handleIconColorChanged();
  void handleSidebarTextColorChanged();
  void handleLinkColorChanged();
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  void handleColorSchemeChanged(Qt::ColorScheme colorScheme);
#endif
};
}

#endif
