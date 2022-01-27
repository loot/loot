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

#ifndef LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_FILE_TABLE_MODEL
#define LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_FILE_TABLE_MODEL

#include <loot/metadata/file.h>

#include <QtCore/QAbstractTableModel>

namespace loot {
class FileTableModel : public QAbstractTableModel {
public:
  FileTableModel(QObject* parent,
                 std::vector<File> nonUserMetadata,
                 std::vector<File> userMetadata,
                 const std::string& language);

  std::vector<File> getUserMetadata() const;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;

  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  QStringList mimeTypes() const override;

  bool setData(const QModelIndex& index,
               const QVariant& value,
               int role) override;

  bool insertRows(int row,
                  int count,
                  const QModelIndex& parent = QModelIndex()) override;

  bool removeRows(int row,
                  int count,
                  const QModelIndex& parent = QModelIndex()) override;

  bool dropMimeData(const QMimeData* data,
                    Qt::DropAction action,
                    int row,
                    int column,
                    const QModelIndex& parent) override;

  static constexpr int NAME_COLUMN = 0;
  static constexpr int DISPLAY_NAME_COLUMN = 1;
  static constexpr int DETAIL_COLUMN = 2;
  static constexpr int CONDITION_COLUMN = 3;

private:
  const std::string& language;

  std::vector<File> nonUserMetadata;
  std::vector<File> userMetadata;
};
}

#endif
