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

#include <alphanum.hpp>

#include <regex>
#include <sstream>

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

    bool Version::operator < (const Version& ver) const {
        //Version string could have a wide variety of formats. Use regex to choose specific comparison types.

        regex reg1("(\\d+\\.?)+");  //a.b.c.d.e.f.... where the letters are all integers, and 'a' is the shortest possible match.

        //regex reg2("(\\d+\\.?)+([a-zA-Z\\-]+(\\d+\\.?)*)+");  //Matches a mix of letters and numbers - from "0.99.xx", "1.35Alpha2", "0.9.9MB8b1", "10.52EV-D", "1.62EV" to "10.0EV-D1.62EV".

        if (regex_match(verString, reg1) && regex_match(ver.AsString(), reg1)) {
            //First type: numbers separated by periods. If two versions have a different number of numbers, then the shorter should be padded
            //with zeros. An arbitrary number of numbers should be supported.
            istringstream parser1(verString);
            istringstream parser2(ver.AsString());
            while (parser1.good() || parser2.good()) {
                //Check if each stringstream is OK for i/o before doing anything with it. If not, replace its extracted value with a 0.
                uint32_t n1, n2;
                if (parser1.good()) {
                    parser1 >> n1;
                    parser1.get();
                }
                else
                    n1 = 0;
                if (parser2.good()) {
                    parser2 >> n2;
                    parser2.get();
                }
                else
                    n2 = 0;
                if (n1 < n2)
                    return true;
                else if (n1 > n2)
                    return false;
            }
            return false;
        }
        else {
            //Wacky format. Use the Alphanum Algorithm. (what a name!)
            return (doj::alphanum_comp(verString, ver.AsString()) < 0);
        }
    }

    bool Version::operator > (const Version& ver) const {
        return (*this != ver && !(*this < ver));
    }

    bool Version::operator >= (const Version& ver) const {
        return (*this == ver || *this > ver);
    }

    bool Version::operator <= (const Version& ver) const {
        return (*this == ver || *this < ver);
    }

    bool Version::operator == (const Version& ver) const {
        return (verString == ver.AsString());
    }

    bool Version::operator != (const Version& ver) const {
        return !(*this == ver);
    }
    }
