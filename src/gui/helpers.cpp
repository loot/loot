/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2014 WrinklyNinja

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

#include "gui/helpers.h"

#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>
#else
#include <unicode/uchar.h>
#include <unicode/unistr.h>
using icu::UnicodeString;
#endif

#include <boost/format.hpp>

#include "gui/state/logging.h"

namespace loot {
void OpenInDefaultApplication(const std::filesystem::path& file) {
#ifdef _WIN32
  HINSTANCE ret =
      ShellExecute(0, NULL, file.wstring().c_str(), NULL, NULL, SW_SHOWNORMAL);
  if (reinterpret_cast<uintptr_t>(ret) <= 32)
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to open file in its default application.");
#else
  if (system(("/usr/bin/xdg-open " + file.u8string()).c_str()) != 0)
    throw std::system_error(errno,
                            std::system_category(),
                            "Failed to open file in its default application.");
#endif
}

#ifdef _WIN32
std::wstring ToWinWide(const std::string& str) {
  size_t len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), 0, 0);
  std::wstring wstr(len, 0);
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], len);
  return wstr;
}

std::string FromWinWide(const std::wstring& wstr) {
  size_t len = WideCharToMultiByte(
      CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
  std::string str(len, 0);
  WideCharToMultiByte(
      CP_UTF8, 0, wstr.c_str(), wstr.length(), &str[0], len, NULL, NULL);
  return str;
}

std::string RegKeyStringValue(const std::string& rootKey,
                              const std::string& subkey,
                              const std::string& value) {
  HKEY hKey = NULL;
  DWORD len = MAX_PATH;
  std::wstring wstr(MAX_PATH, 0);

  if (rootKey == "HKEY_CLASSES_ROOT")
    hKey = HKEY_CLASSES_ROOT;
  else if (rootKey == "HKEY_CURRENT_CONFIG")
    hKey = HKEY_CURRENT_CONFIG;
  else if (rootKey == "HKEY_CURRENT_USER")
    hKey = HKEY_CURRENT_USER;
  else if (rootKey == "HKEY_LOCAL_MACHINE")
    hKey = HKEY_LOCAL_MACHINE;
  else if (rootKey == "HKEY_USERS")
    hKey = HKEY_USERS;
  else
    throw std::invalid_argument("Invalid registry key given.");

  auto logger = getLogger();
  if (logger) {
    logger->trace(
        "Getting string for registry key, subkey and value: {}, {}, "
        "{}",
        rootKey,
        subkey,
        value);
  }

  LONG ret = RegGetValue(hKey,
                         ToWinWide(subkey).c_str(),
                         ToWinWide(value).c_str(),
                         RRF_RT_REG_SZ | RRF_SUBKEY_WOW6432KEY,
                         NULL,
                         &wstr[0],
                         &len);

  if (ret != ERROR_SUCCESS) {
    // Try again using the native registry view. On 32-bit Windows
    // this just does the same thing again. I don't think it's worth
    // trying to skip for the few 32-bit Windows users that remain.
    logger->info(
        "Failed to get string value from 32-bit Registry view, trying 64-bit "
        "Registry view.");
    ret = RegGetValue(hKey,
                      ToWinWide(subkey).c_str(),
                      ToWinWide(value).c_str(),
                      RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY,
                      NULL,
                      &wstr[0],
                      &len);
  }

  if (ret == ERROR_SUCCESS) {
    // Passing c_str() cuts off any unused buffer.
    std::string value = FromWinWide(wstr.c_str());
    if (logger) {
      logger->info("Found string: {}", value);
    }
    return value;
  } else {
    if (logger) {
      logger->info("Failed to get string value.");
    }
    return "";
  }
}
#endif

int CompareFilenames(const std::string& lhs, const std::string& rhs) {
#ifdef _WIN32
  // On Windows, use CompareStringOrdinal as that will perform case conversion
  // using the operating system uppercase table information, which (I think)
  // will give results that match the filesystem, and is not locale-dependent.
  int result = CompareStringOrdinal(
      ToWinWide(lhs).c_str(), -1, ToWinWide(rhs).c_str(), -1, true);
  switch (result) {
    case CSTR_LESS_THAN:
      return -1;
    case CSTR_EQUAL:
      return 0;
    case CSTR_GREATER_THAN:
      return 1;
    default:
      throw std::invalid_argument(
          "One of the filenames to compare was invalid.");
  }
#else
  auto unicodeLhs = UnicodeString::fromUTF8(lhs);
  auto unicodeRhs = UnicodeString::fromUTF8(rhs);
  return unicodeLhs.caseCompare(unicodeRhs, U_FOLD_CASE_DEFAULT);
#endif
}

std::filesystem::path getExecutableDirectory() {
#ifdef _WIN32
  // Despite its name, paths can be longer than MAX_PATH, just not by default.
  // FIXME: Make this work with long paths.
  std::wstring executablePathString(MAX_PATH, 0);

  if (GetModuleFileName(NULL, &executablePathString[0], MAX_PATH) == 0) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to get LOOT executable path.");
    }
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to get LOOT executable path.");
  }

  return std::filesystem::path(executablePathString).parent_path();
