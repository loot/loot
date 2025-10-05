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

#ifndef LOOT_TESTS_GUI_QT_NON_BLOCKING_TEST_TASK
#define LOOT_TESTS_GUI_QT_NON_BLOCKING_TEST_TASK

#include <QtCore/QElapsedTimer>
#include <string>

#include "gui/qt/tasks/tasks.h"

namespace loot {
namespace test {
class NonBlockingTestTask : public Task {
  Q_OBJECT
public:
  static constexpr const char* ERROR_MESSAGE = "Task errored";

  NonBlockingTestTask(bool fail, const QElapsedTimer& timer);

public slots:
  void execute() override;

private:
  const bool fail;
  const QElapsedTimer* timer;
};
}
}

#endif
