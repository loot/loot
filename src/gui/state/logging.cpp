/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

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

namespace loot {
static const char* LOGGER_NAME = "loot_logger";

std::shared_ptr<spdlog::logger> getLogger() {
  auto logger = spdlog::get(LOGGER_NAME);

  if (!logger) {
    spdlog::set_pattern("[%T.%f] [%l]: %v");
    logger = spdlog::stdout_logger_mt(LOGGER_NAME);

    if (logger) {
      logger->flush_on(spdlog::level::trace);
    }
  }

  return logger;
}

void setLogPath(const std::filesystem::path& outputFile) {
  spdlog::set_pattern("[%T.%f] [%l]: %v");

  spdlog::drop(LOGGER_NAME);
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
  auto logger = spdlog::basic_logger_mt(LOGGER_NAME,
                                        outputFile.wstring());
#else
  auto logger = spdlog::basic_logger_mt(LOGGER_NAME,
                                        outputFile.u8string());
#endif

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
      logger->set_level(spdlog::level::level_enum::warn);
    }
  }
}
}
