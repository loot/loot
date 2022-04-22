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

#include "gui/qt/main_window.h"

#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextEdit>

#include "gui/backup.h"
#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"
#include "gui/qt/plugin_card.h"
#include "gui/qt/plugin_item_filter_model.h"
#include "gui/qt/sidebar_plugin_name_delegate.h"
#include "gui/qt/style.h"
#include "gui/qt/tasks/check_for_update_task.h"
#include "gui/qt/tasks/update_masterlist_task.h"
#include "gui/query/types/apply_sort_query.h"
#include "gui/query/types/cancel_sort_query.h"
#include "gui/query/types/change_game_query.h"
#include "gui/query/types/clear_all_metadata_query.h"
#include "gui/query/types/clear_plugin_metadata_query.h"
#include "gui/query/types/copy_load_order_query.h"
#include "gui/query/types/copy_metadata_query.h"
#include "gui/query/types/get_conflicting_plugins_query.h"
#include "gui/query/types/get_game_data_query.h"
#include "gui/query/types/open_log_location_query.h"
#include "gui/query/types/open_readme_query.h"
#include "gui/query/types/sort_plugins_query.h"
#include "gui/version.h"

namespace loot {
std::vector<std::string> GetGroupNames(
    const std::vector<Group>& masterlistGroups,
    const std::vector<Group>& userGroups) {
  std::set<std::string> groupNames;

  for (const auto& group : masterlistGroups) {
    groupNames.insert(group.GetName());
  }

  for (const auto& group : userGroups) {
    groupNames.insert(group.GetName());
  }

  return std::vector<std::string>(groupNames.begin(), groupNames.end());
}

std::vector<std::string> GetGroupNames(const gui::Game& game) {
  return GetGroupNames(game.GetMasterlistGroups(), game.GetUserGroups());
}

bool hasLoadOrderChanged(const std::vector<std::string>& oldLoadOrder,
                         const std::vector<PluginItem>& newLoadOrder) {
  if (oldLoadOrder.size() != newLoadOrder.size()) {
    return true;
  }

  for (size_t i = 0; i < oldLoadOrder.size(); i += 1) {
    if (oldLoadOrder.at(i) != newLoadOrder.at(i).name) {
      return true;
    }
  }

  return false;
}

int calculateSidebarHeaderWidth(const QAbstractItemView& view, int column) {
  const auto headerText =
      view.model()->headerData(column, Qt::Horizontal).toString();

  const auto textWidth = QFontMetricsF(QApplication::font())
                             .size(Qt::TextSingleLine, headerText)
                             .width();

  const auto paddingWidth =
      QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin);

  return textWidth + paddingWidth;
}

int calculateSidebarPositionSectionWidth(size_t pluginCount) {
  // Find the widest digit character in the current font and use that to
  // calculate the load order section width.
  static constexpr std::array<char, 10> DIGIT_CHARACTERS = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

  const auto fontMetrics = QFontMetricsF(QApplication::font());

  qreal maxCharWidth = 0;
  for (const auto hexCharacter : DIGIT_CHARACTERS) {
    const auto width =
        fontMetrics.size(Qt::TextSingleLine, QString(QChar(hexCharacter)))
            .width();
    if (width > maxCharWidth) {
      maxCharWidth = width;
    }
  }

  const auto paddingWidth =
      QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin);

  const int numberOfDigits = log10(pluginCount) + 1;

  return numberOfDigits * static_cast<int>(maxCharWidth) + paddingWidth;
}

int calculateSidebarIndexSectionWidth(GameType gameType) {
  // Find the widest hex character in the current font and use that to
  // calculate the load order section width.
  static constexpr std::array<char, 16> HEX_CHARACTERS = {'0',
                                                          '1',
                                                          '2',
                                                          '3',
                                                          '4',
                                                          '5',
                                                          '6',
                                                          '7',
                                                          '8',
                                                          '9',
                                                          'A',
                                                          'B',
                                                          'C',
                                                          'D',
                                                          'E',
                                                          'F'};

  auto fontMetrics = QFontMetricsF(QApplication::font());

  qreal maxCharWidth = 0;
  for (const auto hexCharacter : HEX_CHARACTERS) {
    auto width =
        fontMetrics.size(Qt::TextSingleLine, QString(QChar(hexCharacter)))
            .width();
    if (width > maxCharWidth) {
      maxCharWidth = width;
    }
  }

  const auto paddingWidth =
      QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin);

  // If the game supports light plugins leave enough space for their longer
  // indexes, otherwise only leave space for two hex digits.
  switch (gameType) {
    case GameType::tes3:
    case GameType::tes4:
    case GameType::tes5:
    case GameType::fo3:
    case GameType::fonv:
      return 2 * static_cast<int>(maxCharWidth) + paddingWidth;
    default:
      auto prefixWidth =
          static_cast<int>(fontMetrics.size(Qt::TextSingleLine, "FE ").width());
      return prefixWidth + 3 * static_cast<int>(maxCharWidth) + paddingWidth;
  }
}

MainWindow::MainWindow(LootState& state, QWidget* parent) :
    QMainWindow(parent), state(state) {
  qRegisterMetaType<QueryResult>("QueryResult");
  qRegisterMetaType<std::string>("std::string");

  setupUi();

  auto installedGames = state.GetInstalledGameFolderNames();

  for (const auto& gameSettings : state.getSettings().getGameSettings()) {
    auto installedGame = std::find(installedGames.cbegin(),
                                   installedGames.cend(),
                                   gameSettings.FolderName());

    if (installedGame != installedGames.cend()) {
      gameComboBox->addItem(QString::fromStdString(gameSettings.Name()),
                            QString::fromStdString(gameSettings.FolderName()));
    }
  }
}

