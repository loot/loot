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

#ifndef LOOT_GUI_QT_QUERY_WORKER_THREAD
#define LOOT_GUI_QT_QUERY_WORKER_THREAD

#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <memory>

#include "gui/query/query.h"

Q_DECLARE_METATYPE(nlohmann::json);

namespace loot {
class ProgressUpdater : public QObject {
  Q_OBJECT
signals:
  void progressUpdate(const QString &message);
};

class QueryWorkerThread : public QThread {
  Q_OBJECT
public:
  QueryWorkerThread(QObject *parent, std::unique_ptr<Query> query) :
      QThread(parent) {
    queries.push_back(std::move(query));
  }

  QueryWorkerThread(QObject *parent,
                    std::vector<std::unique_ptr<Query>> queries) :
      QThread(parent), queries(std::move(queries)) {}

  void run() override {
    for (auto &&query : queries) {
      try {
        if (query == nullptr) {
          throw std::runtime_error(
              "Attempted to run a worker thread with no query set!");
        }

        emit resultReady(query->executeLogic());
      } catch (std::exception &e) {
        auto logger = getLogger();
        if (logger) {
          logger->error("Exception while executing query in worker thread: {}",
                        e.what());
        }

        emit error(query->getErrorMessage());

        // Don't execute any further queries.
        return;
      }
    }
  }

signals:
  void resultReady(nlohmann::json result);
  void error(const std::string &exception);

private:
  std::vector<std::unique_ptr<Query>> queries;
};
}

#endif
