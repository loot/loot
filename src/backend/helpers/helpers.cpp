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

#include "backend/helpers/helpers.h"

#include <cctype>
#include <codecvt>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <locale>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/spirit/include/karma.hpp>

#include "loot/exception/file_access_error.h"

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

using std::string;
using std::wstring;

namespace loot {
size_t GetStreamSize(std::istream& stream) {
  std::streampos startingPosition = stream.tellg();

  stream.seekg(0, std::ios_base::end);
  size_t streamSize = stream.tellg();
  stream.seekg(startingPosition, std::ios_base::beg);
  
  return streamSize;
}

//Calculate the CRC of the given file for comparison purposes.
uint32_t GetCrc32(const boost::filesystem::path& filename) {
  try {
    BOOST_LOG_TRIVIAL(trace) << "Calculating CRC for: " << filename.string();

    boost::filesystem::ifstream ifile(filename, std::ios::binary);
    ifile.exceptions(std::ios_base::badbit | std::ios_base::failbit);

    static const size_t bufferSize = 8192;
    char buffer[bufferSize];
    boost::crc_32_type result;
    size_t bytesLeft = GetStreamSize(ifile);
    while (bytesLeft > 0) {
      if (bytesLeft > bufferSize)
        ifile.read(buffer, bufferSize);
      else
        ifile.read(buffer, bytesLeft);

      result.process_bytes(buffer, ifile.gcount());
      bytesLeft -= ifile.gcount();
    }

    uint32_t checksum = result.checksum();
    BOOST_LOG_TRIVIAL(debug) << "CRC32(\"" << filename.string() << "\"): " << std::hex << checksum << std::dec;
    return checksum;
      
  } catch (std::exception& e) {
    throw FileAccessError((boost::format("Unable to open \"%1%\" for CRC calculation: %2%") % filename.string() % e.what()).str());
  }
}

//Converts an integer to a hex string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
std::string IntToHexString(const uint32_t n) {
  namespace karma = boost::spirit::karma;

  string out;
  std::back_insert_iterator<string> sink(out);
  karma::generate(sink, karma::upper[karma::hex], n);

  return out;
}

//Opens the file in its registered default application.
void OpenInDefaultApplication(const boost::filesystem::path& file) {
#ifdef _WIN32
  HINSTANCE ret = ShellExecute(0, NULL, ToWinWide(file.string()).c_str(), NULL, NULL, SW_SHOWNORMAL);
  if ((int)ret <= 32)
    throw std::system_error(GetLastError(), std::system_category(), "Failed to open file in its default application.");
#else
  if (system(("/usr/bin/xdg-open" + file.string()).c_str()) != 0)
    throw std::system_error(errno, std::system_category(), "Failed to open file in its default application.");
#endif
}

#ifdef _WIN32
    //Get registry subkey value string.
std::string RegKeyStringValue(const std::string& keyStr, const std::string& subkey, const std::string& value) {
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
    throw std::invalid_argument("Invalid registry key given.");

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
  } else {
    BOOST_LOG_TRIVIAL(info) << "Failed to get string value.";
    return "";
  }
}

//Helper to turn UTF8 strings into strings that can be used by WinAPI.
std::wstring ToWinWide(const std::string& str) {
  size_t len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), 0, 0);
  wstring wstr(len, 0);
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], len);
  return wstr;
}

std::string FromWinWide(const std::wstring& wstr) {
  size_t len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
  string str(len, 0);
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &str[0], len, NULL, NULL);
  return str;
}
#endif
}
