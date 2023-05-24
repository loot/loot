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

#ifndef LOOT_GUI_QT_TASKS_UPDATE_MASTERLIST_TASK
#define LOOT_GUI_QT_TASKS_UPDATE_MASTERLIST_TASK

#include <QtNetwork/QNetworkAccessManager>

#include "gui/qt/tasks/network_task.h"

namespace loot {
class UpdateMasterlistTask : public NetworkTask {
  Q_OBJECT
public:
  explicit UpdateMasterlistTask(LootState &state);

public slots:
  void execute() override;

private:
  LootState &state;

  QNetworkAccessManager *networkAccessManager{nullptr};

  bool preludeUpdated{false};
  bool masterlistUpdated{false};

  void updatePrelude();
  void updateMasterlist();
  void finish();

private slots:
  void onMasterlistReplyFinished();
  void onPreludeReplyFinished();
};
}

#endif
