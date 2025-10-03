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

#include "gui/qt/plugin_editor/table_tabs.h"

#include <QtGui/QFontMetrics>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>

#include "gui/qt/helpers.h"
#include "gui/qt/plugin_editor/delegates.h"
#include "gui/qt/plugin_editor/models/cleaning_data_table_model.h"
#include "gui/qt/plugin_editor/models/file_table_model.h"
#include "gui/qt/plugin_editor/models/location_table_model.h"
#include "gui/qt/plugin_editor/models/message_content_table_model.h"
#include "gui/qt/plugin_editor/models/message_table_model.h"
#include "gui/qt/plugin_editor/models/tag_table_model.h"

namespace {
int calculateItemWidth(const QString& text) {
  auto textWidth = QFontMetricsF(QGuiApplication::font())
                       .size(Qt::TextSingleLine, text)
                       .width();

  const auto leftPadding =
      QApplication::style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
  const auto rightPadding =
      QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin);

  return static_cast<int>(textWidth) + leftPadding + rightPadding;
}

int calculateMaxHeaderWidth(const QAbstractItemModel* model,
                            int column,
                            int currentMaxHeaderWidth) {
  auto headerText = model->headerData(column, Qt::Horizontal);
  if (!headerText.isValid()) {
    return currentMaxHeaderWidth;
  }

  auto headerWidth = calculateItemWidth(headerText.toString());

  return headerWidth > currentMaxHeaderWidth ? headerWidth
                                             : currentMaxHeaderWidth;
}

int calculateMaxDropdownRowWidth(const QString& text, int currentMaxRowWidth) {
  const auto rowWidth = calculateItemWidth(text);

  return rowWidth > currentMaxRowWidth ? rowWidth : currentMaxRowWidth;
}

int calculateMinimumHeaderWidth(QAbstractItemModel* model) {
  // Set the minimum header width to the length of the longest header.
  int maxHeaderWidth = 0;
  for (int i = 0; i < model->columnCount(); i += 1) {
    maxHeaderWidth = calculateMaxHeaderWidth(model, i, maxHeaderWidth);
  }

  return maxHeaderWidth;
}

int calculateMinimumColumnWidth(QAbstractItemModel* model,
                                int column,
                                const std::map<std::string, QVariant>& values) {
  // Set the minimum column width longest text (plus padding) in the column.
  // First check the header width.
  const auto maxHeaderWidth = calculateMaxHeaderWidth(model, column, 0);
  auto maxColumnWidth = maxHeaderWidth;

  // The values contain strings that appear in a combo box dropdown menu, check
  // their widths too.
  for (const auto& [key, value] : values) {
    if (value.isValid()) {
      maxColumnWidth =
          calculateMaxDropdownRowWidth(value.toString(), maxColumnWidth);
    }
  }

  // If there are more than 10 (that may be variable, I don't know) items, a
  // scroll bar will be shown, so take its width into account.
  static constexpr size_t MAX_ITEMS_WITHOUT_SCROLLING = 10;
  if (maxColumnWidth > maxHeaderWidth &&
      values.size() > MAX_ITEMS_WITHOUT_SCROLLING) {
    maxColumnWidth +=
        QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent);
  }

  return maxColumnWidth;
}

int calculateMinimumColumnWidth(
    QAbstractItemModel* model,
    int column,
    const std::vector<std::pair<QString, QVariant>>& values) {
  // First check the header width.
  auto maxTextWidth = calculateMaxHeaderWidth(model, column, 0);

  // The values contain strings that appear in a combo box dropdown menu, check
  // their widths too.
  for (const auto& [text, value] : values) {
    maxTextWidth = calculateMaxDropdownRowWidth(text, maxTextWidth);
  }

  return maxTextWidth;
}
}

