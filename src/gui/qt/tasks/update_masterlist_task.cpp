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
UpdateMasterlistTask::UpdateMasterlistTask(LootState &state) : state(state) {}

void UpdateMasterlistTask::execute() {
  try {
    // Delay construction of the manager so that it's created in the correct
    // thread.
    if (networkAccessManager == nullptr) {
      networkAccessManager = new QNetworkAccessManager(this);
    }

    updatePrelude();
  } catch (const std::exception &e) {
    handleException(e);
  }
}

void UpdateMasterlistTask::updatePrelude() {
  auto source = state.getSettings().getPreludeSource();
  if (!isValidUrl(source)) {
    // Treat the source as a local path, and copy the file from there.
    auto sourcePath = std::filesystem::u8path(source);

    preludeUpdated = updateFile(sourcePath, state.getPreludePath());

    // Now update the masterlist.
    updateMasterlist();
    return;
  }

  auto logger = getLogger();
  if (logger) {
    logger->trace("Sending a prelude update request to GET {}", source);
  }

  QNetworkRequest request(QUrl(QString::fromStdString(source)));

  const auto reply = networkAccessManager->get(request);

  connect(reply,
          &QNetworkReply::finished,
          this,
          &UpdateMasterlistTask::onPreludeReplyFinished);

  connect(reply,
          &QNetworkReply::errorOccurred,
          this,
          &UpdateMasterlistTask::onNetworkError);
  connect(reply,
          &QNetworkReply::sslErrors,
          this,
          &UpdateMasterlistTask::onSSLError);
}

void UpdateMasterlistTask::updateMasterlist() {
  auto source = state.GetCurrentGame().GetSettings().MasterlistSource();
  if (!isValidUrl(source)) {
    // Treat the source as a local path, and copy the file from there.
    auto sourcePath = std::filesystem::u8path(source);

    masterlistUpdated =
        updateFile(sourcePath, state.GetCurrentGame().MasterlistPath());

    finish();
    return;
  }

  auto logger = getLogger();
  if (logger) {
    logger->trace("Sending a masterlist update request to GET {}", source);
  }

  QNetworkRequest request(QUrl(QString::fromStdString(source)));

  const auto reply = networkAccessManager->get(request);

  connect(reply,
          &QNetworkReply::finished,
          this,
          &UpdateMasterlistTask::onMasterlistReplyFinished);

  connect(reply,
          &QNetworkReply::errorOccurred,
          this,
          &UpdateMasterlistTask::onNetworkError);
  connect(reply,
          &QNetworkReply::sslErrors,
          this,
          &UpdateMasterlistTask::onSSLError);
}

void UpdateMasterlistTask::finish() {
  if (preludeUpdated || masterlistUpdated) {
    state.GetCurrentGame().LoadMetadata();

    const auto pluginItems =
        GetPluginItems(state.GetCurrentGame().GetLoadOrder(),
                       state.GetCurrentGame(),
                       state.getSettings().getLanguage());

    emit finished(pluginItems);
    return;
  }

  emit finished(std::monostate());
}

void UpdateMasterlistTask::onMasterlistReplyFinished() {
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

    auto masterlistPath = state.GetCurrentGame().MasterlistPath();
    masterlistUpdated =
        updateFileWithData(masterlistPath, responseData.value());

    finish();
  } catch (const std::exception &e) {
    handleException(e);
  }
}

void UpdateMasterlistTask::onPreludeReplyFinished() {
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

    auto preludePath = state.getPreludePath();
    preludeUpdated = updateFileWithData(preludePath, responseData.value());

    // Now update the masterlist.
    updateMasterlist();
  } catch (const std::exception &e) {
    handleException(e);
  }
}
}
