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

#include <QtCore/QJsonDocument>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextEdit>

#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"
#include "gui/qt/plugin_card.h"
#include "gui/qt/plugin_item_filter_model.h"
#include "gui/qt/sidebar_plugin_name_delegate.h"
#include "gui/qt/style.h"
#include "gui/query/json.h"
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
#include "gui/query/types/update_masterlist_query.h"
#include "gui/query/types/update_prelude_query.h"
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

bool hasLoadOrderChanged(
    const std::vector<std::string>& oldLoadOrder,
    const std::vector<DerivedPluginMetadata>& newLoadOrder) {
  if (oldLoadOrder.size() != newLoadOrder.size()) {
    return true;
  }

  for (size_t i = 0; i < oldLoadOrder.size(); i += 1) {
    if (oldLoadOrder[i] != newLoadOrder[i].GetName()) {
      return true;
    }
  }

  return false;
}

void setUserMetadata(DerivedPluginMetadata& plugin, const LootState& state) {
  // This is necessary because converting from nlohmann::json loses the
  // masterlist and userlist metadata values.
  auto userMetadata = state.GetCurrentGame().GetUserMetadata(plugin.GetName());
  if (userMetadata.has_value()) {
    plugin.setUserMetadata(userMetadata.value());
  }
}

std::vector<PluginItem> getPluginItems(
    const nlohmann::json& derivedPluginMetadata,
    const LootState& state) {
  auto plugins =
      derivedPluginMetadata.get<std::vector<DerivedPluginMetadata>>();

  std::vector<PluginItem> pluginItems;
  pluginItems.reserve(plugins.size());
  for (auto& plugin : plugins) {
    // This is necessary because converting from nlohmann::json loses the
    // masterlist and userlist metadata values.
    setUserMetadata(plugin, state);

    pluginItems.push_back(PluginItem(plugin));
  }

  return pluginItems;
}

int compareLOOTVersion(const std::string& version) {
  std::vector<std::string> parts;
  boost::split(parts, version, boost::is_any_of("."));

  if (parts.size() != 3) {
    throw std::runtime_error("Unexpect number of version parts in " + version);
  }

  auto givenMajor = std::stoi(parts[0]);
  if (gui::Version::major > givenMajor) {
    return 1;
  }

  if (gui::Version::major < givenMajor) {
    return -1;
  }

  auto givenMinor = std::stoi(parts[1]);
  if (gui::Version::minor > givenMinor) {
    return 1;
  }

  if (gui::Version::minor < givenMinor) {
    return -1;
  }

  auto givenPatch = std::stoi(parts[2]);
  if (gui::Version::patch > givenPatch) {
    return 1;
  }

  if (gui::Version::patch < givenPatch) {
    return -1;
  }

  return 0;
}

std::optional<QDate> getDateFromCommitJson(const QJsonDocument& document,
                                           const std::string& commitHash) {
  // Committer can be null, but that will just result in an Undefined value.
  auto dateString = document["commit"]["committer"]["date"].toString();
  if (dateString.isEmpty()) {
    auto logger = getLogger();
    if (logger) {
      logger->error(
          "Error while checking for LOOT updates: couldn't get commit date for "
          "commit {}",
          commitHash);
    }
    return std::nullopt;
  }

  return QDate::fromString(dateString, Qt::ISODate);
}

int calculateSidebarLoadOrderSectionWidth(GameType gameType) {
  // Find the widest hex character in the current font and use that to
  // calculate the load order section width.
  static const std::array<char, 16> HEX_CHARACTERS = {'0',
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

  int maxCharWidth = 0;
  for (const auto& hexCharacter : HEX_CHARACTERS) {
    auto width = QApplication::fontMetrics()
                     .size(Qt::TextSingleLine, QString(QChar(hexCharacter)))
                     .width();
    if (width > maxCharWidth) {
      maxCharWidth = width;
    }
  }

  auto paddingWidth =
      QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin);

  // If the game supports light plugins leave enough space for their longer
  // indexes, otherwise only leave space for two hex digits.
  switch (gameType) {
    case GameType::tes3:
    case GameType::tes4:
    case GameType::tes5:
    case GameType::fo3:
    case GameType::fonv:
      return 2 * maxCharWidth + paddingWidth;
    default:
      return QApplication::fontMetrics()
                 .size(Qt::TextSingleLine, "FE ")
                 .width() +
             3 * maxCharWidth + paddingWidth;
  }
}

MainWindow::MainWindow(LootState& state, QWidget* parent) :
    QMainWindow(parent), state(state) {
  setupUi();
}

