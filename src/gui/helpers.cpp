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
#include <mntent.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <cstdio>
#endif

#include <fmt/ranges.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <fstream>

#include "gui/state/logging.h"

namespace {
#ifdef _WIN32
std::vector<std::wstring> SplitOnNulls(std::vector<wchar_t> nullDelimitedList) {
  std::vector<std::wstring> elements;

  auto stringStartIt = nullDelimitedList.begin();
  for (auto it = nullDelimitedList.begin(); it != nullDelimitedList.end();
       ++it) {
    if (*it == 0) {
      const std::wstring element(stringStartIt, it);
      elements.push_back(element);

      stringStartIt = std::next(it);
    }
  }

  return elements;
}

std::vector<std::string> GetPreferredUILanguages(
    std::function<bool(DWORD, PULONG, PZZWSTR, PULONG)> win32Function,
    bool isSystem) {
  const auto logger = loot::getLogger();
  const auto systemOrUser = isSystem ? "system" : "user";

  ULONG wszLanguagesBuffer = 0;
  ULONG cchLanguagesBuffer = 0;
  auto result = win32Function(
      MUI_LANGUAGE_NAME, &wszLanguagesBuffer, NULL, &cchLanguagesBuffer);

  if (!result) {
    if (logger) {
      logger->error("Failed to get size of {} preferred UI languages buffer",
                    systemOrUser);
    }

    return {};
  }
  // The buffer contains an ordered list of null-delimited languages, ending
  // with two null characters.
  std::vector<wchar_t> buffer(size_t{cchLanguagesBuffer});

  result = win32Function(MUI_LANGUAGE_NAME,
                         &wszLanguagesBuffer,
                         buffer.data(),
                         &cchLanguagesBuffer);

  if (!result) {
    if (logger) {
      logger->error("Failed to get the {} preferred UI languages",
                    systemOrUser);
    }
    return {};
  }

  std::vector<std::string> languages;

  for (const auto& languageBytes : SplitOnNulls(buffer)) {
    std::wstring language(languageBytes.begin(), languageBytes.end());
    if (!language.empty()) {
      languages.push_back(loot::FromWinWide(language));
    }
  }

  return languages;
}

const char* getDriveTypeText(UINT driveType) {
  switch (driveType) {
    case DRIVE_UNKNOWN:
      return "DRIVE_UNKNOWN";
    case DRIVE_NO_ROOT_DIR:
      return "DRIVE_NO_ROOT_DIR";
    case DRIVE_REMOVABLE:
      return "DRIVE_REMOVABLE";
    case DRIVE_FIXED:
      return "DRIVE_FIXED";
    case DRIVE_REMOTE:
      return "DRIVE_REMOTE";
    case DRIVE_CDROM:
      return "DRIVE_CDROM";
    case DRIVE_RAMDISK:
      return "DRIVE_RAMDISK";
    default:
      return "unknown type";
  }
}
#endif
}

namespace loot {
#ifdef _WIN32
std::wstring ToWinWide(const std::string& str) {
  size_t len = MultiByteToWideChar(
      CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), 0, 0);
  std::wstring wstr(len, 0);
  MultiByteToWideChar(CP_UTF8,
                      0,
                      str.c_str(),
                      static_cast<int>(str.length()),
                      &wstr[0],
                      static_cast<int>(len));
  return wstr;
}

std::string FromWinWide(const std::wstring& wstr) {
  size_t len = WideCharToMultiByte(CP_UTF8,
                                   0,
                                   wstr.c_str(),
                                   static_cast<int>(wstr.length()),
                                   NULL,
                                   0,
                                   NULL,
                                   NULL);
  std::string str(len, 0);
  WideCharToMultiByte(CP_UTF8,
                      0,
                      wstr.c_str(),
                      static_cast<int>(wstr.length()),
                      &str[0],
                      static_cast<int>(len),
                      NULL,
                      NULL);
  return str;
}
#endif

