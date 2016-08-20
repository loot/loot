/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2016    WrinklyNinja

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

#ifndef LOOT_TESTS_BACKEND_METADATA_PLUGIN_METADATA_TEST
#define LOOT_TESTS_BACKEND_METADATA_PLUGIN_METADATA_TEST

#include "backend/metadata/plugin_metadata.h"

#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class PluginMetadataTest : public CommonGameTestFixture {
protected:
  PluginMetadataTest() : info_(std::vector<MessageContent>({
    MessageContent("info", LanguageCode::english),
  })) {}

  const std::vector<MessageContent> info_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        PluginMetadataTest,
                        ::testing::Values(
                          GameType::tes5));

TEST_P(PluginMetadataTest, defaultConstructorShouldLeaveNameEmptyAndEnableMetadataAndLeaveAllOtherFieldsAtTheirDefaults) {
  PluginMetadata plugin;

  EXPECT_TRUE(plugin.Name().empty());
  EXPECT_TRUE(plugin.Enabled());
}

TEST_P(PluginMetadataTest, stringConstructorShouldSetNameToGivenStringAndEnableMetadataAndLeaveAllOtherFieldsAtTheirDefaults) {
  PluginMetadata plugin(blankEsm);

  EXPECT_EQ(blankEsm, plugin.Name());
  EXPECT_TRUE(plugin.Enabled());
}

TEST_P(PluginMetadataTest, equalityOperatorShouldUseCaseInsensitiveNameComparisonForNonRegexNames) {
  PluginMetadata plugin1(blankEsm);
  PluginMetadata plugin2(boost::to_lower_copy(blankEsm));
  EXPECT_TRUE(plugin1 == plugin2);

  plugin1 = PluginMetadata(blankEsm);
  plugin2 = PluginMetadata(blankDifferentEsm);
  EXPECT_FALSE(plugin1 == plugin2);
}

TEST_P(PluginMetadataTest, equalityOperatorShouldUseCaseInsensitiveNameComparisonForTwoRegexNames) {
  PluginMetadata plugin1("Blan.\\.esm");
  PluginMetadata plugin2("blan.\\.esm");
  EXPECT_TRUE(plugin1 == plugin2);
  EXPECT_TRUE(plugin2 == plugin1);

  plugin1 = PluginMetadata("Blan(k|p).esm");
  plugin2 = PluginMetadata("Blan.\\.esm");
  EXPECT_FALSE(plugin1 == plugin2);
  EXPECT_FALSE(plugin2 == plugin1);
}

TEST_P(PluginMetadataTest, equalityOperatorShouldUseRegexMatchingForARegexNameAndANonRegexName) {
  PluginMetadata plugin1("Blank.esm");
  PluginMetadata plugin2("Blan.\\.esm");
  EXPECT_TRUE(plugin1 == plugin2);
  EXPECT_TRUE(plugin2 == plugin1);

  plugin1 = PluginMetadata("Blan.esm");
  plugin2 = PluginMetadata("Blan.\\.esm");
  EXPECT_FALSE(plugin1 == plugin2);
  EXPECT_FALSE(plugin2 == plugin1);
}

