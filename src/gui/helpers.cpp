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

#include <spdlog/fmt/fmt.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <fstream>

#include "gui/state/logging.h"

namespace {
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

#ifdef _WIN32
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
#ifdef _WIN32
  const auto logger = loot::getLogger();

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

  if (logger) {
    logger->debug("Merged preferred UI languages are: {}",
                  boost::join(preferredLanguages, ", "));
  }

  return preferredLanguages;
#else
  return {};
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

  std::vector<char> bytes;

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
    std::vector<std::string> hexBytes;
    std::transform(bytes.begin(),
                   bytes.end(),
                   std::back_inserter(hexBytes),
                   [](char byte) {
                     std::stringstream stream;
                     stream << "0x" << std::hex << int{byte};
                     return stream.str();
                   });

    logger->debug("Read the following bytes from {}: {}",
                  gamingRootFilePath.u8string(),
                  boost::join(hexBytes, " "));
  }

  // The content of .GamingRoot is the byte sequence 52 47 42 58 01 00 00 00
  // followed by the null-terminated UTF-16LE location of the Xbox games folder
  // on the same drive.

  if (bytes.size() % 2 != 0) {
    logger->error(
        "Found a non-even number of bytes in the file at {}, cannot interpret "
        "it as UTF-16LE",
        gamingRootFilePath.u8string());
    throw std::runtime_error(
        "Found a non-even number of bytes in the file at \"" +
        gamingRootFilePath.u8string() + "\"");
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

    throw std::runtime_error("The file at \"" + gamingRootFilePath.u8string() +
                             "\" is shorter than expected.");
  }

  // Cut off the null char16_t at the end.
  const std::u16string relativePath(content.begin() + CHAR16_PATH_OFFSET,
                                    content.end() - 1);

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

std::filesystem::path getLocalAppDataPath() {
#ifdef _WIN32
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
  size_t wcstrLength = wtext.length() + 1;
  wchar_t* wcstr = new wchar_t[wcstrLength];
  wcscpy_s(wcstr, wcstrLength, wtext.c_str());

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

std::string crcToString(uint32_t crc) { return fmt::format("{:08X}", crc); }

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
