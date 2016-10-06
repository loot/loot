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

#include "backend/metadata/condition_evaluator.h"

#include <boost/log/trivial.hpp>

#include "loot/error.h"
#include "backend/helpers/helpers.h"

namespace loot {
ConditionEvaluator::ConditionEvaluator(Game * game) : game_(game) {}

bool ConditionEvaluator::fileExists(const std::string& filePath) const {
  validatePath(filePath);

  if (game_ == nullptr)
    return false;

  if (filePath == "LOOT")
    return true;

  // Try first checking the plugin cache, as most file entries are
  // for plugins.
  try {
    // GetPlugin throws if it can't find an entry.
    game_->GetPlugin(filePath);

    return true;
  } catch (...) {
    // Not a loaded plugin, check the filesystem.
    if (boost::iends_with(filePath, ".esp") || boost::iends_with(filePath, ".esm"))
      return boost::filesystem::exists(game_->DataPath() / filePath)
      || boost::filesystem::exists(game_->DataPath() / (filePath + ".ghost"));
    else
      return boost::filesystem::exists(game_->DataPath() / filePath);
  }
}

bool ConditionEvaluator::regexMatchExists(const std::string& regexString) const {
  auto pathRegex = splitRegex(regexString);

  if (game_ == nullptr)
    return false;

  return isRegexMatchInDataDirectory(pathRegex,
                                     [](const std::string&) { return true; });
}

bool ConditionEvaluator::regexMatchesExist(const std::string& regexString) const {
  auto pathRegex = splitRegex(regexString);

  if (game_ == nullptr)
    return false;

  return areRegexMatchesInDataDirectory(pathRegex,
                                        [](const std::string&) { return true; });
}

bool ConditionEvaluator::isPluginActive(const std::string& pluginName) const {
  validatePath(pluginName);

  if (game_ == nullptr)
    return false;

  if (pluginName == "LOOT")
    return false;

  return game_->IsPluginActive(pluginName);
}

bool ConditionEvaluator::isPluginMatchingRegexActive(const std::string& regexString) const {
  auto pathRegex = splitRegex(regexString);

  if (game_ == nullptr)
    return false;

  return isRegexMatchInDataDirectory(pathRegex,
                                     [&](const std::string& filename) {
    return game_->IsPluginActive(filename);
  });
}

bool ConditionEvaluator::arePluginsActive(const std::string& regexString) const {
  auto pathRegex = splitRegex(regexString);

  if (game_ == nullptr)
    return false;

  return areRegexMatchesInDataDirectory(pathRegex,
                                        [&](const std::string& filename) {
    return game_->IsPluginActive(filename);
  });
}

bool ConditionEvaluator::checksumMatches(const std::string& filePath, const uint32_t checksum) {
  validatePath(filePath);

  if (game_ == nullptr)
    return false;

  uint32_t realChecksum = 0;
  if (filePath == "LOOT")
    realChecksum = GetCrc32(boost::filesystem::absolute("LOOT.exe"));
  else {
    // CRC could be for a plugin or a file.
    // Get the CRC from the game plugin cache if possible.
    try {
      realChecksum = game_->GetPlugin(filePath).Crc();
    } catch (...) {}

    if (realChecksum == 0) {
      if (boost::filesystem::exists(game_->DataPath() / filePath))
        realChecksum = GetCrc32(game_->DataPath() / filePath);
      else if ((boost::iends_with(filePath, ".esp") || boost::iends_with(filePath, ".esm")) && boost::filesystem::exists(game_->DataPath() / (filePath + ".ghost")))
        realChecksum = GetCrc32(game_->DataPath() / (filePath + ".ghost"));
    }
  }

  return checksum == realChecksum;
}

bool ConditionEvaluator::compareVersions(const std::string & filePath, const std::string & testVersion, const std::string & comparator) const {
  if (!fileExists(filePath))
    return comparator == "!=" || comparator == "<" || comparator == "<=";

  Version givenVersion = Version(testVersion);
  Version trueVersion = getVersion(filePath);

  BOOST_LOG_TRIVIAL(trace) << "Version extracted: " << trueVersion.AsString();

  return ((comparator == "==" && trueVersion == givenVersion)
          || (comparator == "!=" && trueVersion != givenVersion)
          || (comparator == "<" && trueVersion < givenVersion)
          || (comparator == ">" && trueVersion > givenVersion)
          || (comparator == "<=" && trueVersion <= givenVersion)
          || (comparator == ">=" && trueVersion >= givenVersion));
}

void ConditionEvaluator::validatePath(const boost::filesystem::path& path) {
  BOOST_LOG_TRIVIAL(trace) << "Checking to see if the path \"" << path << "\" is safe.";

  boost::filesystem::path temp;
  for (const auto& component : path) {
    if (component == ".")
      continue;

    if (component == ".." && temp.filename() == "..") {
      BOOST_LOG_TRIVIAL(error) << "Invalid file path: " << path;
      throw std::invalid_argument(boost::locale::translate("Invalid file path:").str() + " " + path.string());
    }

    temp /= component;
  }
}
void ConditionEvaluator::validateRegex(const std::string& regexString) {
  try {
    std::regex(regexString, std::regex::ECMAScript | std::regex::icase);
  } catch (std::regex_error& e) {
    throw std::invalid_argument((boost::format(boost::locale::translate("Invalid regex string \"%1%\": %2%")) % regexString % e.what()).str());
  }
}

boost::filesystem::path ConditionEvaluator::getRegexParentPath(const std::string& regexString) {
  size_t pos = regexString.rfind('/');

  if (pos == std::string::npos)
    return boost::filesystem::path();

  return boost::filesystem::path(regexString.substr(0, pos));
}

std::string ConditionEvaluator::getRegexFilename(const std::string& regexString) {
  size_t pos = regexString.rfind('/');

  if (pos == std::string::npos)
    return regexString;

  return regexString.substr(pos + 1);
}

std::pair<boost::filesystem::path, std::regex> ConditionEvaluator::splitRegex(const std::string& regexString) {
  // Can't support a regex string where all path components may be regex, since this could
  // lead to massive scanning if an unfortunately-named directory is encountered.
  // As such, only the filename portion can be a regex. Need to separate that from the rest
  // of the string.

  validateRegex(regexString);

  std::string filename = getRegexFilename(regexString);
  boost::filesystem::path parent = getRegexParentPath(regexString);

  validatePath(parent);

  std::regex reg;
  try {
    reg = std::regex(filename, std::regex::ECMAScript | std::regex::icase);
  } catch (std::regex_error& e) {
    BOOST_LOG_TRIVIAL(error) << "Invalid regex string:" << filename;
    throw std::invalid_argument((boost::format(boost::locale::translate("Invalid regex string \"%1%\": %2%")) % filename % e.what()).str());
  }

  return std::pair<boost::filesystem::path, std::regex>(parent, reg);
}

bool ConditionEvaluator::isGameSubdirectory(const boost::filesystem::path& path) const {
  boost::filesystem::path parentPath = game_->DataPath() / path;

  return boost::filesystem::exists(parentPath) && boost::filesystem::is_directory(parentPath);
}

bool ConditionEvaluator::isRegexMatchInDataDirectory(const std::pair<boost::filesystem::path, std::regex>& pathRegex,
                                                     const std::function<bool(const std::string&)> condition) const {
   // Now we have a valid parent path and a regex filename. Check that the
   // parent path exists and is a directory.
  if (!isGameSubdirectory(pathRegex.first)) {
    BOOST_LOG_TRIVIAL(trace) << "The path \"" << pathRegex.first << "\" is not a game subdirectory.";
    return false;
  }

  return std::any_of(boost::filesystem::directory_iterator(game_->DataPath() / pathRegex.first),
                     boost::filesystem::directory_iterator(),
                     [&](const boost::filesystem::directory_entry& entry) {
    const std::string filename = entry.path().filename().string();
    return std::regex_match(filename, pathRegex.second) && condition(filename);
  });
}

bool ConditionEvaluator::areRegexMatchesInDataDirectory(const std::pair<boost::filesystem::path, std::regex>& pathRegex,
                                                        const std::function<bool(const std::string&)> condition) const {
  bool foundOneFile = false;

  return isRegexMatchInDataDirectory(pathRegex, [&](const std::string& filename) {
    if (condition(filename)) {
      if (foundOneFile)
        return true;

      foundOneFile = true;
    }

    return false;
  });
}
Version ConditionEvaluator::getVersion(const std::string& filePath) const {
  if (filePath == "LOOT")
    return Version(boost::filesystem::absolute("LOOT.exe"));
  else {
    // If the file is a plugin, its version needs to be extracted
    // from its description field. Try getting an entry from the
    // plugin cache.
    try {
      return Version(game_->GetPlugin(filePath).getDescription());
    } catch (...) {
      // The file wasn't in the plugin cache, load it as a plugin
      // if it appears to be valid, otherwise treat it as a non
      // plugin file.
      if (Plugin::IsValid(filePath, *game_))
        return Version(Plugin(*game_, filePath, true).getDescription());

      return Version(game_->DataPath() / filePath);
    }
  }
}
}
