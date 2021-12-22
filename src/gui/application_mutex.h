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

#ifndef LOOT_GUI_APPLICATION_MUTEX
#define LOOT_GUI_APPLICATION_MUTEX

#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace loot {
#ifdef _WIN32
class ApplicationMutexGuard {
public:
  static constexpr const wchar_t * MUTEX_NAME = L"LOOT.Shell.Instance";

  ApplicationMutexGuard() : hMutex(NULL) {
    hMutex = ::CreateMutex(NULL, FALSE, MUTEX_NAME);
  }

  ~ApplicationMutexGuard() {
    if (hMutex != NULL) {
      ::ReleaseMutex(hMutex);
    }
  }

private:
  HANDLE hMutex;
};

bool IsApplicationMutexLocked() {
  return ::OpenMutex(MUTEX_ALL_ACCESS,
                     FALSE,
                     ApplicationMutexGuard::MUTEX_NAME) != NULL;
}
#else
class ApplicationMutexGuard {};

bool IsApplicationMutexLocked() { return false; }
#endif
}
#endif
