/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2017    WrinklyNinja

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

#include "gui/helpers.h"

#include "backend/helpers/helpers.h"

#ifdef _WIN32
#   ifndef UNICODE
#       define UNICODE
#   endif
#   ifndef _UNICODE
#      define _UNICODE
#   endif
#   include "windows.h"
#   include "shlobj.h"
#   include "shlwapi.h"
#endif

namespace loot {
void OpenInDefaultApplication(const boost::filesystem::path& file) {
#ifdef _WIN32
  HINSTANCE ret = ShellExecute(0, NULL, ToWinWide(file.string()).c_str(), NULL, NULL, SW_SHOWNORMAL);
  if ((int)ret <= 32)
    throw std::system_error(GetLastError(), std::system_category(), "Failed to open file in its default application.");
#else
  if (system(("/usr/bin/xdg-open" + file.string()).c_str()) != 0)
    throw std::system_error(errno, std::system_category(), "Failed to open file in its default application.");
#endif
}
}
