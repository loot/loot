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

#ifndef LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_METADATA_TABLE_MODEL
#define LOOT_GUI_QT_PLUGIN_EDITOR_MODELS_METADATA_TABLE_MODEL

#include <QtCore/QAbstractTableModel>
#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>

namespace loot {
template<typename T>
class MetadataTableModel : public QAbstractTableModel {
public:
  std::vector<T> getUserMetadata() const { return userMetadata; }

  int rowCount(const QModelIndex& = QModelIndex()) const final override {
    return static_cast<int>(nonUserMetadata.size() + userMetadata.size());
  }

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const final override {
    if (role != Qt::DisplayRole && role != Qt::EditRole &&
        role != Qt::ForegroundRole) {
      return QVariant();
    }

    if (!index.isValid()) {
      return QVariant();
    }

    if (index.row() >= rowCount()) {
      return QVariant();
    }

    const auto isNonUserMetadata =
        index.row() < static_cast<int>(nonUserMetadata.size());

    const auto rowIndex = static_cast<size_t>(index.row());

    const auto& element =
        isNonUserMetadata ? nonUserMetadata.at(rowIndex)
                          : userMetadata.at(rowIndex - nonUserMetadata.size());

    if (role == Qt::ForegroundRole) {
      // Grey out metadata that's not editable.
      return isNonUserMetadata ? QGuiApplication::palette().color(
                                     QPalette::Disabled, QPalette::WindowText)
                               : QVariant();
    }

    return data(element, index.column(), role);
  }

  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role) const final override {
    if (role != Qt::DisplayRole) {
      return QVariant();
    }

    if (orientation != Qt::Horizontal) {
      return QVariant();
    }

    return headerText(section);
  }

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (!index.isValid()) {
      return Qt::ItemIsEnabled;
    }

    if (index.row() < static_cast<int>(nonUserMetadata.size())) {
      return QAbstractItemModel::flags(index);
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

  bool setData(const QModelIndex& index,
               const QVariant& value,
               int role) final override {
    if (!index.isValid() || role != Qt::EditRole) {
      return false;
    }

    if (index.column() > columnCount() - 1) {
      return false;
    }

    if (index.row() < static_cast<int>(nonUserMetadata.size()) ||
        index.column() > columnCount() - 1) {
      return false;
    }

    auto& element = userMetadata.at(index.row() - nonUserMetadata.size());

    setData(element, index.column(), value);

    emit dataChanged(index, index, {role});
    return true;
  }

  bool insertRows(int row,
                  int count,
                  const QModelIndex& parent = QModelIndex()) final override {
    if (row < static_cast<int>(nonUserMetadata.size()) || row > rowCount()) {
      return false;
    }

    beginInsertRows(parent, row, row + count - 1);

    const auto index = row - nonUserMetadata.size();

    userMetadata.insert(
        std::next(userMetadata.begin(), static_cast<std::ptrdiff_t>(index)),
        static_cast<size_t>(count),
        T());

    endInsertRows();

    return true;
  }

  bool removeRows(int row,
                  int count,
                  const QModelIndex& parent = QModelIndex()) final override {
    if (row < static_cast<int>(nonUserMetadata.size()) || row > rowCount() ||
        row + count > rowCount()) {
      return false;
    }

    beginRemoveRows(parent, row, row + count - 1);

    const auto startIndex = row - nonUserMetadata.size();
    const auto endIndex = startIndex + count;

    userMetadata.erase(
        std::next(userMetadata.begin(),
                  static_cast<std::ptrdiff_t>(startIndex)),
        std::next(userMetadata.begin(), static_cast<std::ptrdiff_t>(endIndex)));

    endRemoveRows();

    return true;
  }

protected:
  MetadataTableModel(QObject* parent,
                     std::vector<T>&& nonUserMetadata,
                     std::vector<T>&& userMetadata) :
      QAbstractTableModel(parent),
      nonUserMetadata(std::move(nonUserMetadata)),
      userMetadata(std::move(userMetadata)) {}

  virtual QVariant data(const T& element, int column, int role) const = 0;

  virtual QVariant headerText(int section) const = 0;

  virtual void setData(T& element, int column, const QVariant& value) = 0;

  size_t getNonUserMetadataSize() const { return nonUserMetadata.size(); }

private:
  std::vector<T> nonUserMetadata;
  std::vector<T> userMetadata;
};
}

#endif
