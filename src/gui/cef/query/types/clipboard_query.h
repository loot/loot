/*  LOOT

A load order optimisation tool for
Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

Copyright (C) 2014 WrinklyNinja

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

#ifndef LOOT_GUI_QUERY_CLIPBOARD_QUERY
#define LOOT_GUI_QUERY_CLIPBOARD_QUERY

#include <cstdlib>
#include <regex>

#include "gui/cef/query/query.h"
#include "gui/helpers.h"

namespace loot {
class ClipboardQuery : public Query {
protected:
  void copyToClipboard(const std::string& text) {
#ifdef _WIN32
    if (!OpenClipboard(NULL)) {
      throw std::system_error(GetLastError(),
                              std::system_category(),
                              "Failed to open the Windows clipboard.");
    }

    if (!EmptyClipboard()) {
      throw std::system_error(GetLastError(),
                              std::system_category(),
                              "Failed to empty the Windows clipboard.");
    }

    // The clipboard takes a Unicode (ie. UTF-16) string that it then owns and
    // must not be destroyed by LOOT. Convert the string, then copy it into a
    // new block of memory for the clipboard.
    std::wstring wtext = ToWinWide(text);
    wchar_t* wcstr = new wchar_t[wtext.length() + 1];
    wcscpy(wcstr, wtext.c_str());

    if (SetClipboardData(CF_UNICODETEXT, wcstr) == NULL) {
      throw std::system_error(
          GetLastError(),
          std::system_category(),
          "Failed to copy metadata to the Windows clipboard.");
    }

    if (!CloseClipboard()) {
      throw std::system_error(GetLastError(),
                              std::system_category(),
                              "Failed to close the Windows clipboard.");
    }
#else
    std::string copyCommand =
        "echo '" + text + "' | xclip -selection clipboard";
    int returnCode = system(copyCommand.c_str());

    if (returnCode != 0) {
      throw std::system_error(
          returnCode,
          std::system_category(),
          "Failed to run clipboard copy command: " + copyCommand);
    }
#endif
  }
};
}

#endif
