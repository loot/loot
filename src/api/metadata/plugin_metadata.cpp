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
    <https://www.gnu.org/licenses/>.
    */

#include "loot/metadata/plugin_metadata.h"

#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "api/game/game.h"

using std::inserter;
using std::regex;
using std::regex_match;
using std::set;
using std::vector;

namespace loot {
PluginMetadata::PluginMetadata() : enabled_(true) {}

PluginMetadata::PluginMetadata(const std::string& n) : name_(n), enabled_(true) {
    //If the name passed ends in '.ghost', that should be trimmed.
  if (boost::iends_with(name_, ".ghost"))
    name_ = name_.substr(0, name_.length() - 6);
}

void PluginMetadata::MergeMetadata(const PluginMetadata& plugin) {
  BOOST_LOG_TRIVIAL(trace) << "Merging metadata for: " << name_;
  if (plugin.HasNameOnly())
    return;

// For 'enabled' and 'priority' metadata, use the given plugin's values,
// but if the 'priority' user value is not explicit, ignore it.
  enabled_ = plugin.IsEnabled();

  if (plugin.localPriority_.IsExplicit()) {
    SetLocalPriority(plugin.localPriority_);
  }

  if (plugin.globalPriority_.IsExplicit()) {
    SetGlobalPriority(plugin.globalPriority_);
  }

  // Merge the following. If any files in the source already exist in the
  // destination, they will be skipped. Files have display strings and
  // condition strings which aren't considered when comparing them, so
  // will be lost if the plugin being merged in has additional data in
  // these strings.
  loadAfter_.insert(begin(plugin.loadAfter_), end(plugin.loadAfter_));
  requirements_.insert(begin(plugin.requirements_), end(plugin.requirements_));
  incompatibilities_.insert(begin(plugin.incompatibilities_), end(plugin.incompatibilities_));

  // Merge Bash Tags too. Conditions are ignored during comparison, but
  // if a tag is added and removed, both instances will be in the set.
  tags_.insert(begin(plugin.tags_), end(plugin.tags_));

  // Messages are in an ordered list, and should be fully merged.
  messages_.insert(end(messages_), begin(plugin.messages_), end(plugin.messages_));

  dirtyInfo_.insert(begin(plugin.dirtyInfo_), end(plugin.dirtyInfo_));
  cleanInfo_.insert(begin(plugin.cleanInfo_), end(plugin.cleanInfo_));
  locations_.insert(begin(plugin.locations_), end(plugin.locations_));

  return;
}

PluginMetadata PluginMetadata::NewMetadata(const PluginMetadata& plugin) const {
  using std::set_difference;

  BOOST_LOG_TRIVIAL(trace) << "Comparing new metadata for: " << name_;
  PluginMetadata p(*this);

  //Compare this plugin against the given plugin.
  set<File> filesDiff;
  set_difference(begin(loadAfter_),
                 end(loadAfter_),
                 begin(plugin.loadAfter_),
                 end(plugin.loadAfter_),
                 inserter(filesDiff, begin(filesDiff)));
  p.SetLoadAfterFiles(filesDiff);

  filesDiff.clear();
  set_difference(begin(requirements_),
                 end(requirements_),
                 begin(plugin.requirements_),
                 end(plugin.requirements_),
                 inserter(filesDiff, begin(filesDiff)));
  p.SetRequirements(filesDiff);

  filesDiff.clear();
  set_difference(begin(incompatibilities_),
                 end(incompatibilities_),
                 begin(plugin.incompatibilities_),
                 end(plugin.incompatibilities_),
                 inserter(filesDiff, begin(filesDiff)));
  p.SetIncompatibilities(filesDiff);

  vector<Message> msgs1 = plugin.GetMessages();
  vector<Message> msgs2 = messages_;
  std::sort(begin(msgs1), end(msgs1));
  std::sort(begin(msgs2), end(msgs2));
  vector<Message> mDiff;
  set_difference(begin(msgs2),
                 end(msgs2),
                 begin(msgs1),
                 end(msgs1),
                 inserter(mDiff, begin(mDiff)));
  p.SetMessages(mDiff);

  set<Tag> tagDiff;
  set_difference(begin(tags_),
                 end(tags_),
                 begin(plugin.tags_),
                 end(plugin.tags_),
                 inserter(tagDiff, begin(tagDiff)));
  p.SetTags(tagDiff);

  set<PluginCleaningData> dirtDiff;
  set_difference(begin(dirtyInfo_),
                 end(dirtyInfo_),
                 begin(plugin.dirtyInfo_),
                 end(plugin.dirtyInfo_),
                 inserter(dirtDiff, begin(dirtDiff)));
  p.SetDirtyInfo(dirtDiff);

  set<PluginCleaningData> cleanDiff;
  set_difference(begin(cleanInfo_),
                 end(cleanInfo_),
                 begin(plugin.cleanInfo_),
                 end(plugin.cleanInfo_),
                 inserter(cleanDiff, begin(cleanDiff)));
  p.SetCleanInfo(cleanDiff);

  set<Location> locationsDiff;
  set_difference(begin(locations_),
                 end(locations_),
                 begin(plugin.locations_),
                 end(plugin.locations_),
                 inserter(locationsDiff, begin(locationsDiff)));
  p.SetLocations(locationsDiff);

  return p;
}

std::string PluginMetadata::GetName() const {
  return name_;
}

std::string PluginMetadata::GetLowercasedName() const {
  return boost::locale::to_lower(name_);
}

bool PluginMetadata::IsEnabled() const {
  return enabled_;
}

Priority PluginMetadata::GetLocalPriority() const {
  return localPriority_;
}

Priority PluginMetadata::GetGlobalPriority() const {
  return globalPriority_;
}

std::set<File> PluginMetadata::GetLoadAfterFiles() const {
  return loadAfter_;
}

std::set<File> PluginMetadata::GetRequirements() const {
  return requirements_;
}

std::set<File> PluginMetadata::GetIncompatibilities() const {
  return incompatibilities_;
}

std::vector<Message> PluginMetadata::GetMessages() const {
  return messages_;
}

std::set<Tag> PluginMetadata::GetTags() const {
  return tags_;
}

std::set<PluginCleaningData> PluginMetadata::GetDirtyInfo() const {
  return dirtyInfo_;
}

std::set<PluginCleaningData> PluginMetadata::GetCleanInfo() const {
  return cleanInfo_;
}

std::set<Location> PluginMetadata::GetLocations() const {
  return locations_;
}

std::vector<SimpleMessage> PluginMetadata::GetSimpleMessages(const LanguageCode language) const {
  std::vector<SimpleMessage> simpleMessages(messages_.size());
  std::transform(begin(messages_), end(messages_), begin(simpleMessages), [&](const Message& message) {
    return message.ToSimpleMessage(language);
  });

  return simpleMessages;
}

void PluginMetadata::SetEnabled(const bool e) {
  enabled_ = e;
}

void PluginMetadata::SetLocalPriority(const Priority& priority) {
  localPriority_ = priority;
}

void PluginMetadata::SetGlobalPriority(const Priority& priority) {
  globalPriority_ = priority;
}

void PluginMetadata::SetLoadAfterFiles(const std::set<File>& l) {
  loadAfter_ = l;
}

void PluginMetadata::SetRequirements(const std::set<File>& r) {
  requirements_ = r;
}

void PluginMetadata::SetIncompatibilities(const std::set<File>& i) {
  incompatibilities_ = i;
}

void PluginMetadata::SetMessages(const std::vector<Message>& m) {
  messages_ = m;
}

void PluginMetadata::SetTags(const std::set<Tag>& t) {
  tags_ = t;
}

void PluginMetadata::SetDirtyInfo(const std::set<PluginCleaningData>& dirtyInfo) {
  dirtyInfo_ = dirtyInfo;
}

void PluginMetadata::SetCleanInfo(const std::set<PluginCleaningData>& info) {
  cleanInfo_ = info;
}

void PluginMetadata::SetLocations(const std::set<Location>& locations) {
  locations_ = locations;
}

bool PluginMetadata::HasNameOnly() const {
  return !localPriority_.IsExplicit()
    && !globalPriority_.IsExplicit()
    && loadAfter_.empty()
    && requirements_.empty()
    && incompatibilities_.empty()
    && messages_.empty()
    && tags_.empty()
    && dirtyInfo_.empty()
    && cleanInfo_.empty()
    && locations_.empty();
}

bool PluginMetadata::IsRegexPlugin() const {
    // Treat as regex if the plugin filename contains any of ":\*?|" as
    // they are not valid Windows filename characters, but have meaning
    // in regexes.
  return strpbrk(name_.c_str(), ":\\*?|") != nullptr;
}

bool PluginMetadata::operator == (const PluginMetadata& rhs) const {
  if (IsRegexPlugin() == rhs.IsRegexPlugin())
    return boost::iequals(name_, rhs.GetName());

  if (IsRegexPlugin())
    return regex_match(rhs.GetName(), regex(name_, regex::ECMAScript | regex::icase));
  else
    return regex_match(name_, regex(rhs.GetName(), regex::ECMAScript | regex::icase));
}

bool PluginMetadata::operator != (const PluginMetadata& rhs) const {
  return !(*this == rhs);
}

bool PluginMetadata::operator == (const std::string& rhs) const {
  if (IsRegexPlugin())
    return regex_match(PluginMetadata(rhs).GetName(), regex(name_, regex::ECMAScript | regex::icase));
  else
    return boost::iequals(name_, PluginMetadata(rhs).GetName());
}

bool PluginMetadata::operator != (const std::string& rhs) const {
  return !(*this == rhs);
}
}
