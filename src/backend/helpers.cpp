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
#include "error.h"
#include "streams.h"

#include <boost/spirit/include/karma.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/regex.hpp>

#include <alphanum.hpp>

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
    using boost::regex;
    using boost::regex_match;
    using boost::regex_search;
    namespace karma = boost::spirit::karma;
    namespace fs = boost::filesystem;
    namespace lc = boost::locale;

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
    const regex version_checks[7] = {
        regex(regex1, regex::ECMAScript | regex::icase),
        regex(regex2, regex::ECMAScript | regex::icase),
        regex(regex3, regex::ECMAScript | regex::icase),
        regex(regex4, regex::ECMAScript | regex::icase),
        regex(regex5, regex::ECMAScript | regex::icase),  //This incorrectly identifies "OBSE v19" where 19 is any integer.
        regex(regex6, regex::ECMAScript | regex::icase),  //This is responsible for metallicow's false positive.
        regex(regex7, regex::ECMAScript | regex::icase)
    };

    //////////////////////////////////////////////////////////////////////////
    // Helper functions
    //////////////////////////////////////////////////////////////////////////

    //Calculate modulo with dividend sign preserved, matching behaviour of C++11.
    int modulo(int dividend, int divisor) {
        divisor = abs(divisor);
        if (dividend < 0) {
            return -1 * (abs(dividend) % divisor);
        }
        else
            return dividend % divisor;
    }

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
        HKEY hKey, key = NULL;
        DWORD BufferSize = 4096;
        wchar_t val[4096];

        if (keyStr == "HKEY_CLASSES_ROOT")
            key = HKEY_CLASSES_ROOT;
        else if (keyStr == "HKEY_CURRENT_CONFIG")
            key = HKEY_CURRENT_CONFIG;
        else if (keyStr == "HKEY_CURRENT_USER")
            key = HKEY_CURRENT_USER;
        else if (keyStr == "HKEY_LOCAL_MACHINE")
            key = HKEY_LOCAL_MACHINE;
        else if (keyStr == "HKEY_USERS")
            key = HKEY_USERS;

        BOOST_LOG_TRIVIAL(trace) << "Getting registry object for key and subkey: " << keyStr << " + " << subkey;
        LONG ret = RegOpenKeyEx(key, ToWinWide(subkey).c_str(), 0, KEY_READ | KEY_WOW64_32KEY, &hKey);

        if (ret == ERROR_SUCCESS) {
            BOOST_LOG_TRIVIAL(trace) << "Getting value for entry: " << value;
            ret = RegQueryValueEx(hKey, ToWinWide(value).c_str(), NULL, NULL, (LPBYTE)&val, &BufferSize);
            RegCloseKey(hKey);

            if (ret == ERROR_SUCCESS)
                return fs::path(val).string();  //Easiest way to convert from wide to narrow character strings.
            else
                return "";
        }
        else
            return "";
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
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &(wstr[0]), len);
        return wstr;
    }

    std::string FromWinWide(const std::wstring& wstr) {
        int len = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr.length(), NULL, 0, NULL, NULL);
        std::string str(len, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &str[0], len, NULL, NULL);
        return str;
    }
#endif

    Language::Language(const unsigned int code) {
        Construct(code);
    }

    Language::Language(const std::string& nameOrCode) {
        if (nameOrCode == Language(Language::english).Name() || nameOrCode == Language(Language::english).Locale())
            Construct(Language::english);
        else if (nameOrCode == Language(Language::spanish).Name() || nameOrCode == Language(Language::spanish).Locale())
            Construct(Language::spanish);
        else if (nameOrCode == Language(Language::russian).Name() || nameOrCode == Language(Language::russian).Locale())
            Construct(Language::russian);
        else if (nameOrCode == Language(Language::french).Name() || nameOrCode == Language(Language::french).Locale())
            Construct(Language::french);
        else if (nameOrCode == Language(Language::chinese).Name() || nameOrCode == Language(Language::chinese).Locale())
            Construct(Language::chinese);
        else if (nameOrCode == Language(Language::polish).Name() || nameOrCode == Language(Language::polish).Locale())
            Construct(Language::polish);
        else if (nameOrCode == Language(Language::brazilian_portuguese).Name() || nameOrCode == Language(Language::brazilian_portuguese).Locale())
            Construct(Language::brazilian_portuguese);
        else if (nameOrCode == Language(Language::finnish).Name() || nameOrCode == Language(Language::finnish).Locale())
            Construct(Language::finnish);
        else if (nameOrCode == Language(Language::german).Name() || nameOrCode == Language(Language::german).Locale())
            Construct(Language::german);
        else if (nameOrCode == Language(Language::danish).Name() || nameOrCode == Language(Language::danish).Locale())
            Construct(Language::danish);
        else if (nameOrCode == Language(Language::korean).Name() || nameOrCode == Language(Language::korean).Locale())
            Construct(Language::korean);
        else
            Construct(Language::english);
    }

    void Language::Construct(const unsigned int code) {
        _code = code;
        if (_code == Language::spanish) {
            _name = "Español";
            _locale = "es";
        }
        else if (_code == Language::russian) {
            _name = "Русский";
            _locale = "ru";
        }
        else if (_code == Language::french) {
            _name = "Français";
            _locale = "fr";
        }
        else if (_code == Language::chinese) {
            _name = "简体中文";
            _locale = "zh_CN";
        }
        else if (_code == Language::polish) {
            _name = "Polski";
            _locale = "pl";
        }
        else if (_code == Language::brazilian_portuguese) {
            _name = "Português do Brasil";
            _locale = "pt_BR";
        }
        else if (_code == Language::finnish) {
            _name = "suomi";
            _locale = "fi";
        }
        else if (_code == Language::german) {
            _name = "Deutsch";
            _locale = "de";
        }
        else if (_code == Language::danish) {
            _name = "Dansk";
            _locale = "da";
        }
        else if (_code == Language::korean) {
            _name = "한국어";
            _locale = "ko";
        }
        else {
            _name = "English";
            _locale = "en";
        }
    }

    unsigned int Language::Code() const {
        return _code;
    }

    std::string Language::Name() const {
        return _name;
    }

    std::string Language::Locale() const {
        return _locale;
    }

    //////////////////////////////
    // Version Class Functions
    //////////////////////////////

    Version::Version() {}

    Version::Version(const std::string& ver)
        : verString(ver) {}

    Version::Version(const fs::path& file) {
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

    Version::Version(const Plugin& plugin) : verString(plugin.Version()) {}

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
