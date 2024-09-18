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

#include <fmt/base.h>

#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QStyleHints>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTextEdit>
#include <boost/algorithm/string.hpp>

#include "gui/backup.h"
#include "gui/qt/helpers.h"
#include "gui/qt/icon_factory.h"
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
#include "gui/query/types/get_game_data_query.h"
#include "gui/query/types/get_overlapping_plugins_query.h"
#include "gui/query/types/sort_plugins_query.h"
#include "gui/version.h"

namespace {
using loot::GameId;
using loot::LootState;
using loot::translate;

void showAmbiguousLoadOrderSetWarning(QWidget* parent, const LootState& state) {
  const auto maybeSTestFile =
      state.GetCurrentGame().GetSettings().Id() == GameId::fo4 ||
      state.GetCurrentGame().GetSettings().Id() == GameId::fo4vr ||
      state.GetCurrentGame().GetSettings().Id() == GameId::starfield;

  const auto message =
      maybeSTestFile
          ? translate(
                "LOOT could not unambiguously set the load order. "
                "This may be due to the presence of one or more sTestFile "
                "properties in the game's ini files. Remove all such "
                "properties and then restart LOOT.")
          : translate("LOOT could not unambiguously set the load order.");

  QMessageBox::warning(
      parent, translate("Ambiguous load order detected"), message);
}
}

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

int calculateSidebarIndexSectionWidth(bool gameSupportsLightPlugins) {
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
  if (gameSupportsLightPlugins) {
    const auto prefixWidth =
        static_cast<int>(fontMetrics.size(Qt::TextSingleLine, "FE ").width());
    return prefixWidth + 3 * static_cast<int>(maxCharWidth) + paddingWidth;
  }

  return 2 * static_cast<int>(maxCharWidth) + paddingWidth;
}

LootSettings::WindowPosition getWindowPosition(QWidget& window) {
  LootSettings::WindowPosition position;

  // Use the normal geometry because that gives the size and position as if
  // the window is neither maximised or minimised. This is useful because if
  // LOOT is opened into a maximised state, unmaximising it should restore
  // the previous unmaximised size and position, and that can't be done
  // without recording the normal geometry.
  const auto geometry = window.normalGeometry();
  position.left = geometry.x();
  position.top = geometry.y();
  position.right = position.left + geometry.width();
  position.bottom = position.top + geometry.height();
  position.maximised = window.isMaximized();

  return position;
}

void setWindowPosition(QWidget& window,
                       const LootSettings::WindowPosition& position) {
  const auto width = position.right - position.left;
  const auto height = position.bottom - position.top;

  const auto geometry = QRect(position.left, position.top, width, height);
  const auto topLeft = geometry.topLeft();

  if (QGuiApplication::screenAt(topLeft) == nullptr) {
    // No screen exists at the old position, just leave the Window at the
    // default position Qt gives it so that it isn't positioned off-screen.
    auto logger = getLogger();
    if (logger) {
      logger->warn(
          "Could not restore window position because no screen exists "
          "at the coordinates ({}, {})",
          topLeft.x(),
          topLeft.y());
    }
    window.resize(width, height);
  } else {
    window.setGeometry(geometry);
  }
}

MainWindow::MainWindow(LootState& state, QWidget* parent) :
    QMainWindow(parent), state(state) {
  qRegisterMetaType<QueryResult>("QueryResult");
  qRegisterMetaType<std::string>("std::string");

  setupUi();
  refreshGamesDropdown();

  qApp->connect(qApp,
                &QGuiApplication::applicationStateChanged,
                this,
                [this](Qt::ApplicationState) {
                  const auto cardDelegate = qobject_cast<CardDelegate*>(
                      this->pluginCardsView->itemDelegate());
                  if (cardDelegate != nullptr) {
                    cardDelegate->refreshStyling();
                  }
                });
}