std::vector<std::string> GetPreferredUILanguages() {
  const auto logger = loot::getLogger();

#ifdef _WIN32
  const auto systemPreferredLanguages =
      ::GetPreferredUILanguages(GetSystemPreferredUILanguages, true);
  if (logger) {
    logger->debug("System preferred UI languages are: {}",
                  boost::join(systemPreferredLanguages, ", "));
  }

  const auto userPreferredLanguages =
      ::GetPreferredUILanguages(GetUserPreferredUILanguages, false);
  if (logger) {
    logger->debug("User preferred UI languages are: {}",
                  boost::join(userPreferredLanguages, ", "));
  }

  std::vector<std::string> preferredLanguages = userPreferredLanguages;
  for (const auto& language : systemPreferredLanguages) {
    // Deduplicate languages, this approach doesn't scale well but that doesn't
    // matter, the language lists are very small.
    if (std::find(preferredLanguages.begin(),
                  preferredLanguages.end(),
                  language) == preferredLanguages.end()) {
      preferredLanguages.push_back(language);
    }
  }

  return preferredLanguages;
#else
  std::vector<std::string> languages;

  try {
    const auto language =
        std::use_facet<boost::locale::info>(std::locale("")).language();

    if (logger) {
      logger->debug("Preferred UI language is \"{}\"", language);
    }

    languages = {language};
  } catch (const std::exception& e) {
    if (logger) {
      logger->debug("Failed to get preferred UI language from locale: {}",
                    e.what());
    }
  }

  try {
    if (languages.empty()) {
      const auto language = getenv("LANGUAGE");
      const auto lcAll = getenv("LC_ALL");
      const auto lcMessages = getenv("LC_MESSAGES");
      const auto lang = getenv("LANG");

      if (language != nullptr) {
        boost::split(languages, language, boost::is_any_of(":"));
      } else if (lcAll != nullptr) {
        languages = {lcAll};
      } else if (lcMessages != nullptr) {
        languages = {lcMessages};
      } else if (lang != nullptr) {
        languages = {lang};
      }
    }
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("Failed to get preferred UI language from environment: {}",
                    e.what());
    }
  }

  if (logger) {
    logger->debug("Preferred UI languages are: {}",
                  boost::join(languages, ", "));
  }

  return languages;
#endif
}

std::vector<std::filesystem::path> GetDriveRootPaths() {
#ifdef _WIN32
  const auto maxBufferLength = GetLogicalDriveStrings(0, nullptr);

  if (maxBufferLength == 0) {
    throw std::system_error(
        GetLastError(),
        std::system_category(),
        "Failed to length of the buffer needed to hold all drive root paths");
  }

  // Add space for the terminating null character.
  std::vector<wchar_t> buffer(size_t{maxBufferLength} + 1);

  const size_t stringsLength =
      GetLogicalDriveStrings(static_cast<DWORD>(buffer.size()), buffer.data());

  // Trim any unused buffer bytes.
  buffer.resize(stringsLength);

  std::vector<std::filesystem::path> paths;

  const auto logger = getLogger();
  auto stringStartIt = buffer.begin();
  for (auto it = buffer.begin(); it != buffer.end(); ++it) {
    if (*it == 0) {
      const std::wstring drive(stringStartIt, it);

      // Check the drive type to avoid looking up paths on drives
      // that games won't be installed on, like optical disk drives,
      // network drives, floppy disk drives, etc.
      const auto driveType = GetDriveType(drive.c_str());

      if (driveType == DRIVE_FIXED || driveType == DRIVE_RAMDISK) {
        paths.push_back(std::filesystem::path(drive));
      } else if (logger) {
        logger->info("Skipping found drive {} as it is of type {}",
                     std::filesystem::path(drive).u8string(),
                     getDriveTypeText(driveType));
      }

      stringStartIt = std::next(it);
    }
  }

  return paths;
#else
  FILE* mountsFile = setmntent("/proc/self/mounts", "r");

  if (mountsFile == nullptr) {
    throw std::runtime_error("Failed to open /proc/self/mounts");
  }

  // Java increased their buffer size to 4 KiB:
  // <https://bugs.openjdk.java.net/browse/JDK-8229872>
  // .NET uses 8KiB:
  // <https://github.com/dotnet/runtime/blob/7414af2a5f6d8d99efc27d3f5ef7a394e0b23c42/src/native/libs/System.Native/pal_mount.c#L24>
  static constexpr size_t BUFFER_SIZE = 8192;
  struct mntent entry {};
  std::array<char, BUFFER_SIZE> stringsBuffer{};
  std::vector<std::filesystem::path> paths;

  while (getmntent_r(
             mountsFile, &entry, stringsBuffer.data(), stringsBuffer.size()) !=
         nullptr) {
    paths.push_back(entry.mnt_dir);
  }

  endmntent(mountsFile);

  return paths;
#endif
}

