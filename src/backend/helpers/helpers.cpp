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

#ifdef _WIN32
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
