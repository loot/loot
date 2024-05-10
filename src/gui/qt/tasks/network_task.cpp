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

#include "gui/qt/tasks/network_task.h"

#include <boost/locale.hpp>

namespace loot {
void NetworkTask::handleException(const std::exception &exception) {
  const auto logger = getLogger();
  if (logger) {
    logger->error("Caught an exception: {}", exception.what());
  }

  const auto message = boost::locale::translate(
                           "Oh no, something went wrong! You can check your "
                           "LOOTDebugLog.txt (you can get to it through the "
                           "main menu) for more information.")
                           .str();

  emit this->error(message);
}

void NetworkTask::onNetworkError(QNetworkReply::NetworkError networkError) {
  try {
    const auto reply = qobject_cast<QIODevice *>(sender());
    const auto errorString = reply->errorString().toStdString();

    const auto logger = getLogger();
    if (logger) {
      logger->error("Network error code {}, description is: {}",
                    static_cast<int>(networkError),
                    errorString);
    }

    emit error(errorString);
  } catch (const std::exception &e) {
    handleException(e);
  }
}

void NetworkTask::onSSLError(const QList<QSslError> &errors) {
  try {
    const auto logger = getLogger();

    std::string errorStrings;
    for (const auto &error : errors) {
      const auto errorString = error.errorString().toStdString();
      errorStrings += errorString + "; ";

      if (logger) {
        logger->error("SSL error: {}", errorString);
      }
    }

    if (!errorStrings.empty()) {
      errorStrings = errorStrings.substr(0, errorStrings.length() - 2);
    }

    emit error(errorStrings);
  } catch (const std::exception &e) {
    handleException(e);
  }
}
}
