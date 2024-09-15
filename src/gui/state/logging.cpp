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

#include "gui/state/logging.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <boost/algorithm/string/replace.hpp>
#include <optional>

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
#endif

namespace loot {
static const char* LOGGER_NAME = "loot_logger";

class CensoringFileSink : public spdlog::sinks::sink {
public:
  explicit CensoringFileSink(
      const spdlog::filename_t& filename,
      const std::vector<std::pair<std::string, std::string>>& stringsToCensor) :
      sink(filename), stringsToCensor_(stringsToCensor) {}

protected:
  void log(const spdlog::details::log_msg& msg) {
    if (!sink.should_log(msg.level)) {
      // Skip potentially expensive string search / replacement.
      return;
    }

    if (!needsCensoring(msg.payload)) {
      // Avoid unnecessary copies.
      sink.log(msg);
      return;
    }

    spdlog::details::log_msg msgCopy = msg;

    std::string payload(msg.payload.data(), msg.payload.size());

    for (const auto& [search, replacement] : stringsToCensor_) {
      boost::replace_all(payload, search, replacement);
    }

    msgCopy.payload = payload;

    sink.log(msgCopy);
  }

  void flush() { sink.flush(); }

  void set_pattern(const std::string& pattern) { sink.set_pattern(pattern); }

  void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) {
    sink.set_formatter(std::move(sink_formatter));
  }

private:
  spdlog::sinks::basic_file_sink_mt sink;
  std::vector<std::pair<std::string, std::string>> stringsToCensor_;

  bool needsCensoring(const spdlog::string_view_t& payload) {
    std::string_view view(payload.data(), payload.size());

    for (const auto& [search, replacement] : stringsToCensor_) {
      if (view.find(search) != std::string_view::npos) {
        return true;
      }
    }

    return false;
  }
};

std::vector<std::pair<std::string, std::string>> getStringsToCensor() {
  const auto userProfilePath = getUserProfilePath();

#ifdef _WIN32
  return {{userProfilePath.u8string(), "%USERPROFILE%"}};
#else
  return {{userProfilePath.u8string(), "$HOME"}};
#endif
}

std::shared_ptr<spdlog::logger> getLogger() {
  auto logger = spdlog::get(LOGGER_NAME);

  if (!logger) {
    spdlog::set_pattern("[%T.%f] [%l]: %v");
    try {
      logger = spdlog::stdout_logger_mt(LOGGER_NAME);

      if (logger) {
        logger->flush_on(spdlog::level::trace);
      }
    } catch (...) {
      return nullptr;
    }
  }

  return logger;
}

void setLogPath(const std::filesystem::path& outputFile) {
  spdlog::set_pattern("[%T.%f] [%l]: %v");

  spdlog::drop(LOGGER_NAME);

#if defined(_WIN32)
  const auto platformFilePath = outputFile.wstring();
#else
  const auto platformFilePath = outputFile.u8string();
#endif
  const auto stringsToCensor = getStringsToCensor();

  auto logger = spdlog::synchronous_factory::create<CensoringFileSink>(
      LOGGER_NAME, platformFilePath, stringsToCensor);

  if (!logger) {
    throw std::runtime_error("Error: Could not initialise logging.");
  }
  logger->flush_on(spdlog::level::trace);
}

void enableDebugLogging(bool enable) {
  auto logger = getLogger();
  if (logger) {
    if (enable) {
      logger->set_level(spdlog::level::level_enum::trace);
    } else {
      logger->set_level(spdlog::level::level_enum::info);
    }
  }
}
}
