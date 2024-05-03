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

#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QLibraryInfo>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>

#include "gui/application_mutex.h"
#include "gui/qt/main_window.h"
#include "gui/qt/style.h"
#include "gui/state/logging.h"
#include "gui/state/loot_state.h"
#include "gui/version.h"

bool isRunningThroughModOrganiser() {
#ifdef _WIN32
  const std::array<const char*, 3> dlls = {
      "hook.dll", "usvfs_x86.dll", "usvfs_x64.dll"};

  for (const auto dll : dlls) {
    const bool isDllLoaded =
        GetModuleHandle(loot::ToWinWide(dll).c_str()) != nullptr;
    if (isDllLoaded) {
      return true;
    }
  }
#endif

  return false;
}

void logRuntimeEnvironment() {
  const auto logger = loot::getLogger();
  if (logger) {
    logger->info("Running {}-bit LOOT on {} and the {} CPU architecture",
                 static_cast<int>(QSysInfo::WordSize),
                 QSysInfo::prettyProductName().toStdString(),
                 QSysInfo::currentCpuArchitecture().toStdString());
    logger->info("LOOT version: {}+{}",
                 loot::gui::Version::string(),
                 loot::gui::Version::revision);
    logger->info("libloot version: {}+{}",
                 loot::GetLiblootVersion(),
                 loot::GetLiblootRevision());

    if (isRunningThroughModOrganiser()) {
      logger->info("LOOT is being run through Mod Organiser.");
    }
  }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
  // Check if LOOT is already running
  //---------------------------------

  if (loot::IsApplicationMutexLocked()) {
    // An instance of LOOT is already running, so focus its window then quit.
    HWND hWnd = ::FindWindow(nullptr, L"LOOT");
    ::SetForegroundWindow(hWnd);
    return 0;
  }
#endif

  loot::ApplicationMutexGuard mutexGuard;

  QApplication app(argc, argv);

  QCommandLineParser parser;
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addOptions(
      {{"game",
        "Set the game that LOOT will initially load",
        "game identifier"},
       {"game-path", "Set the initial game's install path.", "path"},
       {"loot-data-path",
        "Set the directory where LOOT will store its data",
        "path"},
       {"auto-sort", "Automatically sort the load order on launch"}});
  parser.process(app);

  auto lootDataPath =
      std::filesystem::u8path(parser.value("loot-data-path").toStdString());
  auto startupGameFolder = parser.value("game").toStdString();
  auto gamePath =
      std::filesystem::u8path(parser.value("game-path").toStdString());
  auto autoSort = parser.isSet("auto-sort");

  loot::LootState state("", lootDataPath);

  logRuntimeEnvironment();

  state.init(startupGameFolder, gamePath, autoSort);

  // Load Qt's translations.
  QTranslator translator;

  auto translationsPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);

  auto loaded = translator.load(
      QLocale(QString::fromStdString(state.getSettings().getLanguage())),
      QString("qt"),
      QString("_"),
      translationsPath);

  if (loaded) {
    app.installTranslator(&translator);
  }

  loot::MainWindow mainWindow(state);
  mainWindow.applyTheme();

  const auto wasMaximised = state.getSettings()
                                .getMainWindowPosition()
                                .value_or(loot::LootSettings::WindowPosition())
                                .maximised;

  if (wasMaximised) {
    mainWindow.showMaximized();

    // If the main window is opened in its maximised state, its initial
    // geometry (which is used to position dialog boxes) reflects its
    // unmaximised position, so dialogs don't get centered correctly.
    // As a workaround, repeatedly check every millisecond until the
    // window geometry changes, and only then initialise the window
    // content. This should only take a few milliseconds to complete,
    // but it varies
    auto logger = loot::getLogger();
    const auto initialGeometry = mainWindow.geometry();
    auto timer = new QTimer();
    auto loopCounter = 1;
    static constexpr int LOOP_COUNTER_MAX = 100;

    timer->callOnTimeout([&]() {
      if (logger) {
        logger->debug("Checking if maximised window geometry has updated.");
      }

      // Initialise after LOOP_COUNTER_MAX loops just in case the initial
      // geometry is already correct. If it somehow takes over ~
      // LOOP_COUNTER_MAX ms to update the initial geometry, then at least a UI
      // that positions its initial dialog boxes incorrectly is better than one
      // that never initialises at all.
      if (mainWindow.geometry() != initialGeometry ||
          loopCounter == LOOP_COUNTER_MAX) {
        if (logger) {
          auto message =
              loopCounter == LOOP_COUNTER_MAX
                  ? "Window geometry has not changed in " +
                        std::to_string(LOOP_COUNTER_MAX) +
                        " checks, initialising the UI anyway."
                  : "Window geometry has updated, initialising the UI.";

          logger->debug(message);
        }

        mainWindow.initialise();

        timer->stop();
        timer->deleteLater();
      }

      loopCounter += 1;
    });

    timer->start(1);
  } else {
    mainWindow.show();
    mainWindow.initialise();
  }

  return app.exec();
}
