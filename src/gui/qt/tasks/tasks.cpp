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

TaskExecutor::TaskExecutor(QObject *parent, std::vector<Task *> tasks) :
    QObject(parent), tasks(tasks) {
  // Move all the tasks to the worker thread.
  for (auto task : tasks) {
    task->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, task, &QObject::deleteLater);
    connect(task, &Task::finished, this, &TaskExecutor::onTaskFinished);
    connect(task, &Task::error, this, &TaskExecutor::onTaskError);
  }

  connect(&workerThread,
          &QThread::finished,
          this,
          &TaskExecutor::onWorkerThreadFinished);

  if (!tasks.empty()) {
    auto firstTask = tasks.at(currentTask);
    connect(this, &TaskExecutor::start, firstTask, &Task::execute);
  }

  workerThread.setObjectName("workerThread");
  workerThread.start();
}

TaskExecutor::~TaskExecutor() {
  workerThread.quit();
  workerThread.wait();
}

void TaskExecutor::onTaskFinished(QueryResult result) {
  taskResults.push_back(result);

  auto task = qobject_cast<Task *>(sender());

  // Disconnect the finished task from the start signal so that it doesn't
  // get restarted when the start signal is next sent.
  disconnect(this, &TaskExecutor::start, task, &Task::execute);

  // If there are more tasks remaining, connect the start signal to the next
  // task's execute slot.
  currentTask += 1;

  if (currentTask >= tasks.size()) {
    workerThread.quit();
    return;
  }

  connect(this, &TaskExecutor::start, tasks.at(currentTask), &Task::execute);

  // Now start the next task.
  emit start();
}

void TaskExecutor::onTaskError() { workerThread.quit(); }

void TaskExecutor::onWorkerThreadFinished() {
  tasks.clear();

  emit finished(taskResults);
}
}