void MainWindow::initialise() {
  try {
    auto installedGames = state.GetInstalledGameFolderNames();

    for (const auto& gameSettings : state.getGameSettings()) {
      auto installedGame = std::find(installedGames.cbegin(),
                                     installedGames.cend(),
                                     gameSettings.FolderName());

      if (installedGame != installedGames.cend()) {
        gameComboBox->addItem(
            QString::fromStdString(gameSettings.Name()),
            QString::fromStdString(gameSettings.FolderName()));
      }
    }

    auto initErrors = state.getInitErrors();
    if (!initErrors.empty()) {
      std::vector<SimpleMessage> errorMessages;
      errorMessages.reserve(initErrors.size());

      for (const auto error : initErrors) {
        SimpleMessage message;
        message.type = MessageType::error;
        message.text = error;
        errorMessages.push_back(message);
      }

      pluginItemModel->setGeneralMessages(std::move(errorMessages));
      return;
    }

    const auto& filters = state.getFilters();
    filtersWidget->hideVersionNumbers(filters.hideVersionNumbers);
    filtersWidget->hideCRCs(filters.hideCRCs);
    filtersWidget->hideBashTags(filters.hideBashTags);
    filtersWidget->hideNotes(filters.hideNotes);
    filtersWidget->hidePluginMessages(filters.hideAllPluginMessages);
    filtersWidget->hideInactivePlugins(filters.hideInactivePlugins);
    filtersWidget->hideMessagelessPlugins(filters.hideMessagelessPlugins);

    // Apply the filters before loading the game because that avoids having
    // to re-filter the full plugin list.
    pluginItemModel->setCardContentFiltersState(
        filtersWidget->getCardContentFiltersState());
    proxyModel->setFiltersState(filtersWidget->getPluginFiltersState(), {});

    gameComboBox->setCurrentText(
        QString::fromStdString(state.GetCurrentGame().Name()));

    loadGame(true);

    // Check for updates.
    if (state.isLootUpdateCheckEnabled()) {
      sendHttpRequest("https://api.github.com/repos/loot/loot/releases/latest",
                      &MainWindow::handleGetLatestReleaseResponseFinished);
    }
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::setupUi() {
#ifdef _WIN32
  setWindowIcon(QIcon(":/icon.ico"));
#endif

  auto lastWindowPosition = state.getWindowPosition();
  if (lastWindowPosition.has_value()) {
    const auto& windowPosition = lastWindowPosition.value();
    auto width = windowPosition.right - windowPosition.left;
    auto height = windowPosition.bottom - windowPosition.top;

    auto geometry =
        QRect(windowPosition.left, windowPosition.top, width, height);
    auto topLeft = geometry.topLeft();

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
    resize(1024, 768);
  }

  // Set up status bar.
  statusbar = new QStatusBar(this);
  setStatusBar(statusbar);

  setupMenuBar();
  setupToolBar();

  settingsDialog = new SettingsDialog(this);
  settingsDialog->setObjectName("settingsDialog");

  searchDialog = new SearchDialog(this);
  searchDialog->setObjectName("searchDialog");

  auto sidebarSplitter = new QSplitter(this);

  toolBox = new QToolBox(sidebarSplitter);

  auto pluginsTab = new QWidget();
  auto pluginsTabLayout = new QVBoxLayout(pluginsTab);

  sidebarPluginsView = new QTableView(pluginsTab);
  sidebarPluginsView->setObjectName("sidebarPluginsView");

  pluginsTabLayout->addWidget(sidebarPluginsView, 1);
  pluginsTabLayout->setContentsMargins(0, 0, 0, 0);
  toolBox->addItem(pluginsTab, QString("Plugins"));

  filtersWidget = new FiltersWidget(toolBox);
  filtersWidget->setObjectName("filtersWidget");

  toolBox->addItem(filtersWidget, QString("Filters"));

  sidebarSplitter->addWidget(toolBox);

  auto editorSplitter =
      new QSplitter(Qt::Orientation::Vertical, sidebarSplitter);

  pluginCardsView = new QListView(editorSplitter);
  pluginCardsView->setObjectName("pluginCardsView");

  editorSplitter->addWidget(pluginCardsView);

  pluginEditorWidget = new PluginEditorWidget(
      editorSplitter, state.getLanguages(), state.getLanguage());
  pluginEditorWidget->setObjectName("pluginEditorWidget");
  pluginEditorWidget->hide();

  editorSplitter->addWidget(pluginEditorWidget);

  sidebarSplitter->addWidget(editorSplitter);

  sidebarSplitter->setStretchFactor(0, 1);
  sidebarSplitter->setStretchFactor(1, 2);

  setCentralWidget(sidebarSplitter);

  progressDialog = new QProgressDialog(this);
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setCancelButton(nullptr);
  progressDialog->setMinimum(0);
  progressDialog->setMaximum(0);
  progressDialog->reset();

  pluginItemModel = new PluginItemModel(this);
  pluginItemModel->setObjectName("pluginItemModel");

  proxyModel = new PluginItemFilterModel(this);
  proxyModel->setObjectName("proxyModel");
  proxyModel->setSourceModel(pluginItemModel);

  groupsEditor = new GroupsEditorDialog(this, pluginItemModel);
  groupsEditor->setObjectName("groupsEditor");

  setupViews();

  translateUi();

  disableGameActions();

  QMetaObject::connectSlotsByName(this);
}

void MainWindow::setupMenuBar() {
  // Create actions.
  actionSettings = new QAction(this);
  actionSettings->setObjectName("actionSettings");
  actionSettings->setIcon(IconFactory::getSettingsIcon());
  actionQuit = new QAction(this);
  actionQuit->setObjectName("actionQuit");
  actionQuit->setIcon(IconFactory::getQuitIcon());

  actionViewDocs = new QAction(this);
  actionViewDocs->setObjectName("actionViewDocs");
  actionViewDocs->setIcon(IconFactory::getViewDocsIcon());
  actionViewDocs->setShortcut(QKeySequence::HelpContents);
  actionOpenDebugLogLocation = new QAction(this);
  actionOpenDebugLogLocation->setObjectName("actionOpenDebugLogLocation");
  actionOpenDebugLogLocation->setIcon(
      IconFactory::getOpenDebugLogLocationIcon());
  actionAbout = new QAction(this);
  actionAbout->setObjectName("actionAbout");
  actionAbout->setIcon(IconFactory::getAboutIcon());

  actionOpenGroupsEditor = new QAction(this);
  actionOpenGroupsEditor->setObjectName("actionOpenGroupsEditor");
  actionOpenGroupsEditor->setIcon(IconFactory::getOpenGroupsEditorIcon());
  actionSearch = new QAction(this);
  actionSearch->setObjectName("actionSearch");
  actionSearch->setIcon(IconFactory::getSearchIcon());
  actionSearch->setShortcut(QKeySequence::Find);
  actionCopyLoadOrder = new QAction(this);
  actionCopyLoadOrder->setObjectName("actionCopyLoadOrder");
  actionCopyLoadOrder->setIcon(IconFactory::getCopyLoadOrderIcon());
  actionCopyContent = new QAction(this);
  actionCopyContent->setObjectName("actionCopyContent");
  actionCopyContent->setIcon(IconFactory::getCopyContentIcon());
  actionRefreshContent = new QAction(this);
  actionRefreshContent->setObjectName("actionRefreshContent");
  actionRefreshContent->setIcon(IconFactory::getRefreshIcon());
  actionRefreshContent->setShortcut(QKeySequence::Refresh);
  actionRedatePlugins = new QAction(this);
  actionRedatePlugins->setObjectName("actionRedatePlugins");
  actionRedatePlugins->setIcon(IconFactory::getRedateIcon());
  actionClearAllUserMetadata = new QAction(this);
  actionClearAllUserMetadata->setObjectName("actionClearAllUserMetadata");
  actionClearAllUserMetadata->setIcon(IconFactory::getDeleteIcon());

  actionCopyMetadata = new QAction(this);
  actionCopyMetadata->setObjectName("actionCopyMetadata");
  actionCopyCardContent = new QAction(this);
  actionCopyCardContent->setObjectName("actionCopyCardContent");
  actionCopyCardContent->setIcon(IconFactory::getCopyContentIcon());
  actionEditMetadata = new QAction(this);
  actionEditMetadata->setObjectName("actionEditMetadata");
  actionEditMetadata->setIcon(IconFactory::getEditIcon());
  actionEditMetadata->setShortcut(QString("Ctrl+E"));
  actionClearMetadata = new QAction(this);
  actionClearMetadata->setObjectName("actionClearMetadata");
  actionClearMetadata->setIcon(IconFactory::getDeleteIcon());

  // Create menu bar.
  menubar = new QMenuBar(this);
  menuFile = new QMenu(menubar);
  menuHelp = new QMenu(menubar);
  menuGame = new QMenu(menubar);
  menuPlugin = new QMenu(menubar);
  setMenuBar(menubar);

  menubar->addAction(menuFile->menuAction());
  menubar->addAction(menuGame->menuAction());
  menubar->addAction(menuPlugin->menuAction());
  menubar->addAction(menuHelp->menuAction());
  menuFile->addAction(actionSettings);
  menuFile->addSeparator();
  menuFile->addAction(actionQuit);
  menuGame->addAction(actionOpenGroupsEditor);
  menuGame->addSeparator();
  menuGame->addAction(actionSearch);
  menuGame->addAction(actionCopyLoadOrder);
  menuGame->addAction(actionCopyContent);
  menuGame->addAction(actionRefreshContent);
  menuGame->addSeparator();
  menuGame->addAction(actionRedatePlugins);
  menuGame->addAction(actionClearAllUserMetadata);
  menuPlugin->addAction(actionEditMetadata);
  menuPlugin->addSeparator();
  menuPlugin->addAction(actionCopyMetadata);
  menuPlugin->addAction(actionCopyCardContent);
  menuPlugin->addSeparator();
  menuPlugin->addAction(actionClearMetadata);
  menuHelp->addAction(actionViewDocs);
  menuHelp->addAction(actionOpenDebugLogLocation);
  menuHelp->addSeparator();
  menuHelp->addAction(actionAbout);
}

void MainWindow::setupToolBar() {
  // Create actions.
  actionSort = new QAction(this);
  actionSort->setObjectName("actionSort");
  actionSort->setIcon(IconFactory::getSortIcon());
  actionUpdateMasterlist = new QAction(this);
  actionUpdateMasterlist->setObjectName("actionUpdateMasterlist");
  actionUpdateMasterlist->setIcon(IconFactory::getUpdateMasterlistIcon());
  actionApplySort = new QAction(this);
  actionApplySort->setObjectName("actionApplySort");
  actionApplySort->setVisible(false);
  actionDiscardSort = new QAction(this);
  actionDiscardSort->setObjectName("actionDiscardSort");
  actionDiscardSort->setVisible(false);

  // Create toolbar.
  toolBar = new QToolBar(this);
  toolBar->setMovable(false);
  toolBar->setFloatable(false);
  toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  addToolBar(Qt::TopToolBarArea, toolBar);

  gameComboBox = new QComboBox(toolBar);
  gameComboBox->setObjectName("gameComboBox");

  toolBar->addWidget(gameComboBox);

  toolBar->addAction(actionSort);
  toolBar->addAction(actionUpdateMasterlist);
  toolBar->addAction(actionApplySort);
  toolBar->addAction(actionDiscardSort);
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
  horizontalHeader->hide();
  horizontalHeader->setSectionResizeMode(0, QHeaderView::Fixed);
  horizontalHeader->setSectionResizeMode(1, QHeaderView::Stretch);
  horizontalHeader->setSectionResizeMode(2, QHeaderView::Fixed);

  updateSidebarColumnWidths();

  sidebarPluginsView->setItemDelegateForColumn(
      PluginItemModel::SIDEBAR_NAME_COLUMN,
      new SidebarPluginNameDelegate(sidebarPluginsView));

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

  // Don't set the delegates until after the model is set, as the
  // former may depend on the latter.
  pluginCardsView->setItemDelegate(new PluginCardDelegate(pluginCardsView));

  // Plugin selection handling needs to be set up after the model has been
  // set, as before then there is no selection model.
  auto selectionModel = sidebarPluginsView->selectionModel();
  connect(selectionModel,
          &QItemSelectionModel::selectionChanged,
          this,
          &MainWindow::on_sidebarPluginsSelectionModel_selectionChanged);
}

void MainWindow::translateUi() {
  setWindowTitle(translate("LOOT"));

  // Translate toolbar items.
  actionSort->setText(translate("Sort Plugins"));
  actionUpdateMasterlist->setText(translate("Update Masterlist"));
  actionApplySort->setText(translate("Apply Sorted Load Order"));
  actionDiscardSort->setText(translate("Discard Sorted Load Order"));

  // Translate menu bar items.
  menuFile->setTitle(translate("File"));
  actionSettings->setText(translate("Settings..."));
  actionQuit->setText(translate("Quit"));

  menuGame->setTitle(translate("Game"));
  actionOpenGroupsEditor->setText(translate("Edit Groups..."));
  actionSearch->setText(translate("Search Cards..."));
  actionCopyLoadOrder->setText(translate("Copy Load Order"));
  actionCopyContent->setText(translate("Copy Content"));
  actionRefreshContent->setText(translate("Refresh Content"));
  actionRedatePlugins->setText(translate("Redate Plugins..."));
  actionClearAllUserMetadata->setText(translate("Clear All User Metadata..."));

  menuPlugin->setTitle(translate("Plugin"));
  actionCopyMetadata->setText(translate("Copy Metadata"));
  actionCopyCardContent->setText(translate("Copy Card Content"));
  actionEditMetadata->setText(translate("Edit Metadata..."));
  actionClearMetadata->setText(translate("Clear User Metadata..."));

  menuHelp->setTitle(translate("Help"));
  actionViewDocs->setText(translate("View Documentation"));
  actionOpenDebugLogLocation->setText(translate("Open Debug Log Location"));
  actionAbout->setText(translate("About"));

  // Translate sidebar.
  toolBox->setItemText(0, translate("Plugins"));
  toolBox->setItemText(1, translate("Filters"));
}

void MainWindow::enableGameActions() {
  menuGame->setEnabled(true);
  actionSort->setEnabled(true);
  actionUpdateMasterlist->setEnabled(true);

  auto enableRedatePlugins = state.GetCurrentGame().Type() == GameType::tes5 ||
                             state.GetCurrentGame().Type() == GameType::tes5se;
  actionRedatePlugins->setEnabled(enableRedatePlugins);
}

void MainWindow::disableGameActions() {
  menuGame->setEnabled(false);
  actionSort->setEnabled(false);
  actionUpdateMasterlist->setEnabled(false);

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

  std::unique_ptr<Query> query = std::make_unique<GetGameDataQuery<>>(
      state.GetCurrentGame(), state.getLanguage(), sendProgressUpdate);

  auto handler = isOnLOOTStartup ? &MainWindow::handleStartupGameDataLoaded
                                 : &MainWindow::handleRefreshGameDataLoaded;

  executeBackgroundQuery(std::move(query), handler, progressUpdater);
}

void MainWindow::updateCounts(const std::vector<SimpleMessage>& generalMessages,
                              const std::vector<PluginItem>& plugins) {
  auto counters = GeneralInformationCounters(generalMessages, plugins);
  auto hiddenMessageCount =
      countHiddenMessages(plugins, filtersWidget->getCardContentFiltersState());

  filtersWidget->setMessageCounts(hiddenMessageCount, counters.totalMessages);
  filtersWidget->setPluginCounts(
      counters.totalPlugins - (proxyModel->rowCount() - 1),
      counters.totalPlugins);
}

void MainWindow::updateGeneralInformation() {
  auto masterlistInfo = GetFileRevisionToDisplay(
      state.GetCurrentGame().MasterlistPath(), FileType::Masterlist);
  auto preludeInfo = GetFileRevisionToDisplay(state.getPreludePath(),
                                              FileType::MasterlistPrelude);
  auto generalMessages = ToSimpleMessages(state.GetCurrentGame().GetMessages(),
                                          state.getLanguage());

  pluginItemModel->setGeneralInformation(
      masterlistInfo, preludeInfo, generalMessages);
}

void MainWindow::updateGeneralMessages() {
  auto generalMessages = ToSimpleMessages(state.GetCurrentGame().GetMessages(),
                                          state.getLanguage());

  pluginItemModel->setGeneralMessages(std::move(generalMessages));
}

void MainWindow::updateSidebarColumnWidths() {
  auto horizontalHeader = sidebarPluginsView->horizontalHeader();

  auto stateSectionWidth =
      QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize) +
      QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin);

  // If there is no current game set (i.e. on initial construction), use TES5 SE
  // to calculate the load order section width because that's one of the games
  // that uses the wider width.
  auto loadOrderSectionWidth =
      state.HasCurrentGame()
          ? calculateSidebarLoadOrderSectionWidth(state.GetCurrentGame().Type())
          : calculateSidebarLoadOrderSectionWidth(GameType::tes5se);

  auto minimumSectionWidth = stateSectionWidth < loadOrderSectionWidth
                                 ? stateSectionWidth
                                 : loadOrderSectionWidth;

  horizontalHeader->setMinimumSectionSize(minimumSectionWidth);
  horizontalHeader->resizeSection(0, loadOrderSectionWidth);
  horizontalHeader->resizeSection(2, stateSectionWidth);
}