TEST_P(PluginMetadataTest, mergeMetadataShouldNotChangeName) {
  PluginMetadata plugin1(blankEsm);
  PluginMetadata plugin2(blankDifferentEsm);

  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(blankEsm, plugin1.Name());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldNotUseMergedEnabledStateIfMergedMetadataIsEmpty) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin2.Enabled(false);
  ASSERT_TRUE(plugin2.HasNameOnly());
  plugin1.MergeMetadata(plugin2);

  EXPECT_TRUE(plugin1.Enabled());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldUseMergedEnabledStateIfMergedMetadataIsNotEmpty) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin2.Enabled(false);
  plugin2.LocalPriority(Priority(5));
  ASSERT_FALSE(plugin2.HasNameOnly());
  plugin1.MergeMetadata(plugin2);

  EXPECT_FALSE(plugin1.Enabled());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldUseMergedNonZeroLocalPriorityValue) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.LocalPriority(Priority(5));
  plugin2.LocalPriority(Priority(3));
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(3, plugin1.LocalPriority().getValue());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldUseMergedNonZeroGlobalPriorityValue) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.GlobalPriority(Priority(5));
  plugin2.GlobalPriority(Priority(3));
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(3, plugin1.GlobalPriority().getValue());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldNotUseImplicitZeroLocalPriorityValue) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.LocalPriority(Priority(5));
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(5, plugin1.LocalPriority().getValue());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldNotUseImplicitZeroGlobalPriorityValue) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.GlobalPriority(Priority(5));
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(5, plugin1.GlobalPriority().getValue());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldMergeAnExplicitLocalPriorityValueOfZero) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.LocalPriority(Priority(5));
  plugin2.LocalPriority(Priority(0));
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(0, plugin1.LocalPriority().getValue());
  EXPECT_TRUE(plugin1.LocalPriority().isExplicit());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldMergeAnExplicitGlobalPriorityValueOfZero) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.GlobalPriority(Priority(5));
  plugin2.GlobalPriority(Priority(0));
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(0, plugin1.GlobalPriority().getValue());
  EXPECT_TRUE(plugin1.GlobalPriority().isExplicit());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldMergeLoadAfterData) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  File file1(blankEsm);
  File file2(blankDifferentEsm);

  plugin1.LoadAfter({file1});
  plugin2.LoadAfter({file1, file2});
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(std::set<File>({file1, file2}), plugin1.LoadAfter());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldMergeRequirementData) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  File file1(blankEsm);
  File file2(blankDifferentEsm);

  plugin1.Reqs({file1});
  plugin2.Reqs({file1, file2});
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(std::set<File>({file1, file2}), plugin1.Reqs());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldMergeIncompatibilityData) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  File file1(blankEsm);
  File file2(blankDifferentEsm);

  plugin1.Incs({file1});
  plugin2.Incs({file1, file2});
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(std::set<File>({file1, file2}), plugin1.Incs());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldMergeMessages) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  Message message(MessageType::say, "content");

  plugin1.Messages({message});
  plugin2.Messages({message});
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(std::list<Message>({message, message}), plugin1.Messages());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldMergeTags) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  Tag tag1("Relev");
  Tag tag2("Relev", false);
  Tag tag3("Delev");

  plugin1.Tags({tag1});
  plugin2.Tags({tag1, tag2, tag3});
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(std::set<Tag>({tag1, tag2, tag3}), plugin1.Tags());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldMergeDirtyInfoData) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  PluginCleaningData info1(0x5, "utility", info_, 1, 2, 3);
  PluginCleaningData info2(0xA, "utility", info_, 1, 2, 3);

  plugin1.DirtyInfo({info1});
  plugin2.DirtyInfo({info1, info2});
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(std::set<PluginCleaningData>({info1, info2}), plugin1.DirtyInfo());
}
TEST_P(PluginMetadataTest, mergeMetadataShouldMergeCleanInfoData) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  PluginCleaningData info1(0x5, "utility");
  PluginCleaningData info2(0xA, "utility");

  plugin1.CleanInfo({info1});
  plugin2.CleanInfo({info1, info2});
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(std::set<PluginCleaningData>({info1, info2}), plugin1.CleanInfo());
}

TEST_P(PluginMetadataTest, mergeMetadataShouldMergeLocationData) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  Location location1("http://www.example.com/1");
  Location location2("http://www.example.com/2");

  plugin1.Locations({location1});
  plugin2.Locations({location1, location2});
  plugin1.MergeMetadata(plugin2);

  EXPECT_EQ(std::set<Location>({location1, location2}), plugin1.Locations());
}

TEST_P(PluginMetadataTest, diffMetadataShouldUseSourcePluginName) {
  PluginMetadata plugin1(blankEsm);
  PluginMetadata plugin2(blankDifferentEsm);

  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(blankEsm, diff.Name());
}

TEST_P(PluginMetadataTest, diffMetadataShouldUseSourcePluginEnabledState) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin2.Enabled(false);
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_TRUE(diff.Enabled());

  plugin1.Enabled(false);
  diff = plugin1.DiffMetadata(plugin2);

  EXPECT_FALSE(diff.Enabled());
}

TEST_P(PluginMetadataTest, diffMetadataShouldUseSourcePluginLocalPriorityIfItDiffersFromTheTargetPluginLocalPriority) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin2.LocalPriority(Priority(5));
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(0, diff.LocalPriority().getValue());
  EXPECT_FALSE(diff.LocalPriority().isExplicit());
}

