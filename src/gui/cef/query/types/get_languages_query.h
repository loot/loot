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

#ifndef LOOT_GUI_QUERY_GET_LANGUAGES_QUERY
#define LOOT_GUI_QUERY_GET_LANGUAGES_QUERY

#include "gui/cef/query/json.h"
#include "gui/cef/query/query.h"

namespace loot {
class GetLanguagesQuery : public Query {
public:
  std::string executeLogic() {
    BOOST_LOG_TRIVIAL(info) << "Getting LOOT's supported languages.";
    return getLanguagesAsJson();
  }

private:
  static std::string getLanguagesAsJson() {
    YAML::Node response;
    for (const auto& language : getLanguages()) {
      YAML::Node lang;
      lang["locale"] = language.first;
      lang["name"] = language.second;
      response["languages"].push_back(lang);
    }

    return JSON::stringify(response);
  }

  static std::map<std::string, std::string> getLanguages() {
    return std::map<std::string, std::string>({
      {"da", "Dansk"},
      {"de", "Deutsch"},
      {"en", "English"},
      {"es", "Español"},
      {"fi", "suomi"},
      {"fr", "Français"},
      {"ko", "한국어"},
      {"pl", "Polski"},
      {"pt_BR", "Português do Brasil"},
      {"ru", "Русский"},
      {"sv", "Svenska"},
      {"zh_CN", "简体中文"},
      {"ja", "日本語"},
    });
  }
};
}

#endif