void MainWindow::setFiltersState(PluginFiltersState&& state) {
  proxyModel->setFiltersState(std::move(state));

  updateCounts(pluginItemModel->getGeneralMessages(),
               pluginItemModel->getPluginItems());
  searchDialog->reset();
  proxyModel->clearSearchResults();
}

void MainWindow::setFiltersState(
    PluginFiltersState&& state,
    std::vector<std::string>&& conflictingPluginNames) {
  proxyModel->setFiltersState(std::move(state),
                              std::move(conflictingPluginNames));

  updateCounts(pluginItemModel->getGeneralMessages(),
               pluginItemModel->getPluginItems());
  searchDialog->reset();
  proxyModel->clearSearchResults();
}

bool MainWindow::hasErrorMessages() const {
  auto counters = GeneralInformationCounters(
      pluginItemModel->getGeneralMessages(), pluginItemModel->getPluginItems());

  return counters.errors != 0;
}

void MainWindow::sortPlugins(bool isAutoSort) {
  std::vector<
      std::pair<std::unique_ptr<Query>, void (MainWindow::*)(nlohmann::json)>>
      queriesAndHandlers;

  if (state.updateMasterlist()) {
    handleProgressUpdate(translate("Updating and parsing masterlist..."));

    std::unique_ptr<Query> updatePrelude = std::make_unique<UpdatePreludeQuery>(
        state.getPreludePath(),
        state.getPreludeRepositoryURL(),
        state.getPreludeRepositoryBranch());

    std::unique_ptr<Query> updateMasterlistQuery =
        std::make_unique<UpdateMasterlistQuery<>>(state.GetCurrentGame(),
                                                  state.getLanguage());

    queriesAndHandlers.push_back(std::move(std::make_pair(
        std::move(updatePrelude), &MainWindow::handlePreludeUpdated)));
    queriesAndHandlers.push_back(
        std::move(std::make_pair(std::move(updateMasterlistQuery),
                                 &MainWindow::handleMasterlistUpdated)));
  }

  auto progressUpdater = new ProgressUpdater();

  // This lambda will run from the worker thread.
  auto sendProgressUpdate = [progressUpdater](std::string message) {
    emit progressUpdater->progressUpdate(QString::fromStdString(message));
  };

  std::unique_ptr<Query> sortPluginsQuery =
      std::make_unique<SortPluginsQuery<>>(state.GetCurrentGame(),
                                           state,
                                           state.getLanguage(),
                                           sendProgressUpdate);

  auto sortHandler = isAutoSort ? &MainWindow::handlePluginsAutoSorted
                                : &MainWindow::handlePluginsManualSorted;

  queriesAndHandlers.push_back(
      std::move(std::make_pair(std::move(sortPluginsQuery), sortHandler)));

  executeBackgroundQueryChain(std::move(queriesAndHandlers), progressUpdater);
}

