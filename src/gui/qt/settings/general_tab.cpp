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

#include "gui/qt/settings/general_tab.h"

#include <fmt/format.h>

#include <QtGui/QGuiApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QVBoxLayout>
#include <boost/algorithm/string/predicate.hpp>

#include "gui/qt/helpers.h"
#include "gui/qt/style.h"
#include "gui/translate.h"

namespace {
using loot::DARK_THEME_SUFFIX;
using loot::LIGHT_THEME_SUFFIX;
using loot::translate;

std::string getThemeLabel(const std::string& theme) {
  if (boost::ends_with(theme, DARK_THEME_SUFFIX)) {
    const auto name = theme.substr(0, theme.size() - DARK_THEME_SUFFIX.size());
    return fmt::format(translate("{0} (Dark)"), name);
  }

  if (boost::ends_with(theme, LIGHT_THEME_SUFFIX)) {
    const auto name = theme.substr(0, theme.size() - LIGHT_THEME_SUFFIX.size());
    return fmt::format(translate("{0} (Light)"), name);
  }

  return theme;
}
}

namespace loot {
GeneralTab::GeneralTab(QWidget* parent) : QFrame(parent, Qt::Dialog) {
  setupUi();
}

void GeneralTab::initialiseInputs(const LootSettings& settings,
                                  const std::vector<std::string>& themes) {
  while (defaultGameComboBox->count() > 1) {
    defaultGameComboBox->removeItem(1);
  }

  for (const auto& game : settings.getGameSettings()) {
    defaultGameComboBox->addItem(
        QString::fromStdString(game.getName()),
        QVariant(QString::fromStdString(game.getFolderName())));
  }

  auto gameIndex = defaultGameComboBox->findData(
      QVariant(QString::fromStdString(settings.getGame())));
  defaultGameComboBox->setCurrentIndex(gameIndex);

  while (languageComboBox->count() > 0) {
    languageComboBox->removeItem(0);
  }

  for (const auto& language : settings.getLanguages()) {
    languageComboBox->addItem(
        QString::fromStdString(language.name),
        QVariant(QString::fromStdString(language.locale)));
  }

  auto languageIndex = languageComboBox->findData(
      QVariant(QString::fromStdString(settings.getLanguage())));
  languageComboBox->setCurrentIndex(languageIndex);

  while (themeComboBox->count() > 0) {
    themeComboBox->removeItem(0);
  }

  for (const auto& theme : themes) {
    auto qtTheme = QString::fromStdString(theme);
    auto label = getThemeLabel(theme);

    themeComboBox->addItem(QString::fromStdString(label), QVariant(qtTheme));
  }

  auto themeIndex = themeComboBox->findData(
      QVariant(QString::fromStdString(settings.getTheme())));
  themeComboBox->setCurrentIndex(themeIndex);

  updateMasterlistCheckbox->setChecked(
      settings.isMasterlistUpdateBeforeSortEnabled());
  checkUpdatesCheckbox->setChecked(settings.isLootUpdateCheckEnabled());
  loggingCheckbox->setChecked(settings.isDebugLoggingEnabled());
  useNoSortingChangesDialogCheckbox->setChecked(
      settings.isNoSortingChangesDialogEnabled());
  warnOnCaseSensitiveGamePathsCheckbox->setChecked(
      settings.isWarnOnCaseSensitiveGamePathsEnabled());

  preludeSourceInput->setText(
      QString::fromStdString(settings.getPreludeSource()));
}

void GeneralTab::recordInputValues(LootSettings& settings) {
  auto defaultGame =
      defaultGameComboBox->currentData().toString().toStdString();
  auto language = languageComboBox->currentData().toString().toStdString();
  auto theme = themeComboBox->currentData().toString().toStdString();
  const auto enableMasterlistUpdateBeforeSort =
      updateMasterlistCheckbox->isChecked();
  const auto checkForUpdates = checkUpdatesCheckbox->isChecked();
  const auto enableDebugLogging = loggingCheckbox->isChecked();
  const auto enableNoSortingChangesDialog =
      useNoSortingChangesDialogCheckbox->isChecked();
  const auto enableWarnOnCaseSensitiveGamePaths =
      warnOnCaseSensitiveGamePathsCheckbox->isChecked();
  auto preludeSource = preludeSourceInput->text().toStdString();

  settings.setDefaultGame(defaultGame);
  settings.setLanguage(language);
  settings.setTheme(theme);
  settings.enableMasterlistUpdateBeforeSort(enableMasterlistUpdateBeforeSort);
  settings.enableLootUpdateCheck(checkForUpdates);
  settings.enableDebugLogging(enableDebugLogging);
  settings.enableNoSortingChangesDialog(enableNoSortingChangesDialog);
  settings.enableWarnOnCaseSensitiveGamePaths(
      enableWarnOnCaseSensitiveGamePaths);
  settings.setPreludeSource(preludeSource);
}

bool GeneralTab::areInputValuesValid() const {
  if (preludeSourceInput->text().isEmpty()) {
    QToolTip::showText(preludeSourceInput->mapToGlobal(QPoint(0, 0)),
                       preludeSourceInput->toolTip(),
                       preludeSourceInput);
    return false;
  }

  return true;
}

void GeneralTab::setupUi() {
  defaultGameComboBox->addItem(QString(), QVariant(QString("auto")));

  const auto lineHeight = QFontMetricsF(QGuiApplication::font()).height();
  const auto spacer = new QSpacerItem(0, static_cast<int>(lineHeight));

  auto generalLayout = new QFormLayout(this);

  generalLayout->addRow(defaultGameLabel, defaultGameComboBox);
  generalLayout->addRow(languageLabel, languageComboBox);
  generalLayout->addRow(themeLabel, themeComboBox);
  generalLayout->addRow(updateMasterlistLabel, updateMasterlistCheckbox);
  generalLayout->addRow(checkUpdatesLabel, checkUpdatesCheckbox);
  generalLayout->addRow(loggingLabel, loggingCheckbox);
  generalLayout->addRow(useNoSortingChangesDialogLabel,
                        useNoSortingChangesDialogCheckbox);
  generalLayout->addRow(warnOnCaseSensitiveGamePathsLabel,
                        warnOnCaseSensitiveGamePathsCheckbox);
  generalLayout->addRow(preludeSourceLabel, preludeSourceInput);
  generalLayout->addItem(spacer);
  generalLayout->addRow(descriptionLabel);

  translateUi();
}

void GeneralTab::translateUi() {
  defaultGameLabel->setText(qTranslate("Default Game"));
  languageLabel->setText(qTranslate("Language"));
  themeLabel->setText(qTranslate("Theme"));
  updateMasterlistLabel->setText(
      qTranslate("Update masterlist before sorting"));
  checkUpdatesLabel->setText(qTranslate("Check for LOOT updates on startup"));
  loggingLabel->setText(qTranslate("Enable debug logging"));
  preludeSourceLabel->setText(qTranslate("Masterlist prelude source"));
  useNoSortingChangesDialogLabel->setText(
      qTranslate("Display dialog when sorting makes no changes"));
  warnOnCaseSensitiveGamePathsLabel->setText(qTranslate(
      "Warn if the game's paths are in a case-sensitive filesystem"));

  loggingLabel->setToolTip(
      qTranslate("The output is logged to the LOOTDebugLog.txt file."));

  preludeSourceInput->setToolTip(qTranslate("A prelude source is required."));

  descriptionLabel->setText(
      qTranslate("Language changes will be applied after LOOT is restarted."));

  defaultGameComboBox->setItemText(0, qTranslate("Autodetect"));
}
}
