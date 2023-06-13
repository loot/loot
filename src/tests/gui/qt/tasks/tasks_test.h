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

#ifndef LOOT_TESTS_GUI_QT_TASKS_TASKS_TEST
#define LOOT_TESTS_GUI_QT_TASKS_TASKS_TEST

#include <gtest/gtest.h>

#include <QtTest/QSignalSpy>
#include <string>

#include "gui/qt/tasks/tasks.h"
#include "tests/gui/qt/tasks/non_blocking_test_task.h"

namespace loot {
namespace test {
static constexpr const char* GENERIC_QUERY_ERROR_MESSAGE =
    "Oh no, something went wrong! You can check your LOOTDebugLog.txt (you can "
    "get to it through the main menu) for more information.";

static const int THREAD_TIMEOUT_MS = 50;

class TestQuery : public Query {
public:
  TestQuery(int value) : value(value) {}

  QueryResult executeLogic() override {
    if (value < 0) {
      throw std::runtime_error("Value is negative");
    }

    PluginItem item;
    item.name = std::to_string(value);

    return item;
  }

private:
  const int value;
};

TEST(QueryTask, executeShouldEmitAnErrorIfQueryIsANullPointer) {
  auto task = QueryTask(std::unique_ptr<Query>());
  auto finishedSpy = QSignalSpy(&task, &Task::finished);
  auto errorSpy = QSignalSpy(&task, &Task::error);

  task.execute();

  EXPECT_EQ(0, finishedSpy.count());
  ASSERT_EQ(1, errorSpy.count());
  EXPECT_EQ("Attempted to execute a query with no query set!",
            errorSpy.takeFirst().at(0).value<std::string>());
}

TEST(QueryTask, executeShouldEmitAnErrorIfQueryExecutionThrowsAnException) {
  auto task = QueryTask(std::make_unique<TestQuery>(-1));
  auto finishedSpy = QSignalSpy(&task, &Task::finished);
  auto errorSpy = QSignalSpy(&task, &Task::error);

  task.execute();

  EXPECT_EQ(0, finishedSpy.count());
  ASSERT_EQ(1, errorSpy.count());
  EXPECT_EQ(GENERIC_QUERY_ERROR_MESSAGE,
            errorSpy.takeFirst().at(0).value<std::string>());
}

TEST(QueryTask, executeShouldEmitAFinishedIfQueryExecutionSucceeds) {
  auto task = QueryTask(std::make_unique<TestQuery>(1));
  auto finishedSpy = QSignalSpy(&task, &Task::finished);
  auto errorSpy = QSignalSpy(&task, &Task::error);

  task.execute();

  ASSERT_EQ(1, finishedSpy.count());
  EXPECT_EQ(0, errorSpy.count());

  auto result = finishedSpy.takeFirst().at(0).value<QueryResult>();
  EXPECT_EQ("1", std::get<PluginItem>(result).name);
}

TEST(SequentialTaskExecutor, shouldRunEachTaskOnceInSeries) {
  std::vector<Task*> tasks;
  std::vector<std::unique_ptr<QSignalSpy>> taskFinishedSpies;
  std::vector<std::unique_ptr<QSignalSpy>> taskErroredSpies;

  QElapsedTimer timer;

  for (int i = 0; i < 100; i += 1) {
    auto task = new NonBlockingTestTask(false, timer);

    auto taskFinishedSpy = std::make_unique<QSignalSpy>(task, &Task::finished);
    auto taskErroredSpy = std::make_unique<QSignalSpy>(task, &Task::error);

    tasks.push_back(task);

    taskFinishedSpies.push_back(std::move(taskFinishedSpy));
    taskErroredSpies.push_back(std::move(taskErroredSpy));
  }

  auto executor = SequentialTaskExecutor(nullptr, tasks);

  auto executorStartSpy = QSignalSpy(&executor, &TaskExecutor::start);
  auto executorFinishedSpy = QSignalSpy(&executor, &TaskExecutor::finished);

  timer.start();
  executor.start();

  qint64 lastEndTimestamp = 0;
  for (int i = 0; i < 100; i += 1) {
    auto taskSpyFinished = taskFinishedSpies[i]->wait();

    EXPECT_TRUE(taskSpyFinished);
    ASSERT_EQ(1, taskFinishedSpies[i]->count());
    EXPECT_EQ(0, taskErroredSpies[i]->count());

    auto queryResult =
        taskFinishedSpies[i]->takeFirst().at(0).value<QueryResult>();

    // It's important that each task's start time is after (or equal to, due
    // to clock precision) the last task's end time. Those timestamps are
    // stored as elements in a CancelSortResult result.
    auto result = std::get<CancelSortResult>(queryResult);
    auto startTimestamp = std::stoll(result.at(0).first);
    auto endTimestamp = std::stoll(result.at(1).first);

    EXPECT_LT(lastEndTimestamp, startTimestamp);
    EXPECT_LT(startTimestamp, endTimestamp);
    lastEndTimestamp = endTimestamp;
  }

  auto spyFinished =
      executorFinishedSpy.count() == 1 || executorFinishedSpy.wait();

  EXPECT_TRUE(spyFinished);
  EXPECT_EQ(100, executorStartSpy.count());
  EXPECT_EQ(1, executorFinishedSpy.count());
}

TEST(SequentialTaskExecutor, shouldRunUntilTheFirstError) {
  std::vector<Task*> tasks;
  std::vector<std::unique_ptr<QSignalSpy>> taskFinishedSpies;
  std::vector<std::unique_ptr<QSignalSpy>> taskErroredSpies;

  QElapsedTimer timer;

  for (int i = 0; i < 100; i += 1) {
    auto task = new NonBlockingTestTask(i == 50, timer);

    auto taskFinishedSpy = std::make_unique<QSignalSpy>(task, &Task::finished);
    auto taskErroredSpy = std::make_unique<QSignalSpy>(task, &Task::error);

    tasks.push_back(task);

    taskFinishedSpies.push_back(std::move(taskFinishedSpy));
    taskErroredSpies.push_back(std::move(taskErroredSpy));
  }

  auto executor = SequentialTaskExecutor(nullptr, tasks);

  auto executorStartSpy = QSignalSpy(&executor, &TaskExecutor::start);
  auto executorFinishedSpy = QSignalSpy(&executor, &TaskExecutor::finished);

  timer.start();
  executor.start();

  qint64 lastEndTimestamp = 0;
  for (int i = 0; i < 100; i += 1) {
    if (i < 50) {
      auto taskSpyFinished = taskFinishedSpies[i]->wait();

      EXPECT_TRUE(taskSpyFinished);
      ASSERT_EQ(1, taskFinishedSpies[i]->count());
      EXPECT_EQ(0, taskErroredSpies[i]->count());

      auto queryResult =
          taskFinishedSpies[i]->takeFirst().at(0).value<QueryResult>();

      // It's important that each task's start time is after (or equal to, due
      // to clock precision) the last task's end time. Those timestamps are
      // stored as elements in a CancelSortResult result.
      auto result = std::get<CancelSortResult>(queryResult);
      auto startTimestamp = std::stoll(result.at(0).first);
      auto endTimestamp = std::stoll(result.at(1).first);

      EXPECT_LT(lastEndTimestamp, startTimestamp);
      EXPECT_LT(startTimestamp, endTimestamp);
      lastEndTimestamp = endTimestamp;
    } else if (i == 50) {
      auto taskSpyFinished = taskErroredSpies[i]->wait();

      EXPECT_TRUE(taskSpyFinished);
      ASSERT_EQ(0, taskFinishedSpies[i]->count());
      EXPECT_EQ(1, taskErroredSpies[i]->count());

      EXPECT_EQ(NonBlockingTestTask::ERROR_MESSAGE,
                taskErroredSpies[i]->takeFirst().at(0).value<std::string>());
    } else {
      auto taskFinishedSpyFinished = taskFinishedSpies[i]->wait(20);
      auto taskErrorSpyFinished = taskErroredSpies[i]->wait(20);

      EXPECT_FALSE(taskFinishedSpyFinished);
      EXPECT_FALSE(taskErrorSpyFinished);
    }
  }

  auto spyFinished =
      executorFinishedSpy.count() == 1 || executorFinishedSpy.wait();

  EXPECT_TRUE(spyFinished);
  EXPECT_EQ(51, executorStartSpy.count());
  EXPECT_EQ(1, executorFinishedSpy.count());
}

TEST(SequentialTaskExecutor, shouldDeleteTasksOnceFinished) {
  QElapsedTimer timer;
  auto task = new NonBlockingTestTask(false, timer);
  auto taskDestroyedSpy = QSignalSpy(task, &QObject::destroyed);

  auto executor = SequentialTaskExecutor(nullptr, {task});

  auto executorFinishedSpy = QSignalSpy(&executor, &TaskExecutor::finished);

  timer.start();
  executor.start();

  auto spyFinished = executorFinishedSpy.wait();
  auto taskDestroyedSpyFinished =
      taskDestroyedSpy.count() == 1 || taskDestroyedSpy.wait();

  EXPECT_TRUE(spyFinished);
  EXPECT_TRUE(taskDestroyedSpyFinished);
  EXPECT_EQ(1, executorFinishedSpy.count());
  EXPECT_EQ(1, taskDestroyedSpy.count());
}
}
}

#endif
