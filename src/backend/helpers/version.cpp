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
