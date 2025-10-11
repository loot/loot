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

#include "gui/qt/tasks/check_for_update_task.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonValue>
#include <boost/algorithm/string.hpp>

#include "gui/qt/helpers.h"
#include "gui/version.h"

namespace {
using loot::LOOT_VERSION_MAJOR;
using loot::LOOT_VERSION_MINOR;
using loot::LOOT_VERSION_PATCH;

int compareLOOTVersion(const std::string &version) {
  std::vector<std::string> parts;
  boost::split(parts, version, boost::is_any_of("."));

  if (parts.size() != 3) {
    throw std::runtime_error("Unexpected number of version parts in " +
                             version);
  }

  const auto givenMajor = std::stoul(parts.at(0));
#pragma warning(suppress : 4296, justification : "The major version may not be zero at some point in the future.")
  if (LOOT_VERSION_MAJOR > givenMajor) {
    return 1;
  }

  if (LOOT_VERSION_MAJOR < givenMajor) {
    return -1;
  }

  const auto givenMinor = std::stoul(parts.at(1));
  if (LOOT_VERSION_MINOR > givenMinor) {
    return 1;
  }

  if (LOOT_VERSION_MINOR < givenMinor) {
    return -1;
  }

  const auto givenPatch = std::stoul(parts.at(2));
#pragma warning(suppress : 4296, justification : "The minor version may not be zero at some point in the future.")
  if (LOOT_VERSION_PATCH > givenPatch) {
    return 1;
  }

  if (LOOT_VERSION_PATCH < givenPatch) {
    return -1;
  }

  return 0;
}

std::optional<QDate> getDateFromCommitJson(const QJsonDocument &document,
                                           const std::string &commitHash) {
  // Committer can be null, but that will just result in an Undefined value.
  const auto dateString = document["commit"]["committer"]["date"].toString();
  if (dateString.isEmpty()) {
    const auto logger = loot::getLogger();
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
}

namespace loot {
void CheckForUpdateTask::execute() {
  try {
    // Delay construction of the manager so that it's created in the correct
    // thread.
    if (networkAccessManager == nullptr) {
      networkAccessManager = new QNetworkAccessManager(this);
    }

    // Reset the tag commit date in case this task is being run twice somehow.
    tagCommitDate = std::nullopt;

    sendHttpRequest("https://api.github.com/repos/loot/loot/releases/latest",
                    &CheckForUpdateTask::onGetLatestReleaseReplyFinished);
  } catch (const std::exception &e) {
    handleException(e);
  }
}

void CheckForUpdateTask::sendHttpRequest(
    const std::string &url,
    void (CheckForUpdateTask::*onFinished)()) {
  QNetworkRequest request(QUrl(QString::fromStdString(url)));
  request.setTransferTimeout(TRANSFER_TIMEOUT_MS);
  request.setRawHeader("Accept", "application/vnd.github.v3+json");

  const auto reply = networkAccessManager->get(request);

  connect(reply, &QNetworkReply::finished, this, onFinished);
  connect(reply,
          &QNetworkReply::errorOccurred,
          this,
          &CheckForUpdateTask::onNetworkError);
  connect(
      reply, &QNetworkReply::sslErrors, this, &CheckForUpdateTask::onSSLError);
}

void CheckForUpdateTask::onGetLatestReleaseReplyFinished() {
  try {
    const auto logger = getLogger();
    if (logger) {
      logger->trace(
          "Finished receiving a response for getting the latest release's tag");
    }

    const auto responseData =
        readHttpResponse(qobject_cast<QNetworkReply *>(sender()));

    if (!responseData.has_value()) {
      emit error("No response data");
      return;
    }

    const auto json = QJsonDocument::fromJson(responseData.value());
    const auto tagName = json["tag_name"].toString().toStdString();

    const auto comparisonResult = compareLOOTVersion(tagName);

    if (comparisonResult < 0) {
      emit finished(true);
      return;
    }

    if (comparisonResult > 0) {
      emit finished(false);
      return;
    }

    // Versions are equal, get commit dates to compare. First get the
    // tag's commit hash.
    const auto url =
        "https://api.github.com/repos/loot/loot/commits/tags/" + tagName;
    sendHttpRequest(url, &CheckForUpdateTask::onGetTagCommitReplyFinished);
  } catch (const std::exception &e) {
    handleException(e);
  }
}

void CheckForUpdateTask::onGetTagCommitReplyFinished() {
  try {
    const auto logger = getLogger();
    if (logger) {
      logger->trace(
          "Finished receiving a response for getting the latest release tag's "
          "commit");
    }

    const auto responseData =
        readHttpResponse(qobject_cast<QNetworkReply *>(sender()));

    if (!responseData.has_value()) {
      emit error("No response data");
      return;
    }

    const auto json = QJsonDocument::fromJson(responseData.value());
    const auto commitHash = json["sha"].toString().toStdString();

    if (boost::istarts_with(commitHash, getLootRevision())) {
      emit finished(false);
      return;
    }

    // Store the tag commit date for later comparison.
    tagCommitDate = getDateFromCommitJson(json, commitHash);

    if (!tagCommitDate.has_value()) {
      emit error("Tag commit date not found");
      return;
    }

    // Now get the build commit ID to do the final comparison.
    const auto url =
        "https://api.github.com/repos/loot/loot/commits/" + getLootRevision();
    sendHttpRequest(url, &CheckForUpdateTask::onGetBuildCommitReplyFinished);
  } catch (const std::exception &e) {
    handleException(e);
  }
}

void CheckForUpdateTask::onGetBuildCommitReplyFinished() {
  try {
    const auto logger = getLogger();
    if (logger) {
      logger->trace(
          "Finished receiving a response for getting the LOOT build's "
          "commit");
    }

    if (!tagCommitDate.has_value()) {
      emit error("Tag commit date not stored");
      return;
    }

    const auto responseData =
        readHttpResponse(qobject_cast<QNetworkReply *>(sender()));

    if (!responseData.has_value()) {
      emit error("No response data");
      return;
    }

    const auto json = QJsonDocument::fromJson(responseData.value());
    const auto buildCommitDate = getDateFromCommitJson(json, getLootRevision());

    if (!buildCommitDate.has_value()) {
      emit error("Build commit date not found");
      return;
    }

    if (tagCommitDate.value() > buildCommitDate.value()) {
      logger->info("Tag date: {}, build date: {}",
                   tagCommitDate.value().toString().toStdString(),
                   buildCommitDate.value().toString().toStdString());
      emit finished(true);
    } else if (logger) {
      logger->info("No LOOT update is available.");
      emit finished(false);
    }
  } catch (const std::exception &e) {
    handleException(e);
  }
}
}
