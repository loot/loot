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

#include <QtCore/QFuture>
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

QFuture<QueryResult> executeBackgroundQuery(std::unique_ptr<Query> query);

QFuture<QueryResult> taskFuture(Task *task);

QFuture<QList<QFuture<QueryResult>>> whenAllTasks(
    const std::vector<Task *> &tasks);

void executeConcurrentBackgroundTasks(const std::vector<Task *> &tasks,
                                      QFuture<void> whenAll);

QFuture<QueryResult> executeBackgroundTask(Task *task);
}

#endif
