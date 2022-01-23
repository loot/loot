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

#include "gui/qt/general_info.h"

#include "gui/helpers.h"

namespace loot {
std::string GeneralInformation::getMarkdownContent() const {
  std::string content = "# General Information\n\n";

  content += "## Masterlist Revision\n\n";

  content += "- ID: " + masterlistRevision.id + "\n";
  content += "- Update Date: " + masterlistRevision.date + "\n\n";

  content += "## Masterlist Prelude Revision\n\n";

  content += "- ID: " + preludeRevision.id + "\n";
  content += "- Update Date: " + preludeRevision.date + "\n\n";

  if (!generalMessages.empty()) {
    content += messagesAsMarkdown(generalMessages);
  }

  content += "\n";

  return content;
}
}
