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
}
}

#endif
