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
#include "../error.h"
#include "streams.h"

#include <boost/spirit/include/karma.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include <cstring>
#include <iostream>
#include <cctype>
#include <cstdio>
#include <ctime>
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
    using boost::algorithm::replace_all;
    using boost::algorithm::replace_first;
    namespace karma = boost::spirit::karma;
    namespace fs = boost::filesystem;
    namespace lc = boost::locale;

    //////////////////////////////////////////////////////////////////////////
    // Helper functions
    //////////////////////////////////////////////////////////////////////////

    //Calculate the CRC of the given file for comparison purposes.
    uint32_t GetCrc32(const fs::path& filename) {
        uint32_t chksum = 0;
        loot::ifstream ifile(filename, ios::binary);
        BOOST_LOG_TRIVIAL(trace) << "Calculating CRC for: " << filename.string();
        boost::crc_32_type result;
        if (ifile) {
            static const size_t buffer_size = 8192;
            char buffer[buffer_size];
            do {
                ifile.read(buffer, buffer_size);
                result.process_bytes(buffer, ifile.gcount());
            } while (ifile);
            chksum = result.checksum();
        }
        else {
            BOOST_LOG_TRIVIAL(error) << "Unable to open \"" << filename.string() << "\" for CRC calculation.";
            throw error(error::path_read_fail, (boost::format(lc::translate("Unable to open \"%1%\" for CRC calculation.")) % filename.string()).str());
        }
        BOOST_LOG_TRIVIAL(debug) << "CRC32(\"" << filename.string() << "\"): " << std::hex << chksum << std::dec;
        return chksum;
    }

    //Converts an integer to a hex string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
    std::string IntToHexString(const int n) {
        string out;
        back_insert_iterator<string> sink(out);
        karma::generate(sink, karma::upper[karma::hex], n);
        return out;
    }

#ifdef _WIN32
    //Get registry subkey value string.
    string RegKeyStringValue(const std::string& keyStr, const std::string& subkey, const std::string& value) {
        HKEY hKey = NULL;
        DWORD len = MAX_PATH;
        wstring wstr(MAX_PATH, 0);

        if (keyStr == "HKEY_CLASSES_ROOT")
            hKey = HKEY_CLASSES_ROOT;
        else if (keyStr == "HKEY_CURRENT_CONFIG")
            hKey = HKEY_CURRENT_CONFIG;
        else if (keyStr == "HKEY_CURRENT_USER")
            hKey = HKEY_CURRENT_USER;
        else if (keyStr == "HKEY_LOCAL_MACHINE")
            hKey = HKEY_LOCAL_MACHINE;
        else if (keyStr == "HKEY_USERS")
            hKey = HKEY_USERS;
        else
            throw error(error::invalid_args, "Invalid registry key given.");

        BOOST_LOG_TRIVIAL(trace) << "Getting string for registry key, subkey and value: " << keyStr << " + " << subkey << " + " << value;
        LONG ret = RegGetValue(hKey,
                               ToWinWide(subkey).c_str(),
                               ToWinWide(value).c_str(),
                               RRF_RT_REG_SZ | KEY_WOW64_32KEY,
                               NULL,
                               &wstr[0],
                               &len);

        if (ret == ERROR_SUCCESS) {
            BOOST_LOG_TRIVIAL(info) << "Found string: " << wstr.c_str();
            return FromWinWide(wstr.c_str());  // Passing c_str() cuts off any unused buffer.
        }
        else {
            BOOST_LOG_TRIVIAL(error) << "Failed to get string value.";
            return "";
        }
    }

    boost::filesystem::path GetLocalAppDataPath() {
        HWND owner = 0;
        TCHAR path[MAX_PATH];

        HRESULT res = SHGetFolderPath(owner, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

        if (res == S_OK)
            return fs::path(path);
        else
            return fs::path("");
    }

    //Turns an absolute filesystem path into a valid file:// URL.
    std::string ToFileURL(const fs::path& file) {
        BOOST_LOG_TRIVIAL(trace) << "Converting file path " << file << " to a URL.";

        wstring wstr(MAX_PATH, 0);
        DWORD len = MAX_PATH;
        UrlCreateFromPath(ToWinWide(file.string()).c_str(), &wstr[0], &len, NULL);
        string str = FromWinWide(wstr.c_str());  // Passing c_str() cuts off any unused buffer.
        BOOST_LOG_TRIVIAL(trace) << "Converted to: " << str;

        return str;
    }

    //Helper to turn UTF8 strings into strings that can be used by WinAPI.
    std::wstring ToWinWide(const std::string& str) {
        int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), 0, 0);
        std::wstring wstr(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], len);
        return wstr;
    }

    std::string FromWinWide(const std::wstring& wstr) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
        std::string str(len, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &str[0], len, NULL, NULL);
        return str;
    }
#endif
}