TEST_P(PluginMetadataTest, diffMetadataShouldUseSourcePluginGlobalPriorityIfItDiffersFromTheTargetPluginGlobalPriority) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin2.GlobalPriority(Priority(5));
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(0, diff.GlobalPriority().getValue());
  EXPECT_FALSE(diff.GlobalPriority().isExplicit());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputDefaultLocalPriorityIfTheSourceAndTargetPluginLocalPrioritiesMatch) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.LocalPriority(Priority(5));
  plugin2.LocalPriority(Priority(5));
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(0, diff.LocalPriority().getValue());
  EXPECT_FALSE(diff.LocalPriority().isExplicit());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputDefaultGlobalPriorityIfTheSourceAndTargetPluginGlobalPrioritiesMatch) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.GlobalPriority(Priority(5));
  plugin2.GlobalPriority(Priority(5));
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(0, diff.GlobalPriority().getValue());
  EXPECT_FALSE(diff.GlobalPriority().isExplicit());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputLoadAfterDataThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  File file1(blankEsm);
  File file2(blankDifferentEsm);
  File file3(blankEsp);

  plugin1.LoadAfter({file1, file2});
  plugin2.LoadAfter({file1, file3});
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(std::set<File>({file2, file3}), diff.LoadAfter());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputRequirementsDataThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  File file1(blankEsm);
  File file2(blankDifferentEsm);
  File file3(blankEsp);

  plugin1.Reqs({file1, file2});
  plugin2.Reqs({file1, file3});
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(std::set<File>({file2, file3}), diff.Reqs());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputIncompatibilityDataThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  File file1(blankEsm);
  File file2(blankDifferentEsm);
  File file3(blankEsp);

  plugin1.Incs({file1, file2});
  plugin2.Incs({file1, file3});
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(std::set<File>({file2, file3}), diff.Incs());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputMessagesThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  Message message1(MessageType::say, "content1");
  Message message2(MessageType::say, "content2");
  Message message3(MessageType::say, "content3");

  plugin1.Messages({message1, message2});
  plugin2.Messages({message1, message3});
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(std::list<Message>({message2, message3}), diff.Messages());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputTagsThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  Tag tag1("Relev");
  Tag tag2("Relev", false);
  Tag tag3("Delev");

  plugin1.Tags({tag1, tag2});
  plugin2.Tags({tag1, tag3});
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(std::set<Tag>({tag2, tag3}), diff.Tags());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputDirtyInfoObjectsThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  PluginCleaningData info1(0x5, "utility", info_, 1, 2, 3);
  PluginCleaningData info2(0xA, "utility", info_, 1, 2, 3);
  PluginCleaningData info3(0x1, "utility", info_, 1, 2, 3);

  plugin1.DirtyInfo({info1, info2});
  plugin2.DirtyInfo({info1, info3});
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(std::set<PluginCleaningData>({info2, info3}), diff.DirtyInfo());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputCleanInfoObjectsThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  PluginCleaningData info1(0x5, "utility");
  PluginCleaningData info2(0xA, "utility");
  PluginCleaningData info3(0x1, "utility");

  plugin1.CleanInfo({info1, info2});
  plugin2.CleanInfo({info1, info3});
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(std::set<PluginCleaningData>({info2, info3}), diff.CleanInfo());
}

TEST_P(PluginMetadataTest, diffMetadataShouldOutputLocationsThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  Location location1("http://www.example.com/1");
  Location location2("http://www.example.com/2");
  Location location3("http://www.example.com/3");

  plugin1.Locations({location1, location2});
  plugin2.Locations({location1, location3});
  PluginMetadata diff = plugin1.DiffMetadata(plugin2);

  EXPECT_EQ(std::set<Location>({location2, location3}), diff.Locations());
}

TEST_P(PluginMetadataTest, newMetadataShouldUseSourcePluginName) {
  PluginMetadata plugin1(blankEsm);
  PluginMetadata plugin2(blankDifferentEsm);

  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(blankEsm, newMetadata.Name());
}

TEST_P(PluginMetadataTest, newMetadataShouldUseSourcePluginEnabledState) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin2.Enabled(false);
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_TRUE(newMetadata.Enabled());

  plugin1.Enabled(false);
  newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_FALSE(newMetadata.Enabled());
}

TEST_P(PluginMetadataTest, newMetadataShouldUseSourcePluginLocalPriority) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.LocalPriority(Priority(5));
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(5, newMetadata.LocalPriority().getValue());
}

