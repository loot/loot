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
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <boost/spirit/include/karma.hpp>

#include "loot/error.h"

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

using boost::locale::translate;
using std::string;
using std::wstring;

namespace loot {
    //Calculate the CRC of the given file for comparison purposes.
uint32_t GetCrc32(const boost::filesystem::path& filename) {
  uint32_t chksum = 0;
  try {
    boost::filesystem::ifstream ifile(filename, std::ios::binary);
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
    } else
      throw std::exception();
  } catch (std::exception&) {
    BOOST_LOG_TRIVIAL(error) << "Unable to open \"" << filename.string() << "\" for CRC calculation.";
    throw Error(Error::Code::path_read_fail, (boost::format(translate("Unable to open \"%1%\" for CRC calculation.")) % filename.string()).str());
  }
  BOOST_LOG_TRIVIAL(debug) << "CRC32(\"" << filename.string() << "\"): " << std::hex << chksum << std::dec;
  return chksum;
}

//Converts an integer to a hex string using BOOST's Spirit.Karma, which is apparently a lot faster than a stringstream conversion...
std::string IntToHexString(const uint32_t n) {
  namespace karma = boost::spirit::karma;

  string out;
  std::back_insert_iterator<string> sink(out);
  karma::generate(sink, karma::upper[karma::hex], n);

  return out;
}

//Turns an absolute filesystem path into a valid file:// URL.
std::string ToFileURL(const boost::filesystem::path& file) {
  BOOST_LOG_TRIVIAL(trace) << "Converting file path " << file << " to a URL.";
  string url;

#ifdef _WIN32
  wstring wstr(MAX_PATH, 0);
  DWORD len = MAX_PATH;
  UrlCreateFromPath(ToWinWide(file.string()).c_str(), &wstr[0], &len, NULL);
  url = FromWinWide(wstr.c_str());  // Passing c_str() cuts off any unused buffer.
  BOOST_LOG_TRIVIAL(trace) << "Converted to: " << url;
#else
        // Let's be naive about this.
  url = "file://" + file.string();
#endif

  return url;
}

//Opens the file in its registered default application.
void OpenInDefaultApplication(const boost::filesystem::path& file) {
#ifdef _WIN32
  HINSTANCE ret = ShellExecute(0, NULL, ToWinWide(file.string()).c_str(), NULL, NULL, SW_SHOWNORMAL);
  if ((int)ret <= 32)
    throw Error(Error::Code::windows_error, translate("Failed to open file in its default application."));
#else
  if (system(("/usr/bin/xdg-open" + file.string()).c_str()) != 0)
    throw Error(Error::Code::windows_error, translate("Failed to open file in its default application."));
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
    BOOST_LOG_TRIVIAL(error) << "Failed to get string value.";
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
