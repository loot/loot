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

#ifndef LOOT_GUI_QUERY_COPY_LOAD_ORDER_QUERY
#define LOOT_GUI_QUERY_COPY_LOAD_ORDER_QUERY

#include "gui/query/clipboard_query.h"

namespace loot {
class CopyLoadOrderQuery : public ClipboardQuery {
public:
  CopyLoadOrderQuery(LootState& state, const std::vector<std::string>& plugins) :
    state_(state), plugins_(plugins) {}

  std::string executeLogic() {
    int numberOfIndexDigits = getNumberOfIndexDigits();

    size_t activeIndex = 0;
    std::stringstream stream;
    for (const auto& pluginName : plugins_) {
      activeIndex += writePluginLine(stream, pluginName, activeIndex);
    }

    copyToClipboard(stream.str());
    return "";
  }

private:
  unsigned short getNumberOfIndexDigits() const {
    if (plugins_.size() > 99)
      return 3;
    else if (plugins_.size() > 9)
      return 2;
    else
      return 1;
  }

  size_t writePluginLine(std::ostream& stream, const std::string& plugin, size_t activeIndex) {
    if (state_.getCurrentGame().IsPluginActive(plugin)) {
      stream << std::setw(getNumberOfIndexDigits()) << activeIndex << " " << std::hex << std::setw(2) << activeIndex << std::dec << " " << plugin << "\r\n";
      return 1;
    }

    stream << std::setw(getNumberOfIndexDigits() + 4) << "     " << plugin << "\r\n";
    return 0;
  }

  LootState& state_;
  const std::vector<std::string> plugins_;
};
}

#endif