TEST_P(PluginMetadataTest, newMetadataShouldUseSourcePluginGlobalPriority) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;

  plugin1.GlobalPriority(Priority(5));
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(5, newMetadata.GlobalPriority().getValue());
}

TEST_P(PluginMetadataTest, newMetadataShouldOutputLoadAfterDataThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  File file1(blankEsm);
  File file2(blankDifferentEsm);
  File file3(blankEsp);

  plugin1.LoadAfter({file1, file2});
  plugin2.LoadAfter({file1, file3});
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(std::set<File>({file2}), newMetadata.LoadAfter());
}

TEST_P(PluginMetadataTest, newMetadataShouldOutputRequirementsDataThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  File file1(blankEsm);
  File file2(blankDifferentEsm);
  File file3(blankEsp);

  plugin1.Reqs({file1, file2});
  plugin2.Reqs({file1, file3});
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(std::set<File>({file2}), newMetadata.Reqs());
}

TEST_P(PluginMetadataTest, newMetadataShouldOutputIncompatibilityDataThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  File file1(blankEsm);
  File file2(blankDifferentEsm);
  File file3(blankEsp);

  plugin1.Incs({file1, file2});
  plugin2.Incs({file1, file3});
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(std::set<File>({file2}), newMetadata.Incs());
}

TEST_P(PluginMetadataTest, newMetadataShouldOutputMessagesThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  Message message1(MessageType::say, "content1");
  Message message2(MessageType::say, "content2");
  Message message3(MessageType::say, "content3");

  plugin1.Messages({message1, message2});
  plugin2.Messages({message1, message3});
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(std::list<Message>({message2}), newMetadata.Messages());
}

TEST_P(PluginMetadataTest, newMetadataShouldOutputTagsThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  Tag tag1("Relev");
  Tag tag2("Relev", false);
  Tag tag3("Delev");

  plugin1.Tags({tag1, tag2});
  plugin2.Tags({tag1, tag3});
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(std::set<Tag>({tag2}), newMetadata.Tags());
}

TEST_P(PluginMetadataTest, newMetadataShouldOutputDirtyInfoObjectsThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  PluginCleaningData info1(0x5, "utility", info_, 1, 2, 3);
  PluginCleaningData info2(0xA, "utility", info_, 1, 2, 3);
  PluginCleaningData info3(0x1, "utility", info_, 1, 2, 3);

  plugin1.DirtyInfo({info1, info2});
  plugin2.DirtyInfo({info1, info3});
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(std::set<PluginCleaningData>({info2}), newMetadata.DirtyInfo());
}

TEST_P(PluginMetadataTest, newMetadataShouldOutputCleanInfoObjectsThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  PluginCleaningData info1(0x5, "utility");
  PluginCleaningData info2(0xA, "utility");
  PluginCleaningData info3(0x1, "utility");

  plugin1.CleanInfo({info1, info2});
  plugin2.CleanInfo({info1, info3});
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(std::set<PluginCleaningData>({info2}), newMetadata.CleanInfo());
}

TEST_P(PluginMetadataTest, newMetadataShouldOutputLocationsThatAreNotCommonToBothInputPlugins) {
  PluginMetadata plugin1;
  PluginMetadata plugin2;
  Location location1("http://www.example.com/1");
  Location location2("http://www.example.com/2");
  Location location3("http://www.example.com/3");

  plugin1.Locations({location1, location2});
  plugin2.Locations({location1, location3});
  PluginMetadata newMetadata = plugin1.NewMetadata(plugin2);

  EXPECT_EQ(std::set<Location>({location2}), newMetadata.Locations());
}

TEST_P(PluginMetadataTest, simpleMessagesShouldReturnMessagesAsSimpleMessages) {
  PluginMetadata plugin;
  plugin.Messages({
    Message(MessageType::say, "content1"),
    Message(MessageType::warn, {{"content2",LanguageCode::french}, {"other content2", LanguageCode::english}}),
    Message(MessageType::error, "content3"),
  });

  auto simpleMessages = plugin.SimpleMessages(LanguageCode::french);

  EXPECT_EQ(3, simpleMessages.size());
  EXPECT_EQ(MessageType::say, simpleMessages.front().type);
  EXPECT_EQ(LanguageCode::english, simpleMessages.front().language);
  EXPECT_EQ("content1", simpleMessages.front().text);
  EXPECT_EQ(MessageType::warn, (++simpleMessages.begin())->type);
  EXPECT_EQ(LanguageCode::french, (++simpleMessages.begin())->language);
  EXPECT_EQ("content2", (++simpleMessages.begin())->text);
  EXPECT_EQ(MessageType::error, simpleMessages.back().type);
  EXPECT_EQ(LanguageCode::english, simpleMessages.back().language);
  EXPECT_EQ("content3", simpleMessages.back().text);
}