std::optional<std::filesystem::path> FindXboxGamingRootPath(
    const std::filesystem::path& driveRootPath) {
  const auto logger = getLogger();
  const auto gamingRootFilePath = driveRootPath / ".GamingRoot";

  std::vector<uint8_t> bytes;

  try {
    if (!std::filesystem::is_regular_file(gamingRootFilePath)) {
      return std::nullopt;
    }

    std::ifstream in(gamingRootFilePath, std::ios::binary);

    std::copy(std::istreambuf_iterator<char>(in),
              std::istreambuf_iterator<char>(),
              std::back_inserter(bytes));
  } catch (const std::exception& e) {
    if (logger) {
      logger->error("Failed to read file at {}: {}",
                    gamingRootFilePath.u8string(),
                    e.what());
    }

    // Don't propagate this error as it could be due to a legitimate failure
    // case like the drive not being ready (e.g. a removable disk drive with
    // nothing in it).
    return std::nullopt;
  }

  if (logger) {
    // Log the contents of .GamingRoot because I'm not sure of the format and
    // this would help debugging.
    logger->debug("Read the following bytes from {}: {::#04x}",
                  gamingRootFilePath.u8string(),
                  bytes);
  }

  // The content of .GamingRoot seems to be the byte sequence 52 47 42 58 01 00
  // 00 00 followed by the null-terminated UTF-16LE location of the Xbox games
  // folder on the same drive.

  if (bytes.size() % 2 != 0) {
    logger->error(
        "Found a non-even number of bytes in the file at {}, cannot interpret "
        "it as UTF-16LE",
        gamingRootFilePath.u8string());

    return std::nullopt;
  }

  std::vector<char16_t> content;
  for (size_t i = 0; i < bytes.size(); i += 2) {
    // char16_t is little-endian on all platforms LOOT runs on.
    char16_t highByte = bytes.at(i);
    char16_t lowByte = bytes.at(i + 1);
    char16_t value = highByte | (lowByte << CHAR_BIT);
    content.push_back(value);
  }

  static constexpr size_t CHAR16_PATH_OFFSET = 4;
  if (content.size() < CHAR16_PATH_OFFSET + 1) {
    if (logger) {
      logger->error(
          ".GamingRoot content was unexpectedly short at {} char16_t long",
          content.size());
    }

    return std::nullopt;
  }

  // Cut off the null char16_t at the end.
  const std::u16string relativePath(content.begin() + CHAR16_PATH_OFFSET,
                                    content.end() - 1);

  // Check that the string does not contain any nul characters (i.e. 0x00 0x00
  // in UTF-16), as while they're valid in UTF-16, they'll be passed around as
  // a C string later, and nul characters are not allowed in Windows or Linux
  // paths.
  const auto containsNul =
      std::find(relativePath.begin(), relativePath.end(), char16_t{0}) !=
      relativePath.end();
  if (containsNul) {
    if (logger) {
      logger->error(
          "The relative path read from .GamingRoot contains a nul "
          "character");
    }

    return std::nullopt;
  }

  if (logger) {
    logger->debug("Read the following relative path from .GamingRoot: {}",
                  std::filesystem::path(relativePath).u8string());
  }

  return driveRootPath / relativePath;
}

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
  auto unicodeLhs = icu::UnicodeString::fromUTF8(lhs);
  auto unicodeRhs = icu::UnicodeString::fromUTF8(rhs);
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
  std::array<char, PATH_MAX> result{};

  ssize_t count = readlink("/proc/self/exe", result.data(), result.size());
  if (count < 0) {
    auto logger = getLogger();
    if (logger) {
      logger->error("Failed to get LOOT executable path.");
    }
    throw std::system_error(
        count, std::system_category(), "Failed to get LOOT executable path.");
  }

  return std::filesystem::u8path(std::string(result.data(), count))
      .parent_path();
#endif
}

std::filesystem::path getUserProfilePath() {
#ifdef _WIN32
  PWSTR path;

  if (SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path) != S_OK)
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to get %USERPROFILE% path.");

  std::filesystem::path localAppDataPath(path);
  CoTaskMemFree(path);

  return localAppDataPath;
#else
  const auto home = getenv("HOME");

  if (home == nullptr) {
    // The POSIX spec requires the HOME environment variable to be set, don't
    // try to work around it being missing.
    throw std::runtime_error("The HOME environment variable has no value");
  }

  return std::filesystem::u8path(home);
#endif
}

std::filesystem::path getLocalAppDataPath() {
#ifdef _WIN32
  PWSTR path;

  if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path) != S_OK) {
    throw std::system_error(GetLastError(),
                            std::system_category(),
                            "Failed to get %LOCALAPPDATA% path.");
  }

  std::filesystem::path localAppDataPath(path);
  CoTaskMemFree(path);

  return localAppDataPath;
#else
  // Use XDG_DATA_HOME environmental variable if it's available.
  const auto xdgDataHome = getenv("XDG_DATA_HOME");

  if (xdgDataHome != nullptr) {
    return std::filesystem::u8path(xdgDataHome);
  }

  return getUserProfilePath() / ".local" / "share";
#endif
}

std::string crcToString(uint32_t crc) { return fmt::format("{:08X}", crc); }
}
