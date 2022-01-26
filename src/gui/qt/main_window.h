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

#include <QtCore/QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QtGui/QAction>
#else
#include <QtWidgets/QAction>
#endif

#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
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
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

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

public:
  MainWindow(LootState &state, QWidget *parent = nullptr);

  void initialise();

private:
  QAction *actionSort;
  QAction *actionApplySort;
  QAction *actionDiscardSort;
  QAction *actionUpdateMasterlist;
  QAction *actionSearch;
  QAction *actionViewDocs;
  QAction *actionOpenLOOTDataFolder;
  QAction *actionJoinDiscordServer;
  QAction *actionAbout;
  QAction *actionQuit;
  QAction *actionOpenGroupsEditor;
  QAction *actionCopyLoadOrder;
  QAction *actionCopyContent;
  QAction *actionRefreshContent;
  QAction *actionRedatePlugins;
  QAction *actionClearAllUserMetadata;
  QAction *actionCopyMetadata;
  QAction *actionCopyCardContent;
  QAction *actionEditMetadata;
  QAction *actionClearMetadata;
  QAction *actionSettings;
  QAction *actionBackupData;
  QComboBox *gameComboBox;
  QToolBox *toolBox;
  QTableView *sidebarPluginsView;
  QListView *pluginCardsView;
  QMenuBar *menubar;
  QMenu *menuFile;
  QMenu *menuHelp;
  QMenu *menuGame;
  QMenu *menuPlugin;
  QStatusBar *statusbar;
  QToolBar *toolBar;
  QProgressDialog *progressDialog;

  FiltersWidget *filtersWidget;
  SettingsDialog *settingsDialog;
  PluginEditorWidget *pluginEditorWidget;
  SearchDialog *searchDialog;
  GroupsEditorDialog *groupsEditor;

  PluginItemModel *pluginItemModel;
  PluginItemFilterModel *proxyModel;

  QNetworkAccessManager networkAccessManager;

  LootState &state;

  std::optional<QPersistentModelIndex> lastEnteredCardIndex;

  void setupUi();
  void setupMenuBar();
  void setupToolBar();
  void setupViews();

  void translateUi();

  void enableGameActions();
  void disableGameActions();
  void enablePluginActions();
  void disablePluginActions();

  void enterEditingState();
  void exitEditingState();

  void enterSortingState();
  void exitSortingState();

  void loadGame(bool isOnLOOTStartup);
  void updateCounts(const std::vector<SimpleMessage> &generalMessages,
                    const std::vector<PluginItem> &plugins);
  void updateGeneralInformation();
  void updateGeneralMessages();
  void updateSidebarColumnWidths();
  void setFiltersState(PluginFiltersState &&state);
  void setFiltersState(PluginFiltersState &&state,
                       std::vector<std::string> &&conflictingPluginNames);

  bool hasErrorMessages() const;

  void sortPlugins(bool isAutoSort);

  void showFirstRunDialog();
  void showNotification(const QString &message);

  PluginItem getSelectedPlugin() const;

  void closeEvent(QCloseEvent *event);

  void executeBackgroundQuery(std::unique_ptr<Query> query,
                              void (MainWindow::*onComplete)(QueryResult),
                              ProgressUpdater *progressUpdater);
  void executeBackgroundTasks(std::vector<Task *> tasks,
                              ProgressUpdater *progressUpdater);

  void sendHttpRequest(const std::string &url,
                       void (MainWindow::*onFinished)());

  void addUpdateAvailableMessage();
  void addUpdateCheckErrorMessage();

  void handleError(const std::string &message);
  void handleException(const std::exception &exception);
  void handleQueryException(const std::unique_ptr<Query> query,
                            const std::exception &exception);

  void handleGameDataLoaded(QueryResult result);
  void handlePluginsSorted(QueryResult results);

  QMenu *createPopupMenu();

  std::optional<std::filesystem::path> createBackup();

private slots:
  void on_actionSettings_triggered();
  void on_actionBackupData_triggered();
  void on_actionQuit_triggered();
  void on_actionOpenGroupsEditor_triggered();
  void on_actionSearch_triggered();
  void on_actionCopyLoadOrder_triggered();
  void on_actionCopyContent_triggered();
  void on_actionRefreshContent_triggered();
  void on_actionRedatePlugins_triggered();
  void on_actionClearAllUserMetadata_triggered();
  void on_actionEditMetadata_triggered();
  void on_actionCopyMetadata_triggered();
  void on_actionCopyCardContent_triggered();
  void on_actionClearMetadata_triggered();
  void on_actionViewDocs_triggered();
  void on_actionOpenLOOTDataFolder_triggered();
  void on_actionJoinDiscordServer_triggered();
  void on_actionAbout_triggered();

  void on_gameComboBox_activated(int index);
  void on_actionSort_triggered();
  void on_actionApplySort_triggered();
  void on_actionDiscardSort_triggered();
  void on_actionUpdateMasterlist_triggered();

  void on_sidebarPluginsView_doubleClicked(const QModelIndex &index);
  void on_sidebarPluginsSelectionModel_selectionChanged(
      const QItemSelection &selected);

  void on_pluginCardsView_entered(const QModelIndex &index);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
  void on_pluginItemModel_dataChanged(const QModelIndex &topLeft,
                                      const QModelIndex &bottomRight,
                                      const QList<int> &roles);
#else
  void on_pluginItemModel_dataChanged(const QModelIndex &topLeft,
                                      const QModelIndex &bottomRight,
                                      const QVector<int> &roles);
#endif

  void on_pluginEditorWidget_accepted(PluginMetadata userMetadata);
  void on_pluginEditorWidget_rejected();

  void on_filtersWidget_pluginFilterChanged(PluginFiltersState state);
  void on_filtersWidget_conflictsFilterChanged(
      std::optional<std::string> targetPluginName);
  void on_filtersWidget_cardContentFilterChanged(CardContentFiltersState state);

  void on_settingsDialog_accepted();

  void on_groupsEditor_accepted();

  void on_searchDialog_finished();
  void on_searchDialog_textChanged(const QString &text);
  void on_searchDialog_currentResultChanged(size_t resultIndex);

  void handleGameChanged(QueryResult result);
  void handleRefreshGameDataLoaded(QueryResult result);
  void handleStartupGameDataLoaded(QueryResult result);
  void handlePluginsManualSorted(QueryResult results);
  void handlePluginsAutoSorted(QueryResult results);
  void handleMasterlistUpdated(QueryResult result);
  void handleConflictsChecked(QueryResult result);
  void handleProgressUpdate(const QString &message);
  void handleWorkerThreadFinished();

  void handleGetLatestReleaseResponseFinished();
  void handleGetTagCommitResponseFinished();
  void handleUpdateCheckNetworkError(QNetworkReply::NetworkError error);
  void handleUpdateCheckSSLError(const QList<QSslError> &errors);
};
}

#endif
