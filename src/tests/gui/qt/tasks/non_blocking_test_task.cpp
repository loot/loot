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

#include "tests/gui/qt/tasks/non_blocking_test_task.h"

#include <QtCore/QTimer>

namespace loot {
namespace test {
NonBlockingTestTask::NonBlockingTestTask(bool fail,
                                         const QElapsedTimer& timer) :
    fail(fail), timer(timer) {}

void NonBlockingTestTask::execute() {
  auto start = QString::number(timer.nsecsElapsed()).toStdString();

  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  QTimer::singleShot(5, [this, start]() {
    if (fail) {
      emit error(ERROR_MESSAGE);
      return;
    }

    auto end = QString::number(timer.nsecsElapsed()).toStdString();

    std::vector<std::pair<std::string, std::optional<short>>> result(
        {{start, std::nullopt}, {end, std::nullopt}});

    emit finished(result);
  });
}
}
}