TEST_P(PluginMetadataTest, evalAllConditionsShouldEvaluateAllMetadataConditions) {
  Game game(GetParam());
  game.SetGamePath(dataPath.parent_path());

  PluginMetadata plugin(blankEsm);

  File file1(blankEsp);
  File file2(blankDifferentEsm, "", "file(\"" + missingEsp + "\")");
  plugin.LoadAfter({file1, file2});
  plugin.Reqs({file1, file2});
  plugin.Incs({file1, file2});

  Message message1(MessageType::say, "content");
  Message message2(MessageType::say, "content", "file(\"" + missingEsp + "\")");
  plugin.Messages({message1, message2});

  Tag tag1("Relev");
  Tag tag2("Relev", true, "file(\"" + missingEsp + "\")");
  plugin.Tags({tag1, tag2});

  PluginCleaningData info1(blankEsmCrc, "utility", info_, 1, 2, 3);
  PluginCleaningData info2(0xDEADBEEF, "utility", info_, 1, 2, 3);
  plugin.DirtyInfo({info1, info2});
  plugin.CleanInfo({info1, info2});

  EXPECT_NO_THROW(plugin.EvalAllConditions(game));

  std::set<File> expectedFiles({file1});
  EXPECT_EQ(expectedFiles, plugin.LoadAfter());
  EXPECT_EQ(expectedFiles, plugin.Reqs());
  EXPECT_EQ(expectedFiles, plugin.Incs());
  EXPECT_EQ(std::list<Message>({message1}), plugin.Messages());
  EXPECT_EQ(std::set<Tag>({tag1}), plugin.Tags());
  EXPECT_EQ(std::set<PluginCleaningData>({info1}), plugin.DirtyInfo());
  EXPECT_EQ(std::set<PluginCleaningData>({info1}), plugin.CleanInfo());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeTrueForADefaultConstructedPluginMetadataObject) {
  PluginMetadata plugin;

  EXPECT_TRUE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeTrueForAPluginMetadataObjectConstructedWithAName) {
  PluginMetadata plugin(blankEsp);

  EXPECT_TRUE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeTrueIfThePluginMetadataIsDisabled) {
  PluginMetadata plugin(blankEsp);
  plugin.Enabled(false);

  EXPECT_TRUE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfTheLocalPriorityIsExplicit) {
  PluginMetadata plugin(blankEsp);
  plugin.LocalPriority(Priority(0));

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfTheGlobalPriorityIsExplicit) {
  PluginMetadata plugin(blankEsp);
  plugin.GlobalPriority(Priority(0));

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfLoadAfterMetadataExists) {
  PluginMetadata plugin(blankEsp);
  plugin.LoadAfter({File(blankEsm)});

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfRequirementMetadataExists) {
  PluginMetadata plugin(blankEsp);
  plugin.Reqs({File(blankEsm)});

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfIncompatibilityMetadataExists) {
  PluginMetadata plugin(blankEsp);
  plugin.Incs({File(blankEsm)});

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfMessagesExist) {
  PluginMetadata plugin(blankEsp);
  plugin.Messages({Message(MessageType::say, "content")});

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfTagsExist) {
  PluginMetadata plugin(blankEsp);
  plugin.Tags({Tag("Relev")});

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfDirtyInfoExists) {
  PluginMetadata plugin(blankEsp);
  plugin.DirtyInfo({PluginCleaningData(5, "utility", info_, 0, 1, 2)});

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfCleanInfoExists) {
  PluginMetadata plugin(blankEsp);
  plugin.CleanInfo({PluginCleaningData(5, "utility")});

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, hasNameOnlyShouldBeFalseIfLocationsExist) {
  PluginMetadata plugin(blankEsp);
  plugin.Locations({Location("http://www.example.com")});

  EXPECT_FALSE(plugin.HasNameOnly());
}

