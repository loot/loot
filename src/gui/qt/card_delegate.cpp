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

#include "gui/qt/card_delegate.h"

#include "gui/qt/counters.h"
#include "gui/qt/plugin_item_model.h"

namespace loot {
std::vector<std::string> getMessageTexts(
    const std::vector<SimpleMessage>& messages) {
  std::vector<std::string> texts;
  for (const auto& message : messages) {
    texts.push_back(message.text);
  }

  return texts;
}

std::vector<std::string> getLocationNames(
    const std::vector<Location>& locations,
    bool hideLocations) {
  if (hideLocations) {
    return {};
  }

  std::vector<std::string> names;
  std::transform(locations.begin(),
                 locations.end(),
                 std::back_inserter(names),
                 [](const auto& location) { return location.GetName(); });

  return names;
}

SizeHintCacheKey getSizeHintCacheKey(const QModelIndex& index) {
  if (index.row() == 0) {
    auto generalInfo = index.data(RawDataRole).value<GeneralInformation>();

    return SizeHintCacheKey(QString(),
                            QString(),
                            QString(),
                            getMessageTexts(generalInfo.generalMessages),
                            {},
                            true);
  } else {
    auto pluginItem = index.data(RawDataRole).value<PluginItem>();
    auto filters =
        index.data(CardContentFiltersRole).value<CardContentFiltersState>();

    return SizeHintCacheKey(
        getTagsText(pluginItem.currentTags, filters.hideBashTags),
        getTagsText(pluginItem.addTags, filters.hideBashTags),
        getTagsText(pluginItem.removeTags, filters.hideBashTags),
        getMessageTexts(filterMessages(pluginItem.messages, filters)),
        getLocationNames(pluginItem.locations, filters.hideLocations),
        false);
  }
}

void prepareWidget(QWidget* widget) {
  auto sizePolicy = widget->sizePolicy();
  sizePolicy.setRetainSizeWhenHidden(true);
  widget->setSizePolicy(sizePolicy);
  widget->setHidden(true);
}

GeneralInfoCard* setGeneralInfoCardContent(GeneralInfoCard* card,
                                           const QModelIndex& index) {
  auto generalInfo = index.data(RawDataRole).value<GeneralInformation>();
  auto counters = index.data(CountersRole).value<GeneralInformationCounters>();

  card->setGameType(generalInfo.gameType);
  card->setMasterlistInfo(generalInfo.masterlistRevision);
  card->setPreludeInfo(generalInfo.preludeRevision);
  card->setMessageCounts(
      counters.warnings, counters.errors, counters.totalMessages);
  card->setPluginCounts(counters.activeLight,
                        counters.activeRegular,
                        counters.dirty,
                        counters.totalPlugins);
  card->setGeneralMessages(generalInfo.generalMessages);

  return card;
}

PluginCard* setPluginCardContent(PluginCard* card, const QModelIndex& index) {
  auto pluginItem = index.data(RawDataRole).value<PluginItem>();
  auto filters =
      index.data(CardContentFiltersRole).value<CardContentFiltersState>();
  auto searchResultData =
      index.data(SearchResultRole).value<SearchResultData>();

  card->setContent(pluginItem, filters);

  card->setSearchResult(searchResultData.isResult,
                        searchResultData.isCurrentResult);

  return card;
}

QSize calculateSize(const QWidget* widget, const QStyleOptionViewItem& option) {
  const auto minLayoutWidth = widget->layout()->minimumSize().width();
  const auto rectWidth = option.rect.width();

  const auto width = rectWidth > minLayoutWidth ? rectWidth : minLayoutWidth;
  const auto height = widget->hasHeightForWidth()
                          ? widget->layout()->minimumHeightForWidth(width)
                          : widget->minimumHeight();

  return QSize(width, height);
}

CardDelegate::CardDelegate(QListView* parent) :
    QStyledItemDelegate(parent),
    generalInfoCard(new GeneralInfoCard(parent->viewport())),
    pluginCard(new PluginCard(parent->viewport())) {
  prepareWidget(generalInfoCard);
  prepareWidget(pluginCard);
}

void CardDelegate::setIcons() { pluginCard->setIcons(); }

void CardDelegate::refreshMessages() {
  generalInfoCard->refreshMessages();
  pluginCard->refreshMessages();
}

void CardDelegate::paint(QPainter* painter,
                         const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
  if (!index.isValid()) {
    return QStyledItemDelegate::paint(painter, option, index);
  }

  auto styleOption = QStyleOptionViewItem(option);
  initStyleOption(&styleOption, index);

  // This drawControl is needed to draw the styling that's used to
  // indicate when an item is hovered over or selected.
  styleOption.widget->style()->drawControl(
      QStyle::CE_ItemViewItem, &styleOption, painter, styleOption.widget);
  painter->save();

  painter->translate(styleOption.rect.topLeft());

  QWidget* widget = nullptr;

  if (index.row() == 0) {
    widget = setGeneralInfoCardContent(generalInfoCard, index);
  } else {
    widget = setPluginCardContent(pluginCard, index);
  }

  const auto sizeHint = calculateSize(widget, styleOption);

  widget->setFixedSize(sizeHint);

  widget->render(painter, QPoint(), QRegion(), QWidget::DrawChildren);

  painter->restore();
}

QSize CardDelegate::sizeHint(const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
  if (!index.isValid()) {
    return QStyledItemDelegate::sizeHint(option, index);
  }

  auto styleOption = QStyleOptionViewItem(option);
  initStyleOption(&styleOption, index);

  if (!styleOption.rect.isValid()) {
    return QStyledItemDelegate::sizeHint(option, index);
  }

  // Cache widgets and size hints by SizeHintCacheKey because that contains all
  // the data that the card size could depend on, aside from the available
  // width, and so it means that different plugins with cards of the same size
  // can share cached widget objects and size data.
  //
  // It's a bit inefficient to do all the message and tag transformations here
  // and again when setting the actual card content (if that happens), but in
  // practice the cost is negligible.
  auto cacheKey = getSizeHintCacheKey(index);

  auto it = sizeHintCache.find(cacheKey);
  if (it != sizeHintCache.end()) {
    // Found a cached size, check if its width matches the current available
    // width.
    if (it->second.second.width() == styleOption.rect.width()) {
      // The cached size is valid, return it.
      return it->second.second;
    }
  } else {
    // There is no size hint cached, create the relevant class of widget, set
    // relevant content so that a correct size hint can be calculated, then
    // add a cache entry with an invalid size - the size will be overwritten
    // before this function returns.
    QWidget* parentWidget = qobject_cast<QListView*>(parent())->viewport();

    QWidget* widget = nullptr;
    if (index.row() == 0) {
      widget =
          setGeneralInfoCardContent(new GeneralInfoCard(parentWidget), index);
      // The general info widget needs to be prepared because unlike the plugin
      // cards it's got static text that is visible over the painted content.
      prepareWidget(widget);
    } else {
      widget = setPluginCardContent(new PluginCard(parentWidget), index);
    }

    it = sizeHintCache.emplace(cacheKey, std::make_pair(widget, QSize())).first;
  }

  // If the widget is new, it's already been initalised with the appopriate data
  // above. If the widget is not new then it already has appropriate data and a
  // size just needs to be calculated for the current available width.
  auto sizeHint = calculateSize(it->second.first, styleOption);

  it->second.second = sizeHint;

  return sizeHint;
}

QWidget* CardDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem&,
                                    const QModelIndex& index) const {
  if (!index.isValid()) {
    return nullptr;
  }

  if (index.row() == 0) {
    return new GeneralInfoCard(parent);
  } else {
    return new PluginCard(parent);
  }
}

void CardDelegate::setEditorData(QWidget* editor,
                                 const QModelIndex& index) const {
  if (!index.isValid()) {
    return;
  }

  if (index.row() == 0) {
    setGeneralInfoCardContent(qobject_cast<GeneralInfoCard*>(editor), index);
  } else {
    setPluginCardContent(qobject_cast<PluginCard*>(editor), index);
  }
}

void CardDelegate::setModelData(QWidget*,
                                QAbstractItemModel*,
                                const QModelIndex&) const {
  // Do nothing, it's not actually an editor.
}
}
