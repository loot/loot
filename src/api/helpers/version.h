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

#ifndef LOOT_BACKEND_HELPERS_VERSION
#define LOOT_BACKEND_HELPERS_VERSION

#include <regex>
#include <string>

#include <boost/filesystem.hpp>

namespace loot {
    //Version class for more robust version comparisons.
class Version {
public:
  Version();
  Version(const std::string& ver);
  Version(const boost::filesystem::path& file);

  std::string AsString() const;

  bool operator > (const Version&) const;
  bool operator < (const Version&) const;
  bool operator >= (const Version&) const;
  bool operator <= (const Version&) const;
  bool operator == (const Version&) const;
  bool operator != (const Version&) const;
private:
  std::string verString_;
  static const std::vector<std::regex> versionRegexes;
};
}

#endif