#else
  char result[PATH_MAX];

  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  if (count < 0) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to get LOOT executable path.");
    }
    throw std::system_error(
        count, std::system_category(), "Failed to get LOOT executable path.");
  }

  return std::filesystem::u8path(std::string(result, count)).parent_path();
#endif
}

std::filesystem::path getLocalAppDataPath() {
#ifdef _WIN32
  HWND owner = 0;
  PWSTR path;

  if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path) != S_OK)
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to get %LOCALAPPDATA% path.");

  std::filesystem::path localAppDataPath(path);
  CoTaskMemFree(path);

  return localAppDataPath;
#else
  // Use XDG_CONFIG_HOME environmental variable if it's available.
  const char* xdgConfigHome = getenv("XDG_CONFIG_HOME");

  if (xdgConfigHome != nullptr)
    return std::filesystem::u8path(xdgConfigHome);

  // Otherwise, use the HOME env. var. if it's available.
  xdgConfigHome = getenv("HOME");

  if (xdgConfigHome != nullptr)
    return std::filesystem::u8path(xdgConfigHome) / ".config";

  // If somehow both are missing, use the executable's directory.
  return getExecutableDirectory();
#endif
}

MessageType mapMessageType(const std::string& type) {
  if (type == "say") {
    return MessageType::say;
  } else if (type == "warn") {
    return MessageType::warn;
  } else {
    return MessageType::error;
  }
}

void CopyToClipboard(const std::string& text) {
#ifdef _WIN32
  if (!OpenClipboard(NULL)) {
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to open the Windows clipboard.");
  }

  if (!EmptyClipboard()) {
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to empty the Windows clipboard.");
  }

  // The clipboard takes a Unicode (ie. UTF-16) string that it then owns and
  // must not be destroyed by LOOT. Convert the string, then copy it into a
  // new block of memory for the clipboard.
  std::wstring wtext = ToWinWide(text);
  wchar_t* wcstr = new wchar_t[wtext.length() + 1];
  wcscpy(wcstr, wtext.c_str());

  if (SetClipboardData(CF_UNICODETEXT, wcstr) == NULL) {
    throw std::system_error(
        GetLastError(),
        std::system_category(),
        "Failed to copy metadata to the Windows clipboard.");
  }

  if (!CloseClipboard()) {
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to close the Windows clipboard.");
  }
#else
  std::string copyCommand = "echo '" + text + "' | xclip -selection clipboard";
  int returnCode = system(copyCommand.c_str());

  if (returnCode != 0) {
    throw std::system_error(
        returnCode,
        std::system_category(),
        "Failed to run clipboard copy command: " + copyCommand);
  }
#endif
}

std::string crcToString(uint32_t crc) {
  return (boost::format("%08X") % crc).str();
}

std::string messagesAsMarkdown(const std::vector<SimpleMessage>& messages) {
  if (messages.empty()) {
    return "";
  }

  std::string content = "## Messages\n\n";

  for (const auto& message : messages) {
    content += "- ";

    if (message.type == MessageType::warn) {
      content += "Warning: ";
    } else if (message.type == MessageType::error) {
      content += "Error: ";
    } else {
      content += "Note: ";
    }

    content += message.text + "\n";
  }

  return content;
}
}