TEST_P(PluginMetadataTest, isRegexPluginShouldBeFalseForAnEmptyPluginName) {
  PluginMetadata plugin;

  EXPECT_FALSE(plugin.IsRegexPlugin());
}

TEST_P(PluginMetadataTest, isRegexPluginShouldBeFalseForAnExactPluginFilename) {
  PluginMetadata plugin(blankEsm);

  EXPECT_FALSE(plugin.IsRegexPlugin());
}

TEST_P(PluginMetadataTest, isRegexPluginShouldBeTrueIfThePluginNameContainsAColon) {
  PluginMetadata plugin("Blank:.esm");

  EXPECT_TRUE(plugin.IsRegexPlugin());
}

TEST_P(PluginMetadataTest, isRegexPluginShouldBeTrueIfThePluginNameContainsABackslash) {
  PluginMetadata plugin("Blank\\.esm");

  EXPECT_TRUE(plugin.IsRegexPlugin());
}

TEST_P(PluginMetadataTest, isRegexPluginShouldBeTrueIfThePluginNameContainsAnAsterisk) {
  PluginMetadata plugin("Blank*.esm");

  EXPECT_TRUE(plugin.IsRegexPlugin());
}

TEST_P(PluginMetadataTest, isRegexPluginShouldBeTrueIfThePluginNameContainsAQuestionMark) {
  PluginMetadata plugin("Blank?.esm");

  EXPECT_TRUE(plugin.IsRegexPlugin());
}

TEST_P(PluginMetadataTest, isRegexPluginShouldBeTrueIfThePluginNameContainsAVerticalBar) {
  PluginMetadata plugin("Blank|.esm");

  EXPECT_TRUE(plugin.IsRegexPlugin());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithNoMetadataAsABlankString) {
  PluginMetadata plugin(blankEsm);
  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithAnExplicitLocalPriorityCorrectly) {
  PluginMetadata plugin(blankEsm);
  plugin.LocalPriority(Priority(0));

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esm'\n"
               "priority: 0", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithAnExplicitGlobalPriorityCorrectly) {
  PluginMetadata plugin(blankEsm);
  plugin.GlobalPriority(Priority(0));

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esm'\n"
               "global_priority: 0", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginThatIsDisabledAndIsNotNameOnlyCorrectly) {
  PluginMetadata plugin(blankEsm);
  plugin.GlobalPriority(Priority(0));
  plugin.Enabled(false);

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esm'\n"
               "enabled: false\n"
               "global_priority: 0", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginThatIsDisabledAndIsNameOnlyAsAnEmptyString) {
  PluginMetadata plugin(blankEsm);
  plugin.Enabled(false);

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithLoadAfterMetadataCorrectly) {
  PluginMetadata plugin(blankEsp);
  plugin.LoadAfter({File(blankEsm)});

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esp'\n"
               "after:\n"
               "  - 'Blank.esm'", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithRequirementsCorrectly) {
  PluginMetadata plugin(blankEsp);
  plugin.Reqs({File(blankEsm)});

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esp'\n"
               "req:\n"
               "  - 'Blank.esm'", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithIncompatibilitiesCorrectly) {
  PluginMetadata plugin(blankEsp);
  plugin.Incs({File(blankEsm)});

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esp'\n"
               "inc:\n"
               "  - 'Blank.esm'", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithMessagesCorrectly) {
  PluginMetadata plugin(blankEsp);
  plugin.Messages({Message(MessageType::say, "content")});

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esp'\n"
               "msg:\n"
               "  - type: say\n"
               "    content: 'content'", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithTagsCorrectly) {
  PluginMetadata plugin(blankEsp);
  plugin.Tags({Tag("Relev")});

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esp'\n"
               "tag:\n"
               "  - Relev", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithDirtyInfoCorrectly) {
  PluginMetadata plugin(blankEsp);
  plugin.DirtyInfo({PluginCleaningData(5, "utility", info_, 0, 1, 2)});

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esp'\n"
               "dirty:\n"
               "  - crc: 0x5\n"
               "    utility: 'utility'\n"
               "    info: 'info'\n"
               "    udr: 1\n"
               "    nav: 2", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithCleanInfoCorrectly) {
  PluginMetadata plugin(blankEsp);
  plugin.CleanInfo({PluginCleaningData(5, "utility")});

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esp'\n"
               "clean:\n"
               "  - crc: 0x5\n"
               "    utility: 'utility'", emitter.c_str());
}

