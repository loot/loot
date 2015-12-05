/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2015    WrinklyNinja

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
    <http://www.gnu.org/licenses/>.
    */

#include "helpers.h"
#include "version.h"

#include <regex>

#include <boost/algorithm/string.hpp>
#include <pseudosem.h>

#ifdef _WIN32
#   ifndef UNICODE
#       define UNICODE
#   endif
#   ifndef _UNICODE
#      define _UNICODE
#   endif
#   include "windows.h"
#endif

namespace loot {
    using namespace std;

    /// REGEX expression definition
    ///  Each expression is composed of three parts:
    ///    1. The marker string "version", "ver", "rev", "v" or "r"
    ///    2. The version string itself.

    const char* regex1 =
        "^(?:\\bversion\\b[ ]*(?:[:.\\-]?)|\\brevision\\b(?:[:.\\-]?))[ ]*"
        "((?:alpha|beta|test|debug)?\\s*[-0-9a-zA-Z._+]+\\s*(?:alpha|beta|test|debug)?\\s*(?:[0-9]*))$"
        ;

    const char* regex2 =
        "(?:\\bversion\\b(?:[ :]?)|\\brevision\\b(?:[:.\\-]?))[ ]*"
        "([0-9][-0-9a-zA-Z._]+\\+?)"
        ;

    const char* regex3 =
        "(?:\\bver(?:[:.]?)|\\brev(?:[:.]?))\\s*"
        "([0-9][-0-9a-zA-Z._]*\\+?)"
        ;

    // Matches "Updated: <date>" for the Bashed patch
    const char* regex4 =
        "(?:Updated:)\\s*"
        "([-0-9aAmMpP/ :]+)$"
        ;

    // Matches isolated versions as last resort
    const char* regex5 =
        "(?:(?:\\bv|\\br)(?:\\s?)(?:[-.:])?(?:\\s*))"
        "((?:(?:\\balpha\\b)?|(?:\\bbeta\\b)?)\\s*[0-9]+([-._]*(?!esp|esm)[0-9a-zA-Z]+)*\\+?)"
        ;

    // Matches isolated versions as last resort
    const char* regex6 =
        "((?:(?:\\balpha\\b)?|(?:\\bbeta\\b)?)\\s*\\b[0-9][-0-9a-zA-Z._]*\\+?)$"
        ;

    const char* regex7 =
        "(^\\bmark\\b\\s*\\b[IVX0-9][-0-9a-zA-Z._+]*\\s*(?:alpha|beta|test|debug)?\\s*(?:[0-9]*)?)$"
        ;

    /// Array used to try each of the expressions defined above using
    /// an iteration for each of them.
    const vector<regex> version_checks({
        regex(regex1, regex::ECMAScript | regex::icase),
        regex(regex2, regex::ECMAScript | regex::icase),
        regex(regex3, regex::ECMAScript | regex::icase),
        regex(regex4, regex::ECMAScript | regex::icase),
        regex(regex5, regex::ECMAScript | regex::icase),  //This incorrectly identifies "OBSE v19" where 19 is any integer.
        //regex(regex6, regex::ECMAScript | regex::icase),  //This is responsible for metallicow's false positive.
        regex(regex7, regex::ECMAScript | regex::icase)
    });

    Version::Version() {}

    Version::Version(const std::string& ver) : verString(ver) {
        for (size_t i = 0; i < version_checks.size(); ++i) {
            smatch what;
            if (regex_search(verString, what, version_checks[i])) {
                //Use the first sub-expression match.
                verString = string(what[1].first, what[1].second);
                boost::trim(verString);
                break;
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

            verString = to_string(dwLeftMost) + '.' + to_string(dwSecondLeft) + '.' + to_string(dwSecondRight) + '.' + to_string(dwRightMost);
        }
#else
        // ensure filename has no quote characters in it to avoid command injection attacks
        if (string::npos != file.string().find('"')) {
            // command mostly borrowed from the gnome-exe-thumbnailer.sh script
            // wrestool is part of the icoutils package
            string cmd = "wrestool --extract --raw --type=version \"" + file.string() + "\" | tr '\\0, ' '\\t.\\0' | sed 's/\\t\\t/_/g' | tr -c -d '[:print:]' | sed -r 's/.*Version[^0-9]*([0-9]+(\\.[0-9]+)+).*/\\1/'";

            FILE *fp = popen(cmd.c_str(), "r");

            // read out the version string
            static const uint32_t BUFSIZE = 32;
            char buf[BUFSIZE];
            if (nullptr != fgets(buf, BUFSIZE, fp)) {
                verString = string(buf);
            }
            pclose(fp);
        }
#endif
    }

    string Version::AsString() const {
        return verString;
    }

    bool Version::operator < (const Version& ver) const {
        return pseudosem::compare(this->verString, ver.AsString()) < 0;
    }

    bool Version::operator > (const Version& ver) const {
        return pseudosem::compare(this->verString, ver.AsString()) > 0;
    }

    bool Version::operator >= (const Version& ver) const {
        return pseudosem::compare(this->verString, ver.AsString()) >= 0;
    }

    bool Version::operator <= (const Version& ver) const {
        return pseudosem::compare(this->verString, ver.AsString()) <= 0;
    }

    bool Version::operator == (const Version& ver) const {
        return pseudosem::compare(this->verString, ver.AsString()) == 0;
    }

    bool Version::operator != (const Version& ver) const {
        return pseudosem::compare(this->verString, ver.AsString()) != 0;
    }
}
