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
#include "backend/helpers/version.h"

#include <regex>

#include <boost/algorithm/string.hpp>
#include <pseudosem.h>

#include "backend/helpers/helpers.h"

#ifdef _WIN32
#   ifndef UNICODE
#       define UNICODE
#   endif
#   ifndef _UNICODE
#      define _UNICODE
#   endif
#   include "windows.h"
#endif

using std::regex;

namespace loot {
    /* The string below matches timestamps that use forwardslashes for date
       separators. However, Pseudosem v1.0.1 will only compare the first
       two digits as it does not recognise forwardslashes as separators. */
const std::string dateRegex = R"((\d{1,2}/\d{1,2}/\d{1,4} \d{1,2}:\d{1,2}:\d{1,2}))";

/* The string below matches the range of version strings supported by
   Pseudosem v1.0.1, excluding space separators, as they make version
   extraction from inside sentences very tricky and have not been
   seen "in the wild". */
const std::string pseudosemVersionRegex =
R"((\d+(?:\.\d+)+(?:[-._:]?[A-Za-z0-9]+)*))"
// The string below prevents version numbers followed by a comma from
// matching.
R"((?!,))";

/* There are a few different version formats that can appear in strings
   together, and in order to extract the correct one, they must be searched
   for in order of priority. */
const std::vector<regex> Version::versionRegexes({
    regex(dateRegex, regex::ECMAScript | regex::icase),
    regex(R"(version:?\s)" +
          pseudosemVersionRegex, regex::ECMAScript | regex::icase),
    regex(R"((?:^|v|\s))" +
          pseudosemVersionRegex, regex::ECMAScript | regex::icase),
    regex(
          /* The string below matches a number containing one or more digits
             found at the start of the search string or preceded by 'v'. */
          R"((?:^|v)(\d+))", regex::ECMAScript | regex::icase),
});

Version::Version() {}

Version::Version(const std::string& ver) {
  std::smatch what;
  for (const auto& versionRegex : versionRegexes) {
    if (std::regex_search(ver, what, versionRegex)) {
      for (auto it = next(begin(what)); it != end(what); ++it) {
        if (it->str().empty())
          continue;

      //Use the first non-empty sub-match.
        verString_ = *it;
        boost::trim(verString_);
        return;
      }
    }
  }
}

Version::Version(const boost::filesystem::path& file) {
#ifdef _WIN32
  DWORD dummy = 0;
  DWORD size = GetFileVersionInfoSize(ToWinWide(file.string()).c_str(), &dummy);

  if (size > 0) {
    LPBYTE point = new BYTE[size];
    UINT uLen;
    VS_FIXEDFILEINFO *info;

    GetFileVersionInfo(ToWinWide(file.string()).c_str(), 0, size, point);

    VerQueryValue(point, L"\\", (LPVOID *)&info, &uLen);

    DWORD dwLeftMost = HIWORD(info->dwFileVersionMS);
    DWORD dwSecondLeft = LOWORD(info->dwFileVersionMS);
    DWORD dwSecondRight = HIWORD(info->dwFileVersionLS);
    DWORD dwRightMost = LOWORD(info->dwFileVersionLS);

    delete[] point;

    verString_ = std::to_string(dwLeftMost) + '.' + std::to_string(dwSecondLeft) + '.' + std::to_string(dwSecondRight) + '.' + std::to_string(dwRightMost);
  }
#else
        // ensure filename has no quote characters in it to avoid command injection attacks
  if (std::string::npos != file.string().find('"')) {
      // command mostly borrowed from the gnome-exe-thumbnailer.sh script
      // wrestool is part of the icoutils package
    std::string cmd = "wrestool --extract --raw --type=version \"" + file.string() + "\" | tr '\\0, ' '\\t.\\0' | sed 's/\\t\\t/_/g' | tr -c -d '[:print:]' | sed -r 's/.*Version[^0-9]*([0-9]+(\\.[0-9]+)+).*/\\1/'";

    FILE *fp = popen(cmd.c_str(), "r");

    // read out the version string
    static const uint32_t BUFSIZE = 32;
    char buf[BUFSIZE];
    if (nullptr != fgets(buf, BUFSIZE, fp)) {
      verString_ = std::string(buf);
    }
    pclose(fp);
  }
#endif
}

std::string Version::AsString() const {
  return verString_;
}

bool Version::operator < (const Version& ver) const {
  return pseudosem::compare(this->verString_, ver.AsString()) < 0;
}

bool Version::operator > (const Version& ver) const {
  return pseudosem::compare(this->verString_, ver.AsString()) > 0;
}

bool Version::operator >= (const Version& ver) const {
  return pseudosem::compare(this->verString_, ver.AsString()) >= 0;
}

bool Version::operator <= (const Version& ver) const {
  return pseudosem::compare(this->verString_, ver.AsString()) <= 0;
}

bool Version::operator == (const Version& ver) const {
  return pseudosem::compare(this->verString_, ver.AsString()) == 0;
}

bool Version::operator != (const Version& ver) const {
  return pseudosem::compare(this->verString_, ver.AsString()) != 0;
}
}