void MainWindow::showFirstRunDialog() {
  std::string textTemplate = R"(
<p>%1%</p>
<ul>
  <li>%2%</li>
  <li>%3%</li>
  <li>%4%</li>
</ul>
<p>%5%</p>
)";

  auto paragraph1 =
      (boost::format(boost::locale::translate(
           "This appears to be the first time you have run LOOT v%s. Here are "
           "some tips to help you get started with the interface.")) %
       gui::Version::string())
          .str();

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

  auto paragraph2 =
      (boost::format(boost::locale::translate(
           "LOOT is free, but if you want to show your appreciation with some "
           "money, donations may be made to WrinklyNinja (LOOT's creator and "
           "main developer) using %s.")) %
       "<a href=\"https://www.paypal.me/OliverHamlet\">PayPal</a>")
          .str();

  std::string text = (boost::format(textTemplate) % paragraph1 % listItem1 %
                      listItem2 % listItem3 % paragraph2)
                         .str();

  auto messageBox = QMessageBox(QMessageBox::NoIcon,
                                translate("First-Time Tips"),
                                QString::fromStdString(text),
                                QMessageBox::Ok,
                                this);
  messageBox.exec();
}

void MainWindow::showNotification(const QString& message) {
  statusBar()->showMessage(message, 5000);
}

