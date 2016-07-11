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
    <http://www.gnu.org/licenses/>.
    */

#ifndef LOOT_BACKEND_HELPERS_HELPERS
#define LOOT_BACKEND_HELPERS_HELPERS

#include <regex>
#include <string>

#include <boost/filesystem.hpp>

namespace loot {
    //Calculate the CRC of the given file for comparison purposes.
uint32_t GetCrc32(const boost::filesystem::path& filename);

//Converts an unsigned 32-bit integer to a hex string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
std::string IntToHexString(const uint32_t n);

//Turns an absolute filesystem path into a valid file:// URL.
std::string ToFileURL(const boost::filesystem::path& file);

//Opens the file in its registered default application.
void OpenInDefaultApplication(const boost::filesystem::path& file);

#ifdef _WIN32
    //Get registry subkey value string.
std::string RegKeyStringValue(const std::string& keyStr, const std::string& subkey, const std::string& value);

//Helper to turn UTF8 strings into strings that can be used by WinAPI.
std::wstring ToWinWide(const std::string& str);

std::string FromWinWide(const std::wstring& wstr);
#endif
}

#endif