TEST_P(PluginMetadataTest, emittingAsYamlShouldOutputAPluginWithLocationsCorrectly) {
  PluginMetadata plugin(blankEsp);
  plugin.Locations({Location("http://www.example.com")});

  YAML::Emitter emitter;
  emitter << plugin;

  EXPECT_STREQ("name: 'Blank.esp'\n"
               "url:\n"
               "  - 'http://www.example.com'", emitter.c_str());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldOmitAllUnsetFields) {
  PluginMetadata plugin(blankEsp);
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(plugin.Name(), node["name"].as<std::string>());
  EXPECT_FALSE(node["enabled"]);
  EXPECT_FALSE(node["priority"]);
  EXPECT_FALSE(node["after"]);
  EXPECT_FALSE(node["req"]);
  EXPECT_FALSE(node["inc"]);
  EXPECT_FALSE(node["msg"]);
  EXPECT_FALSE(node["tag"]);
  EXPECT_FALSE(node["dirty"]);
  EXPECT_FALSE(node["clean"]);
  EXPECT_FALSE(node["url"]);
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetPriorityFieldIfLocalPriorityIsExplicit) {
  PluginMetadata plugin(blankEsp);
  plugin.LocalPriority(Priority(0));
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(0, node["priority"].as<short>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetGlobalPriorityFieldIfGlobalPriorityIsExplicit) {
  PluginMetadata plugin(blankEsp);
  plugin.GlobalPriority(Priority(0));
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(0, node["global_priority"].as<short>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldNotSetPriorityFieldIfLocalPriorityIsImplicit) {
  PluginMetadata plugin(blankEsp);
  YAML::Node node;
  node = plugin;

  EXPECT_FALSE(node["priority"]);
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldNotSetPriorityFieldIfGlobalPriorityIsImplicit) {
  PluginMetadata plugin(blankEsp);
  YAML::Node node;
  node = plugin;

  EXPECT_FALSE(node["global_priority"]);
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetEnabledFieldIfItIsFalse) {
  PluginMetadata plugin(blankEsp);
  plugin.Enabled(false);
  YAML::Node node;
  node = plugin;

  EXPECT_FALSE(node["enabled"].as<bool>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetAfterFieldIfLoadAfterMetadataExists) {
  PluginMetadata plugin(blankEsp);
  plugin.LoadAfter({File(blankEsm)});
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(plugin.LoadAfter(), node["after"].as<std::set<File>>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetReqFieldIfRequirementsExist) {
  PluginMetadata plugin(blankEsp);
  plugin.Reqs({File(blankEsm)});
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(plugin.Reqs(), node["req"].as<std::set<File>>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetIncFieldIfIncompatibilitiesExist) {
  PluginMetadata plugin(blankEsp);
  plugin.Incs({File(blankEsm)});
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(plugin.Incs(), node["inc"].as<std::set<File>>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetMsgFieldIfMessagesExist) {
  PluginMetadata plugin(blankEsp);
  plugin.Messages({Message(MessageType::say, "content")});
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(plugin.Messages(), node["msg"].as<std::list<Message>>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetTagFieldIfTagsExist) {
  PluginMetadata plugin(blankEsp);
  plugin.Tags({Tag("Relev")});
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(plugin.Tags(), node["tag"].as<std::set<Tag>>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetDirtyFieldIfDirtyInfoExists) {
  PluginMetadata plugin(blankEsp);
  plugin.DirtyInfo({PluginCleaningData(5, "utility", info_, 0, 1, 2)});
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(plugin.DirtyInfo(), node["dirty"].as<std::set<PluginCleaningData>>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetCleanFieldIfCleanInfoExists) {
  PluginMetadata plugin(blankEsp);
  plugin.CleanInfo({PluginCleaningData(5, "utility")});
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(plugin.CleanInfo(), node["clean"].as<std::set<PluginCleaningData>>());
}

TEST_P(PluginMetadataTest, encodingAsYamlShouldSetUrlFieldIfLocationsExist) {
  PluginMetadata plugin(blankEsp);
  plugin.Locations({Location("http://www.example.com")});
  YAML::Node node;
  node = plugin;

  EXPECT_EQ(plugin.Locations(), node["url"].as<std::set<Location>>());
}

TEST_P(PluginMetadataTest, decodingFromYamlShouldSetDefaultPriorityValuesIfNoneAreSpecified) {
  YAML::Node node = YAML::Load("name: " + blankEsp);
  PluginMetadata plugin = node.as<PluginMetadata>();

  EXPECT_EQ(blankEsp, plugin.Name());
  EXPECT_EQ(0, plugin.LocalPriority().getValue());
  EXPECT_FALSE(plugin.LocalPriority().isExplicit());
  EXPECT_EQ(0, plugin.GlobalPriority().getValue());
  EXPECT_FALSE(plugin.GlobalPriority().isExplicit());
}

TEST_P(PluginMetadataTest, decodingFromYamlShouldStoreAllGivenData) {
  YAML::Node node = YAML::Load("name: 'Blank.esp'\n"
                               "enabled: false\n"
                               "priority: 5\n"
                               "global_priority: 3\n"
                               "after:\n"
                               "  - 'Blank.esm'\n"
                               "req:\n"
                               "  - 'Blank.esm'\n"
                               "inc:\n"
                               "  - 'Blank.esm'\n"
                               "msg:\n"
                               "  - type: say\n"
                               "    content: 'content'\n"
                               "tag:\n"
                               "  - Relev\n"
                               "dirty:\n"
                               "  - crc: 0x5\n"
                               "    utility: 'utility'\n"
                               "    udr: 1\n"
                               "    nav: 2\n"
                               "clean:\n"
                               "  - crc: 0x6\n"
                               "    utility: 'utility'\n"
                               "url:\n"
                               "  - 'http://www.example.com'");
  PluginMetadata plugin = node.as<PluginMetadata>();

  EXPECT_EQ("Blank.esp", plugin.Name());
  EXPECT_EQ(5, plugin.LocalPriority().getValue());
  EXPECT_EQ(3, plugin.GlobalPriority().getValue());
  EXPECT_EQ(std::set<File>({
      File("Blank.esm")
  }), plugin.LoadAfter());
  EXPECT_EQ(std::set<File>({
      File("Blank.esm")
  }), plugin.Reqs());
  EXPECT_EQ(std::set<File>({
      File("Blank.esm")
  }), plugin.Incs());
  EXPECT_EQ(std::list<Message>({
      Message(MessageType::say, "content")
  }), plugin.Messages());
  EXPECT_EQ(std::set<Tag>({
      Tag("Relev")
  }), plugin.Tags());
  EXPECT_EQ(std::set<PluginCleaningData>({
      PluginCleaningData(5, "utility", info_, 0, 1, 2)
  }), plugin.DirtyInfo());
  EXPECT_EQ(std::set<PluginCleaningData>({
    PluginCleaningData(6, "utility")
  }), plugin.CleanInfo());
  EXPECT_EQ(std::set<Location>({
      Location("http://www.example.com")
  }), plugin.Locations());
}

TEST_P(PluginMetadataTest, decodingFromYamlWithDirtyInfoInARegexPluginMetadataObjectShouldThrow) {
  YAML::Node node = YAML::Load("name: 'Blank\\.esp'\n"
                               "dirty:\n"
                               "  - crc: 0x5\n"
                               "    utility: 'utility'\n"
                               "    udr: 1\n"
                               "    nav: 2");

  EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);
}

TEST_P(PluginMetadataTest, decodingFromYamlWithCleanInfoInARegexPluginMetadataObjectShouldThrow) {
  YAML::Node node = YAML::Load("name: 'Blank\\.esp'\n"
                               "clean:\n"
                               "  - crc: 0x5\n"
                               "    utility: 'utility'");

  EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);
}

TEST_P(PluginMetadataTest, decodingFromYamlWithAnInvalidRegexNameShouldThrow) {
  YAML::Node node = YAML::Load("name: 'RagnvaldBook(Farengar(+Ragnvald)?)?\\.esp'\n"
                               "dirty:\n"
                               "  - crc: 0x5\n"
                               "    utility: 'utility'\n"
                               "    udr: 1\n"
                               "    nav: 2");

  EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);
}

TEST_P(PluginMetadataTest, decodingFromAYamlScalarShouldThrow) {
  YAML::Node node = YAML::Load("scalar");

  EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);
}

TEST_P(PluginMetadataTest, decodingFromAYamlListShouldThrow) {
  YAML::Node node = YAML::Load("[0, 1, 2]");

  EXPECT_THROW(node.as<PluginMetadata>(), YAML::RepresentationException);
}
}
}

#endif