namespace loot {
BaseTableTab::BaseTableTab(QWidget* parent) : QWidget(parent) { setupUi(); }

QAbstractItemModel* BaseTableTab::getTableModel() const {
  return tableView->model();
}

void BaseTableTab::setTableModel(QAbstractItemModel* model) {
  auto oldModel = tableView->model();
  auto oldSelectionModel = tableView->selectionModel();

  tableView->setModel(model);
  tableView->horizontalHeader()->setMinimumSectionSize(
      calculateMinimumHeaderWidth(model));

  if (oldModel != nullptr) {
    oldModel->deleteLater();
  }

  if (oldSelectionModel != nullptr) {
    oldSelectionModel->deleteLater();
  }

  // Selection handling needs to be set up after the model has been set, as
  // before then there is no selection model.
  connect(tableView->selectionModel(),
          &QItemSelectionModel::selectionChanged,
          this,
          &BaseTableTab::onSelectionModelSelectionChanged);

  connect(model,
          &QAbstractItemModel::rowsInserted,
          this,
          &BaseTableTab::onModelRowsInserted);
  connect(model,
          &QAbstractItemModel::rowsRemoved,
          this,
          &BaseTableTab::onModelRowsRemoved);

  emit tableRowCountChanged(hasUserMetadata());

  // Disable the delete row button in case something was selected before the
  // new model was set.
  deleteRowButton->setDisabled(true);
}

void BaseTableTab::setColumnHidden(int column, bool hide) {
  tableView->setColumnHidden(column, hide);
}

void BaseTableTab::setItemDelegateForColumn(int column,
                                            QStyledItemDelegate* delegate) {
  tableView->setItemDelegateForColumn(column, delegate);
}

void BaseTableTab::setColumnFixedWidth(int column, int width) {
  tableView->horizontalHeader()->setSectionResizeMode(column,
                                                      QHeaderView::Fixed);
  tableView->horizontalHeader()->resizeSection(column, width);
}

void BaseTableTab::configureAsDropTarget() {
  tableView->setAcceptDrops(true);
  tableView->setDropIndicatorShown(true);
}

void BaseTableTab::resizeEvent(QResizeEvent*) {
  const auto header = tableView->horizontalHeader();
  for (int i = 0; i < header->count(); i += 1) {
    if (!header->isSectionHidden(i) &&
        header->sectionResizeMode(i) != QHeaderView::Fixed) {
      tableView->resizeColumnToContents(i);
    }
  }

  const auto tableWidth = tableView->width();

  // Some header sections are fixed width, we don't want to resize them,
  // so exclude them from the headers width scaling factor calculation.
  double headersWidth = 0;
  for (int i = 0; i < header->count(); i += 1) {
    if (!header->isSectionHidden(i) &&
        header->sectionResizeMode(i) != QHeaderView::Fixed) {
      headersWidth += header->sectionSize(i);
    }
  }

  if (headersWidth == 0) {
    // Don't divide by zero.
    return;
  }

  const auto scalingFactor = static_cast<double>(tableWidth) / headersWidth;

  for (auto i = 0; i < header->count(); i += 1) {
    const int newColumnWidth =
        static_cast<int>(tableView->columnWidth(i) * scalingFactor);
    if (header->sectionResizeMode(i) != QHeaderView::Fixed) {
      tableView->setColumnWidth(i, newColumnWidth);
    }
  }

  // If the headers are now wider than the table view (e.g. due to rounding
  // errors), reduce the width of the final visible header to fit.
  // Add a fudge factor of 2 to account for border widths.
  const auto excess = header->length() + 2 - tableView->width();
  if (excess > 0) {
    auto index = header->count() - 1;
    while (index >= 0 &&
           (tableView->isColumnHidden(index) ||
            header->sectionResizeMode(index) == QHeaderView::Fixed)) {
      index -= 1;
    }
    if (index >= 0) {
      tableView->setColumnWidth(index, tableView->columnWidth(index) - excess);
    }
  }
}

void BaseTableTab::setupUi() {
  tableView->setTabKeyNavigation(false);
  tableView->verticalHeader()->hide();
  tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

  addNewRowButton->setObjectName("addNewRowButton");

  deleteRowButton->setObjectName("deleteRowButton");
  deleteRowButton->setEnabled(false);

  auto layout = new QVBoxLayout();

  auto buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(addNewRowButton);
  buttonLayout->addWidget(deleteRowButton);

  layout->addWidget(tableView);
  layout->addLayout(buttonLayout);

  setLayout(layout);

  translateUi();

  QMetaObject::connectSlotsByName(this);
}

void BaseTableTab::translateUi() {
  addNewRowButton->setText(translate("Add new row"));
  deleteRowButton->setText(translate("Delete row"));
}

void BaseTableTab::on_addNewRowButton_clicked() {
  auto model = tableView->model();
  model->insertRow(model->rowCount());
}

void BaseTableTab::on_deleteRowButton_clicked() {
  auto model = tableView->model();
  auto selectedIndexes = tableView->selectionModel()->selectedIndexes();

  // First convert all selected indices to persistent indices so that
  // deleting rows doesn't change what data they point to.
  std::vector<QPersistentModelIndex> persistentIndices;
  persistentIndices.reserve(selectedIndexes.size());
  for (const auto& index : selectedIndexes) {
    persistentIndices.push_back(QPersistentModelIndex(index));
  }

  for (const auto& index : persistentIndices) {
    // We don't need to check if the row is editable here because that's
    // done during row removal.
    model->removeRow(index.row());
  }
}

void BaseTableTab::onSelectionModelSelectionChanged() {
  // Ignore the 'selected' parameter because it only contains what's
  // changed and so would omit any already-selected editable rows.
  auto selectedIndexes = tableView->selectionModel()->selectedIndexes();
  if (selectedIndexes.isEmpty()) {
    deleteRowButton->setEnabled(false);
    return;
  }

  const auto model = tableView->model();
  for (const auto& index : selectedIndexes) {
    const auto flags = model->flags(index);

    if (flags.testFlag(Qt::ItemIsEditable)) {
      deleteRowButton->setEnabled(true);
      return;
    }
  }

  deleteRowButton->setEnabled(false);
}

void BaseTableTab::onModelRowsInserted() {
  emit tableRowCountChanged(hasUserMetadata());
}

void BaseTableTab::onModelRowsRemoved() {
  emit tableRowCountChanged(hasUserMetadata());
}

FileTableTab::FileTableTab(QWidget* parent,
                           const std::vector<LootSettings::Language>& languages,
                           const std::string& language,
                           const QStringList& completions) :
    MetadataTableTab(parent),
    languages(languages),
    language(language),
    completions(completions) {
  configureAsDropTarget();
}

void FileTableTab::initialiseInputs(const std::vector<File>& nonUserMetadata,
                                    const std::vector<File>& userMetadata) {
  auto tableModel =
      new FileTableModel(this, nonUserMetadata, userMetadata, language);

  setTableModel(tableModel);

  auto filenameDelegate = new AutocompletingLineEditDelegate(this, completions);
  auto detailDelegate = new MessageContentDelegate(this, languages);

  setItemDelegateForColumn(tableModel->NAME_COLUMN, filenameDelegate);
  setItemDelegateForColumn(tableModel->DETAIL_COLUMN, detailDelegate);
}

std::vector<File> FileTableTab::getUserMetadata() const {
  return dynamic_cast<FileTableModel*>(getTableModel())->getUserMetadata();
}

bool FileTableTab::hasUserMetadata() const {
  return !getUserMetadata().empty();
}

void LoadAfterFileTableTab::initialiseInputs(
    const std::vector<File>& nonUserMetadata,
    const std::vector<File>& userMetadata) {
  FileTableTab::initialiseInputs(nonUserMetadata, userMetadata);

  setColumnHidden(FileTableModel::DISPLAY_NAME_COLUMN, true);
  setColumnHidden(FileTableModel::DETAIL_COLUMN, true);
}

MessageContentTableWidget::MessageContentTableWidget(
    QWidget* parent,
    const std::vector<LootSettings::Language>& languages) :
    MetadataTableTab(parent) {
  for (const auto& language : languages) {
    auto name = QString::fromStdString(language.name);
    this->languages.push_back(
        std::make_pair(name, QString::fromStdString(language.locale)));

    languageMap.insert({language.locale, name});
  }
}

void MessageContentTableWidget::initialiseInputs(
    const std::vector<MessageContent>& nonUserMetadata,
    const std::vector<MessageContent>& userMetadata) {
  std::map<MessageType, std::pair<QString, QVariant>> messageTypeMap = {
      {MessageType::say, {translate("Note"), QString("say")}},
      {MessageType::warn, {translate("Warning"), QString("warn")}},
      {MessageType::error, {translate("Error"), QString("error")}}};

  auto tableModel = new MessageContentTableModel(
      this, nonUserMetadata, userMetadata, languageMap);

  setTableModel(tableModel);
  setColumnFixedWidth(
      tableModel->LANGUAGE_COLUMN,
      calculateMinimumColumnWidth(
          tableModel, tableModel->LANGUAGE_COLUMN, languageMap));

  auto languageDelegate = new ComboBoxDelegate(this, languages);

  setItemDelegateForColumn(tableModel->LANGUAGE_COLUMN, languageDelegate);
}

std::vector<MessageContent> MessageContentTableWidget::getUserMetadata() const {
  return dynamic_cast<MessageContentTableModel*>(getTableModel())
      ->getUserMetadata();
}

bool MessageContentTableWidget::hasUserMetadata() const {
  return !getUserMetadata().empty();
}

MessageTableTab::MessageTableTab(
    QWidget* parent,
    const std::vector<LootSettings::Language>& languages,
    const std::string& language) :
    MetadataTableTab(parent), languages(languages), language(language) {}

void MessageTableTab::initialiseInputs(
    const std::vector<Message>& nonUserMetadata,
    const std::vector<Message>& userMetadata) {
  std::map<MessageType, std::pair<QString, QVariant>> messageTypeMap = {
      {MessageType::say, {translate("Note"), QString("say")}},
      {MessageType::warn, {translate("Warning"), QString("warn")}},
      {MessageType::error, {translate("Error"), QString("error")}}};

  auto tableModel = new MessageTableModel(
      this, nonUserMetadata, userMetadata, messageTypeMap, language);

  setTableModel(tableModel);

  std::vector<std::pair<QString, QVariant>> messageTypes;
  for (const auto& entry : messageTypeMap) {
    messageTypes.push_back(entry.second);
  }

  setColumnFixedWidth(0,
                      calculateMinimumColumnWidth(tableModel, 0, messageTypes));

  auto messageTypeDelegate = new ComboBoxDelegate(this, messageTypes);
  auto contentDelegate = new MessageContentDelegate(this, languages);

  setItemDelegateForColumn(tableModel->TYPE_COLUMN, messageTypeDelegate);
  setItemDelegateForColumn(tableModel->CONTENT_COLUMN, contentDelegate);
}

std::vector<Message> MessageTableTab::getUserMetadata() const {
  return dynamic_cast<MessageTableModel*>(getTableModel())->getUserMetadata();
}

bool MessageTableTab::hasUserMetadata() const {
  return !getUserMetadata().empty();
}

void LocationTableTab::initialiseInputs(
    const std::vector<Location>& nonUserMetadata,
    const std::vector<Location>& userMetadata) {
  auto tableModel = new LocationTableModel(this, nonUserMetadata, userMetadata);

  setTableModel(tableModel);
}

std::vector<Location> LocationTableTab::getUserMetadata() const {
  return dynamic_cast<LocationTableModel*>(getTableModel())->getUserMetadata();
}

bool LocationTableTab::hasUserMetadata() const {
  return !getUserMetadata().empty();
}

CleaningDataTableTab::CleaningDataTableTab(
    QWidget* parent,
    const std::vector<LootSettings::Language>& languages,
    const std::string& language) :
    MetadataTableTab(parent), languages(languages), language(language) {}

void CleaningDataTableTab::initialiseInputs(
    const std::vector<PluginCleaningData>& nonUserMetadata,
    const std::vector<PluginCleaningData>& userMetadata) {
  auto tableModel =
      new CleaningDataTableModel(this, nonUserMetadata, userMetadata, language);

  setTableModel(tableModel);

  auto crcDelegate = new CrcLineEditDelegate(this);

  setItemDelegateForColumn(tableModel->CRC_COLUMN, crcDelegate);

  auto detailDelegate = new MessageContentDelegate(this, languages);

  setItemDelegateForColumn(tableModel->DETAIL_COLUMN, detailDelegate);
}

void CleaningDataTableTab::hideCounts(bool hide) {
  setColumnHidden(CleaningDataTableModel::ITM_COLUMN, hide);
  setColumnHidden(CleaningDataTableModel::DELETED_REFERENCE_COLUMN, hide);
  setColumnHidden(CleaningDataTableModel::DELETED_NAVMESH_COLUMN, hide);
  setColumnHidden(CleaningDataTableModel::DETAIL_COLUMN, hide);
}

std::vector<PluginCleaningData> CleaningDataTableTab::getUserMetadata() const {
  return dynamic_cast<CleaningDataTableModel*>(getTableModel())
      ->getUserMetadata();
}

bool CleaningDataTableTab::hasUserMetadata() const {
  return !getUserMetadata().empty();
}

TagTableTab::TagTableTab(QWidget* parent, const QStringList& completions) :
    MetadataTableTab(parent), completions(completions) {}

void TagTableTab::initialiseInputs(const std::vector<Tag>& nonUserMetadata,
                                   const std::vector<Tag>& userMetadata) {
  std::map<bool, std::pair<QString, QVariant>> suggestionTypeMap = {
      {true, {translate("Add"), true}}, {false, {translate("Remove"), false}}};

  std::vector<std::pair<QString, QVariant>> suggestionTypes;
  for (const auto& entry : suggestionTypeMap) {
    suggestionTypes.push_back(entry.second);
  }

  auto tableModel =
      new TagTableModel(this, nonUserMetadata, userMetadata, suggestionTypeMap);

  setTableModel(tableModel);
  setColumnFixedWidth(
      tableModel->TYPE_COLUMN,
      calculateMinimumColumnWidth(
          tableModel, tableModel->TYPE_COLUMN, suggestionTypes));

  auto addRemoveDelegate = new ComboBoxDelegate(this, suggestionTypes);
  auto nameDelegate = new AutocompletingLineEditDelegate(this, completions);

  setItemDelegateForColumn(tableModel->TYPE_COLUMN, addRemoveDelegate);
  setItemDelegateForColumn(tableModel->NAME_COLUMN, nameDelegate);
}

std::vector<Tag> TagTableTab::getUserMetadata() const {
  return dynamic_cast<TagTableModel*>(getTableModel())->getUserMetadata();
}

bool TagTableTab::hasUserMetadata() const { return !getUserMetadata().empty(); }
}