void MainWindow::initialise() {
  try {
    themes = findThemes(state.getResourcesPath());

    if (state.getSettings().getLastVersion() != gui::Version::string()) {
      showFirstRunDialog();
    }

    state.initCurrentGame();

    auto initMessages = state.getInitMessages();
    const auto initHasErrored =
        std::any_of(initMessages.begin(),
                    initMessages.end(),
                    [](const SimpleMessage& message) {
                      return message.type == MessageType::error;
                    });
    pluginItemModel->setGeneralMessages(std::move(initMessages));

    if (initHasErrored) {
      return;
    }

    const auto& filters = state.getSettings().getFilters();
    filtersWidget->setFilterStates(filters);

    // Apply the filters before loading the game because that avoids having
    // to re-filter the full plugin list.
    pluginItemModel->setCardContentFiltersState(
        filtersWidget->getCardContentFiltersState());
    proxyModel->setFiltersState(filtersWidget->getPluginFiltersState(), {});

    gameComboBox->setCurrentText(
        QString::fromStdString(state.GetCurrentGame().GetSettings().Name()));

    loadGame(true);

    // Check for updates.
    if (state.getSettings().isLootUpdateCheckEnabled()) {
      // This task can be run in the main thread because it's non-blocking.
      const auto task = new CheckForUpdateTask();

      connect(
          task, &Task::finished, this, &MainWindow::handleUpdateCheckFinished);
      connect(task, &Task::finished, task, &QObject::deleteLater);
      connect(task, &Task::error, this, &MainWindow::handleUpdateCheckError);
      connect(task, &Task::error, task, &QObject::deleteLater);

      task->execute();
    }
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::setupUi() {
#ifdef _WIN32
  setWindowIcon(QIcon(":/icon.ico"));
#endif

  auto lastWindowPosition = state.getSettings().getWindowPosition();
  if (lastWindowPosition.has_value()) {
    const auto& windowPosition = lastWindowPosition.value();
    const auto width = windowPosition.right - windowPosition.left;
    const auto height = windowPosition.bottom - windowPosition.top;

    const auto geometry =
        QRect(windowPosition.left, windowPosition.top, width, height);
    const auto topLeft = geometry.topLeft();

    if (QGuiApplication::screenAt(topLeft) == nullptr) {
      // No screen exists at the old position, just leave the Window at the
      // default position Qt gives it so that it isn't positioned off-screen.
      auto logger = getLogger();
      if (logger) {
        logger->warn(
            "Could not restore LOOT window position because no screen exists "
            "at the coordinates ({}, {})",
            topLeft.x(),
            topLeft.y());
      }
      resize(width, height);
    } else {
      setGeometry(geometry);
    }
  } else {
    static constexpr int DEFAULT_WIDTH = 1024;
    static constexpr int DEFAULT_HEIGHT = 768;
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  }

  // Set up status bar.
  setStatusBar(statusbar);

  setupMenuBar();
  setupToolBar();

  settingsDialog->setObjectName("settingsDialog");
  searchDialog->setObjectName("searchDialog");
  sidebarPluginsView->setObjectName("sidebarPluginsView");

  toolBox->addItem(sidebarPluginsView, QString("Plugins"));

  filtersWidget->setObjectName("filtersWidget");

  toolBox->addItem(filtersWidget, QString("Filters"));

  sidebarSplitter->addWidget(toolBox);

  pluginCardsView->setObjectName("pluginCardsView");

  editorSplitter->addWidget(pluginCardsView);

  pluginEditorWidget->setObjectName("pluginEditorWidget");
  pluginEditorWidget->hide();

  editorSplitter->addWidget(pluginEditorWidget);

  sidebarSplitter->addWidget(editorSplitter);

  sidebarSplitter->setStretchFactor(0, 1);
  sidebarSplitter->setStretchFactor(1, 2);

  const auto horizontalSpace =
      style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
  const auto verticalSpace =
      style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing);
  sidebarSplitter->setContentsMargins(
      horizontalSpace, verticalSpace, horizontalSpace, verticalSpace);

  setCentralWidget(sidebarSplitter);

  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setCancelButton(nullptr);
  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->reset();

  pluginItemModel->setObjectName("pluginItemModel");

  proxyModel->setObjectName("proxyModel");
  proxyModel->setSourceModel(pluginItemModel);

  groupsEditor->setObjectName("groupsEditor");

  setupViews();

  translateUi();

  disableGameActions();

  QMetaObject::connectSlotsByName(this);
}

void MainWindow::setupMenuBar() {
  // Create actions.
  actionSettings->setObjectName("actionSettings");
  actionSettings->setIcon(IconFactory::getSettingsIcon());

  actionBackupData->setObjectName("actionBackupData");
  actionBackupData->setIcon(IconFactory::getArchiveIcon());

  actionQuit->setObjectName("actionQuit");
  actionQuit->setIcon(IconFactory::getQuitIcon());

  actionViewDocs->setObjectName("actionViewDocs");
  actionViewDocs->setIcon(IconFactory::getViewDocsIcon());
  actionViewDocs->setShortcut(QKeySequence::HelpContents);

  actionOpenLOOTDataFolder->setObjectName("actionOpenLOOTDataFolder");
  actionOpenLOOTDataFolder->setIcon(IconFactory::getOpenLOOTDataFolderIcon());

  actionJoinDiscordServer->setObjectName("actionJoinDiscordServer");
  actionJoinDiscordServer->setIcon(IconFactory::getJoinDiscordServerIcon());

  actionAbout->setObjectName("actionAbout");
  actionAbout->setIcon(IconFactory::getAboutIcon());

  actionOpenGroupsEditor->setObjectName("actionOpenGroupsEditor");
  actionOpenGroupsEditor->setIcon(IconFactory::getOpenGroupsEditorIcon());

  actionSearch->setObjectName("actionSearch");
  actionSearch->setIcon(IconFactory::getSearchIcon());
  actionSearch->setShortcut(QKeySequence::Find);

  actionCopyLoadOrder->setObjectName("actionCopyLoadOrder");
  actionCopyLoadOrder->setIcon(IconFactory::getCopyLoadOrderIcon());

  actionCopyContent->setObjectName("actionCopyContent");
  actionCopyContent->setIcon(IconFactory::getCopyContentIcon());

  actionRefreshContent->setObjectName("actionRefreshContent");
  actionRefreshContent->setIcon(IconFactory::getRefreshIcon());
  actionRefreshContent->setShortcut(QKeySequence::Refresh);

  actionRedatePlugins->setObjectName("actionRedatePlugins");
  actionRedatePlugins->setIcon(IconFactory::getRedateIcon());

  actionFixAmbiguousLoadOrder->setObjectName("actionFixAmbiguousLoadOrder");
  actionFixAmbiguousLoadOrder->setIcon(IconFactory::getFixIcon());

  actionClearAllUserMetadata->setObjectName("actionClearAllUserMetadata");
  actionClearAllUserMetadata->setIcon(IconFactory::getDeleteIcon());

  actionCopyPluginName->setObjectName("actionCopyPluginName");
  actionCopyPluginName->setIcon(IconFactory::getCopyContentIcon());

  actionCopyCardContent->setObjectName("actionCopyCardContent");
  actionCopyCardContent->setIcon(IconFactory::getCopyContentIcon());

  actionCopyMetadata->setObjectName("actionCopyMetadata");
  actionCopyMetadata->setIcon(IconFactory::getCopyMetadataIcon());

  actionEditMetadata->setObjectName("actionEditMetadata");
  actionEditMetadata->setIcon(IconFactory::getEditIcon());
  actionEditMetadata->setShortcut(QString("Ctrl+E"));

  actionClearMetadata->setObjectName("actionClearMetadata");
  actionClearMetadata->setIcon(IconFactory::getDeleteIcon());

  // Create menu bar.
  setMenuBar(menubar);

  menubar->addAction(menuFile->menuAction());
  menubar->addAction(menuGame->menuAction());
  menubar->addAction(menuPlugin->menuAction());
  menubar->addAction(menuHelp->menuAction());
  menuFile->addAction(actionSettings);
  menuFile->addAction(actionBackupData);
  menuFile->addAction(actionOpenLOOTDataFolder);
  menuFile->addSeparator();
  menuFile->addAction(actionQuit);
  menuGame->addAction(actionOpenGroupsEditor);
  menuGame->addSeparator();
  menuGame->addAction(actionSearch);
  menuGame->addAction(actionCopyLoadOrder);
  menuGame->addAction(actionCopyContent);
  menuGame->addAction(actionRefreshContent);
  menuGame->addSeparator();
  menuGame->addAction(actionFixAmbiguousLoadOrder);
  menuGame->addAction(actionRedatePlugins);
  menuGame->addAction(actionClearAllUserMetadata);
  menuPlugin->addAction(actionEditMetadata);
  menuPlugin->addSeparator();
  menuPlugin->addAction(actionCopyPluginName);
  menuPlugin->addAction(actionCopyCardContent);
  menuPlugin->addAction(actionCopyMetadata);
  menuPlugin->addSeparator();
  menuPlugin->addAction(actionClearMetadata);
  menuHelp->addAction(actionViewDocs);
  menuHelp->addAction(actionJoinDiscordServer);
  menuHelp->addSeparator();
  menuHelp->addAction(actionAbout);
}

void MainWindow::setupToolBar() {
  // Create actions.
  actionSort->setObjectName("actionSort");
  actionSort->setIcon(IconFactory::getSortIcon());

  actionUpdateMasterlist->setObjectName("actionUpdateMasterlist");
  actionUpdateMasterlist->setIcon(IconFactory::getUpdateMasterlistIcon());

  actionApplySort->setObjectName("actionApplySort");
  actionApplySort->setIcon(IconFactory::getApplySortIcon());
  actionApplySort->setVisible(false);

  actionDiscardSort->setObjectName("actionDiscardSort");
  actionDiscardSort->setIcon(IconFactory::getDiscardSortIcon());
  actionDiscardSort->setVisible(false);

  // Create toolbar.
  toolBar->setMovable(false);
  toolBar->setFloatable(false);
  toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  addToolBar(Qt::TopToolBarArea, toolBar);

  gameComboBox->setObjectName("gameComboBox");

  toolBar->addWidget(gameComboBox);

  toolBar->addAction(actionSort);
  toolBar->addAction(actionUpdateMasterlist);
  toolBar->addAction(actionApplySort);
  toolBar->addAction(actionDiscardSort);
  toolBar->addAction(actionSearch);
}

void MainWindow::setupViews() {
  sidebarPluginsView->setModel(proxyModel);
  sidebarPluginsView->setDragEnabled(true);
  sidebarPluginsView->setSelectionMode(QAbstractItemView::SingleSelection);
  sidebarPluginsView->setSelectionBehavior(QAbstractItemView::SelectRows);
  sidebarPluginsView->setHorizontalScrollMode(
      QAbstractItemView::ScrollPerPixel);
  sidebarPluginsView->setShowGrid(false);
  sidebarPluginsView->hideRow(0);
  sidebarPluginsView->hideColumn(PluginItemModel::CARDS_COLUMN);

  auto verticalHeader = sidebarPluginsView->verticalHeader();
  verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader->setMaximumSectionSize(SIDEBAR_EDIT_MODE_ROW_HEIGHT);
  verticalHeader->setMinimumSectionSize(SIDEBAR_NORMAL_ROW_HEIGHT);
  verticalHeader->setDefaultSectionSize(SIDEBAR_NORMAL_ROW_HEIGHT);
  verticalHeader->hide();

  auto horizontalHeader = sidebarPluginsView->horizontalHeader();
  horizontalHeader->setDefaultAlignment(Qt::AlignLeft);
  horizontalHeader->setHighlightSections(false);
  horizontalHeader->setSectionResizeMode(
      PluginItemModel::SIDEBAR_POSITION_COLUMN, QHeaderView::Interactive);
  horizontalHeader->setSectionResizeMode(PluginItemModel::SIDEBAR_INDEX_COLUMN,
                                         QHeaderView::Interactive);
  horizontalHeader->setSectionResizeMode(PluginItemModel::SIDEBAR_NAME_COLUMN,
                                         QHeaderView::Stretch);
  horizontalHeader->setSectionResizeMode(PluginItemModel::SIDEBAR_STATE_COLUMN,
                                         QHeaderView::Fixed);

  updateSidebarColumnWidths();

  sidebarPluginsView->setItemDelegateForColumn(
      PluginItemModel::SIDEBAR_NAME_COLUMN,
      new SidebarPluginNameDelegate(sidebarPluginsView));

  // Enable the right-click Plugin context menu.
  sidebarPluginsView->setContextMenuPolicy(Qt::CustomContextMenu);

  pluginCardsView->setModel(proxyModel);
  pluginCardsView->setModelColumn(PluginItemModel::CARDS_COLUMN);
  pluginCardsView->setSelectionMode(QAbstractItemView::NoSelection);
  pluginCardsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  // Resize mode must be set to Adjust or else the items widths can grow but
  // never shrink past their starting size.
  pluginCardsView->setResizeMode(QListView::Adjust);
  // Mouse tracking must be enabled for the "entered" signal to be emitted,
  // so that editors can be opened and closed on demand.
  pluginCardsView->setMouseTracking(true);
  // Word wrap must be enabled as although LOOT uses a custom delegate, with
  // word wrap set to false the delegate's sizeHint() method is always called
  // with the initial size of the view's viewport, not the actual current size,
  // leading to layout issues.
  pluginCardsView->setWordWrap(true);

  pluginCardsView->setItemDelegate(new PluginCardDelegate(pluginCardsView));

  // Enable the right-click Plugin context menu.
  pluginCardsView->setContextMenuPolicy(Qt::CustomContextMenu);

  // Plugin selection handling needs to be set up after the model has been
  // set, as before then there is no selection model.
  const auto selectionModel = sidebarPluginsView->selectionModel();
  connect(selectionModel,
          &QItemSelectionModel::selectionChanged,
          this,
          &MainWindow::on_sidebarPluginsSelectionModel_selectionChanged);

  // Scrolling the cards jumps around too much by default, because the step
  // size is related to the height of list items, which can be quite a lot
  // larger than usual for a list of lines of text. A step size of 24 is the
  // height of two empty cards, which seems reasonable.
  static constexpr int CARD_SCROLL_STEP_SIZE = 24;
  pluginCardsView->verticalScrollBar()->setSingleStep(CARD_SCROLL_STEP_SIZE);
}

void MainWindow::translateUi() {
  setWindowTitle("LOOT");

  // Translate toolbar items.
  actionSort->setText(translate("Sort Plugins"));
  actionUpdateMasterlist->setText(translate("Update Masterlist"));
  actionApplySort->setText(translate("Apply Sorted Load Order"));
  actionDiscardSort->setText(translate("Discard Sorted Load Order"));

  // Translate menu bar items.
  menuFile->setTitle(translate("File"));
  actionSettings->setText(translate("Settings..."));
  actionBackupData->setText(translate("Backup LOOT Data"));
  actionOpenLOOTDataFolder->setText(translate("Open LOOT Data Folder"));
  actionQuit->setText(translate("Quit"));

  menuGame->setTitle(translate("Game"));
  actionOpenGroupsEditor->setText(translate("Edit Groups..."));
  actionSearch->setText(translate("Search Cards..."));
  actionCopyLoadOrder->setText(translate("Copy Load Order"));
  actionCopyContent->setText(translate("Copy Content"));
  actionRefreshContent->setText(translate("Refresh Content"));
  actionRedatePlugins->setText(translate("Redate Plugins..."));
  actionFixAmbiguousLoadOrder->setText(translate("Fix Ambiguous Load Order"));
  actionClearAllUserMetadata->setText(translate("Clear All User Metadata..."));

  menuPlugin->setTitle(translate("Plugin"));
  actionCopyPluginName->setText(translate("Copy Plugin Name"));
  actionCopyCardContent->setText(translate("Copy Card Content"));
  actionCopyMetadata->setText(translate("Copy Metadata"));
  actionEditMetadata->setText(translate("Edit Metadata..."));
  actionClearMetadata->setText(translate("Clear User Metadata..."));

  menuHelp->setTitle(translate("Help"));
  actionViewDocs->setText(translate("View Documentation"));
  actionJoinDiscordServer->setText(translate("Join Discord Server"));
  actionAbout->setText(translate("About"));

  // Translate sidebar.
  toolBox->setItemText(0, translate("Plugins"));
  toolBox->setItemText(1, translate("Filters"));
}

void MainWindow::enableGameActions() {
  menuGame->setEnabled(true);
  actionSort->setEnabled(true);
  actionUpdateMasterlist->setEnabled(true);
  actionSearch->setEnabled(true);

  const auto enableRedatePlugins =
      state.GetCurrentGame().GetSettings().Type() == GameType::tes5 ||
      state.GetCurrentGame().GetSettings().Type() == GameType::tes5se;
  actionRedatePlugins->setEnabled(enableRedatePlugins);

  actionFixAmbiguousLoadOrder->setEnabled(false);
}

void MainWindow::disableGameActions() {
  menuGame->setEnabled(false);
  actionSort->setEnabled(false);
  actionUpdateMasterlist->setEnabled(false);
  actionSearch->setEnabled(false);

  // Also disable plugin actions because they
  // only make sense within the context of a game.
  disablePluginActions();
}

void MainWindow::enablePluginActions() { menuPlugin->setEnabled(true); }

void MainWindow::disablePluginActions() { menuPlugin->setEnabled(false); }

void MainWindow::enterEditingState() {
  actionSettings->setDisabled(true);
  actionOpenGroupsEditor->setDisabled(true);
  actionRefreshContent->setDisabled(true);
  actionClearAllUserMetadata->setDisabled(true);
  actionEditMetadata->setDisabled(true);
  actionClearMetadata->setDisabled(true);
  gameComboBox->setDisabled(true);
  actionUpdateMasterlist->setDisabled(true);
  actionSort->setDisabled(true);

  sidebarPluginsView->verticalHeader()->setDefaultSectionSize(
      SIDEBAR_EDIT_MODE_ROW_HEIGHT);
}

void MainWindow::exitEditingState() {
  actionSettings->setEnabled(true);
  actionOpenGroupsEditor->setEnabled(true);
  actionRefreshContent->setEnabled(true);
  actionClearAllUserMetadata->setEnabled(true);
  actionEditMetadata->setEnabled(true);
  actionClearMetadata->setEnabled(true);
  gameComboBox->setEnabled(true);
  actionUpdateMasterlist->setEnabled(true);
  actionSort->setEnabled(true);

  sidebarPluginsView->verticalHeader()->setDefaultSectionSize(
      SIDEBAR_NORMAL_ROW_HEIGHT);
}

void MainWindow::enterSortingState() {
  actionUpdateMasterlist->setVisible(false);
  actionSort->setVisible(false);

  actionApplySort->setVisible(true);
  actionDiscardSort->setVisible(true);

  actionSettings->setDisabled(true);
  gameComboBox->setDisabled(true);
  actionRefreshContent->setDisabled(true);
  actionCopyLoadOrder->setDisabled(true);
}

void MainWindow::exitSortingState() {
  actionUpdateMasterlist->setVisible(true);
  actionSort->setVisible(true);

  actionApplySort->setVisible(false);
  actionDiscardSort->setVisible(false);

  actionSettings->setDisabled(false);
  gameComboBox->setDisabled(false);
  actionRefreshContent->setDisabled(false);
  actionCopyLoadOrder->setDisabled(false);
}

void MainWindow::loadGame(bool isOnLOOTStartup) {
  auto progressUpdater = new ProgressUpdater();

  // This lambda will run from the worker thread.
  auto sendProgressUpdate = [progressUpdater](std::string message) {
    emit progressUpdater->progressUpdate(QString::fromStdString(message));
  };

  std::unique_ptr<Query> query =
      std::make_unique<GetGameDataQuery>(state.GetCurrentGame(),
                                         state.getSettings().getLanguage(),
                                         sendProgressUpdate);

  const auto handler = isOnLOOTStartup
                           ? &MainWindow::handleStartupGameDataLoaded
                           : &MainWindow::handleRefreshGameDataLoaded;

  executeBackgroundQuery(std::move(query), handler, progressUpdater);
}

void MainWindow::updateCounts(const std::vector<SimpleMessage>& generalMessages,
                              const std::vector<PluginItem>& plugins) {
  const auto counters = GeneralInformationCounters(generalMessages, plugins);
  const auto hiddenMessageCount =
      countHiddenMessages(plugins, filtersWidget->getCardContentFiltersState());
  const auto hiddenPluginCount =
      counters.totalPlugins - static_cast<size_t>(proxyModel->rowCount()) + 1;

  filtersWidget->setMessageCounts(hiddenMessageCount, counters.totalMessages);
  filtersWidget->setPluginCounts(hiddenPluginCount, counters.totalPlugins);
}

void MainWindow::updateGeneralInformation() {
  auto masterlistInfo = getFileRevisionSummary(
      state.GetCurrentGame().MasterlistPath(), FileType::Masterlist);
  auto preludeInfo = getFileRevisionSummary(state.getPreludePath(),
                                            FileType::MasterlistPrelude);

  auto initMessages = state.getInitMessages();
  auto gameMessages = ToSimpleMessages(state.GetCurrentGame().GetMessages(),
                                       state.getSettings().getLanguage());
  initMessages.insert(
      initMessages.end(), gameMessages.begin(), gameMessages.end());

  pluginItemModel->setGeneralInformation(
      state.GetCurrentGame().GetSettings().Type(),
      masterlistInfo,
      preludeInfo,
      initMessages);
}

void MainWindow::updateGeneralMessages() {
  auto initMessages = state.getInitMessages();
  auto gameMessages = ToSimpleMessages(state.GetCurrentGame().GetMessages(),
                                       state.getSettings().getLanguage());
  initMessages.insert(
      initMessages.end(), gameMessages.begin(), gameMessages.end());

  pluginItemModel->setGeneralMessages(std::move(initMessages));
}

void MainWindow::updateSidebarColumnWidths() {
  const auto horizontalHeader = sidebarPluginsView->horizontalHeader();

  const auto positionSectionHeaderWidth = calculateSidebarHeaderWidth(
      *sidebarPluginsView, PluginItemModel::SIDEBAR_POSITION_COLUMN);

  // If a game hasn't loaded yet, assume that the player will have something in
  // the order of hundreds of plugins enabled - it's the number of digits that
  // matters, not the number itself.
  static constexpr size_t DEFAULT_LOAD_ORDER_SIZE_ESTIMATE = 255;

  const auto positionSectionWidth =
      state.HasCurrentGame() && state.GetCurrentGame().IsInitialised()
          ? calculateSidebarPositionSectionWidth(
                state.GetCurrentGame().GetPlugins().size())
          : calculateSidebarPositionSectionWidth(
                DEFAULT_LOAD_ORDER_SIZE_ESTIMATE);

  const auto indexSectionHeaderWidth = calculateSidebarHeaderWidth(
      *sidebarPluginsView, PluginItemModel::SIDEBAR_INDEX_COLUMN);

  // If there is no current game set (i.e. on initial construction), use TES5 SE
  // to calculate the load order section width because that's one of the games
  // that uses the wider width.
  const auto indexSectionWidth =
      state.HasCurrentGame()
          ? calculateSidebarIndexSectionWidth(
                state.GetCurrentGame().GetSettings().Type())
          : calculateSidebarIndexSectionWidth(GameType::tes5se);

  const auto stateSectionWidth =
      QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize) +
      QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin);

  const auto minimumSectionWidth = std::min({positionSectionHeaderWidth,
                                             positionSectionWidth,
                                             indexSectionHeaderWidth,
                                             indexSectionWidth,
                                             stateSectionWidth});

  horizontalHeader->setMinimumSectionSize(minimumSectionWidth);
  horizontalHeader->resizeSection(
      PluginItemModel::SIDEBAR_POSITION_COLUMN,
      std::max({positionSectionHeaderWidth, positionSectionWidth}));
  horizontalHeader->resizeSection(
      PluginItemModel::SIDEBAR_INDEX_COLUMN,
      std::max({indexSectionHeaderWidth, indexSectionWidth}));
  horizontalHeader->resizeSection(PluginItemModel::SIDEBAR_STATE_COLUMN,
                                  stateSectionWidth);
}

