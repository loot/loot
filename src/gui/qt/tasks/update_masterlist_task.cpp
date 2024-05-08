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

#include "gui/qt/tasks/update_masterlist_task.h"

#include "gui/qt/helpers.h"

namespace loot {
UpdatePreludeTask::UpdatePreludeTask(const LootState &state) :
    preludeSource(state.getSettings().getPreludeSource()),
    preludePath(state.getPreludePath()) {}

void UpdatePreludeTask::execute() {
  try {
    // Delay construction of the manager so that it's created in the correct
    // thread.
    if (networkAccessManager == nullptr) {
      networkAccessManager = new QNetworkAccessManager(this);
    }

    if (!isValidUrl(preludeSource)) {
      // Treat the source as a local path, and copy the file from there.
      auto sourcePath = std::filesystem::u8path(preludeSource);

      const auto preludeUpdated = updateFile(sourcePath, preludePath);

      emit finished(preludeUpdated);
      return;
    }

    auto logger = getLogger();
    if (logger) {
      logger->trace("Sending a prelude update request to GET {}",
                    preludeSource);
    }

    QNetworkRequest request(QUrl(QString::fromStdString(preludeSource)));
    request.setTransferTimeout(QNetworkRequest::DefaultTransferTimeout);

    const auto reply = networkAccessManager->get(request);

    connect(reply,
            &QNetworkReply::finished,
            this,
            &UpdatePreludeTask::onReplyFinished);

    connect(reply,
            &QNetworkReply::errorOccurred,
            this,
            &UpdatePreludeTask::onNetworkError);
    connect(
        reply, &QNetworkReply::sslErrors, this, &UpdatePreludeTask::onSSLError);
  } catch (const std::exception &e) {
    handleException(e);
  }
}

void UpdatePreludeTask::onReplyFinished() {
  try {
    auto logger = getLogger();
    if (logger) {
      logger->trace("Finished receiving a response for prelude update");
    }

    auto responseData =
        readHttpResponse(qobject_cast<QNetworkReply *>(sender()));

    if (!responseData.has_value()) {
      emit error("Prelude update response errored");
      return;
    }

    const auto preludeUpdated =
        updateFileWithData(preludePath, responseData.value());

    emit finished(preludeUpdated);
  } catch (const std::exception &e) {
    handleException(e);
  }
}

UpdateMasterlistTask::UpdateMasterlistTask(const gui::Game &game) :
    UpdateMasterlistTask(game.GetSettings().FolderName(),
                         game.GetSettings().MasterlistSource(),
                         game.MasterlistPath()) {}

UpdateMasterlistTask::UpdateMasterlistTask(
    const std::string &gameFolderName,
    const std::string &masterlistSource,
    const std::filesystem::path &masterlistPath) :
    gameFolderName(gameFolderName),
    masterlistSource(masterlistSource),
    masterlistPath(masterlistPath) {}

void UpdateMasterlistTask::execute() {
  try {
    // Delay construction of the manager so that it's created in the correct
    // thread.
    if (networkAccessManager == nullptr) {
      networkAccessManager = new QNetworkAccessManager(this);
    }

    if (!isValidUrl(masterlistSource)) {
      // Treat the source as a local path, and copy the file from there.
      const auto sourcePath = std::filesystem::u8path(masterlistSource);

      const auto masterlistUpdated = updateFile(sourcePath, masterlistPath);

      emit finished(std::make_pair(gameFolderName, masterlistUpdated));
      return;
    }

    auto logger = getLogger();
    if (logger) {
      logger->trace("Sending a masterlist update request to GET {}",
                    masterlistSource);
    }

    QNetworkRequest request(QUrl(QString::fromStdString(masterlistSource)));
    request.setTransferTimeout(QNetworkRequest::DefaultTransferTimeout);

    const auto reply = networkAccessManager->get(request);

    connect(reply,
            &QNetworkReply::finished,
            this,
            &UpdateMasterlistTask::onReplyFinished);

    connect(reply,
            &QNetworkReply::errorOccurred,
            this,
            &UpdateMasterlistTask::onNetworkError);
    connect(reply,
            &QNetworkReply::sslErrors,
            this,
            &UpdateMasterlistTask::onSSLError);
  } catch (const std::exception &e) {
    handleException(e);
  }
}

void UpdateMasterlistTask::onReplyFinished() {
  try {
    auto logger = getLogger();
    if (logger) {
      logger->trace("Finished receiving a response for masterlist update");
    }

    auto responseData =
        readHttpResponse(qobject_cast<QNetworkReply *>(sender()));

    if (!responseData.has_value()) {
      emit error("Masterlist update response errored");
      return;
    }

    const auto masterlistUpdated =
        updateFileWithData(masterlistPath, responseData.value());

    emit finished(std::make_pair(gameFolderName, masterlistUpdated));
  } catch (const std::exception &e) {
    handleException(e);
  }
}
}