PluginItem MainWindow::getSelectedPlugin() const {
  auto selectedPluginIndices =
      sidebarPluginsView->selectionModel()->selectedIndexes();
  if (selectedPluginIndices.isEmpty()) {
    throw std::runtime_error(
        "Cannot copy plugin metadata when no plugin is selected");
  }

  auto data = selectedPluginIndices.first().data(RawDataRole);
  if (!data.canConvert<PluginItem>()) {
    throw std::runtime_error("Cannot convert data to PluginItem");
  }

  return data.value<PluginItem>();
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
    auto geometry = normalGeometry();
    position.left = geometry.x();
    position.top = geometry.y();
    position.right = position.left + geometry.width();
    position.bottom = position.top + geometry.height();
    position.maximised = isMaximized();

    state.storeWindowPosition(position);
  } catch (std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to record window position: {}", e.what());
    }
  }

  try {
    state.storeFilters(filtersWidget->getFilterSettings());
  } catch (std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to record filter states: {}", e.what());
    }
  }

  try {
    state.save(state.getSettingsPath());
  } catch (std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to save LOOT's settings. Error: {}", e.what());
    }
  }

  event->accept();
}

void MainWindow::executeBackgroundQuery(
    std::unique_ptr<Query> query,
    void (MainWindow::*onComplete)(nlohmann::json),
    ProgressUpdater* progressUpdater) {
  auto workerThread = new QueryWorkerThread(this, std::move(query));

  if (progressUpdater != nullptr) {
    connect(progressUpdater,
            &ProgressUpdater::progressUpdate,
            this,
            &MainWindow::handleProgressUpdate);

    connect(workerThread,
            &QueryWorkerThread::finished,
            progressUpdater,
            &QObject::deleteLater);
  }

  connect(workerThread, &QueryWorkerThread::resultReady, this, onComplete);
  connect(
      workerThread, &QueryWorkerThread::error, this, &MainWindow::handleError);
  connect(workerThread,
          &QueryWorkerThread::finished,
          workerThread,
          &QObject::deleteLater);

  workerThread->start();
}

void MainWindow::executeBackgroundQueryChain(
    std::vector<
        std::pair<std::unique_ptr<Query>, void (MainWindow::*)(nlohmann::json)>>
        queriesAndHandlers,
    ProgressUpdater* progressUpdater) {
  // Run each query in the vector sequentially, handling completion using each
  // query's corresponding handler. If a query fails, all earlier query handlers
  // should have run and the failed query's handler and later handlers should
  // not run.
  // In order to ensure that the correct handler runs for each query, use a
  // demultiplexing handler that counts how many signals it's received.

  std::vector<std::unique_ptr<Query>> queries;
  auto resultDemultiplexer = new ResultDemultiplexer(this);

  for (auto&& [query, handler] : queriesAndHandlers) {
    queries.push_back(std::move(query));
    resultDemultiplexer->addHandler(handler);
  }

  auto workerThread = new QueryWorkerThread(this, std::move(queries));

  if (progressUpdater != nullptr) {
    connect(progressUpdater,
            &ProgressUpdater::progressUpdate,
            this,
            &MainWindow::handleProgressUpdate);

    connect(workerThread,
            &QueryWorkerThread::finished,
            progressUpdater,
            &QObject::deleteLater);
  }

  connect(workerThread,
          &QueryWorkerThread::resultReady,
          resultDemultiplexer,
          &ResultDemultiplexer::onResultReady);
  connect(
      workerThread, &QueryWorkerThread::error, this, &MainWindow::handleError);
  connect(workerThread,
          &QueryWorkerThread::finished,
          workerThread,
          &QObject::deleteLater);
  connect(workerThread,
          &QueryWorkerThread::finished,
          resultDemultiplexer,
          &QObject::deleteLater);

  // Reset (i.e. close) the progress dialog once the thread has finished in case
  // it was used while running these queries. This can't be done from any one
  // handler because none of them know if they are the last to run.
  connect(workerThread,
          &QueryWorkerThread::finished,
          this,
          &MainWindow::handleWorkerThreadFinished);

  workerThread->start();
}

void MainWindow::sendHttpRequest(const std::string& url,
                                 void (MainWindow::*onFinished)()) {
  QNetworkRequest request(QUrl(QString::fromStdString(url)));
  request.setRawHeader("Accept", "application/vnd.github.v3+json");
  auto reply = networkAccessManager.get(request);

  connect(reply, &QNetworkReply::finished, this, onFinished);
  connect(reply,
          &QNetworkReply::errorOccurred,
          this,
          &MainWindow::handleUpdateCheckNetworkError);
  connect(reply,
          &QNetworkReply::sslErrors,
          this,
          &MainWindow::handleUpdateCheckSSLError);
}

std::optional<QJsonDocument> MainWindow::readHttpResponse(
    QNetworkReply* reply) {
  auto statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

  auto data = reply->readAll();
  reply->deleteLater();

  if (statusCode < 200 || statusCode >= 400) {
    auto logger = getLogger();
    if (logger) {
      auto json = QJsonDocument::fromJson(data);

      logger->error(
          "Error while checking for LOOT updates: unexpected HTTP response "
          "status code: {}. Error message in response is: {}",
          statusCode,
          json["message"].toString().toStdString());
    }

    addUpdateCheckErrorMessage();
    return std::nullopt;
  }

  return QJsonDocument::fromJson(data);
}

void MainWindow::addUpdateAvailableMessage() {
  auto logger = getLogger();
  if (logger) {
    logger->info("LOOT update is available.");
  }

  auto text = (boost::format(boost::locale::translate(
                   "A [new release](%s) of LOOT is available.")) %
               "https://github.com/loot/loot/releases/latest")
                  .str();

  state.GetCurrentGame().AppendMessage(Message(MessageType::error, text));
  updateGeneralMessages();
}