void MainWindow::setFiltersState(PluginFiltersState&& filtersState) {
  proxyModel->setFiltersState(std::move(filtersState));

  updateCounts(pluginItemModel->getGeneralMessages(),
               pluginItemModel->getPluginItems());
  searchDialog->reset();
  proxyModel->clearSearchResults();
}

void MainWindow::setFiltersState(
    PluginFiltersState&& filtersState,
    std::vector<std::string>&& conflictingPluginNames) {
  proxyModel->setFiltersState(std::move(filtersState),
                              std::move(conflictingPluginNames));

  updateCounts(pluginItemModel->getGeneralMessages(),
               pluginItemModel->getPluginItems());
  searchDialog->reset();
  proxyModel->clearSearchResults();
}

bool MainWindow::hasErrorMessages() const {
  const auto counters = GeneralInformationCounters(
      pluginItemModel->getGeneralMessages(), pluginItemModel->getPluginItems());

  return counters.errors != 0;
}

void MainWindow::sortPlugins(bool isAutoSort) {
  std::vector<Task*> tasks;

  if (state.getSettings().isMasterlistUpdateBeforeSortEnabled()) {
    handleProgressUpdate(translate("Updating and parsing masterlist..."));

    auto task = new UpdateMasterlistTask(state);

    connect(task, &Task::finished, this, &MainWindow::handleMasterlistUpdated);
    connect(task, &Task::error, this, &MainWindow::handleError);

    tasks.push_back(task);
  }

  auto progressUpdater = new ProgressUpdater();

  // This lambda will run from the worker thread.
  auto sendProgressUpdate = [progressUpdater](std::string message) {
    emit progressUpdater->progressUpdate(QString::fromStdString(message));
  };

  std::unique_ptr<Query> sortPluginsQuery =
      std::make_unique<SortPluginsQuery>(state.GetCurrentGame(),
                                         state,
                                         state.getSettings().getLanguage(),
                                         sendProgressUpdate);

  auto sortTask = new QueryTask(std::move(sortPluginsQuery));

  const auto sortHandler = isAutoSort ? &MainWindow::handlePluginsAutoSorted
                                      : &MainWindow::handlePluginsManualSorted;

  connect(sortTask, &Task::finished, this, sortHandler);
  connect(sortTask, &Task::error, this, &MainWindow::handleError);

  tasks.push_back(sortTask);

  executeBackgroundTasks(tasks, progressUpdater);
}

