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

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <regex>
#include <sstream>

#include <iostream>

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
    using namespace std;

    Version::Version() {}

    Version::Version(const std::string& ver)
        : verString(ver) {}

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

    void Version::Decompose(std::vector<unsigned long>& release,
                            std::vector<std::string>& preRelease) const {
        if (verString.empty())
            return;

        release.clear();
        preRelease.clear();

        // Find first '+' and trim to it.
        string version(verString.substr(0, verString.find('+')));

        // Split release string.
        size_t pos = version.find_first_of(" :_-");
        string relStr(version.substr(0, pos));
        boost::trim_if(relStr, boost::is_any_of("."));

        // Split release portion of version into what should be digit strings.
        vector<string> temp;
        boost::split(temp,
                     relStr,
                     boost::is_any_of("."),
                     boost::token_compress_on);

        if (!temp.empty()) {
            // Function to test if string does not contains only digits.
            auto notDigits = [](const std::string& str) {
                return !std::all_of(str.begin(), str.end(), ::isdigit);
            };

            // Unary string to unsigned long converter.
            auto toul = [](const std::string& str) {
                return stoul(str);
            };

            auto firstNonInt = find_if(temp.begin(), temp.end(), notDigits);

            transform(temp.begin(),
                      firstNonInt,
                      back_inserter(release),
                      toul);

            if (firstNonInt != temp.end())
                preRelease.assign(firstNonInt, temp.end());
        }

        // Now split the pre-release portion of the version string.
        if (pos != string::npos) {
            string preRelStr(version.substr(pos));
            boost::trim_if(preRelStr, boost::is_any_of(". :_-"));

            boost::split(temp,
                         preRelStr,
                         boost::is_any_of(". :_-"),
                         boost::token_compress_on);
            preRelease.insert(preRelease.end(), temp.begin(), temp.end());
        }
    }

    bool Version::operator < (const Version& ver) const {
        /* Version strings have a wide variety of possible formats.
       The precedence rules set out by Semantic Versioning <http://semver.org>
       are sufficient for comparisons, with the following extensions:

       1. Strings must be lowercased before comparison.
       2. Spaces (" "), colons (":") and underscores ("_") should be treated as
       separator characters between prerelease version identifiers.
       3. Version integers should be allowed to contain leading zeroes.
       4. An arbitrary number of version integers should be supported.
       5. If two strings contain a different number of version integers,
       their version integers should be padded (preceding any pre-release
       version or metadata) to equal length before comparison.

       These extensions are to ensure that executable versions and plugin
       versions are well-supported.
       */
        vector<unsigned long> v1Release, v2Release;
        vector<string> v1PreRelease, v2PreRelease;
        this->Decompose(v1Release, v1PreRelease);
        ver.Decompose(v2Release, v2PreRelease);

        // Compare lengths of release vectors, and pad shorter with zeroes.
        if (v1Release.size() < v2Release.size())
            v1Release.resize(v2Release.size(), 0);
        else if (v2Release.size() < v1Release.size())
            v2Release.resize(v1Release.size(), 0);

        // Now compare release numbers one by one.
        for (size_t i = 0; i < v1Release.size(); ++i) {
            if (v1Release[i] < v2Release[i])
                return true;

            if (v2Release[i] < v1Release[i])
                return false;
        }

        // Release numbers are the same. Check pre-release strings. If one has
        // pre-release strings while the other does not, the first is less.
        if (v1PreRelease.empty() != v2PreRelease.empty())
            return !v1PreRelease.empty();

        if (v1PreRelease.empty())
            return false;

        auto isDigits = [](const std::string& str) {
            return std::all_of(str.begin(), str.end(), ::isdigit);
        };

        // Compare pre-release strings one by one.
        size_t i = 0;
        while (i < v1PreRelease.size() && i < v2PreRelease.size()) {
            bool v1IsInt = isDigits(v1PreRelease[i]);
            bool v2IsInt = isDigits(v2PreRelease[i]);

            // Integers have lower precedence than non-integer strings.
            if (v1IsInt != v2IsInt)
                return v1IsInt;

            if (v1IsInt) {
                // Compare integer values.
                unsigned long v1Int = stoul(v1PreRelease[i]);
                unsigned long v2Int = stoul(v2PreRelease[i]);

                if (v1Int < v2Int)
                    return true;
                if (v2Int < v1Int)
                    return false;
            }
            else {
                // Compare string values.
                if (v1PreRelease[i] < v2PreRelease[i])
                    return true;

                if (v2PreRelease[i] < v1PreRelease[i])
                    return false;
            }

            ++i;
        }

        // Have reached the end of one or both pre-release string vectors.
        // If only the end of one was reached, it is less.
        if (v1PreRelease.size() == v2PreRelease.size())
            return false;

        return i == v1PreRelease.size();
    }

    bool Version::operator > (const Version& ver) const {
        return ver < *this;
    }

    bool Version::operator >= (const Version& ver) const {
        return ver < *this || !(*this < ver);
    }

    bool Version::operator <= (const Version& ver) const {
        return *this < ver || !(ver < *this);
    }

    bool Version::operator == (const Version& ver) const {
        return !(*this < ver) && !(ver < *this);
    }

    bool Version::operator != (const Version& ver) const {
        return *this < ver || ver < *this;
    }
}
