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

TaskExecutor::TaskExecutor(QObject *parent) : QObject(parent) {}

SequentialTaskExecutor::SequentialTaskExecutor(QObject *parent,
                                               std::vector<Task *> tasks) :
    TaskExecutor(parent), tasks(tasks) {
  // Move all the tasks to the worker thread.
  for (auto task : tasks) {
    task->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, task, &QObject::deleteLater);
    connect(
        task, &Task::finished, this, &SequentialTaskExecutor::onTaskFinished);
    connect(task, &Task::error, this, &SequentialTaskExecutor::onTaskError);
  }

  connect(&workerThread,
          &QThread::finished,
          this,
          &SequentialTaskExecutor::onWorkerThreadFinished);

  if (!tasks.empty()) {
    auto firstTask = tasks.at(currentTask);
    connect(this, &TaskExecutor::start, firstTask, &Task::execute);
  }

  workerThread.setObjectName("workerThread");
  workerThread.start();
}

SequentialTaskExecutor::~SequentialTaskExecutor() {
  workerThread.quit();
  workerThread.wait();
}

void SequentialTaskExecutor::onTaskFinished(QueryResult result) {
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

void SequentialTaskExecutor::onTaskError() { workerThread.quit(); }

void SequentialTaskExecutor::onWorkerThreadFinished() {
  tasks.clear();

  emit finished(taskResults);
}

ParallelTaskExecutor::ParallelTaskExecutor(QObject *parent,
                                           std::vector<Task *> tasks) :
    TaskExecutor(parent), tasks(tasks) {
  // Create a worker thread for each task.
  for (auto task : tasks) {
    const auto workerThread = new QThread(this);
    task->moveToThread(workerThread);

    workerThreads.push_back(workerThread);

    connect(this, &TaskExecutor::start, task, &Task::execute);
    connect(task, &Task::finished, this, &ParallelTaskExecutor::onTaskFinished);
    connect(task, &Task::error, this, &ParallelTaskExecutor::onTaskError);

    connect(workerThread, &QThread::finished, task, &QObject::deleteLater);
    connect(workerThread,
            &QThread::finished,
            this,
            &ParallelTaskExecutor::onWorkerThreadFinished);

    workerThread->setObjectName("workerThread");
    workerThread->start();
  }
}

ParallelTaskExecutor::~ParallelTaskExecutor() {
  for (auto &workerThread : workerThreads) {
    workerThread->quit();
    workerThread->wait();
  }
}

void ParallelTaskExecutor::onTaskFinished(QueryResult result) {
  std::lock_guard guard(mutex);

  taskResults.push_back(result);

  const auto task = qobject_cast<Task *>(sender());

  task->thread()->quit();
}

void ParallelTaskExecutor::onTaskError() {
  const auto task = qobject_cast<Task *>(sender());

  task->thread()->quit();
}

void ParallelTaskExecutor::onWorkerThreadFinished() {
  for (auto &workerThread : workerThreads) {
    if (workerThread->isRunning()) {
      return;
    }
  }

  std::lock_guard guard(mutex);

  tasks.clear();

  emit finished(taskResults);
}
}
