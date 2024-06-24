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
#include "gui/state/logging.h"

namespace loot {
std::vector<std::string> getMessageTexts(
    const std::vector<SourcedMessage>& messages) {
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

QString getLongestString(std::initializer_list<std::string> list) {
  if (list.size() == 0) {
    return QString();
  }

  const std::string* longestString = list.begin();
  size_t longestStringLength = 0;

  for (auto& string : list) {
    const auto length = string.size();
    if (longestStringLength < length) {
      longestString = &string;
      longestStringLength = length;
    }
  }

  return QString::fromStdString(*longestString);
}

SizeHintCacheKey getSizeHintCacheKey(const QModelIndex& index) {
  if (index.row() == 0) {
    auto generalInfo = index.data(RawDataRole).value<GeneralInformation>();
    auto counters =
        index.data(CountersRole).value<GeneralInformationCounters>();

    const auto secondColumnString =
        getLongestString({generalInfo.masterlistRevision.id,
                          generalInfo.masterlistRevision.date,
                          generalInfo.preludeRevision.id,
                          generalInfo.preludeRevision.date});
    const auto fourthColumnString = QString::number(counters.totalMessages);
    const auto sixthColumnString = QString::number(counters.totalPlugins);

    auto pluginTypeRowCount = 1;
    if (generalInfo.gameSupportsMediumPlugins) {
      pluginTypeRowCount += 1;
    }
    if (generalInfo.gameSupportsLightPlugins) {
      pluginTypeRowCount += 1;
    }

    return SizeHintCacheKey(secondColumnString,
                            fourthColumnString,
                            sixthColumnString,
                            getMessageTexts(generalInfo.generalMessages),
                            {std::to_string(pluginTypeRowCount)},
                            true);
  } else {
    auto pluginItem = index.data(RawDataRole).value<PluginItem>();
    auto filters =
        index.data(CardContentFiltersRole).value<CardContentFiltersState>();

    return SizeHintCacheKey(
        getTagsText(pluginItem.currentTags, filters.hideBashTags),
        getTagsText(pluginItem.addTags, filters.hideBashTags),
        getTagsText(pluginItem.removeTags, filters.hideBashTags),
        getMessageTexts(filterMessages(pluginItem, filters)),
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

  card->setShowSeparateLightPluginCount(generalInfo.gameSupportsLightPlugins);
  card->setShowSeparateMediumPluginCount(generalInfo.gameSupportsMediumPlugins);
  card->setMasterlistInfo(generalInfo.masterlistRevision);
  card->setPreludeInfo(generalInfo.preludeRevision);
  card->setMessageCounts(
      counters.warnings, counters.errors, counters.totalMessages);
  card->setPluginCounts(counters.activeLight,
                        counters.activeMedium,
                        counters.activeFull,
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

static std::map<QWidget*, int> minWidthByCard;

QSize calculateSize(const QWidget* card,
                    const QStyleOptionViewItem& option,
                    int largestMinCardWidth) {
  /*
   * There are three relevant widths:
   *
   * 1. The width of the viewport
   * 2. The card's minimum width
   * 3. The largest minimum width of the cards in the list
   *
   * If the viewport is wider than the card's minimum width, this function
   * should return the viewport width so that the card expands to fill the
   * viewport.
   *
   * If the viewport is narrower than the card's minimum width, this function
   * should return the card's minimum width so that Qt can use that to
   * calculate how much it should be possible to horizontally scroll the
   * viewport by (which it does using the largest width it's given).
   *
   * The largest minimum width is needed because text wrapping means that the
   * card's height depends on its width, and all cards must have the same
   * width, so when the viewport is narrow enough that scroll bars appear, all
   * cards must continue to have the same width even out-of-view, which means
   * they must all continue to have the same width as the card with the largest
   * minimum width.
   *
   * The problem is that you don't know what the largest minimum width is when
   * you're calculating card sizes for the first time, so you can't get an
   * accurate height. That's why the card sizing cache is used to get it, as
   * that cache is populated before the delegate tries to size anything.
   */
  const auto minCardWidth = card->layout()->minimumSize().width();

  // option.rect is the width of the scrollable area that the viewport looks
  // into. It's initially set to some not very useful value, and is subsequently
  // set equal to largestMinWidth once the scrollable area has been resized to
  // fit the cards it contains. This repeats whenever the viewport is resized.
  const auto rectWidth = option.rect.width();

  const auto cardWidth = std::max(rectWidth, minCardWidth);

  const auto widthForHeight = std::max(rectWidth, largestMinCardWidth);

  const auto height =
      card->hasHeightForWidth()
          ? card->layout()->minimumHeightForWidth(widthForHeight)
          : card->minimumHeight();

  return QSize(cardWidth, height);
}

CardSizingCache::CardSizingCache(QWidget* cardParentWidget) :
    cardParentWidget(cardParentWidget) {}

void CardSizingCache::update(const QAbstractItemModel* model) {
  update(model, 0, model->rowCount());
}

void CardSizingCache::update(const QModelIndex& topLeft,
                             const QModelIndex& bottomRight) {
  update(topLeft.model(), topLeft.row(), bottomRight.row());
}

void CardSizingCache::update(const QAbstractItemModel* model,
                             int firstRow,
                             int lastRow) {
  for (int row = firstRow; row <= lastRow; row += 1) {
    const auto index = model->index(row, PluginItemModel::CARDS_COLUMN);

    update(index);
  }
}

QWidget* CardSizingCache::update(const QModelIndex& index) {
  if (!index.isValid()) {
    return nullptr;
  }

  // Get the key cache entry if it exists, and the new cache key and its
  // entry.
  const auto keyCacheIt = keyCache.find(index.row());
  const auto newCacheKey = getSizeHintCacheKey(index);
  auto newCardCacheIt = cardCache.find(newCacheKey);

  if (keyCacheIt != keyCache.end()) {
    // This row has a cached key, get it.
    const auto oldCacheKey = keyCacheIt->second;
    if (*oldCacheKey == newCacheKey) {
      // The cache key hasn't changed, no need to make any changes.
      // Just return the key's card. It should never be null but handle that
      // for safety.
      return newCardCacheIt == cardCache.end() ? nullptr
                                               : newCardCacheIt->second.first;
    } else {
      // The cache key has changed, get the old key's card cache entry and
      // reduce its count by 1.
      const auto oldCardCacheIt = cardCache.find(*oldCacheKey);
      if (oldCardCacheIt != cardCache.end()) {
        oldCardCacheIt->second.second -= 1;

        // If the old key's count is now 0, remove it from the card cache.
        if (oldCardCacheIt->second.second == 0) {
          cardCache.erase(oldCardCacheIt);
        }
      }
    }
  }

  // If there is no entry for the new cache key, create one.
  if (newCardCacheIt == cardCache.end()) {
    QWidget* widget = nullptr;
    if (index.row() == 0) {
      widget = setGeneralInfoCardContent(new GeneralInfoCard(cardParentWidget),
                                         index);
    } else {
      widget = setPluginCardContent(new PluginCard(cardParentWidget), index);
    }

    prepareWidget(widget);

    newCardCacheIt =
        cardCache.emplace(newCacheKey, std::make_pair(widget, 0)).first;
  }

  // Increase the new cache key's usage count by 1.
  newCardCacheIt->second.second += 1;

  if (keyCacheIt == keyCache.end()) {
    // This row has no cached key, add a pointer to the new key.
    keyCache.emplace(index.row(), &newCardCacheIt->first);
  } else {
    // Now update the key cache entry to point to the new key for this row.
    keyCacheIt->second = &newCardCacheIt->first;
  }

  // Return the new cache key entry's card.
  return newCardCacheIt->second.first;
}

QWidget* CardSizingCache::getCard(const SizeHintCacheKey& key) const {
  auto it = cardCache.find(key);
  if (it != cardCache.end()) {
    return it->second.first;
  }

  return nullptr;
}

int CardSizingCache::getLargestMinWidth() const {
  int largest = 0;
  for (const auto& [key, value] : cardCache) {
    const auto minWidth = value.first->layout()->minimumSize().width();
    if (minWidth > largest) {
      largest = minWidth;
    }
  }

  return largest;
}

CardDelegate::CardDelegate(QListView* parent,
                           CardSizingCache& cardSizingCache) :
    QStyledItemDelegate(parent),
    generalInfoCard(new GeneralInfoCard(parent->viewport())),
    pluginCard(new PluginCard(parent->viewport())),
    cardSizingCache(&cardSizingCache) {
  prepareWidget(generalInfoCard);
  prepareWidget(pluginCard);
}

void CardDelegate::setIcons() { pluginCard->setIcons(); }

void CardDelegate::refreshMessages() {
  generalInfoCard->refreshMessages();
  pluginCard->refreshMessages();
}

void CardDelegate::refreshStyling() {
  generalInfoCard->setVisible(true);
  generalInfoCard->setVisible(false);

  pluginCard->setVisible(true);
  pluginCard->setVisible(false);
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

  const auto sizeHint =
      calculateSize(widget, styleOption, cardSizingCache->getLargestMinWidth());

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

  const auto cacheKey = getSizeHintCacheKey(index);

  auto it = sizeHintCache.find(cacheKey);
  if (it != sizeHintCache.end()) {
    // Found a cached size, check if its width matches the current available
    // width.
    if (it->second.width() == styleOption.rect.width()) {
      // The cached size is valid, return it.
      return it->second;
    }
  } else {
    // Store an invalid size so that it can be replaced below.
    it = sizeHintCache.emplace(cacheKey, QSize()).first;
  }

  auto card = cardSizingCache->getCard(cacheKey);
  if (card == nullptr) {
    const auto logger = getLogger();
    logger->warn(
        "No cached card exists for row {}, card sizes may not be calculated "
        "correctly",
        index.row());
    card = cardSizingCache->update(index);
  }

  const auto sizeHint =
      calculateSize(card, styleOption, cardSizingCache->getLargestMinWidth());

  it->second = sizeHint;

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