void MainWindow::showFirstRunDialog() {
  auto zipPath = createBackup();

  std::string textTemplate = R"(
<p>%1%</p>
<p>%2%</p>
<ul>
  <li>%3%</li>
  <li>%4%</li>
  <li>%5%</li>
</ul>
<p>%6%</p>
)";

  std::string paragraph1;
  if (zipPath.has_value()) {
    auto zipPathString = zipPath.value().u8string();
    auto link = "<pre><a href=\"file:" + zipPathString +
                "\" style=\"white-space: nowrap\">" + zipPathString +
                "</a></pre>";

    paragraph1 =
        (boost::format(boost::locale::translate(
             "This appears to be the first time you have run LOOT v%1%. Your "
             "current LOOT data has been backed up to: %2%")) %
         gui::Version::string() % link)
            .str();
  } else {
    paragraph1 =
        (boost::format(boost::locale::translate(
             "This appears to be the first time you have run LOOT v%1%.")) %
         gui::Version::string())
            .str();
  }

  auto paragraph2 = boost::locale::translate(
      "Here are some tips to help you get started with the interface.");

  auto listItem1 = boost::locale::translate(
      "CRCs are only displayed after plugins have been loaded, either by "
      "conflict filtering, or by sorting.");

  auto listItem2 = boost::locale::translate(
      "Plugins can be drag and dropped from the sidebar into the metadata "
      "editor's \"load after\", \"requirements\" and \"incompatibility\" "
      "tables.");

  auto listItem3 = boost::locale::translate(
      "Some features are disabled while the metadata editor is open, or while "
      "there is a sorted load order that has not been applied or discarded.");

  auto paragraph3 =
      (boost::format(boost::locale::translate(
           "LOOT is free, but if you want to show your appreciation with some "
           "money, donations may be made to WrinklyNinja (LOOT's creator and "
           "main developer) using %s.")) %
       "<a href=\"https://www.paypal.me/OliverHamlet\">PayPal</a>")
          .str();

  std::string text = (boost::format(textTemplate) % paragraph1 % paragraph2 %
                      listItem1 % listItem2 % listItem3 % paragraph3)
                         .str();

  auto messageBox = QMessageBox(QMessageBox::NoIcon,
                                translate("First-Time Tips"),
                                QString::fromStdString(text),
                                QMessageBox::Ok,
                                this);
  messageBox.exec();
}

void MainWindow::showNotification(const QString& message) {
  static constexpr int NOTIFICATION_LIFETIME_MS = 5000;
  statusBar()->showMessage(message, NOTIFICATION_LIFETIME_MS);
}