void MainWindow::addUpdateCheckErrorMessage() {
  auto text =
      boost::locale::translate(
          "Failed to check for LOOT updates! You can check your "
          "LOOTDebugLog.txt "
          "(you can get to it through the main menu) for more information.")
          .str();

  auto generalMessages = pluginItemModel->getGeneralMessages();
  for (const auto message : generalMessages) {
    if (message.text == text) {
      return;
    }
  }

  state.GetCurrentGame().AppendMessage(Message(MessageType::error, text));
  updateGeneralMessages();
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

void MainWindow::handleQueryException(const std::unique_ptr<Query> query,
                                      const std::exception& exception) {
  if (query == nullptr) {
    handleException(exception);
  } else {
    auto logger = getLogger();
    if (logger) {
      logger->error("Caught an exception: {}", exception.what());
    }

    handleError(query->getErrorMessage());
  }
}

void MainWindow::handleGameDataLoaded(nlohmann::json result) {
  progressDialog->reset();

  pluginItemModel->setPluginItems(getPluginItems(result.at("plugins"), state));

  updateGeneralInformation();

  filtersWidget->setGroups(GetGroupNames(state.GetCurrentGame()));

  pluginEditorWidget->setBashTagCompletions(
      state.GetCurrentGame().GetKnownBashTags());

  enableGameActions();
}

void MainWindow::handlePluginsSorted(nlohmann::json result) {
  filtersWidget->resetConflictsAndGroupsFilters();

  auto sortedPlugins = result.at("plugins");

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
    return;
  }

  auto currentLoadOrder = state.GetCurrentGame().GetLoadOrder();
  auto loadOrderHasChanged =
      hasLoadOrderChanged(currentLoadOrder, sortedPlugins);

  if (loadOrderHasChanged) {
    enterSortingState();
  } else {
    state.DecrementUnappliedChangeCounter();

    showNotification(translate("Sorting made no changes to the load order."));
  }

  handleGameDataLoaded(result);
}

QMenu* MainWindow::createPopupMenu() {
  QMenu* filteredMenu = QMainWindow::createPopupMenu();
  // Don't allow the toolbar to be hidden.
  filteredMenu->removeAction(toolBar->toggleViewAction());
  return filteredMenu;
}