void MainWindow::initialise() {
  try {
    themes = findThemes(state.getThemesPath());

    if (state.getSettings().getLastVersion() != gui::Version::string()) {
      showFirstRunDialog();
    }

    if (state.HasCurrentGame()) {
      state.initCurrentGame();
    }

    auto initMessages = state.getInitMessages();
    const auto initHasErrored =
        std::any_of(initMessages.begin(),
                    initMessages.end(),
                    [](const SourcedMessage& message) {
                      return message.type == MessageType::error;
                    });
    pluginItemModel->setGeneralMessages(std::move(initMessages));

    if (initHasErrored) {
      return;
    }

    const auto& filters = state.getSettings().getFilters();
    filtersWidget->setGameId(state.GetCurrentGame().GetSettings().Id());
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

void MainWindow::applyTheme() {
  // Apply theme.
  bool loadingDefault = state.getSettings().getTheme() == "default";

  auto styleSheet = loot::loadStyleSheet(state.getThemesPath(),
                                         state.getSettings().getTheme());
  if (!styleSheet.has_value()) {
    // Fall back to the default theme.
    styleSheet = loot::loadStyleSheet(state.getThemesPath(), "default");
    loadingDefault = true;
  }

  const auto logger = getLogger();
  if (logger) {
    logger->debug("Current style name is {}",
                  qApp->style()->name().toStdString());
  }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  // Don't load default-dark when Qt is using the windowsvista style,
  // as that ignores the system color scheme.
  if (loadingDefault && qApp->style()->name() != "windowsvista" &&
      QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
    // Load the default-dark theme instead as it gives better results.
    if (logger) {
      logger->debug(
          "Loading default-dark theme instead of the default theme as a dark "
          "colour scheme has been detected");
    }
    styleSheet = loot::loadStyleSheet(state.getThemesPath(), "default-dark");
  }
#endif

  if (styleSheet.has_value()) {
    qApp->setStyleSheet(styleSheet.value());

    qApp->style()->polish(qApp);

    const auto cardDelegate =
        qobject_cast<CardDelegate*>(pluginCardsView->itemDelegate());
    cardDelegate->refreshStyling();
  }
}

void MainWindow::setupUi() {
  setWindowIcon(QIcon(":/icons/loot.svg"));

  auto lastWindowPosition = state.getSettings().getMainWindowPosition();
  if (lastWindowPosition.has_value()) {
    setWindowPosition(*this, lastWindowPosition.value());
  } else {
    static constexpr int DEFAULT_WIDTH = 1024;
    static constexpr int DEFAULT_HEIGHT = 768;
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  }

  const auto groupsEditorWindowPosition =
      state.getSettings().getGroupsEditorWindowPosition();
  if (groupsEditorWindowPosition.has_value()) {
    setWindowPosition(*groupsEditor, groupsEditorWindowPosition.value());

    if (groupsEditorWindowPosition.value().maximised) {
      groupsEditor->setWindowState(groupsEditor->windowState() |
                                   Qt::WindowMaximized);
    }
  }

  // Set up status bar.
  setStatusBar(statusbar);

  setupMenuBar();
  setupToolBar();

  settingsDialog->setObjectName("settingsDialog");
  searchDialog->setObjectName("searchDialog");
  sidebarPluginsView->setObjectName("sidebarPluginsView");

  toolBox->addItem(sidebarPluginsView, QString("P&lugins"));

  filtersWidget->setObjectName("filtersWidget");

  toolBox->addItem(filtersWidget, QString("F&ilters"));

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

  const auto progressBar = new QProgressBar();
  progressBar->setTextVisible(false);
  progressBar->setMinimum(0);
  progressBar->setMaximum(0);

  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setCancelButton(nullptr);
  progressDialog->setBar(progressBar);
  progressDialog->reset();

  pluginItemModel->setObjectName("pluginItemModel");

  proxyModel->setObjectName("proxyModel");
  proxyModel->setSourceModel(pluginItemModel);

  groupsEditor->setObjectName("groupsEditor");

  setupViews();

  translateUi();
  setIcons();

  disableGameActions();

  QMetaObject::connectSlotsByName(this);

  connect(this,
          &MainWindow::normalIconColorChanged,
          this,
          &MainWindow::handleIconColorChanged);
  connect(this,
          &MainWindow::disabledIconColorChanged,
          this,
          &MainWindow::handleIconColorChanged);
  connect(this,
          &MainWindow::selectedIconColorChanged,
          this,
          &MainWindow::handleIconColorChanged);
  connect(this,
          &MainWindow::selectedSidebarPluginTextColorChanged,
          this,
          &MainWindow::handleSidebarTextColorChanged);
  connect(this,
          &MainWindow::unselectedSidebarPluginGroupColorChanged,
          this,
          &MainWindow::handleSidebarTextColorChanged);
  connect(this,
          &MainWindow::linkColorChanged,
          this,
          &MainWindow::handleLinkColorChanged);

#if !defined(_WIN32) && QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  // Only run this on Linux, because it intentionally has no effect on Windows.
  connect(QGuiApplication::styleHints(),
          &QStyleHints::colorSchemeChanged,
          this,
          &MainWindow::handleColorSchemeChanged);
#endif
}

void MainWindow::setupMenuBar() {
  // Create actions.
  actionUpdateMasterlists->setObjectName("actionUpdateMasterlists");

  actionSettings->setObjectName("actionSettings");

  actionBackupData->setObjectName("actionBackupData");

  actionQuit->setObjectName("actionQuit");

  actionViewDocs->setObjectName("actionViewDocs");
  actionViewDocs->setShortcut(QKeySequence::HelpContents);

  actionOpenFAQs->setObjectName("actionOpenFAQs");

  actionOpenLOOTDataFolder->setObjectName("actionOpenLOOTDataFolder");

  actionJoinDiscordServer->setObjectName("actionJoinDiscordServer");

  actionAbout->setObjectName("actionAbout");

  actionOpenGroupsEditor->setObjectName("actionOpenGroupsEditor");

  actionSearch->setObjectName("actionSearch");
  actionSearch->setShortcut(QKeySequence::Find);

  actionCopyLoadOrder->setObjectName("actionCopyLoadOrder");

  actionCopyContent->setObjectName("actionCopyContent");

  actionRefreshContent->setObjectName("actionRefreshContent");
  actionRefreshContent->setShortcut(QKeySequence::Refresh);

  actionRedatePlugins->setObjectName("actionRedatePlugins");

  actionFixAmbiguousLoadOrder->setObjectName("actionFixAmbiguousLoadOrder");

  actionClearAllUserMetadata->setObjectName("actionClearAllUserMetadata");

  actionCopyPluginName->setObjectName("actionCopyPluginName");

  actionCopyCardContent->setObjectName("actionCopyCardContent");

  actionCopyMetadata->setObjectName("actionCopyMetadata");

  actionEditMetadata->setObjectName("actionEditMetadata");
  actionEditMetadata->setShortcut(QString("Ctrl+E"));

  actionClearMetadata->setObjectName("actionClearMetadata");

  // Create menu bar.
  setMenuBar(menubar);

  menubar->addAction(menuFile->menuAction());
  menubar->addAction(menuGame->menuAction());
  menubar->addAction(menuPlugin->menuAction());
  menubar->addAction(menuHelp->menuAction());
  menuFile->addAction(actionSettings);
  menuFile->addSeparator();
  menuFile->addAction(actionUpdateMasterlists);
  menuFile->addSeparator();
  menuFile->addAction(actionBackupData);
  menuFile->addAction(actionOpenLOOTDataFolder);
  menuFile->addSeparator();
  menuFile->addAction(actionQuit);
  menuGame->addAction(actionOpenGroupsEditor);
  menuGame->addSeparator();
  menuGame->addAction(actionSort);
  menuGame->addAction(actionUpdateMasterlist);
  menuGame->addAction(actionApplySort);
  menuGame->addAction(actionDiscardSort);
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
  menuHelp->addAction(actionOpenFAQs);
  menuHelp->addAction(actionJoinDiscordServer);
  menuHelp->addSeparator();
  menuHelp->addAction(actionAbout);
}

void MainWindow::setupToolBar() {
  // Create actions.
  actionSort->setObjectName("actionSort");

  actionUpdateMasterlist->setObjectName("actionUpdateMasterlist");

  actionApplySort->setObjectName("actionApplySort");
  actionApplySort->setVisible(false);

  actionDiscardSort->setObjectName("actionDiscardSort");
  actionDiscardSort->setVisible(false);

  // Create toolbar.
  toolBar->setMovable(false);
  toolBar->setFloatable(false);
  toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  addToolBar(Qt::TopToolBarArea, toolBar);

  gameComboBox->setObjectName("gameComboBox");
  gameComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  toolBar->addWidget(gameComboBox);

  toolBar->addAction(actionSort);
  toolBar->addAction(actionUpdateMasterlist);
  toolBar->addAction(actionApplySort);
  toolBar->addAction(actionDiscardSort);
  toolBar->addAction(actionSearch);
}

void MainWindow::setupViews() {
  sidebarPluginsView->setModel(proxyModel);
  sidebarPluginsView->setTabKeyNavigation(false);
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
  verticalHeader->setMaximumSectionSize(getSidebarRowHeight(true));
  verticalHeader->setMinimumSectionSize(getSidebarRowHeight(false));
  verticalHeader->setDefaultSectionSize(getSidebarRowHeight(false));
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

  cardSizingCache.update(pluginItemModel);

  pluginCardsView->setItemDelegate(
      new CardDelegate(pluginCardsView, cardSizingCache));

  // Enable the right-click Plugin context menu.
  pluginCardsView->setContextMenuPolicy(Qt::CustomContextMenu);

  // Plugin selection handling needs to be set up after the model has been
  // set, as before then there is no selection model.
  connect(sidebarPluginsView->selectionModel(),
          &QItemSelectionModel::selectionChanged,
          this,
          &MainWindow::handleSidebarPluginsSelectionChanged);

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
  /* translators: This string is also an action in the Game menu. */
  actionSort->setText(translate("&Sort Plugins"));
  /* translators: This string is also an action in the Game menu. */
  actionUpdateMasterlist->setText(translate("Update &Masterlist"));
  /* translators: This string is also an action in the Game menu. */
  actionApplySort->setText(translate("&Apply Sorted Load Order"));
  /* translators: This string is also an action in the Game menu. */
  actionDiscardSort->setText(translate("&Discard Sorted Load Order"));

  // Translate menu bar items.
  /* translators: The mnemonic in this string shouldn't conflict with other
     menus or sidebar sections. */
  menuFile->setTitle(translate("&File"));
  /* translators: This string is an action in the File menu. */
  actionSettings->setText(translate("&Settings…"));
  /* translators: This string is an action in the File menu. */
  actionUpdateMasterlists->setText(translate("&Update All Masterlists"));
  /* translators: This string is an action in the File menu. */
  actionBackupData->setText(translate("&Backup LOOT Data"));
  /* translators: This string is an action in the File menu. */
  actionOpenLOOTDataFolder->setText(translate("&Open LOOT Data Folder"));
  /* translators: This string is an action in the File menu. */
  actionQuit->setText(translate("&Quit"));

  /* translators: The mnemonic in this string shouldn't conflict with other
     menus or sidebar sections. */
  menuGame->setTitle(translate("&Game"));
  /* translators: This string is an action in the Game menu. */
  actionOpenGroupsEditor->setText(translate("&Edit Groups…"));
  /* translators: This string is an action in the Game menu. */
  actionSearch->setText(translate("Searc&h Cards…"));
  /* translators: This string is an action in the Game menu. */
  actionCopyLoadOrder->setText(translate("Copy &Load Order"));
  /* translators: This string is an action in the Game menu. */
  actionCopyContent->setText(translate("&Copy Content"));
  /* translators: This string is an action in the Game menu. */
  actionRefreshContent->setText(translate("&Refresh Content"));
  /* translators: This string is an action in the Game menu. */
  actionRedatePlugins->setText(translate("Redate &Plugins…"));
  /* translators: This string is an action in the Game menu. */
  actionFixAmbiguousLoadOrder->setText(translate("&Fix Ambiguous Load Order"));
  /* translators: This string is an action in the Game menu. */
  actionClearAllUserMetadata->setText(translate("Clear All &User Metadata…"));

  /* translators: The mnemonic in this string shouldn't conflict with other
     menus or sidebar sections. */
  menuPlugin->setTitle(translate("&Plugin"));
  /* translators: This string is an action in the Plugin menu. */
  actionCopyPluginName->setText(translate("Copy &Plugin Name"));
  /* translators: This string is an action in the Plugin menu. */
  actionCopyCardContent->setText(translate("Copy &Card Content"));
  /* translators: This string is an action in the Plugin menu. */
  actionCopyMetadata->setText(translate("Copy &Metadata"));
  /* translators: This string is an action in the Plugin menu. */
  actionEditMetadata->setText(translate("&Edit Metadata…"));
  /* translators: This string is an action in the Plugin menu. */
  actionClearMetadata->setText(translate("Clear &User Metadata…"));

  /* translators: The mnemonic in this string shouldn't conflict with other
     menus or sidebar sections. */
  menuHelp->setTitle(translate("&Help"));
  /* translators: This string is an action in the Help menu. */
  actionViewDocs->setText(translate("&View Documentation"));
  /* translators: This string is an action in the Help menu. */
  actionOpenFAQs->setText(translate("&Open FAQs"));
  /* translators: This string is an action in the Help menu. */
  actionJoinDiscordServer->setText(translate("&Join Discord Server"));
  /* translators: This string is an action in the Help menu. */
  actionAbout->setText(translate("&About"));

  // Translate sidebar.
  /* translators: The mnemonic in this string shouldn't conflict with other
     menus or sidebar sections. */
  toolBox->setItemText(0, translate("P&lugins"));
  /* translators: The mnemonic in this string shouldn't conflict with other
     menus or sidebar sections. */
  toolBox->setItemText(1, translate("F&ilters"));
}

void MainWindow::setIcons() {
  actionSettings->setIcon(IconFactory::getSettingsIcon());
  actionUpdateMasterlists->setIcon(IconFactory::getUpdateMasterlistIcon());
  actionBackupData->setIcon(IconFactory::getArchiveIcon());
  actionQuit->setIcon(IconFactory::getQuitIcon());
  actionViewDocs->setIcon(IconFactory::getViewDocsIcon());
  actionOpenFAQs->setIcon(IconFactory::getOpenFAQsIcon());
  actionOpenLOOTDataFolder->setIcon(IconFactory::getOpenLOOTDataFolderIcon());
  actionJoinDiscordServer->setIcon(IconFactory::getJoinDiscordServerIcon());
  actionAbout->setIcon(IconFactory::getAboutIcon());
  actionOpenGroupsEditor->setIcon(IconFactory::getOpenGroupsEditorIcon());
  actionSearch->setIcon(IconFactory::getSearchIcon());
  actionCopyLoadOrder->setIcon(IconFactory::getCopyLoadOrderIcon());
  actionCopyContent->setIcon(IconFactory::getCopyContentIcon());
  actionRefreshContent->setIcon(IconFactory::getRefreshIcon());
  actionRedatePlugins->setIcon(IconFactory::getRedateIcon());
  actionFixAmbiguousLoadOrder->setIcon(IconFactory::getFixIcon());
  actionClearAllUserMetadata->setIcon(IconFactory::getDeleteIcon());
  actionCopyPluginName->setIcon(IconFactory::getCopyContentIcon());
  actionCopyCardContent->setIcon(IconFactory::getCopyContentIcon());
  actionCopyMetadata->setIcon(IconFactory::getCopyMetadataIcon());
  actionEditMetadata->setIcon(IconFactory::getEditIcon());
  actionClearMetadata->setIcon(IconFactory::getDeleteIcon());

  actionSort->setIcon(IconFactory::getSortIcon());
  actionUpdateMasterlist->setIcon(IconFactory::getUpdateMasterlistIcon());
  actionApplySort->setIcon(IconFactory::getApplySortIcon());
  actionDiscardSort->setIcon(IconFactory::getDiscardSortIcon());
}

void MainWindow::enableGameActions() {
  menuGame->setEnabled(true);
  actionSort->setEnabled(true);
  actionUpdateMasterlist->setEnabled(true);
  actionSearch->setEnabled(true);

  const auto enableRedatePlugins =
      ShouldAllowRedating(state.GetCurrentGame().GetSettings().Type());
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
  actionUpdateMasterlists->setDisabled(true);
  actionOpenGroupsEditor->setDisabled(true);
  actionRefreshContent->setDisabled(true);
  actionClearAllUserMetadata->setDisabled(true);
  actionEditMetadata->setDisabled(true);
  actionClearMetadata->setDisabled(true);
  gameComboBox->setDisabled(true);
  actionUpdateMasterlist->setDisabled(true);
  actionSort->setDisabled(true);

  sidebarPluginsView->verticalHeader()->setDefaultSectionSize(
      getSidebarRowHeight(true));
}

void MainWindow::exitEditingState() {
  actionSettings->setEnabled(true);
  actionUpdateMasterlists->setEnabled(true);
  actionOpenGroupsEditor->setEnabled(true);
  actionRefreshContent->setEnabled(true);
  actionClearAllUserMetadata->setEnabled(true);
  actionEditMetadata->setEnabled(true);
  actionClearMetadata->setEnabled(true);
  gameComboBox->setEnabled(true);
  actionUpdateMasterlist->setEnabled(true);
  actionSort->setEnabled(true);

  sidebarPluginsView->verticalHeader()->setDefaultSectionSize(
      getSidebarRowHeight(false));
}

void MainWindow::enterSortingState() {
  actionUpdateMasterlist->setVisible(false);
  actionSort->setVisible(false);

  actionApplySort->setVisible(true);
  actionDiscardSort->setVisible(true);

  actionSettings->setDisabled(true);
  actionUpdateMasterlists->setDisabled(true);
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
  actionUpdateMasterlists->setDisabled(false);
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

void MainWindow::updateCounts(
    const std::vector<SourcedMessage>& generalMessages,
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
  const auto preludeInfo = getFileRevisionSummary(state.getPreludePath(),
                                                  FileType::MasterlistPrelude);
  auto initMessages = state.getInitMessages();

  if (!state.HasCurrentGame()) {
    pluginItemModel->setGeneralInformation(
        false, false, FileRevisionSummary(), preludeInfo, initMessages);
    return;
  }

  const auto masterlistInfo = getFileRevisionSummary(
      state.GetCurrentGame().MasterlistPath(), FileType::Masterlist);

  const auto gameMessages = state.GetCurrentGame().GetMessages(
      state.getSettings().getLanguage(),
      state.getSettings().isWarnOnCaseSensitiveGamePathsEnabled());
  initMessages.insert(
      initMessages.end(), gameMessages.begin(), gameMessages.end());

  pluginItemModel->setGeneralInformation(
      state.GetCurrentGame().SupportsLightPlugins(),
      state.GetCurrentGame().SupportsMediumPlugins(),
      masterlistInfo,
      preludeInfo,
      initMessages);
}

void MainWindow::updateGeneralMessages() {
  auto initMessages = state.getInitMessages();
  auto gameMessages = state.GetCurrentGame().GetMessages(
      state.getSettings().getLanguage(),
      state.getSettings().isWarnOnCaseSensitiveGamePathsEnabled());
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
  const auto gameSupportsLightPlugins =
      state.HasCurrentGame() ? state.GetCurrentGame().SupportsLightPlugins()
                             : false;
  const auto indexSectionWidth =
      calculateSidebarIndexSectionWidth(gameSupportsLightPlugins);

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
  refreshSearch();
}

void MainWindow::setFiltersState(
    PluginFiltersState&& filtersState,
    std::vector<std::string>&& overlappingPluginNames) {
  proxyModel->setFiltersState(std::move(filtersState),
                              std::move(overlappingPluginNames));

  updateCounts(pluginItemModel->getGeneralMessages(),
               pluginItemModel->getPluginItems());
  refreshSearch();
}

void MainWindow::refreshSearch() {
  on_searchDialog_textChanged(searchDialog->getSearchText());
}

void MainWindow::refreshPluginRawData(const std::string& pluginName) {
  const auto loadOrder = state.GetCurrentGame().GetLoadOrder();

  for (int i = 1; i < pluginItemModel->rowCount(); i += 1) {
    const auto index = pluginItemModel->index(i, 0);
    const auto pluginItem = index.data(RawDataRole).value<PluginItem>();

    if (pluginItem.name == pluginName) {
      const auto& plugin = *state.GetCurrentGame().GetPlugin(pluginName);
      const auto newPluginItem = PluginItem(
          state.GetCurrentGame().GetSettings().Id(),
          plugin,
          state.GetCurrentGame(),
          state.GetCurrentGame().GetActiveLoadOrderIndex(plugin, loadOrder),
          state.GetCurrentGame().IsPluginActive(plugin.GetName()),
          state.getSettings().getLanguage());

      const auto indexData = QVariant::fromValue(newPluginItem);
      pluginItemModel->setData(index, indexData, RawDataRole);
      break;
    }
  }
}

bool MainWindow::hasErrorMessages() const {
  const auto counters = GeneralInformationCounters(
      pluginItemModel->getGeneralMessages(), pluginItemModel->getPluginItems());

  return counters.errors != 0;
}

void MainWindow::sortPlugins(bool isAutoSort) {
  std::vector<Task*> updateTasks;

  if (state.getSettings().isMasterlistUpdateBeforeSortEnabled()) {
    handleProgressUpdate(translate("Updating and parsing masterlist…"));

    const auto preludeTask = new UpdatePreludeTask(state);

    updateTasks.push_back(preludeTask);

    const auto masterlistTask =
        new UpdateMasterlistTask(state.GetCurrentGame());

    updateTasks.push_back(masterlistTask);
  }

  auto progressUpdater = new ProgressUpdater();
  connect(progressUpdater,
          &ProgressUpdater::progressUpdate,
          this,
          &MainWindow::handleProgressUpdate);

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

  auto updatesFuture =
      whenAllTasks(updateTasks)
          .then(this,
                [this](const QList<QFuture<QueryResult>> futures) {
                  std::vector<QueryResult> results;
                  for (const auto& future : futures) {
                    results.push_back(future.result());
                  }

                  handleMasterlistUpdated(results);
                })
          .onFailed(this,
                    [this](const std::exception& e) { handleError(e.what()); })
          .then(this, [sortTask]() { executeBackgroundTask(sortTask); });

  auto sortFuture =
      taskFuture(sortTask)
          .then(this,
                [this, sortHandler](QueryResult result) {
                  (this->*sortHandler)(result);
                })
          .onFailed(this,
                    [this](const std::exception& e) { handleError(e.what()); })
          .then(this, [this, progressUpdater]() {
            progressDialog->reset();
            progressUpdater->deleteLater();
          });

  executeConcurrentBackgroundTasks(updateTasks, updatesFuture);
}

void MainWindow::showFirstRunDialog() {
  auto zipPath = createBackup();

  std::string textTemplate = R"(
<p>{}</p>
<p>{}</p>
<ul>
  <li>{}</li>
  <li>{}</li>
  <li>{}</li>
</ul>
<p>{}</p>
)";

  std::string paragraph1;
  if (zipPath.has_value()) {
    auto zipPathString = zipPath.value().u8string();
    auto link = "<pre><a href=\"file:" + zipPathString +
                "\" style=\"white-space: nowrap\">" + zipPathString +
                "</a></pre>";

    paragraph1 = fmt::format(
        boost::locale::translate(
            "This appears to be the first time you have run LOOT v{0}. Your "
            "current LOOT data has been backed up to: {1}")
            .str(),
        gui::Version::string(),
        link);
  } else {
    paragraph1 = fmt::format(
        boost::locale::translate(
            "This appears to be the first time you have run LOOT v{0}.")
            .str(),
        gui::Version::string());
  }

  auto paragraph2 =
      boost::locale::translate(
          "Here are some tips to help you get started with the interface.")
          .str();

  auto listItem1 =
      boost::locale::translate(
          "CRCs are only displayed after plugins have been loaded, either by "
          "overlap filtering, or by sorting.")
          .str();

  auto listItem2 =
      boost::locale::translate(
          "Plugins can be drag and dropped from the sidebar into the metadata "
          "editor's \"load after\", \"requirements\" and \"incompatibility\" "
          "tables.")
          .str();

  auto listItem3 = boost::locale::translate(
                       "Some features are disabled while the metadata editor "
                       "is open, or while there is a sorted load order that "
                       "has not been applied or discarded.")
                       .str();

  auto paragraph3 = fmt::format(
      boost::locale::translate(
          "LOOT is free, but if you want to show your appreciation with some "
          "money, donations may be made to WrinklyNinja (LOOT's creator and "
          "main developer) using {0}.")
          .str(),
      "<a href=\"https://www.paypal.me/OliverHamlet\">PayPal</a>");

  std::string text = fmt::format(textTemplate,
                                 paragraph1,
                                 paragraph2,
                                 listItem1,
                                 listItem2,
                                 listItem3,
                                 paragraph3);

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
    auto questionText =
        fmt::format(boost::locale::translate(
                        "You have not yet applied or cancelled your {0}. "
                        "Are you sure you want to quit?")
                        .str(),
                    changeType.str());

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
    const auto position = getWindowPosition(*this);

    state.getSettings().storeMainWindowPosition(position);

    const auto groupsEditorPosition = getWindowPosition(*groupsEditor);

    state.getSettings().storeGroupsEditorWindowPosition(groupsEditorPosition);
  } catch (const std::exception& e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to record window positions: {}", e.what());
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
  if (progressUpdater != nullptr) {
    connect(progressUpdater,
            &ProgressUpdater::progressUpdate,
            this,
            &MainWindow::handleProgressUpdate);
  }

  loot::executeBackgroundQuery(std::move(query))
      .then(this,
            [this, onComplete](QueryResult result) {
              (this->*onComplete)(result);
            })
      .onFailed(this, [this](std::exception& e) { handleError(e.what()); })
      .then(this, [progressUpdater]() {
        if (progressUpdater) {
          progressUpdater->deleteLater();
        }
      });
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
  filtersWidget->showCreationClubPluginsFilter(
      state.GetCurrentGame().HadCreationClub());

  pluginEditorWidget->setBashTagCompletions(
      state.GetCurrentGame().GetKnownBashTags());

  enableGameActions();
}

bool MainWindow::handlePluginsSorted(QueryResult result) {
  filtersWidget->resetOverlapAndGroupsFilters();

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

void MainWindow::refreshGamesDropdown() {
  const auto installedGames = state.GetInstalledGameFolderNames();

  gameComboBox->clear();

  for (const auto& gameSettings : state.getSettings().getGameSettings()) {
    auto installedGame = std::find(installedGames.cbegin(),
                                   installedGames.cend(),
                                   gameSettings.FolderName());

    if (installedGame != installedGames.cend()) {
      gameComboBox->addItem(QString::fromStdString(gameSettings.Name()),
                            QString::fromStdString(gameSettings.FolderName()));
    }
  }

  if (state.HasCurrentGame()) {
    gameComboBox->setCurrentText(
        QString::fromStdString(state.GetCurrentGame().GetSettings().Name()));
  }
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

void MainWindow::on_actionUpdateMasterlists_triggered() {
  try {
    handleProgressUpdate(translate("Updating and parsing masterlist…"));

    const auto preludeSource = state.getSettings().getPreludeSource();
    const auto preludePath = state.getPreludePath();

    std::vector<Task*> tasks;

    const auto preludeTask = new UpdatePreludeTask(state);

    tasks.push_back(preludeTask);

    for (const auto& settings : state.getSettings().getGameSettings()) {
      // Masterlist update assumes that the game folder exists, so ensure that.
      InitLootGameFolder(state.getLootDataPath(), settings);

      const auto task = new UpdateMasterlistTask(
          settings.FolderName(),
          settings.MasterlistSource(),
          GetMasterlistPath(state.getLootDataPath(), settings));

      tasks.push_back(task);
    }

    handleProgressUpdate(translate("Updating all masterlists…"));

    auto whenAll = whenAllTasks(tasks)
                       .then(this,
                             [this](const QList<QFuture<QueryResult>> futures) {
                               std::vector<QueryResult> results;
                               for (const auto& future : futures) {
                                 results.push_back(future.result());
                               }

                               handleMasterlistsUpdated(results);
                             })
                       .onFailed(this, [this](const std::exception& e) {
                         handleError(e.what());
                       });

    executeConcurrentBackgroundTasks(tasks, whenAll);
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
      auto message = fmt::format(
          boost::locale::translate("Your LOOT data has been backed up to: {0}")
              .str(),
          link);

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

    const auto groupNodePositions =
        LoadGroupNodePositions(state.GetCurrentGame().GroupNodePositionsPath());

    groupsEditor->setGroups(state.GetCurrentGame().GetMasterlistGroups(),
                            state.GetCurrentGame().GetUserGroups(),
                            installedPluginGroups,
                            groupNodePositions);

    groupsEditor->show();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionSearch_triggered() { searchDialog->show(); }

void MainWindow::on_actionCopyLoadOrder_triggered() {
  try {
    const auto text = GetLoadOrderAsTextTable(
        state.GetCurrentGame(), state.GetCurrentGame().GetLoadOrder());

    CopyToClipboard(text);

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

    if (state.GetCurrentGame().IsLoadOrderAmbiguous()) {
      showAmbiguousLoadOrderSetWarning(this, state);
    } else {
      actionFixAmbiguousLoadOrder->setEnabled(false);
    }
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
    const auto selectedPluginName = getSelectedPlugin().name;

    const auto text =
        GetMetadataAsBBCodeYaml(state.GetCurrentGame(), selectedPluginName);

    CopyToClipboard(text);

    const auto logger = getLogger();
    if (logger) {
      logger->debug("Exported userlist metadata text for \"{}\": {}",
                    selectedPluginName,
                    text);
    }

    const auto message = fmt::format(
        boost::locale::translate(
            "The metadata for \"{0}\" has been copied to the clipboard.")
            .str(),
        selectedPluginName);

    showNotification(QString::fromStdString(message));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionCopyPluginName_triggered() {
  try {
    const auto selectedPluginName = getSelectedPlugin().name;

    CopyToClipboard(selectedPluginName);

    const auto text = fmt::format(
        boost::locale::translate(
            "The plugin name \"{0}\" has been copied to the clipboard.")
            .str(),
        selectedPluginName);

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

    auto text = fmt::format(
        boost::locale::translate(
            "The card content for \"{0}\" has been copied to the clipboard.")
            .str(),
        selectedPlugin.name);

    showNotification(QString::fromStdString(text));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionClearMetadata_triggered() {
  try {
    auto selectedPluginName = getSelectedPlugin().name;

    auto questionText = fmt::format(
        boost::locale::translate("Are you sure you want to clear all existing "
                                 "user-added metadata from \"{0}\"?")
            .str(),
        selectedPluginName);

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
        fmt::format(boost::locale::translate(
                        "The user-added metadata for \"{0}\" has been cleared.")
                        .str(),
                    selectedPluginName);

    showNotification(QString::fromStdString(notificationText));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionViewDocs_triggered() {
  try {
    const auto logger = getLogger();
    if (logger) {
      logger->trace("Opening LOOT's readme.");
    }

    const auto readmePath = state.getReadmePath();
    const auto canonicalPath =
        std::filesystem::canonical(readmePath / "index.html");
    const auto canonicalReadmePath = std::filesystem::canonical(readmePath);

    if (!boost::starts_with(canonicalPath.u8string(),
                            canonicalReadmePath.u8string())) {
      throw std::runtime_error(
          "Attempted to open readme file outside of recognised readme "
          "directory.");
    }

    OpenInDefaultApplication(canonicalPath);
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionOpenFAQs_triggered() {
  try {
    const auto logger = getLogger();
    if (logger) {
      logger->trace("Opening LOOT's FAQs.");
    }

    QDesktopServices::openUrl(
        QUrl("https://loot.github.io/docs/help/LOOT-FAQs"));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_actionOpenLOOTDataFolder_triggered() {
  try {
    const auto logger = getLogger();
    if (logger) {
      logger->trace("Opening LOOT's local appdata folder.");
    }

    OpenInDefaultApplication(state.getLogPath().parent_path());
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
<p>{}</p>
<p>{}</p>
<p><a href="https://loot.github.io">https://loot.github.io</a></p>
<p>{}</p>
<blockquote>
  LOOT is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

  LOOT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with LOOT. If not, see &lt;https://www.gnu.org/licenses/&gt;.
</blockquote>
)";

    auto paragraph1 =
        fmt::format(boost::locale::translate("Version {0} (build {1})").str(),
                    gui::Version::string(),
                    gui::Version::revision);

    auto paragraph2 =
        boost::locale::translate(
            "Load order optimisation for Starfield, Morrowind, Oblivion, "
            "Nehrim, Skyrim, Enderal, Skyrim Special Edition, Enderal Special "
            "Edition, Skyrim VR, Fallout 3, Fallout: New Vegas, Fallout 4 and "
            "Fallout 4 VR.")
            .str();

    auto paragraph3 = fmt::format(
        boost::locale::translate(
            "LOOT is free, but if you want to show your appreciation with "
            "some money, donations may be made to WrinklyNinja (LOOT's "
            "creator and main developer) using {0}.")
            .str(),
        "<a href=\"https://www.paypal.me/OliverHamlet\">PayPal</a>");

    std::string text =
        fmt::format(textTemplate, paragraph1, paragraph2, paragraph3);

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

      if (state.GetCurrentGame().IsLoadOrderAmbiguous()) {
        actionFixAmbiguousLoadOrder->setEnabled(true);

        showAmbiguousLoadOrderSetWarning(this, state);
      }
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
    handleProgressUpdate(translate("Updating and parsing masterlist…"));

    const auto preludeTask = new UpdatePreludeTask(state);
    const auto masterlistTask =
        new UpdateMasterlistTask(state.GetCurrentGame());

    const std::vector<Task*> tasks{preludeTask, masterlistTask};

    auto whenAll =
        whenAllTasks(tasks)
            .then(this,
                  [this](const QList<QFuture<QueryResult>> futures) {
                    auto preludeResult = futures[0].result();
                    auto masterlistResult = futures[1].result();

                    handleMasterlistUpdated({preludeResult, masterlistResult});
                  })
            .onFailed(this, [this](const std::exception& e) {
              handleError(e.what());
            });

    executeConcurrentBackgroundTasks(tasks, whenAll);
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

void MainWindow::handleSidebarPluginsSelectionChanged(
    const QItemSelection& selected,
    const QItemSelection& deselected) {
  if (selected.isEmpty() && deselected.isEmpty()) {
    // Selection hasn't changed but indices of selected items have.
    return;
  }

  bool hasPluginSelected = false;
  for (const auto& index : selected.indexes()) {
    if (index.row() > 0) {
      // The zeroth row is for the general information card.
      hasPluginSelected = true;
      break;
    }
  }

  if (hasPluginSelected) {
    enablePluginActions();
  } else {
    disablePluginActions();
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

void MainWindow::on_pluginItemModel_dataChanged(const QModelIndex& topLeft,
                                                const QModelIndex& bottomRight,
                                                const QList<int>& roles) {
  if (!topLeft.isValid() || !bottomRight.isValid()) {
    return;
  }

  cardSizingCache.update(topLeft, bottomRight);

  if (roles.isEmpty() || roles.contains(CardContentFiltersRole)) {
    proxyModel->invalidate();
  }

  if (roles.isEmpty() || roles.contains(RawDataRole) ||
      roles.contains(CardContentFiltersRole)) {
    updateCounts(pluginItemModel->getGeneralMessages(),
                 pluginItemModel->getPluginItems());
    refreshSearch();
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

void MainWindow::on_pluginItemModel_rowsInserted(const QModelIndex&,
                                                 int first,
                                                 int last) {
  cardSizingCache.update(pluginItemModel, first, last);
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

    refreshPluginRawData(pluginName);

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

void MainWindow::on_filtersWidget_overlapFilterChanged(
    std::optional<std::string> targetPluginName) {
  try {
    if (!targetPluginName.has_value()) {
      setFiltersState(filtersWidget->getPluginFiltersState(), {});
      return;
    }

    if (!state.HasCurrentGame()) {
      return;
    }

    handleProgressUpdate(translate("Identifying overlapping plugins…"));

    std::unique_ptr<Query> query = std::make_unique<GetOverlappingPluginsQuery>(
        state.GetCurrentGame(),
        state.getSettings().getLanguage(),
        targetPluginName.value());

    executeBackgroundQuery(
        std::move(query), &MainWindow::handleOverlapFilterChecked, nullptr);
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
    const auto currentTheme = state.getSettings().getTheme();
    settingsDialog->recordInputValues(state);

    state.getSettings().save(state.getSettingsPath());

    // Update the games dropdown in case names have changed.
    refreshGamesDropdown();

    if (state.getSettings().getTheme() != currentTheme) {
      applyTheme();
    }
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::on_groupsEditor_accepted() {
  try {
    state.GetCurrentGame().SetUserGroups(groupsEditor->getUserGroups());

    for (const auto& [pluginName, groupName] :
         groupsEditor->getNewPluginGroups()) {
      // Update the plugin's group in user metadata.
      auto userMetadata = state.GetCurrentGame().GetUserMetadata(pluginName);

      if (userMetadata.has_value()) {
        userMetadata.value().SetGroup(groupName);
        state.GetCurrentGame().AddUserMetadata(userMetadata.value());
      } else {
        PluginMetadata metadata(pluginName);
        metadata.SetGroup(groupName);
        state.GetCurrentGame().AddUserMetadata(metadata);
      }

      // Now update the plugin in the UI's plugin item model.
      refreshPluginRawData(pluginName);
    }

    state.GetCurrentGame().SaveUserMetadata();

    SaveGroupNodePositions(state.GetCurrentGame().GroupNodePositionsPath(),
                           groupsEditor->getNodePositions());
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

  if (text.userType() == QMetaType::QRegularExpression &&
      !text.toRegularExpression().isValid()) {
    // Do nothing if given an invalid regex.
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
    filtersWidget->setGameId(state.GetCurrentGame().GetSettings().Id());
    filtersWidget->resetOverlapAndGroupsFilters();
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
        state.GetCurrentGame().AppendMessage(CreatePlainTextSourcedMessage(
            MessageType::error,
            MessageSource::autoSortCancellation,
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

void MainWindow::handleMasterlistUpdated(std::vector<QueryResult> results) {
  try {
    if (results.empty()) {
      return;
    }

    const auto wasPreludeUpdated = std::get<bool>(results.at(0));
    const auto wasMasterlistUpdated =
        std::get<MasterlistUpdateResult>(results.at(1)).second;

    if (!wasPreludeUpdated && !wasMasterlistUpdated) {
      progressDialog->reset();
      showNotification(translate("No masterlist update was necessary."));

      // Update general info as the timestamp may have changed and if metadata
      // was previously missing it can now be displayed.
      updateGeneralInformation();
      return;
    }

    state.GetCurrentGame().LoadMetadata();

    const auto pluginItems =
        GetPluginItems(state.GetCurrentGame().GetLoadOrder(),
                       state.GetCurrentGame(),
                       state.getSettings().getLanguage());

    handleGameDataLoaded(pluginItems);

    auto masterlistInfo = getFileRevisionSummary(
        state.GetCurrentGame().MasterlistPath(), FileType::Masterlist);
    auto infoText = fmt::format(
        boost::locale::translate("Masterlist updated to revision {0}.").str(),
        masterlistInfo.id);

    showNotification(QString::fromStdString(infoText));
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleMasterlistsUpdated(std::vector<QueryResult> results) {
  try {
    if (results.empty()) {
      progressDialog->reset();
      return;
    }

    // The results are in an unknown order due to parallel task execution.
    bool wasPreludeUpdated{false};
    for (const auto& result : results) {
      if (std::holds_alternative<bool>(result)) {
        wasPreludeUpdated = std::get<bool>(result);
        break;
      }
    }

    const auto logger = getLogger();
    const auto gamesSettings = state.getSettings().getGameSettings();

    std::vector<std::string> updatedGameNames;
    bool wasCurrentGameMasterlistUpdated{false};
    for (const auto& result : results) {
      if (!std::holds_alternative<MasterlistUpdateResult>(result)) {
        continue;
      }

      const auto updateResult = std::get<MasterlistUpdateResult>(result);

      if (wasPreludeUpdated || updateResult.second) {
        const auto it =
            std::find_if(gamesSettings.begin(),
                         gamesSettings.end(),
                         [&](const GameSettings& settings) {
                           return settings.FolderName() == updateResult.first;
                         });

        if (it != gamesSettings.end()) {
          updatedGameNames.push_back(it->Name());
        } else if (logger) {
          logger->error(
              "Unrecognised game folder name {} encountered while "
              "updating all masterlists",
              updateResult.first);
        }

        if (state.HasCurrentGame() &&
            updateResult.first ==
                state.GetCurrentGame().GetSettings().FolderName()) {
          wasCurrentGameMasterlistUpdated = true;
        }
      }
    }

    if (updatedGameNames.empty()) {
      progressDialog->reset();
      showNotification(translate("No masterlist updates were necessary."));

      // Update general info as the timestamp may have changed and if
      // metadata was previously missing it can now be displayed.
      updateGeneralInformation();
      return;
    }

    if (wasCurrentGameMasterlistUpdated) {
      // Need to reload the current game data.
      state.GetCurrentGame().LoadMetadata();

      const auto pluginItems =
          GetPluginItems(state.GetCurrentGame().GetLoadOrder(),
                         state.GetCurrentGame(),
                         state.getSettings().getLanguage());

      handleGameDataLoaded(pluginItems);
    } else {
      progressDialog->reset();
    }

    auto message =
        translate("Masterlists updated for the following games:\n\n");
    for (const auto& gameName : updatedGameNames) {
      message += QString::fromStdString(gameName + "\n");
    }

    auto messageBox = QMessageBox(
        QMessageBox::NoIcon, "LOOT", message, QMessageBox::Ok, this);
    messageBox.exec();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleOverlapFilterChecked(QueryResult result) {
  try {
    progressDialog->reset();

    // The result is not an array of PluginItem objects.
    // Instead it's an array of objects, with each having a metadata property
    // that is a PluginItem object, and an overlap property that is
    // a boolean.
    std::vector<PluginItem> gameDataLoadedResult;
    std::vector<std::string> overlappingPluginNames;

    for (const auto& pluginPair :
         std::get<GetOverlappingPluginsResult>(result)) {
      if (pluginPair.second) {
        overlappingPluginNames.push_back(pluginPair.first.name);
      }

      gameDataLoadedResult.push_back(pluginPair.first);
    }

    handleGameDataLoaded(gameDataLoadedResult);

    setFiltersState(filtersWidget->getPluginFiltersState(),
                    std::move(overlappingPluginNames));

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
  progressDialog->adjustSize();
}

void MainWindow::handleUpdateCheckFinished(QueryResult result) {
  try {
    const bool updateIsAvailable = std::get<bool>(result);
    if (updateIsAvailable) {
      const auto logger = getLogger();
      if (logger) {
        logger->info("LOOT update is available.");
      }

      const auto text = fmt::format(
          boost::locale::translate("A [new release]({0}) of LOOT is available.")
              .str(),
          "https://github.com/loot/loot/releases/latest");

      state.GetCurrentGame().AppendMessage(
          SourcedMessage{MessageType::error, MessageSource::updateCheck, text});
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

    state.GetCurrentGame().AppendMessage(
        SourcedMessage{MessageType::error, MessageSource::updateCheck, text});
    updateGeneralMessages();
  } catch (const std::exception& e) {
    handleException(e);
  }
}

void MainWindow::handleIconColorChanged() {
  IconFactory::setColours(
      normalIconColor, disabledIconColor, selectedIconColor);

  setIcons();

  sidebarPluginsView->reset();

  const auto cardDelegate =
      qobject_cast<CardDelegate*>(pluginCardsView->itemDelegate());

  if (cardDelegate) {
    cardDelegate->setIcons();
    pluginCardsView->reset();
  }
}

void MainWindow::handleSidebarTextColorChanged() {
  const auto sidebarDelegate = qobject_cast<SidebarPluginNameDelegate*>(
      sidebarPluginsView->itemDelegateForColumn(
          PluginItemModel::SIDEBAR_NAME_COLUMN));

  if (sidebarDelegate) {
    sidebarDelegate->setColors(selectedSidebarPluginTextColor,
                               unselectedSidebarPluginGroupColor);
    sidebarPluginsView->reset();
  }
}

void MainWindow::handleLinkColorChanged() {
  auto palette = qApp->palette();
  palette.setColor(QPalette::Active, QPalette::Link, linkColor);
  qApp->setPalette(palette);

  const auto cardDelegate =
      qobject_cast<CardDelegate*>(pluginCardsView->itemDelegate());

  if (cardDelegate) {
    cardDelegate->refreshMessages();
    pluginCardsView->reset();
  }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
void MainWindow::handleColorSchemeChanged(Qt::ColorScheme colorScheme) {
  const auto logger = getLogger();
  if (logger) {
    std::string description;
    if (colorScheme == Qt::ColorScheme::Unknown) {
      description = "unknown";
    } else if (colorScheme == Qt::ColorScheme::Light) {
      description = "light";
    } else if (colorScheme == Qt::ColorScheme::Dark) {
      description = "dark";
    } else {
      description = std::to_string(static_cast<int>(colorScheme));
    }
    logger->info("The system color scheme is now {}", description);
  }

  applyTheme();
}
#endif
}