QModelIndex MainWindow::getSelectedPluginIndex() const {
  auto selectedPluginIndices =
      sidebarPluginsView->selectionModel()->selectedIndexes();
  if (selectedPluginIndices.isEmpty()) {
    throw std::runtime_error(
        "Cannot copy plugin metadata when no plugin is selected");
  }

  return selectedPluginIndices.first();
}

PluginItem MainWindow::getSelectedPlugin() const {
  auto indexData = getSelectedPluginIndex().data(RawDataRole);
  if (!indexData.canConvert<PluginItem>()) {
    throw std::runtime_error("Cannot convert data to PluginItem");
  }

  return indexData.value<PluginItem>();
}

void MainWindow::closeEvent(QCloseEvent* event) {
  if (state.HasUnappliedChanges()) {
    auto changeType = pluginEditorWidget->isVisible()
                          ? boost::locale::translate("metadata edits")
                          : boost::locale::translate("sorted load order");
    auto questionText = (boost::format(boost::locale::translate(
                             "You have not yet applied or cancelled your %s. "
                             "Are you sure you want to quit?")) %
                         changeType)
                            .str();

    auto button = QMessageBox::question(
        this,
        "LOOT",
        QString::fromStdString(questionText),
        QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
        QMessageBox::StandardButton::No);

    if (button != QMessageBox::StandardButton::Yes) {
      event->ignore();
      return;
    }
  }

  try {
    LootSettings::WindowPosition position;

    // Use the normal geometry because that gives the size and position as if
    // the window is neither maximised or minimised. This is useful because if
    // LOOT is opened into a maximised state, unmaximising it should restore
    // the previous unmaximised size and position, and that can't be done
    // without recording the normal geometry.
    const auto geometry = normalGeometry();
    position.left = geometry.x();
    position.top = geometry.y();
    position.right = position.left + geometry.width();
    position.bottom = position.top + geometry.height();
    position.maximised = isMaximized();

    state.getSettings().storeWindowPosition(position);
  } catch (const std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to record window position: {}", e.what());
    }
  }

  try {
    state.getSettings().storeFilters(filtersWidget->getFilterSettings());
  } catch (const std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to record filter states: {}", e.what());
    }
  }

  try {
    state.getSettings().storeLastGame(
        state.GetCurrentGame().GetSettings().FolderName());
  } catch (const std::runtime_error& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Couldn't set last game: {}", e.what());
    }
  }

  try {
    state.getSettings().updateLastVersion();
    state.getSettings().save(state.getSettingsPath());
  } catch (const std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to save LOOT's settings. Error: {}", e.what());
    }
  }

  event->accept();
}

void MainWindow::executeBackgroundQuery(
    std::unique_ptr<Query> query,
    void (MainWindow::*onComplete)(QueryResult),
    ProgressUpdater* progressUpdater) {
  auto task = new QueryTask(std::move(query));

  connect(task, &Task::finished, this, onComplete);
  connect(task, &Task::error, this, &MainWindow::handleError);

  executeBackgroundTasks({task}, progressUpdater);
}

void MainWindow::executeBackgroundTasks(
    std::vector<Task*> tasks,
    const ProgressUpdater* progressUpdater) {
  auto executor = new TaskExecutor(this, tasks);

  if (progressUpdater != nullptr) {
    connect(progressUpdater,
            &ProgressUpdater::progressUpdate,
            this,
            &MainWindow::handleProgressUpdate);

    connect(executor,
            &TaskExecutor::finished,
            progressUpdater,
            &QObject::deleteLater);
  }

  connect(executor, &TaskExecutor::finished, executor, &QObject::deleteLater);

  // Reset (i.e. close) the progress dialog once the thread has finished in case
  // it was used while running these queries. This can't be done from any one
  // handler because none of them know if they are the last to run.
  connect(executor,
          &TaskExecutor::finished,
          this,
          &MainWindow::handleWorkerThreadFinished);

  executor->start();
}

void MainWindow::handleError(const std::string& message) {
  progressDialog->reset();

  QMessageBox::critical(
      this, translate("Error"), QString::fromStdString(message));
}

void MainWindow::handleException(const std::exception& exception) {
  auto logger = getLogger();
  if (logger) {
    logger->error("Caught an exception: {}", exception.what());
  }

  auto message = boost::locale::translate(
                     "Oh no, something went wrong! You can check your "
                     "LOOTDebugLog.txt (you can get to it through the "
                     "main menu) for more information.")
                     .str();

  handleError(message);
}

void MainWindow::handleQueryException(const Query& query,
                                      const std::exception& exception) {
  auto logger = getLogger();
  if (logger) {
    logger->error("Caught an exception: {}", exception.what());
  }

  handleError(query.getErrorMessage());
}

void MainWindow::handleGameDataLoaded(QueryResult result) {
  progressDialog->reset();

  pluginItemModel->setPluginItems(std::move(std::get<PluginItems>(result)));

  updateGeneralInformation();

  filtersWidget->setGroups(GetGroupNames(state.GetCurrentGame()));

  pluginEditorWidget->setBashTagCompletions(
      state.GetCurrentGame().GetKnownBashTags());

  enableGameActions();
}

bool MainWindow::handlePluginsSorted(QueryResult result) {
  filtersWidget->resetConflictsAndGroupsFilters();

  auto sortedPlugins = std::get<PluginItems>(result);

  if (sortedPlugins.empty()) {
    // If there was a sorting failure the array of plugins will be empty.
    // If sorting fails because of a cyclic interaction or a nonexistent
    // group, the last general message will detail that error.
    QMessageBox::critical(
        this,
        "LOOT",
        translate("Failed to sort plugins. Details may be provided in the "
                  "General Information section."));

    // Plugins didn't change but general messages may have.
    updateGeneralMessages();
    return false;
  }

  auto currentLoadOrder = state.GetCurrentGame().GetLoadOrder();
  const auto loadOrderHasChanged =
      hasLoadOrderChanged(currentLoadOrder, sortedPlugins);

  if (loadOrderHasChanged) {
    enterSortingState();
  } else {
    state.DecrementUnappliedChangeCounter();

    const auto message =
        translate("Sorting made no changes to the load order.");

    if (state.getSettings().isNoSortingChangesDialogEnabled()) {
      QMessageBox::information(this, "LOOT", message);
    } else {
      showNotification(message);
    }
  }

  handleGameDataLoaded(sortedPlugins);

  return loadOrderHasChanged;
}

QMenu* MainWindow::createPopupMenu() {
  QMenu* filteredMenu = QMainWindow::createPopupMenu();
  // Don't allow the toolbar to be hidden.
  filteredMenu->removeAction(toolBar->toggleViewAction());
  return filteredMenu;
}

std::optional<std::filesystem::path> MainWindow::createBackup() {
  auto backupBasename =
      "LOOT-backup-" +
      QDateTime::currentDateTime().toString("yyyyMMddThhmmss").toStdString();

  auto sourceDir = state.getLootDataPath();
  auto destDir = state.getLootDataPath() / "backups" / backupBasename;

  loot::createBackup(sourceDir, destDir);

  if (!std::filesystem::exists(destDir)) {
    return std::nullopt;
  }

  auto zipPath = compressDirectory(destDir);

  std::filesystem::remove_all(destDir);

  return zipPath;
}

void MainWindow::checkForAmbiguousLoadOrder() {
  if (!state.GetCurrentGame().IsLoadOrderAmbiguous()) {
    actionFixAmbiguousLoadOrder->setEnabled(false);
    return;
  }

  QMessageBox::warning(
      this,
      translate("Ambiguous load order detected"),
      translate("LOOT has detected that the current load order is ambiguous, "
                "which means that other applications might use a different "
                "load order than what LOOT displays.\n\n"
                "You can use the \"Fix Ambiguous Load Order\" action in the "
                "Game menu to unambiguously set the load order to what is "
                "displayed by LOOT."));

  actionFixAmbiguousLoadOrder->setEnabled(true);
}

