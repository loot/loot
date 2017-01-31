/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

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

#ifndef LOOT_WINDOWS_ENCODING_CONVERTERS
#define LOOT_WINDOWS_ENCODING_CONVERTERS

#ifdef _WIN32

#include <string>

#   ifndef UNICODE
#       define UNICODE
#   endif
#   ifndef _UNICODE
#      define _UNICODE
#   endif
#   include "windows.h"
#   include "shlobj.h"
#   include "shlwapi.h"

namespace loot {
inline std::wstring ToWinWide(const std::string& str) {
  size_t len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), 0, 0);
  std::wstring wstr(len, 0);
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], len);
  return wstr;
}

inline std::string FromWinWide(const std::wstring& wstr) {
  size_t len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
  std::string str(len, 0);
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &str[0], len, NULL, NULL);
  return str;
}
}

#endif

#endif
