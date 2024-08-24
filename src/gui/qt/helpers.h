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

#ifndef LOOT_GUI_QT_HELPERS
#define LOOT_GUI_QT_HELPERS

#include <loot/metadata/message_content.h>

#include <QtCore/QByteArray>
#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QLabel>
#include <filesystem>
#include <vector>

#include "gui/state/game/helpers.h"

namespace loot {
enum class FileType { Masterlist, MasterlistPrelude };

struct FileRevision {
  std::string id;
  std::string date;
  bool is_modified{false};
};

struct FileRevisionSummary {
  FileRevisionSummary() = default;
  explicit FileRevisionSummary(const FileRevision& fileRevision);
  FileRevisionSummary(const std::string& id, const std::string& date);

  std::string id;
  std::string date;
};

QString translate(const char* text);

void scaleCardHeading(QLabel& label);

std::string calculateGitBlobHash(const QByteArray& data);

std::string calculateGitBlobHash(const std::filesystem::path& filePath);

FileRevision getFileRevision(const std::filesystem::path& filePath);

FileRevisionSummary getFileRevisionSummary(
    const std::filesystem::path& filePath,
    FileType fileType);

bool updateFileWithData(const std::filesystem::path& filePath,
                        const QByteArray& data);

bool updateFile(const std::filesystem::path& source,
                const std::filesystem::path& destination);

bool isValidUrl(const std::string& location);

std::optional<QByteArray> readHttpResponse(QNetworkReply* reply);

void showInvalidRegexTooltip(QWidget& widget, const std::string& details);

void CopyToClipboard(const std::string& text);

void OpenInDefaultApplication(const std::filesystem::path& path);

void PaintCardBorderShadows(QWidget* card, bool paintTop);
}

Q_DECLARE_METATYPE(loot::MessageContent);

#endif
