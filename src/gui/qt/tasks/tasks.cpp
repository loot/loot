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

#include "gui/qt/tasks/tasks.h"

#include <QtConcurrent/QtConcurrent>

namespace loot {
QueryTask::QueryTask(std::unique_ptr<Query> query) : query(std::move(query)) {}

void QueryTask::execute() {
  try {
    if (query == nullptr) {
      throw std::runtime_error(
          "Attempted to execute a query with no query set!");
    }

    emit finished(query->executeLogic());
  } catch (const std::exception &e) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Exception while executing query: {}", e.what());
    }

    if (query == nullptr) {
      emit error(e.what());
    } else {
      emit error(query->getErrorMessage());
    }
  }
}

QFuture<QueryResult> executeBackgroundQuery(std::unique_ptr<Query> query) {
  const auto sharedQuery = std::shared_ptr<Query>(std::move(query));

  return QtConcurrent::run([sharedQuery]() {
    if (sharedQuery == nullptr) {
      throw std::runtime_error(
          "Attempted to execute a query with no query set!");
    }

    try {
      return sharedQuery->executeLogic();
    } catch (const std::exception &e) {
      const auto logger = getLogger();
      if (logger) {
        logger->error("Exception while executing query: {}", e.what());
      }

      const auto errorMessage = sharedQuery == nullptr
                                    ? std::string(e.what())
                                    : sharedQuery->getErrorMessage();

      throw std::runtime_error(errorMessage.c_str());
    }
  });
}

QFuture<QueryResult> taskFuture(Task *task) {
  QFuture<QueryResult> taskFinishedFuture =
      QtFuture::connect(task, &Task::finished);
  QFuture<std::string> taskErrorFuture = QtFuture::connect(task, &Task::error);

  return QtFuture::whenAny(taskFinishedFuture, taskErrorFuture)
      .then(
          [](std::variant<QFuture<QueryResult>, QFuture<std::string>> variant) {
            if (std::holds_alternative<QFuture<QueryResult>>(variant)) {
              return std::get<QFuture<QueryResult>>(variant).result();
            } else {
              throw std::runtime_error(
                  std::get<QFuture<std::string>>(variant).result().c_str());
            }
          });
}

QFuture<QList<QFuture<QueryResult>>> whenAllTasks(
    const std::vector<Task *> &tasks) {
  std::vector<QFuture<QueryResult>> futures;
  for (const auto task : tasks) {
    futures.push_back(taskFuture(task));
  }

  return QtFuture::whenAll(futures.begin(), futures.end());
}

void executeConcurrentBackgroundTasks(const std::vector<Task *> &tasks,
                                      QFuture<void> whenAll) {
  QThread *workerThread = new QThread();

  QObject::connect(
      workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

  whenAll.then(workerThread, [workerThread]() { workerThread->quit(); });

  workerThread->start();

  for (auto task : tasks) {
    task->moveToThread(workerThread);

    QMetaObject::invokeMethod(task, "execute", Qt::QueuedConnection);
  }
}

QFuture<QueryResult> executeBackgroundTask(Task *task) {
  QThread *workerThread = new QThread();

  QObject::connect(
      workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

  workerThread->start();

  task->moveToThread(workerThread);

  QMetaObject::invokeMethod(task, "execute", Qt::QueuedConnection);

  return taskFuture(task).then(workerThread,
                               [workerThread](QueryResult result) {
                                 workerThread->quit();
                                 return result;
                               });
}
}