void MainWindow::on_actionSettings_triggered(bool checked) {
  try {
    auto themes = findThemes(state.getResourcesPath());
    auto currentGameFolder =
        state.HasCurrentGame()
            ? std::optional(state.GetCurrentGame().FolderName())
            : std::nullopt;

    settingsDialog->initialiseInputs(state, themes, currentGameFolder);
    settingsDialog->show();

    // Adjust size because otherwise the size is slightly too small the first
    // time the dialog is opened.
    settingsDialog->adjustSize();
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionQuit_triggered(bool checked) { this->close(); }

void MainWindow::on_actionOpenGroupsEditor_triggered(bool checked) {
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
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionSearch_triggered(bool checked) {
  searchDialog->show();
}

void MainWindow::on_actionCopyLoadOrder_triggered(bool checked) {
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
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionCopyContent_triggered(bool checked) {
  try {
    auto content =
        pluginItemModel->getGeneralInfo().getMarkdownContent() + "\n\n";

    for (const auto& plugin : pluginItemModel->getPluginItems()) {
      content += plugin.getMarkdownContent() + "\n\n";
    }

    CopyToClipboard(content);

    showNotification(
        translate("LOOT's content has been copied to the clipboard."));
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionRefreshContent_triggered(bool checked) {
  try {
    loadGame(false);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionRedatePlugins_triggered(bool checked) {
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
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionClearAllUserMetadata_triggered(bool checked) {
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

    ClearAllMetadataQuery query(state.GetCurrentGame(), state.getLanguage());

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
    auto pluginItems = getPluginItems(result.at("plugins"), state);

    // For each item, find its existing index in the model and update its data.
    // The sidebar item and card will be updated by handling the resulting
    // dataChanged signal.
    auto nameToRowMap = pluginItemModel->getPluginNameToRowMap();
    for (const auto& item : pluginItems) {
      auto it = nameToRowMap.find(item.name);
      if (it == nameToRowMap.end()) {
        throw std::runtime_error(std::string("Could not find plugin named \"") +
                                 item.name + "\" in the plugin item model.");
      }

      // It doesn't matter which index column is used, it's the same data.
      auto index = pluginItemModel->index(it->second, 0);
      pluginItemModel->setData(index, QVariant::fromValue(item), RawDataRole);
    }

    showNotification(translate("All user-added metadata has been cleared."));
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionEditMetadata_triggered(bool checked) {
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

    auto plugin = getSelectedPlugin();
    auto groups = GetGroupNames(state.GetCurrentGame());

    pluginEditorWidget->initialiseInputs(
        groups,
        plugin.name,
        state.GetCurrentGame().GetNonUserMetadata(
            state.GetCurrentGame().GetPlugin(plugin.name)),
        state.GetCurrentGame().GetUserMetadata(plugin.name));

    pluginEditorWidget->show();

    state.IncrementUnappliedChangeCounter();

    // Refresh the sidebar items so that all their groups are displayed.
    pluginItemModel->setEditorPluginName(plugin.name);

    enterEditingState();
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionCopyMetadata_triggered(bool checked) {
  try {
    auto selectedPluginName = getSelectedPlugin().name;

    CopyMetadataQuery query(
        state.GetCurrentGame(), state.getLanguage(), selectedPluginName);

    query.executeLogic();

    auto text =
        (boost::format(boost::locale::translate(
             "The metadata for \"%s\" has been copied to the clipboard.")) %
         selectedPluginName)
            .str();

    showNotification(QString::fromStdString(text));
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionCopyCardContent_triggered(bool checked) {
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
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionClearMetadata_triggered(bool checked) {
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

    ClearPluginMetadataQuery query(
        state.GetCurrentGame(), state.getLanguage(), selectedPluginName);

    auto result = query.executeLogic();

    // The result is the changed plugin's derived metadata. Update the
    // model's data and also the message counts.

    auto plugin = result.get<DerivedPluginMetadata>();
    setUserMetadata(plugin, state);
    auto newPluginItem = PluginItem(plugin);

    for (int i = 1; i < pluginItemModel->rowCount(); i += 1) {
      auto index = pluginItemModel->index(i, 0);
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
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionViewDocs_triggered(bool checked) {
  try {
    OpenReadmeQuery query(state.getReadmePath(), "index.html");

    query.executeLogic();
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionOpenDebugLogLocation_triggered(bool checked) {
  try {
    OpenLogLocationQuery query(state.getLogPath());

    query.executeLogic();
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionAbout_triggered(bool checked) {
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
  } catch (std::exception& e) {
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
         folderName == state.GetCurrentGame().FolderName())) {
      return;
    }

    auto progressUpdater = new ProgressUpdater();

    // This lambda will run from the worker thread.
    auto sendProgressUpdate = [progressUpdater](std::string message) {
      emit progressUpdater->progressUpdate(QString::fromStdString(message));
    };

    std::unique_ptr<Query> query = std::make_unique<ChangeGameQuery<>>(
        state, state.getLanguage(), folderName, sendProgressUpdate);

    executeBackgroundQuery(
        std::move(query), &MainWindow::handleGameChanged, progressUpdater);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionSort_triggered(bool checked) {
  try {
    sortPlugins(false);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionApplySort_triggered(bool checked) {
  std::unique_ptr<Query> query;
  try {
    auto sortedPluginNames = pluginItemModel->getPluginNames();

    query = std::make_unique<ApplySortQuery<>>(
        state.GetCurrentGame(), state, sortedPluginNames);

    query->executeLogic();

    exitSortingState();
  } catch (std::exception& e) {
    handleQueryException(std::move(query), e);
  }
}

void MainWindow::on_actionDiscardSort_triggered(bool checked) {
  try {
    auto query =
        CancelSortQuery(state.GetCurrentGame(), state, state.getLanguage());

    auto result = query.executeLogic();

    auto pluginItems = pluginItemModel->getPluginItems();

    std::vector<PluginItem> newPluginItems;
    newPluginItems.reserve(pluginItems.size());
    for (const auto& plugin : result.at("plugins")) {
      auto pluginName = plugin.at("name").get<std::string>();

      auto it = std::find_if(pluginItems.cbegin(),
                             pluginItems.cend(),
                             [&](const auto& pluginItem) {
                               return pluginItem.name == pluginName;
                             });

      if (it != pluginItems.end()) {
        auto newLoadOrderIndex =
            plugin.contains("loadOrderIndex")
                ? std::make_optional(plugin.at("loadOrderIndex").get<short>())
                : std::nullopt;

        auto newPluginItem = *it;
        newPluginItem.loadOrderIndex = newLoadOrderIndex;
        newPluginItems.push_back(newPluginItem);
      }
    }

    pluginItemModel->setPluginItems(std::move(newPluginItems));

    updateGeneralMessages();

    exitSortingState();
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionUpdateMasterlist_triggered(bool checked) {
  try {
    handleProgressUpdate(translate("Updating and parsing masterlist..."));

    std::unique_ptr<Query> updateMasterlistQuery =
        std::make_unique<UpdateMasterlistQuery<>>(state.GetCurrentGame(),
                                                  state.getLanguage());
    std::unique_ptr<Query> updatePrelude = std::make_unique<UpdatePreludeQuery>(
        state.getPreludePath(),
        state.getPreludeRepositoryURL(),
        state.getPreludeRepositoryBranch());

    std::vector<
        std::pair<std::unique_ptr<Query>, void (MainWindow::*)(nlohmann::json)>>
        queriesAndHandlers;
    queriesAndHandlers.push_back(std::move(std::make_pair(
        std::move(updatePrelude), &MainWindow::handlePreludeUpdated)));
    queriesAndHandlers.push_back(
        std::move(std::make_pair(std::move(updateMasterlistQuery),
                                 &MainWindow::handleMasterlistUpdated)));

    executeBackgroundQueryChain(std::move(queriesAndHandlers), nullptr);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_sidebarPluginsView_doubleClicked(const QModelIndex& index) {
  auto cardIndex = index.siblingAtColumn(PluginItemModel::CARDS_COLUMN);
  pluginCardsView->scrollTo(cardIndex, QAbstractItemView::PositionAtTop);
}

void MainWindow::on_sidebarPluginsSelectionModel_selectionChanged(
    const QItemSelection& selected,
    const QItemSelection& deselected) {
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

void MainWindow::on_pluginItemModel_dataChanged(const QModelIndex& topLeft,
                                                const QModelIndex& bottomRight,
                                                const QList<int>& roles) {
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
      auto index = pluginItemModel->index(i, 0);
      auto pluginItem = index.data(RawDataRole).value<PluginItem>();

      if (pluginItem.name == pluginName) {
        auto plugin =
            DerivePluginMetadata(state.GetCurrentGame().GetPlugin(pluginName),
                                 state.GetCurrentGame(),
                                 state.getLanguage());

        auto data = QVariant::fromValue(PluginItem(plugin));
        pluginItemModel->setData(index, data, RawDataRole);
        break;
      }
    }

    state.DecrementUnappliedChangeCounter();

    exitEditingState();
  } catch (std::exception& e) {
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

    std::unique_ptr<Query> query =
        std::make_unique<GetConflictingPluginsQuery<>>(
            state.GetCurrentGame(),
            state.getLanguage(),
            targetPluginName.value());

    executeBackgroundQuery(
        std::move(query), &MainWindow::handleConflictsChecked, nullptr);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_filtersWidget_cardContentFilterChanged(
    CardContentFiltersState state) {
  pluginItemModel->setCardContentFiltersState(std::move(state));
}

void MainWindow::on_settingsDialog_accepted() {
  try {
    settingsDialog->recordInputValues(state);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_groupsEditor_accepted() {
  try {
    state.GetCurrentGame().SetUserGroups(groupsEditor->getUserGroups());
    state.GetCurrentGame().SaveUserMetadata();
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_searchDialog_finished(int result) { searchDialog->reset(); }

void MainWindow::on_searchDialog_textChanged(const QString& text) {
  if (text.isEmpty()) {
    proxyModel->clearSearchResults();
    return;
  }

  auto results = proxyModel->match(
      proxyModel->index(0, PluginItemModel::CARDS_COLUMN),
      ContentSearchRole,
      QVariant(text),
      -1,
      Qt::MatchFlag::MatchContains | Qt::MatchFlag::MatchWrap);

  proxyModel->setSearchResults(results);
  searchDialog->setSearchResults(results.size());
}

void MainWindow::on_searchDialog_currentResultChanged(size_t resultIndex) {
  auto sourceIndex = pluginItemModel->setCurrentSearchResult(resultIndex);
  auto proxyIndex = proxyModel->mapFromSource(sourceIndex);

  pluginCardsView->scrollTo(proxyIndex, QAbstractItemView::PositionAtTop);
}

void MainWindow::handleGameChanged(nlohmann::json result) {
  try {
    filtersWidget->resetConflictsAndGroupsFilters();
    disablePluginActions();

    handleGameDataLoaded(result);

    updateSidebarColumnWidths();
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleRefreshGameDataLoaded(nlohmann::json result) {
  try {
    handleGameDataLoaded(result);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleStartupGameDataLoaded(nlohmann::json result) {
  try {
    handleGameDataLoaded(result);

    if (state.shouldAutoSort()) {
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
    } else if (state.getLastVersion() != gui::Version::string()) {
      showFirstRunDialog();
    }
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handlePluginsManualSorted(nlohmann::json result) {
  try {
    handlePluginsSorted(result);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handlePluginsAutoSorted(nlohmann::json result) {
  try {
    handlePluginsSorted(result);

    if (actionApplySort->isVisible()) {
      actionApplySort->trigger();
    }

    if (!hasErrorMessages()) {
      on_actionQuit_triggered(false);
    }
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handlePreludeUpdated(nlohmann::json result) {
  try {
    if (!result.is_null()) {
      auto preludeInfo = result.get<FileRevisionSummary>();

      pluginItemModel->setPreludeRevision(preludeInfo);
    }
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleMasterlistUpdated(nlohmann::json result) {
  try {
    if (result.is_null()) {
      showNotification(translate("No masterlist update was necessary."));
      return;
    }

    handleGameDataLoaded(result);

    auto masterlistInfo = result.at("masterlist").get<FileRevisionSummary>();
    auto infoText = (boost::format(boost::locale::translate(
                         "Masterlist updated to revision %s.")) %
                     masterlistInfo.id)
                        .str();

    showNotification(QString::fromStdString(infoText));
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleConflictsChecked(nlohmann::json result) {
  try {
    progressDialog->reset();

    // The result has generalMessages and plugins properties, but the
    // plugins property is not an array of DerivedPluginMetadata objects.
    // Instead it's an array of objects, with each having a metadata property
    // that is a DerivedPluginMetadata object, and a conflicts property that is
    // a boolean.
    nlohmann::json gameDataLoadedResult = {
        {"plugins", nlohmann::json::array()}};
    std::vector<std::string> conflictingPluginNames;

    for (const auto& plugin : result.at("plugins")) {
      auto conflicts = plugin.at("conflicts").get<bool>();
      if (conflicts) {
        auto name = plugin.at("metadata").at("name").get<std::string>();
        conflictingPluginNames.push_back(name);
      }

      gameDataLoadedResult["plugins"].push_back(plugin.at("metadata"));
    }

    handleGameDataLoaded(gameDataLoadedResult);

    setFiltersState(filtersWidget->getPluginFiltersState(),
                    std::move(conflictingPluginNames));
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleProgressUpdate(const QString& message) {
  progressDialog->open();
  progressDialog->setLabelText(message);
}

void MainWindow::handleWorkerThreadFinished() { progressDialog->reset(); }

void MainWindow::handleGetLatestReleaseResponseFinished() {
  try {
    auto logger = getLogger();
    if (logger) {
      logger->trace(
          "Finished receiving a response for getting the latest release's tag");
    }

    auto json = readHttpResponse(qobject_cast<QNetworkReply*>(sender()));

    if (!json.has_value()) {
      return;
    }

    auto tagName = json.value()["tag_name"].toString().toStdString();

    auto comparisonResult = compareLOOTVersion(tagName);
    if (comparisonResult < 0) {
      addUpdateAvailableMessage();
      return;
    }
    if (comparisonResult > 0) {
      return;
    }

    // Versions are equal, get commit dates to compare. First get the
    // tag's commit hash.
    auto url = "https://api.github.com/repos/loot/loot/commits/tags/" + tagName;
    sendHttpRequest(url, &MainWindow::handleGetTagCommitResponseFinished);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleGetTagCommitResponseFinished() {
  try {
    auto logger = getLogger();
    if (logger) {
      logger->trace(
          "Finished receiving a response for getting the latest release tag's "
          "commit");
    }

    auto json = readHttpResponse(qobject_cast<QNetworkReply*>(sender()));

    if (!json.has_value()) {
      return;
    }

    auto commitHash = json.value()["sha"].toString().toStdString();

    if (boost::istarts_with(commitHash, gui::Version::revision)) {
      return;
    }

    auto tagCommitDate = getDateFromCommitJson(json.value(), commitHash);
    if (!tagCommitDate.has_value()) {
      addUpdateCheckErrorMessage();
      return;
    }

    auto url = "https://api.github.com/repos/loot/loot/commits/" +
               gui::Version::revision;

    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setRawHeader("Accept", "application/vnd.github.v3+json");
    auto reply = networkAccessManager.get(request);

    connect(reply, &QNetworkReply::finished, [this, tagCommitDate, reply]() {
      auto logger = getLogger();
      if (logger) {
        logger->trace(
            "Finished receiving a response for getting the LOOT build's "
            "commit");
      }

      auto json = readHttpResponse(reply);

      if (!json.has_value()) {
        return;
      }

      auto buildCommitDate =
          getDateFromCommitJson(json.value(), gui::Version::revision);
      if (!buildCommitDate.has_value()) {
        addUpdateCheckErrorMessage();
        return;
      }

      if (tagCommitDate.value() > buildCommitDate.value()) {
        logger->info("Tag date: {}, build date: {}",
                     tagCommitDate.value().toString().toStdString(),
                     buildCommitDate.value().toString().toStdString());
        addUpdateAvailableMessage();
      } else if (logger) {
        logger->info("No LOOT update is available.");
      }
    });
    connect(reply,
            &QNetworkReply::errorOccurred,
            this,
            &MainWindow::handleUpdateCheckNetworkError);
    connect(reply,
            &QNetworkReply::sslErrors,
            this,
            &MainWindow::handleUpdateCheckSSLError);
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleUpdateCheckNetworkError(
    QNetworkReply::NetworkError error) {
  try {
    auto reply = qobject_cast<QIODevice*>(sender());

    auto logger = getLogger();
    if (logger) {
      logger->error(
          "Error while checking for LOOT updates: error code is {}, "
          "description "
          "is: {}",
          error,
          reply->errorString().toStdString());
    }

    addUpdateCheckErrorMessage();
  } catch (std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleUpdateCheckSSLError(const QList<QSslError>& errors) {
  try {
    auto logger = getLogger();
    for (const auto& error : errors) {
      if (logger) {
        logger->error("Error while checking for LOOT updates: {}",
                      error.errorString().toStdString());
      }
    }

    addUpdateCheckErrorMessage();
  } catch (std::exception& e) {
    handleException(e);
  }
}

ResultDemultiplexer::ResultDemultiplexer(MainWindow* target) :
    QObject(), signalCounter(0), target(target) {}

void ResultDemultiplexer::addHandler(
    void (MainWindow::*handler)(nlohmann::json)) {
  handlers.push_back(handler);
}

void ResultDemultiplexer::onResultReady(nlohmann::json result) {
  if (signalCounter > handlers.size() - 1) {
    throw std::runtime_error("Received more signals than expected");
  }

  auto handler = handlers[signalCounter];
  (target->*handler)(result);
  signalCounter += 1;
}
}
