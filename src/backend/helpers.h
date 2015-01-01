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

#ifndef __LOOT_HELPERS__
#define __LOOT_HELPERS__

#include "metadata.h"

#include <cstdint>
#include <string>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

namespace loot {
    /// Array used to try each of the expressions defined using
    /// an iteration for each of them.
    extern const boost::regex version_checks[7];

    //////////////////////////////////////////////////////////////////////////
    // Helper functions
    //////////////////////////////////////////////////////////////////////////

    //Calculate modulo with dividend sign preserved, matching behaviour of C++11.
    int modulo(int dividend, int divisor);

    //Calculate the CRC of the given file for comparison purposes.
    uint32_t GetCrc32(const boost::filesystem::path& filename);

    //Converts an integer to a hex string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
    std::string IntToHexString(const int n);

#ifdef _WIN32
    //Get registry subkey value string.
    std::string RegKeyStringValue(const std::string& keyStr, const std::string& subkey, const std::string& value);

    //Get the local application data path, within which LOOT's data folder should be stored.
    boost::filesystem::path GetLocalAppDataPath();

    //Turns an absolute filesystem path into a valid file:// URL.
    std::string ToFileURL(const boost::filesystem::path& file);

    //Helper to turn UTF8 strings into strings that can be used by WinAPI.
    std::wstring ToWinWide(const std::string& str);

    std::string FromWinWide(const std::wstring& wstr);
#endif

    //Language class for simpler language support.
    class Language {
    public:
        Language(const unsigned int code);
        Language(const std::string& nameOrCode);

        unsigned int Code() const;
        std::string Name() const;
        std::string Locale() const;

        static const unsigned int any = 0; // This shouldn't be used as a selectable language, just for when picking any string in a message.
        static const unsigned int english = 1;
        static const unsigned int spanish = 2;
        static const unsigned int russian = 3;
        static const unsigned int french = 4;
        static const unsigned int chinese = 5;
        static const unsigned int polish = 6;
        static const unsigned int brazilian_portuguese = 7;
        static const unsigned int finnish = 8;
        static const unsigned int german = 9;
        static const unsigned int danish = 10;

        static std::vector<std::string> Names() {
            std::vector<std::string> vec;
            vec.push_back(Language(Language::english).Name());
            vec.push_back(Language(Language::spanish).Name());
            vec.push_back(Language(Language::russian).Name());
            vec.push_back(Language(Language::french).Name());
            vec.push_back(Language(Language::chinese).Name());
            vec.push_back(Language(Language::polish).Name());
            vec.push_back(Language(Language::brazilian_portuguese).Name());
            vec.push_back(Language(Language::finnish).Name());
            vec.push_back(Language(Language::german).Name());
            vec.push_back(Language(Language::danish).Name());
            return vec;
        }
    private:
        unsigned int _code;
        std::string _name;
        std::string _locale;

        void Construct(const unsigned int code);
    };

    //Version class for more robust version comparisons.
    class Version {
    private:
        std::string verString;
    public:
        Version();
        Version(const std::string& ver);
        Version(const boost::filesystem::path& file);
        Version(const Plugin& plugin);

        std::string AsString() const;

        bool operator > (const Version&) const;
        bool operator < (const Version&) const;
        bool operator >= (const Version&) const;
        bool operator <= (const Version&) const;
        bool operator == (const Version&) const;
        bool operator != (const Version&) const;
    };
}

#endif
