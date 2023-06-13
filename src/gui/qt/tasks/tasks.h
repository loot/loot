/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2022    Oliver Hamlet

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

#ifndef LOOT_GUI_QT_TASKS_TASKS
#define LOOT_GUI_QT_TASKS_TASKS

#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QThread>

#include "gui/query/query.h"

Q_DECLARE_METATYPE(loot::QueryResult);
Q_DECLARE_METATYPE(std::string);

namespace loot {
class ProgressUpdater : public QObject {
  Q_OBJECT
signals:
  void progressUpdate(const QString &message);
};

class Task : public QObject {
  Q_OBJECT
public slots:
  virtual void execute() = 0;

signals:
  void finished(QueryResult result);
  void error(const std::string &exception);
};

class QueryTask : public Task {
  Q_OBJECT
public:
  explicit QueryTask(std::unique_ptr<Query> query);

public slots:
  void execute() override;

private:
  std::unique_ptr<Query> query;
};

class TaskExecutor : public QObject {
  Q_OBJECT
public:
  TaskExecutor(QObject *parent, std::vector<Task *> tasks);
  TaskExecutor(const TaskExecutor &) = delete;
  TaskExecutor(TaskExecutor &&) = delete;
  ~TaskExecutor();

  TaskExecutor &operator=(const TaskExecutor &) = delete;
  TaskExecutor &operator=(TaskExecutor &&) = delete;

signals:
  void start();
  void finished(std::vector<QueryResult> results);

private:
  QThread workerThread;
  std::vector<Task *> tasks;
  size_t currentTask{0};

  std::vector<QueryResult> taskResults;

private slots:
  void onTaskFinished(QueryResult result);
  void onTaskError();
  void onWorkerThreadFinished();
};
}

#endif