void MainWindow::on_actionSettings_triggered() {
  try {
    auto currentGameFolder =
        state.HasCurrentGame()
            ? std::optional(state.GetCurrentGame().GetSettings().FolderName())
            : std::nullopt;

    settingsDialog->initialiseInputs(
        state.getSettings(), themes, currentGameFolder);
    settingsDialog->show();

    // Adjust size because otherwise the size is slightly too small the first
    // time the dialog is opened.
    settingsDialog->adjustSize();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionBackupData_triggered() {
  try {
    auto zipPath = createBackup();

    if (zipPath.has_value()) {
      auto zipPathString = zipPath.value().u8string();
      auto link = "<pre><a href=\"file:" + zipPathString +
                  "\" style=\"white-space: nowrap\">" + zipPathString +
                  "</a></pre>";
      auto message = (boost::format(boost::locale::translate(
                          "Your LOOT data has been backed up to: %1%")) %
                      link)
                         .str();

      QMessageBox::information(this, "LOOT", QString::fromStdString(message));
    } else {
      auto message = translate(
          "No backup has been created as LOOT has no data to backup.");

      QMessageBox::information(this, "LOOT", message);
    }
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionQuit_triggered() { this->close(); }

void MainWindow::on_actionOpenGroupsEditor_triggered() {
  try {
    std::set<std::string> installedPluginGroups;
    for (const auto& plugin : pluginItemModel->getPluginItems()) {
      if (plugin.group.has_value()) {
        installedPluginGroups.insert(plugin.group.value());
      }
    }

    groupsEditor->setGroups(state.GetCurrentGame().GetMasterlistGroups(),
                            state.GetCurrentGame().GetUserGroups(),
                            installedPluginGroups);

    groupsEditor->show();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionSearch_triggered() { searchDialog->show(); }

void MainWindow::on_actionCopyLoadOrder_triggered() {
  try {
    auto plugins = state.GetCurrentGame().GetPluginsInLoadOrder();
    std::vector<std::string> pluginNames;
    pluginNames.reserve(plugins.size());

    for (const auto& plugin : plugins) {
      pluginNames.push_back(plugin->GetName());
    }

    CopyLoadOrderQuery query(state.GetCurrentGame(), pluginNames);

    query.executeLogic();

    showNotification(
        translate("The load order has been copied to the clipboard."));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionCopyContent_triggered() {
  try {
    auto content =
        pluginItemModel->getGeneralInfo().getMarkdownContent() + "\n\n";

    for (const auto& plugin : pluginItemModel->getPluginItems()) {
      content += plugin.getMarkdownContent() + "\n\n";
    }

    CopyToClipboard(content);

    showNotification(
        translate("LOOT's content has been copied to the clipboard."));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionFixAmbiguousLoadOrder_triggered() {
  try {
    auto loadOrder = state.GetCurrentGame().GetLoadOrder();
    state.GetCurrentGame().SetLoadOrder(loadOrder);

    showNotification(
        translate("The load order displayed by LOOT has been set."));

    actionFixAmbiguousLoadOrder->setEnabled(false);
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionRefreshContent_triggered() {
  try {
    loadGame(false);
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionRedatePlugins_triggered() {
  try {
    auto button = QMessageBox::question(
        this,
        /* translators: Title of a dialog box. */
        translate("Redate Plugins?"),
        translate(
            "This feature is provided so that modders using the Creation Kit "
            "may "
            "set the load order it uses. A side-effect is that any subscribed "
            "Steam Workshop mods will be re-downloaded by Steam (this does not "
            "affect Skyrim Special Edition). Do you wish to continue?"),
        QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
        QMessageBox::StandardButton::No);

    if (button == QMessageBox::StandardButton::Yes) {
      state.GetCurrentGame().RedatePlugins();
      showNotification(
          /* translators: Notification text. */
          translate("Plugins were successfully redated."));
    }
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionClearAllUserMetadata_triggered() {
  try {
    auto button = QMessageBox::question(
        this,
        "LOOT",
        translate("Are you sure you want to clear all existing user-added "
                  "metadata from all plugins?"),
        QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
        QMessageBox::StandardButton::No);

    if (button != QMessageBox::StandardButton::Yes) {
      return;
    }

    ClearAllMetadataQuery query(state.GetCurrentGame(),
                                state.getSettings().getLanguage());

    auto result = query.executeLogic();

    // Clearing all user metadata can clear general messages (though
    // user-defined general messages aren't editable through the LOOT GUI),
    // change the known Bash Tags (though again they aren't editable in the GUI)
    // and change plugin metadata that may be displayed in the sidebar or on
    // cards. However, only plugins that had user metadata are affected, which
    // is probably a small fraction of the total number, so doing a full refresh
    // of the game-related UI would be overkill.

    filtersWidget->setGroups(GetGroupNames(state.GetCurrentGame()));

    pluginEditorWidget->setBashTagCompletions(
        state.GetCurrentGame().GetKnownBashTags());

    updateGeneralMessages();

    // These plugin items are only those that had their user metadata removed.
    auto pluginItems = std::get<PluginItems>(result);

    // For each item, find its existing index in the model and update its data.
    // The sidebar item and card will be updated by handling the resulting
    // dataChanged signal.
    auto nameToRowMap = pluginItemModel->getPluginNameToRowMap();
    for (const auto& item : pluginItems) {
      const auto it = nameToRowMap.find(item.name);
      if (it == nameToRowMap.end()) {
        throw std::runtime_error(std::string("Could not find plugin named \"") +
                                 item.name + "\" in the plugin item model.");
      }

      // It doesn't matter which index column is used, it's the same data.
      const auto index = pluginItemModel->index(it->second, 0);
      pluginItemModel->setData(index, QVariant::fromValue(item), RawDataRole);
    }

    showNotification(translate("All user-added metadata has been cleared."));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionEditMetadata_triggered() {
  try {
    if (pluginEditorWidget->isVisible()) {
      QMessageBox::warning(
          this,
          "LOOT",
          translate(
              "The plugin metadata editor is already open, first close the "
              "editor before attempting to edit another plugin's metadata."));
      return;
    }

    const auto selectedPluginName = getSelectedPlugin().name;
    const auto groups = GetGroupNames(state.GetCurrentGame());

    pluginEditorWidget->initialiseInputs(
        groups,
        selectedPluginName,
        state.GetCurrentGame().GetNonUserMetadata(
            *state.GetCurrentGame().GetPlugin(selectedPluginName)),
        state.GetCurrentGame().GetUserMetadata(selectedPluginName));

    pluginEditorWidget->show();

    state.IncrementUnappliedChangeCounter();

    // Refresh the sidebar items so that all their groups are displayed.
    pluginItemModel->setEditorPluginName(selectedPluginName);

    enterEditingState();

    // Scroll the sidebar and cards lists to the plugin being edited.
    const auto sidebarIndex = getSelectedPluginIndex();
    const auto cardIndex =
        sidebarIndex.siblingAtColumn(PluginItemModel::CARDS_COLUMN);

    // Use a timeout of 1 ms so that scrolling is done after the widget is
    // actually opened by Qt's event loop. Use 1 instead of 0 because the
    // ordering between zero timers and other event sources is undefined.
    QTimer::singleShot(1, [=]() {
      try {
        sidebarPluginsView->scrollTo(sidebarIndex,
                                     QAbstractItemView::PositionAtTop);
        pluginCardsView->scrollTo(cardIndex, QAbstractItemView::PositionAtTop);
      } catch (const std::exception& e) {
        handleException(e);
      }
    });
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionCopyMetadata_triggered() {
  try {
    auto selectedPluginName = getSelectedPlugin().name;

    CopyMetadataQuery query(state.GetCurrentGame(),
                            state.getSettings().getLanguage(),
                            selectedPluginName);

    query.executeLogic();

    auto text =
        (boost::format(boost::locale::translate(
             "The metadata for \"%s\" has been copied to the clipboard.")) %
         selectedPluginName)
            .str();

    showNotification(QString::fromStdString(text));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionCopyPluginName_triggered() {
  try {
    const auto selectedPluginName = getSelectedPlugin().name;

    CopyToClipboard(selectedPluginName);

    const auto text =
        (boost::format(boost::locale::translate(
             "The plugin name \"%s\" has been copied to the clipboard.")) %
         selectedPluginName)
            .str();

    showNotification(QString::fromStdString(text));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionCopyCardContent_triggered() {
  try {
    auto selectedPlugin = getSelectedPlugin();
    auto content = selectedPlugin.getMarkdownContent();

    CopyToClipboard(content);

    auto text =
        (boost::format(boost::locale::translate(
             "The card content for \"%s\" has been copied to the clipboard.")) %
         selectedPlugin.name)
            .str();

    showNotification(QString::fromStdString(text));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionClearMetadata_triggered() {
  try {
    auto selectedPluginName = getSelectedPlugin().name;

    auto questionText = (boost::format(boost::locale::translate(
                             "Are you sure you want to clear all existing "
                             "user-added metadata from \"%s\"?")) %
                         selectedPluginName)
                            .str();

    auto button = QMessageBox::question(
        this,
        "LOOT",
        QString::fromStdString(questionText),
        QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
        QMessageBox::StandardButton::No);

    if (button != QMessageBox::StandardButton::Yes) {
      return;
    }

    ClearPluginMetadataQuery query(state.GetCurrentGame(),
                                   state.getSettings().getLanguage(),
                                   selectedPluginName);

    auto result = query.executeLogic();

    // The result is the changed plugin's derived metadata. Update the
    // model's data and also the message counts.

    auto newPluginItem = std::get<PluginItem>(result);

    for (int i = 1; i < pluginItemModel->rowCount(); i += 1) {
      const auto index = pluginItemModel->index(i, 0);
      auto pluginItem = index.data(RawDataRole).value<PluginItem>();

      if (pluginItem.name == selectedPluginName) {
        pluginItemModel->setData(
            index, QVariant::fromValue(newPluginItem), RawDataRole);
        break;
      }
    }

    auto notificationText =
        (boost::format(boost::locale::translate(
             "The user-added metadata for \"%s\" has been cleared.")) %
         selectedPluginName)
            .str();

    showNotification(QString::fromStdString(notificationText));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionViewDocs_triggered() {
  try {
    OpenReadmeQuery query(state.getReadmePath(), "index.html");

    query.executeLogic();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionOpenLOOTDataFolder_triggered() {
  try {
    OpenLogLocationQuery query(state.getLogPath());

    query.executeLogic();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionJoinDiscordServer_triggered() {
  QDesktopServices::openUrl(QUrl("https://loot.github.io/discord/"));
}

void MainWindow::on_actionAbout_triggered() {
  try {
    std::string textTemplate = R"(
<p>%1%</p>
<p>%2%</p>
<p><a href="https://loot.github.io">https://loot.github.io</a></p>
<p>%3%</p>
<blockquote>
  LOOT is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

  LOOT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with LOOT. If not, see &lt;https://www.gnu.org/licenses/&gt;.
</blockquote>
)";

    auto paragraph1 =
        (boost::format(boost::locale::translate("Version %s (build %s)")) %
         gui::Version::string() % gui::Version::revision)
            .str();

    auto paragraph2 = boost::locale::translate(
        "Load order optimisation for Morrowind, Oblivion, Nehrim, Skyrim, "
        "Enderal, Skyrim Special Edition, Enderal Special Edition, Skyrim VR, "
        "Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.");

    auto paragraph4 =
        (boost::format(boost::locale::translate(
             "LOOT is free, but if you want to show your appreciation with "
             "some money, donations may be made to WrinklyNinja (LOOT's "
             "creator and main developer) using %s.")) %
         "<a href=\"https://www.paypal.me/OliverHamlet\">PayPal</a>")
            .str();

    std::string text =
        (boost::format(textTemplate) % paragraph1 % paragraph2 % paragraph4)
            .str();

    QMessageBox::about(
        this, translate("About LOOT"), QString::fromStdString(text));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_gameComboBox_activated(int index) {
  try {
    if (index < 0) {
      return;
    }

    auto folderName = gameComboBox->currentData().toString().toStdString();
    if (folderName.empty() ||
        (state.HasCurrentGame() &&
         folderName == state.GetCurrentGame().GetSettings().FolderName())) {
      return;
    }

    auto progressUpdater = new ProgressUpdater();

    // This lambda will run from the worker thread.
    auto sendProgressUpdate = [progressUpdater](std::string message) {
      emit progressUpdater->progressUpdate(QString::fromStdString(message));
    };

    std::unique_ptr<Query> query =
        std::make_unique<ChangeGameQuery>(state,
                                          state.getSettings().getLanguage(),
                                          folderName,
                                          sendProgressUpdate);

    executeBackgroundQuery(
        std::move(query), &MainWindow::handleGameChanged, progressUpdater);
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionSort_triggered() {
  try {
    sortPlugins(false);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionApplySort_triggered() {
  try {
    auto sortedPluginNames = pluginItemModel->getPluginNames();

    auto query =
        ApplySortQuery<>(state.GetCurrentGame(), state, sortedPluginNames);

    try {
      query.executeLogic();

      exitSortingState();
    } catch (const std::exception& e) {
      handleQueryException(query, e);
    }
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionDiscardSort_triggered() {
  try {
    auto query = CancelSortQuery(state.GetCurrentGame(), state);

    auto result = query.executeLogic();

    auto pluginItems = pluginItemModel->getPluginItems();

    std::vector<PluginItem> newPluginItems;
    newPluginItems.reserve(pluginItems.size());
    for (const auto& pluginPair : std::get<CancelSortResult>(result)) {
      auto pluginName = pluginPair.first;

      auto it = std::find_if(pluginItems.cbegin(),
                             pluginItems.cend(),
                             [&](const auto& pluginItem) {
                               return pluginItem.name == pluginName;
                             });

      if (it != pluginItems.end()) {
        auto newPluginItem = *it;
        newPluginItem.loadOrderIndex = pluginPair.second;
        newPluginItems.push_back(newPluginItem);
      }
    }

    pluginItemModel->setPluginItems(std::move(newPluginItems));

    updateGeneralMessages();

    exitSortingState();

    // Perform ambiguous load order check because load order state was refreshed
    // at start of sorting.
    checkForAmbiguousLoadOrder();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionUpdateMasterlist_triggered() {
  try {
    handleProgressUpdate(translate("Updating and parsing masterlist..."));

    auto task = new UpdateMasterlistTask(state);

    connect(task, &Task::finished, this, &MainWindow::handleMasterlistUpdated);
    connect(task, &Task::error, this, &MainWindow::handleError);

    executeBackgroundTasks({task}, nullptr);
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_sidebarPluginsView_doubleClicked(const QModelIndex& index) {
  const auto cardIndex = index.siblingAtColumn(PluginItemModel::CARDS_COLUMN);
  pluginCardsView->scrollTo(cardIndex, QAbstractItemView::PositionAtTop);
}

void MainWindow::on_sidebarPluginsView_customContextMenuRequested(
    const QPoint& position) {
  const auto itemIndex = sidebarPluginsView->indexAt(position);

  if (!itemIndex.isValid() || itemIndex.row() == 0) {
    return;
  }

  sidebarPluginsView->selectRow(itemIndex.row());

  // For some reason mapToGlobal() doesn't include the height of the table's
  // horizontal header, so add that so that the menu gets displayed in the
  // correct position.
  const auto headerHeight = sidebarPluginsView->horizontalHeader()->height();

  auto globalPos = sidebarPluginsView->mapToGlobal(position);
  globalPos.setY(globalPos.y() + headerHeight);

  menuPlugin->exec(globalPos);
}

void MainWindow::on_sidebarPluginsSelectionModel_selectionChanged(
    const QItemSelection& selected) {
  if (selected.isEmpty()) {
    disablePluginActions();
  } else {
    enablePluginActions();
  }
}

void MainWindow::on_pluginCardsView_entered(const QModelIndex& index) {
  if (lastEnteredCardIndex.has_value() &&
      lastEnteredCardIndex.value().isValid() &&
      lastEnteredCardIndex.value() != index) {
    pluginCardsView->closePersistentEditor(lastEnteredCardIndex.value());
  }

  if (!pluginCardsView->isPersistentEditorOpen(index)) {
    pluginCardsView->openPersistentEditor(index);
  }

  lastEnteredCardIndex = QPersistentModelIndex(index);
}

void MainWindow::on_pluginCardsView_pressed(const QModelIndex& index) {
  // Open the persistent editor on mouse button press too because if a
  // card is entered while a modal (e.g. a context menu) is open, the
  // entered signal will not fire, but a click to dismiss the modal will
  // fire.
  on_pluginCardsView_entered(index);
}

void MainWindow::on_pluginCardsView_customContextMenuRequested(
    const QPoint& position) {
  const auto itemIndex = pluginCardsView->indexAt(position);

  if (!itemIndex.isValid() || itemIndex.row() == 0) {
    return;
  }

  sidebarPluginsView->selectRow(itemIndex.row());

  const auto globalPos = pluginCardsView->mapToGlobal(position);

  menuPlugin->exec(globalPos);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void MainWindow::on_pluginItemModel_dataChanged(const QModelIndex& topLeft,
                                                const QModelIndex& bottomRight,
                                                const QList<int>& roles) {
#else
void MainWindow::on_pluginItemModel_dataChanged(const QModelIndex& topLeft,
                                                const QModelIndex& bottomRight,
                                                const QVector<int>& roles) {
#endif
  if (!topLeft.isValid() || !bottomRight.isValid()) {
    return;
  }

  if (roles.isEmpty() || roles.contains(CardContentFiltersRole)) {
    proxyModel->invalidate();
  }

  if (roles.isEmpty() || roles.contains(RawDataRole) ||
      roles.contains(CardContentFiltersRole)) {
    updateCounts(pluginItemModel->getGeneralMessages(),
                 pluginItemModel->getPluginItems());

    searchDialog->reset();
    proxyModel->clearSearchResults();
  }

  if (roles.isEmpty() || roles.contains(RawDataRole)) {
    // Also update the plugin name lists in the filters sidebar panel and the
    // metadata editor's autocompletions in case raw data changed because the
    // game was changed or content was refreshed.
    auto pluginNames = pluginItemModel->getPluginNames();

    filtersWidget->setPlugins(pluginNames);
    pluginEditorWidget->setFilenameCompletions(pluginNames);
  }
}

void MainWindow::on_pluginEditorWidget_accepted(PluginMetadata userMetadata) {
  try {
    auto logger = getLogger();
    auto pluginName = userMetadata.GetName();

    // Erase any existing userlist entry.
    if (logger) {
      logger->trace("Erasing the existing userlist entry.");
    }
    state.GetCurrentGame().ClearUserMetadata(pluginName);

    // Add a new userlist entry if necessary.
    if (!userMetadata.HasNameOnly()) {
      if (logger) {
        logger->trace("Adding new metadata to new userlist entry.");
      }
      state.GetCurrentGame().AddUserMetadata(userMetadata);
    }

    // Save edited userlist.
    state.GetCurrentGame().SaveUserMetadata();

    pluginItemModel->setEditorPluginName(std::nullopt);

    for (int i = 1; i < pluginItemModel->rowCount(); i += 1) {
      const auto index = pluginItemModel->index(i, 0);
      auto pluginItem = index.data(RawDataRole).value<PluginItem>();

      if (pluginItem.name == pluginName) {
        auto plugin = PluginItem(*state.GetCurrentGame().GetPlugin(pluginName),
                                 state.GetCurrentGame(),
                                 state.getSettings().getLanguage());

        auto indexData = QVariant::fromValue(plugin);
        pluginItemModel->setData(index, indexData, RawDataRole);
        break;
      }
    }

    state.DecrementUnappliedChangeCounter();

    exitEditingState();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_pluginEditorWidget_rejected() {
  state.DecrementUnappliedChangeCounter();

  pluginItemModel->setEditorPluginName(std::nullopt);

  exitEditingState();
}

void MainWindow::on_filtersWidget_pluginFilterChanged(
    PluginFiltersState filtersState) {
  setFiltersState(std::move(filtersState));
}

void MainWindow::on_filtersWidget_conflictsFilterChanged(
    std::optional<std::string> targetPluginName) {
  try {
    if (!targetPluginName.has_value()) {
      setFiltersState(filtersWidget->getPluginFiltersState(), {});
      return;
    }

    if (!state.HasCurrentGame()) {
      return;
    }

    handleProgressUpdate(translate("Identifying conflicting plugins..."));

    std::unique_ptr<Query> query = std::make_unique<GetConflictingPluginsQuery>(
        state.GetCurrentGame(),
        state.getSettings().getLanguage(),
        targetPluginName.value());

    executeBackgroundQuery(
        std::move(query), &MainWindow::handleConflictsChecked, nullptr);
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_filtersWidget_cardContentFilterChanged(
    CardContentFiltersState filtersState) {
  pluginItemModel->setCardContentFiltersState(std::move(filtersState));
}

void MainWindow::on_settingsDialog_accepted() {
  try {
    settingsDialog->recordInputValues(state);

    state.getSettings().save(state.getSettingsPath());
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_groupsEditor_accepted() {
  try {
    state.GetCurrentGame().SetUserGroups(groupsEditor->getUserGroups());
    state.GetCurrentGame().SaveUserMetadata();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_searchDialog_finished() { searchDialog->reset(); }

void MainWindow::on_searchDialog_textChanged(const QVariant& text) {
  const auto isEmpty =
      (text.userType() == QMetaType::QString && text.toString().isEmpty()) ||
      (text.userType() == QMetaType::QRegularExpression &&
       text.toRegularExpression().pattern().isEmpty());

  if (isEmpty) {
    proxyModel->clearSearchResults();
    return;
  }

  const auto flags = text.userType() == QMetaType::QRegularExpression
                         ? Qt::MatchRegularExpression | Qt::MatchWrap
                         : Qt::MatchContains | Qt::MatchWrap;

  auto results =
      proxyModel->match(proxyModel->index(0, PluginItemModel::CARDS_COLUMN),
                        ContentSearchRole,
                        text,
                        -1,
                        flags);

  proxyModel->setSearchResults(results);
  searchDialog->setSearchResults(results.size());
}

void MainWindow::on_searchDialog_currentResultChanged(size_t resultIndex) {
  const auto sourceIndex = pluginItemModel->setCurrentSearchResult(resultIndex);
  const auto proxyIndex = proxyModel->mapFromSource(sourceIndex);

  pluginCardsView->scrollTo(proxyIndex, QAbstractItemView::PositionAtTop);
}

void MainWindow::handleGameChanged(QueryResult result) {
  try {
    filtersWidget->resetConflictsAndGroupsFilters();
    disablePluginActions();

    handleGameDataLoaded(result);

    updateSidebarColumnWidths();

    // Perform ambiguous load order check because load order state was refreshed
    // when loading the new game's data.
    checkForAmbiguousLoadOrder();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleRefreshGameDataLoaded(QueryResult result) {
  try {
    handleGameDataLoaded(result);

    // Perform ambiguous load order check because load order state was refreshed
    // when refreshing game data.
    checkForAmbiguousLoadOrder();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleStartupGameDataLoaded(QueryResult result) {
  try {
    handleGameDataLoaded(result);

    if (state.getSettings().isAutoSortEnabled()) {
      if (hasErrorMessages()) {
        state.GetCurrentGame().AppendMessage(
            Message(MessageType::error,
                    boost::locale::translate(
                        "Auto-sort has been cancelled as there is at "
                        "least one error message displayed.")));

        updateGeneralMessages();
      } else {
        sortPlugins(true);
      }
    }

    // Perform ambiguous load order check because load order state was refreshed
    // when loading game data.
    checkForAmbiguousLoadOrder();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handlePluginsManualSorted(QueryResult result) {
  try {
    const auto loadOrderChanged = handlePluginsSorted(result);

    if (!loadOrderChanged) {
      // Perform ambiguous load order check because load order state was
      // refreshed at the start of sorting.
      checkForAmbiguousLoadOrder();
    }
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handlePluginsAutoSorted(QueryResult result) {
  try {
    handlePluginsSorted(result);

    if (actionApplySort->isVisible()) {
      actionApplySort->trigger();
    }

    if (!hasErrorMessages()) {
      on_actionQuit_triggered();
    }
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleMasterlistUpdated(QueryResult result) {
  try {
    if (!std::holds_alternative<PluginItems>(result)) {
      progressDialog->reset();
      showNotification(translate("No masterlist update was necessary."));

      // Update general info as the timestamp may have changed and if metadata
      // was previously missing it can now be displayed.
      updateGeneralInformation();
      return;
    }

    handleGameDataLoaded(result);

    auto masterlistInfo = getFileRevisionSummary(
        state.GetCurrentGame().MasterlistPath(), FileType::Masterlist);
    auto infoText = (boost::format(boost::locale::translate(
                         "Masterlist updated to revision %s.")) %
                     masterlistInfo.id)
                        .str();

    showNotification(QString::fromStdString(infoText));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleConflictsChecked(QueryResult result) {
  try {
    progressDialog->reset();

    // The result is not an array of PluginItem objects.
    // Instead it's an array of objects, with each having a metadata property
    // that is a PluginItem object, and a conflicts property that is
    // a boolean.
    std::vector<PluginItem> gameDataLoadedResult;
    std::vector<std::string> conflictingPluginNames;

    for (const auto& pluginPair :
         std::get<GetConflictingPluginsResult>(result)) {
      if (pluginPair.second) {
        conflictingPluginNames.push_back(pluginPair.first.name);
      }

      gameDataLoadedResult.push_back(pluginPair.first);
    }

    handleGameDataLoaded(gameDataLoadedResult);

    setFiltersState(filtersWidget->getPluginFiltersState(),
                    std::move(conflictingPluginNames));

    // Load order state was refreshed when plugins were loaded, so check for
    // ambiguity.
    checkForAmbiguousLoadOrder();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleProgressUpdate(const QString& message) {
  progressDialog->open();
  progressDialog->setLabelText(message);
}

void MainWindow::handleUpdateCheckFinished(QueryResult result) {
  try {
    const bool updateIsAvailable = std::get<bool>(result);
    if (updateIsAvailable) {
      const auto logger = getLogger();
      if (logger) {
        logger->info("LOOT update is available.");
      }

      const auto text = (boost::format(boost::locale::translate(
                             "A [new release](%s) of LOOT is available.")) %
                         "https://github.com/loot/loot/releases/latest")
                            .str();

      state.GetCurrentGame().AppendMessage(Message(MessageType::error, text));
      updateGeneralMessages();
    }
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleUpdateCheckError(const std::string&) {
  try {
    const auto text =
        boost::locale::translate(
            "Failed to check for LOOT updates! You can check your "
            "LOOTDebugLog.txt "
            "(you can get to it through the main menu) for more information.")
            .str();

    const auto generalMessages = pluginItemModel->getGeneralMessages();
    for (const auto& message : generalMessages) {
      if (message.text == text) {
        return;
      }
    }

    state.GetCurrentGame().AppendMessage(Message(MessageType::error, text));
    updateGeneralMessages();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleWorkerThreadFinished() { progressDialog->reset(); }

}
