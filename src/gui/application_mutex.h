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
class [[maybe_unused]] ApplicationMutexGuard {
public:
  static constexpr const wchar_t* MUTEX_NAME = L"LOOT.Shell.Instance";

  ApplicationMutexGuard() = default;
  ApplicationMutexGuard(const ApplicationMutexGuard&) = delete;
  ApplicationMutexGuard(ApplicationMutexGuard&&) = delete;

  ~ApplicationMutexGuard() {
    if (hMutex != nullptr) {
      ::ReleaseMutex(hMutex);
    }
  }

  ApplicationMutexGuard& operator=(const ApplicationMutexGuard&) = delete;
  ApplicationMutexGuard& operator=(ApplicationMutexGuard&&) = delete;

private:
  HANDLE hMutex{::CreateMutex(nullptr, FALSE, MUTEX_NAME)};
};

bool IsApplicationMutexLocked() {
  return ::OpenMutex(MUTEX_ALL_ACCESS,
                     FALSE,
                     ApplicationMutexGuard::MUTEX_NAME) != nullptr;
}
#else
class [[maybe_unused]] ApplicationMutexGuard {};

bool IsApplicationMutexLocked() { return false; }
#endif
}
#endif
