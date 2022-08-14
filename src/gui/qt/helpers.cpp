/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2021    Oliver Hamlet

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

#include "gui/qt/helpers.h"

#include <loot/exception/file_access_error.h>
#include <toml++/toml.h>

#include <QtCore/QCryptographicHash>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QPoint>
#include <QtCore/QUrl>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QWidget>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <fstream>

#include "gui/state/logging.h"

namespace loot {
static constexpr const char* METADATA_PATH_SUFFIX = ".metadata.toml";
static constexpr const char* METADATA_ID_KEY = "blob_sha1";
static constexpr const char* METADATA_DATE_KEY = "update_timestamp";
static constexpr int SHORT_HASH_LENGTH = 7;
static constexpr const char* HEADER_PREFIX = "blob ";
static constexpr size_t HEADER_PREFIX_LENGTH =
    std::char_traits<char>::length(HEADER_PREFIX);

std::filesystem::path getFileMetadataPath(std::filesystem::path filePath) {
  filePath += METADATA_PATH_SUFFIX;
  return filePath;
}

void writeFileRevision(const std::filesystem::path& filePath,
                       const std::string& id,
                       const std::string& date) {
  auto metadataPath = getFileMetadataPath(filePath);

  auto logger = getLogger();
  if (logger) {
    logger->info("Writing file revision info for {} with ID {} and date {}",
                 metadataPath.u8string(),
                 id,
                 date);
  }

  const auto table =
      toml::table{{METADATA_ID_KEY, id}, {METADATA_DATE_KEY, date}};

  std::ofstream out(metadataPath);
  if (!out.is_open()) {
    throw std::runtime_error(metadataPath.u8string() +
                             " could not be opened for writing");
  }

  out << table;
}

FileRevisionSummary::FileRevisionSummary(const FileRevision& fileRevision) :
    id(fileRevision.id.substr(0, SHORT_HASH_LENGTH)), date(fileRevision.date) {
  if (fileRevision.is_modified) {
    auto suffix =
        " " +
        /* translators: this text is displayed if LOOT has detected that the
           masterlist has been modified since it was downloaded. */
        boost::locale::translate("(edited)").str();
    date += suffix;
    id += suffix;
  }
}

FileRevisionSummary::FileRevisionSummary(const std::string& id,
                                         const std::string& date) :
    id(id), date(date) {}

QString translate(const char* text) {
  return QString::fromStdString(boost::locale::translate(text).str());
}

std::string calculateGitBlobHash(const QByteArray& data) {
  auto sizeString = std::to_string(data.size());

  auto hasher = QCryptographicHash(QCryptographicHash::Sha1);
  hasher.addData(HEADER_PREFIX, HEADER_PREFIX_LENGTH);
  hasher.addData(sizeString.c_str(), sizeString.size() + 1);
  hasher.addData(data);

  return QString(hasher.result().toHex()).toStdString();
}

std::string calculateGitBlobHash(const std::filesystem::path& filePath) {
  QFile file(QString::fromStdString(filePath.u8string()));

  auto result = file.open(QIODevice::ReadOnly);
  if (!result) {
    throw FileAccessError(filePath.u8string() + " is not a regular file");
  }

  QByteArray fileContent = file.readAll();

  // Files in LOOT's repositories are committed with LF line endings, but if the
  // file being read is from the working directory of a local Git repository
  // that has autocrlf enabled, it will have CRLF line endings, so the
  // hash won't match the value calculated by Git unless the line endings
  // are replaced.
  fileContent.replace(QByteArray("\r\n"), QByteArray("\n"));

  return calculateGitBlobHash(fileContent);
}

FileRevision getFileRevision(const std::filesystem::path& filePath) {
  FileRevision revision;

  revision.id = calculateGitBlobHash(filePath);

  auto metadataPath = getFileMetadataPath(filePath);

  if (!std::filesystem::is_regular_file(filePath)) {
    throw FileAccessError(metadataPath.u8string() + " is not a regular file");
  }

  // Don't use toml::parse_file() as it just uses a std stream,
  // which don't support UTF-8 paths on Windows.
  std::ifstream in(metadataPath);
  if (!in.is_open()) {
    throw std::runtime_error(metadataPath.u8string() +
                             " could not be opened for parsing");
  }

  const auto metadata = toml::parse(in, metadataPath.u8string());

  auto hash = metadata[METADATA_ID_KEY].value<std::string>();
  auto timestamp = metadata[METADATA_DATE_KEY].value<std::string>();

  if (!hash.has_value()) {
    throw std::runtime_error("blob_sha1 field is missing");
  }

  if (!timestamp.has_value()) {
    throw std::runtime_error("update_timestamp field is missing");
  }

  revision.is_modified = revision.id != hash.value();
  revision.date = timestamp.value();

  return revision;
}

FileRevisionSummary getFileRevisionSummary(
    const std::filesystem::path& filePath,
    FileType fileType) {
  using boost::locale::translate;

  auto logger = getLogger();

  try {
    return FileRevisionSummary(getFileRevision(filePath));
  } catch (FileAccessError&) {
    if (logger) {
      if (fileType == FileType::Masterlist) {
        logger->warn("No masterlist present at {}", filePath.u8string());
      } else {
        logger->warn("No masterlist prelude present at {}",
                     filePath.u8string());
      }
    }
    auto text = fileType == FileType::Masterlist
                    ? translate("N/A: No masterlist present").str()
                    :
                    /* translators: N/A is an abbreviation for Not Applicable. A
                       masterlist is a database that contains information for
                       various mods. */
                    translate("N/A: No masterlist prelude present").str();

    return FileRevisionSummary(text, text);
  } catch (std::runtime_error&) {
    if (logger) {
      logger->warn("Failed to read metadata for: {}",
                   filePath.parent_path().u8string());
    }
    auto text = translate("Unknown: No revision metadata found").str();
    return FileRevisionSummary(text, text);
  }
}

bool isFileUpToDate(const std::filesystem::path& filePath,
                    const std::string& expectedHash) {
  auto logger = getLogger();

  try {
    auto existingFileHash = calculateGitBlobHash(filePath);

    if (logger) {
      logger->info("Calculated blob hash for file at {}: {}",
                   filePath.u8string(),
                   existingFileHash);
    }

    return expectedHash == existingFileHash;
  } catch (std::exception& e) {
    if (logger) {
      logger->error(
          "Caught exception when getting file revision, assuming file is not "
          "up to date: {}",
          e.what());
    }

    return false;
  }
}

bool updateFileWithData(const std::filesystem::path& filePath,
                        const QByteArray& data) {
  auto logger = getLogger();

  auto newHash = calculateGitBlobHash(data);
  auto hasChanged = !isFileUpToDate(filePath, newHash);

  if (hasChanged) {
    QFile masterlist(QString::fromStdString(filePath.u8string()));
    masterlist.open(QIODevice::WriteOnly);
    masterlist.write(data);
    masterlist.close();
  }

  if (logger) {
    auto logMessage = hasChanged ? "Updated file at {}, new blob hash is {}"
                                 : "{} is already up to date with blob hash {}";
    logger->info(logMessage, filePath.u8string(), newHash);
  }

  // Update the metadata file even if the file is up to date, as the
  // update timestamp may have changed.
  auto updateTimestamp =
      QDate::currentDate().toString(Qt::ISODate).toStdString();
  writeFileRevision(filePath, newHash, updateTimestamp);

  return hasChanged;
}

bool updateFile(const std::filesystem::path& source,
                const std::filesystem::path& destination) {
  auto logger = getLogger();
  if (logger) {
    logger->info("Updating file at {} using file at {}",
                 destination.u8string(),
                 source.u8string());
  }

  auto newHash = calculateGitBlobHash(source);
  auto hasChanged = !isFileUpToDate(destination, newHash);

  if (hasChanged) {
    std::filesystem::copy_file(
        source, destination, std::filesystem::copy_options::overwrite_existing);
  }

  if (logger) {
    auto logMessage = hasChanged ? "Updated file at {}, new blob hash is {}"
                                 : "{} is already up to date with blob hash {}";
    logger->info(logMessage, destination.u8string(), newHash);
  }

  // Update the metadata file even if the file is up to date, as the
  // update timestamp may have changed.
  auto updateTimestamp =
      QDate::currentDate().toString(Qt::ISODate).toStdString();
  writeFileRevision(destination, newHash, updateTimestamp);

  return hasChanged;
}

bool isValidUrl(const std::string& location) {
  auto url = QUrl(QString::fromStdString(location), QUrl::StrictMode);
  auto scheme = url.scheme().toStdString();
  return url.isValid() && !url.isLocalFile() &&
         (scheme == "http" || scheme == "https");
}

std::optional<QByteArray> readHttpResponse(QNetworkReply* reply) {
  auto statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

  auto data = reply->readAll();
  reply->deleteLater();

  static constexpr int HTTP_STATUS_OK = 200;
  static constexpr int HTTP_STATUS_BAD_REQUEST = 400;

  if (statusCode < HTTP_STATUS_OK || statusCode >= HTTP_STATUS_BAD_REQUEST) {
    auto logger = getLogger();
    if (logger) {
      logger->error(
          "Error while checking for LOOT updates: unexpected HTTP response "
          "status code: {}. Response body is: {}",
          statusCode,
          QString::fromUtf8(data).toStdString());
    }

    return std::nullopt;
  }

  return data;
}

void showInvalidRegexTooltip(QWidget& widget, const std::string& details) {
  auto message = (boost::format(boost::locale::translate(
                      "Invalid regular expression: %1%")) %
                  details)
                     .str();

  QToolTip::showText(widget.mapToGlobal(QPoint(0, 0)),
                     QString::fromStdString(message),
                     &widget);
}
}
