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

#ifndef LOOT_TESTS_API_DATABASE_INTERFACE_TEST
#define LOOT_TESTS_API_DATABASE_INTERFACE_TEST

#include "loot/api.h"

#include "tests/api/api_game_operations_test.h"

namespace loot {
namespace test {
class DatabaseInterfaceTest : public ApiGameOperationsTest {
protected:
  DatabaseInterfaceTest() :
    userlistPath_(localPath / "userlist.yaml"),
    url_("https://github.com/loot/testing-metadata.git"),
    branch_("2.x"),
    minimalOutputPath_(localPath / "minimal.yml") {}

  void SetUp() {
    ApiGameOperationsTest::SetUp();

    ASSERT_FALSE(boost::filesystem::exists(minimalOutputPath_));
  }

  void TearDown() {
    if (boost::filesystem::exists(minimalOutputPath_)) {
      boost::filesystem::permissions(minimalOutputPath_,
                                     boost::filesystem::perms::add_perms
                                     | boost::filesystem::perms::owner_write);
    }

    ApiGameOperationsTest::TearDown();

    // The userlist may have been created during the test, so delete it.
    ASSERT_NO_THROW(boost::filesystem::remove(userlistPath_));

    // Also remove the ".git" folder if it has been created.
    ASSERT_NO_THROW(boost::filesystem::remove_all(masterlistPath.parent_path() / ".git"));

    ASSERT_NO_THROW(boost::filesystem::remove(minimalOutputPath_));
  }

  std::string GetExpectedMinimalContent() const {
    using std::endl;

    std::stringstream expectedContent;
    expectedContent
      << "plugins:" << endl
      << "  - name: '" << blankEsm << "'" << endl
      << "    tag:" << endl
      << "      - Actors.ACBS" << endl
      << "      - Actors.AIData" << endl
      << "      - -C.Water" << endl
      << "  - name: '" << blankDifferentEsm << "'" << endl
      << "    dirty:" << endl
      << "      - crc: 0x7d22f9df" << endl
      << "        utility: 'TES4Edit'" << endl
      << "        udr: 4";

    return expectedContent.str();
  }

  const boost::filesystem::path userlistPath_;
  const boost::filesystem::path minimalOutputPath_;
  const std::string url_;
  const std::string branch_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        DatabaseInterfaceTest,
                        ::testing::Values(
                          GameType::tes4,
                          GameType::tes5,
                          GameType::fo3,
                          GameType::fonv,
                          GameType::fo4,
                          GameType::tes5se));

TEST_P(DatabaseInterfaceTest, loadListsShouldThrowIfNoMasterlistIsPresent) {
  EXPECT_ANY_THROW(db_->LoadLists(masterlistPath.string(), ""));
}

TEST_P(DatabaseInterfaceTest, loadListsShouldThrowIfAMasterlistIsPresentButAUserlistDoesNotExistAtTheGivenPath) {
  ASSERT_NO_THROW(GenerateMasterlist());
  EXPECT_ANY_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));
}

TEST_P(DatabaseInterfaceTest, loadListsShouldSucceedIfTheMasterlistIsPresentAndTheUserlistPathIsAnEmptyString) {
  ASSERT_NO_THROW(GenerateMasterlist());

  EXPECT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));
}

TEST_P(DatabaseInterfaceTest, loadListsShouldSucceedIfTheMasterlistAndUserlistAreBothPresent) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(boost::filesystem::copy(masterlistPath, userlistPath_));

  EXPECT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));
}

TEST_P(DatabaseInterfaceTest, evalListsShouldReturnOkWithNoListsLoaded) {
  EXPECT_NO_THROW(db_->EvalLists());
}

TEST_P(DatabaseInterfaceTest, evalListsShouldReturnOKForAllLanguagesWithAMasterlistLoaded) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  EXPECT_NO_THROW(db_->EvalLists());
}

TEST_P(DatabaseInterfaceTest, sortPluginsShouldSucceedIfPassedValidArguments) {
  std::vector<std::string> expectedOrder = {
    masterFile,
    blankEsm,
    blankMasterDependentEsm,
    blankDifferentEsm,
    blankDifferentMasterDependentEsm,
    blankMasterDependentEsp,
    blankDifferentMasterDependentEsp,
    blankEsp,
    blankPluginDependentEsp,
    blankDifferentEsp,
    blankDifferentPluginDependentEsp,
  };

  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  std::vector<std::string> actualOrder = db_->SortPlugins({
    blankEsp,
    blankPluginDependentEsp,
    blankDifferentMasterDependentEsm,
    blankMasterDependentEsp,
    blankDifferentMasterDependentEsp,
    blankDifferentEsp,
    blankDifferentPluginDependentEsp,
    masterFile,
    blankEsm,
    blankMasterDependentEsm,
    blankDifferentEsm,
  });

  ASSERT_EQ(expectedOrder, actualOrder);
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldReturnAnInvalidArgsErrorIfTheMasterlistPathGivenIsInvalid) {
  EXPECT_ANY_THROW(db_->UpdateMasterlist(";//\?", url_, branch_));
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldReturnAnInvalidArgsErrorIfTheMasterlistPathGivenIsEmpty) {
  EXPECT_ANY_THROW(db_->UpdateMasterlist("", url_, branch_));
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldReturnAGitErrorIfTheRepositoryUrlGivenCannotBeFound) {
  EXPECT_ANY_THROW(db_->UpdateMasterlist(masterlistPath.string(), "https://github.com/loot/oblivion-does-not-exist.git", branch_));
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldReturnAnInvalidArgsErrorIfTheRepositoryUrlGivenIsEmpty) {
  EXPECT_ANY_THROW(db_->UpdateMasterlist(masterlistPath.string(), "", branch_));
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldReturnAGitErrorIfTheRepositoryBranchGivenCannotBeFound) {
  EXPECT_ANY_THROW(db_->UpdateMasterlist(masterlistPath.string(), url_, "missing-branch"));
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldReturnAnInvalidArgsErrorIfTheRepositoryBranchGivenIsEmpty) {
  EXPECT_ANY_THROW(db_->UpdateMasterlist(masterlistPath.string(), url_, ""));
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldSucceedIfPassedValidParametersAndOutputTrueIfTheMasterlistWasUpdated) {
  bool updated = false;
  EXPECT_NO_THROW(updated = db_->UpdateMasterlist(masterlistPath.string(), url_, branch_));
  EXPECT_TRUE(updated);
  EXPECT_TRUE(boost::filesystem::exists(masterlistPath));
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldSucceedIfCalledRepeatedlyButOnlyOutputTrueForTheFirstCall) {
  bool updated = false;
  EXPECT_NO_THROW(updated = db_->UpdateMasterlist(masterlistPath.string(), url_, branch_));
  EXPECT_TRUE(updated);

  EXPECT_NO_THROW(updated = db_->UpdateMasterlist(masterlistPath.string(), url_, branch_));
  EXPECT_FALSE(updated);
  EXPECT_TRUE(boost::filesystem::exists(masterlistPath));
}

TEST_P(DatabaseInterfaceTest, getMasterlistRevisionShouldThrowIfNoMasterlistIsPresent) {
  MasterlistInfo info;
  EXPECT_THROW(info = db_->GetMasterlistRevision(masterlistPath.string(), false), FileAccessError);
  EXPECT_TRUE(info.revision_id.empty());
  EXPECT_TRUE(info.revision_date.empty());
  EXPECT_FALSE(info.is_modified);
}

TEST_P(DatabaseInterfaceTest, getMasterlistRevisionShouldThrowIfANonVersionControlledMasterlistIsPresent) {
  ASSERT_NO_THROW(GenerateMasterlist());

  MasterlistInfo info;
  EXPECT_THROW(info = db_->GetMasterlistRevision(masterlistPath.string(), false), GitStateError);
  EXPECT_TRUE(info.revision_id.empty());
  EXPECT_TRUE(info.revision_date.empty());
  EXPECT_FALSE(info.is_modified);
}

TEST_P(DatabaseInterfaceTest, getMasterlistRevisionShouldOutputLongStringsAndBooleanFalseIfAVersionControlledMasterlistIsPresentAndGetShortIdParameterIsFalse) {
  ASSERT_NO_THROW(db_->UpdateMasterlist(masterlistPath.string(), url_, branch_));

  MasterlistInfo info;
  EXPECT_NO_THROW(info = db_->GetMasterlistRevision(masterlistPath.string(), false));
  EXPECT_EQ(40, info.revision_id.length());
  EXPECT_EQ(10, info.revision_date.length());
  EXPECT_FALSE(info.is_modified);
}

TEST_P(DatabaseInterfaceTest, getMasterlistRevisionShouldOutputShortStringsAndBooleanFalseIfAVersionControlledMasterlistIsPresentAndGetShortIdParameterIsTrue) {
  ASSERT_NO_THROW(db_->UpdateMasterlist(masterlistPath.string(), url_, branch_));

  MasterlistInfo info;
  EXPECT_NO_THROW(info = db_->GetMasterlistRevision(masterlistPath.string(), false));
  EXPECT_GE(size_t(40), info.revision_id.length());
  EXPECT_LE(size_t(7), info.revision_id.length());
  EXPECT_EQ(10, info.revision_date.length());
  EXPECT_FALSE(info.is_modified);
}

TEST_P(DatabaseInterfaceTest, getMasterlistRevisionShouldSucceedIfAnEditedVersionControlledMasterlistIsPresent) {
  ASSERT_NO_THROW(db_->UpdateMasterlist(masterlistPath.string(), url_, branch_));
  ASSERT_NO_THROW(GenerateMasterlist());

  MasterlistInfo info;
  EXPECT_NO_THROW(info = db_->GetMasterlistRevision(masterlistPath.string(), false));
  EXPECT_EQ(40, info.revision_id.length());
  EXPECT_EQ(10, info.revision_date.length());
  EXPECT_TRUE(info.is_modified);
}

TEST_P(DatabaseInterfaceTest, getPluginTagsShouldReturnOkAndOutputEmptyNonModifiedArraysIfAPluginWithoutTagsIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  PluginTags tags;
  EXPECT_NO_THROW(tags = db_->GetPluginTags(blankEsp));

  EXPECT_TRUE(tags.added.empty());
  EXPECT_TRUE(tags.removed.empty());
  EXPECT_FALSE(tags.userlist_modified);
}

TEST_P(DatabaseInterfaceTest, getPluginTagsShouldReturnOkAndNonEmptyNonModifiedArraysIfAPluginWithTagsIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  PluginTags tags;
  EXPECT_NO_THROW(tags = db_->GetPluginTags(blankEsm));

  EXPECT_EQ(std::set<std::string>({
    "Actors.ACBS",
    "Actors.AIData",
  }), tags.added);
  EXPECT_EQ(std::set<std::string>({
    "C.Water",
  }), tags.removed);
  EXPECT_FALSE(tags.userlist_modified);
}

TEST_P(DatabaseInterfaceTest, getPluginTagsShouldReturnOkAndNonEmptyModifiedArraysIfAPluginWithTagsIsQueriedAndMetadataWasAlsoLoadedFromAUserlist) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), masterlistPath.string()));

  PluginTags tags;
  EXPECT_NO_THROW(tags = db_->GetPluginTags(blankEsm));

  EXPECT_EQ(std::set<std::string>({
    "Actors.ACBS",
    "Actors.AIData",
  }), tags.added);
  EXPECT_EQ(std::set<std::string>({
    "C.Water",
  }), tags.removed);
  EXPECT_TRUE(tags.userlist_modified);
}

TEST_P(DatabaseInterfaceTest, getPluginTagsShouldOutputTheCorrectBashTagsForPluginsWhenMakingConsecutiveCalls) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  PluginTags tags;
  EXPECT_NO_THROW(tags = db_->GetPluginTags(blankEsm));

  EXPECT_EQ(std::set<std::string>({
    "Actors.ACBS",
    "Actors.AIData",
  }), tags.added);
  EXPECT_EQ(std::set<std::string>({
    "C.Water",
  }), tags.removed);
  EXPECT_FALSE(tags.userlist_modified);

  EXPECT_NO_THROW(tags = db_->GetPluginTags(blankEsp));

  EXPECT_TRUE(tags.added.empty());
  EXPECT_TRUE(tags.removed.empty());
  EXPECT_FALSE(tags.userlist_modified);
}

TEST_P(DatabaseInterfaceTest, getPluginMessagesShouldReturnOkAndOutputANullArrayIfAPluginWithNoMessagesIsQueried) {
  std::vector<SimpleMessage> messages;
  EXPECT_NO_THROW(messages = db_->GetPluginMessages(blankEsp, LanguageCode::english));
  EXPECT_TRUE(messages.empty());
}

TEST_P(DatabaseInterfaceTest, getPluginMessagesShouldReturnOkAndOutputANoteIfAPluginWithANoteMessageIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  std::vector<SimpleMessage> messages;
  EXPECT_NO_THROW(messages = db_->GetPluginMessages(blankEsm, LanguageCode::english));
  ASSERT_EQ(1, messages.size());
  EXPECT_EQ(MessageType::say, messages[0].type);
  EXPECT_EQ(LanguageCode::english, messages[0].language);
  EXPECT_EQ(noteMessage, messages[0].text);
}

TEST_P(DatabaseInterfaceTest, getPluginMessagesShouldReturnOkAndOutputAWarningIfAPluginWithAWarningMessageIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  std::vector<SimpleMessage> messages;
  EXPECT_NO_THROW(messages = db_->GetPluginMessages(blankDifferentEsm, LanguageCode::english));
  ASSERT_EQ(1, messages.size());
  EXPECT_EQ(MessageType::warn, messages[0].type);
  EXPECT_EQ(LanguageCode::english, messages[0].language);
  EXPECT_EQ(warningMessage, messages[0].text);
}

TEST_P(DatabaseInterfaceTest, getPluginMessagesShouldReturnOkAndOutputAnErrorIfAPluginWithAnErrorMessageIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  std::vector<SimpleMessage> messages;
  EXPECT_NO_THROW(messages = db_->GetPluginMessages(blankDifferentEsp, LanguageCode::english));
  ASSERT_EQ(1, messages.size());
  EXPECT_EQ(MessageType::error, messages[0].type);
  EXPECT_EQ(LanguageCode::english, messages[0].language);
  EXPECT_EQ(errorMessage, messages[0].text);
}

TEST_P(DatabaseInterfaceTest, getPluginMessagesShouldReturnOkAndOutputMultipleMessagesIfAPluginWithMultipleMessagesIsQueried) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  std::vector<SimpleMessage> messages;
  EXPECT_NO_THROW(messages = db_->GetPluginMessages(blankDifferentMasterDependentEsp, LanguageCode::english));
  ASSERT_EQ(3, messages.size());
  EXPECT_EQ(MessageType::say, messages[0].type);
  EXPECT_EQ(LanguageCode::english, messages[0].language);
  EXPECT_EQ(noteMessage, messages[0].text);
  EXPECT_EQ(MessageType::warn, messages[1].type);
  EXPECT_EQ(LanguageCode::english, messages[1].language);
  EXPECT_EQ(warningMessage, messages[1].text);
  EXPECT_EQ(MessageType::error, messages[2].type);
  EXPECT_EQ(LanguageCode::english, messages[2].language);
  EXPECT_EQ(errorMessage, messages[2].text);
}

TEST_P(DatabaseInterfaceTest, getPluginCleanlinessShouldReturnOkAndOutputUnknownForAPluginWithNoDirtyInfo) {
  PluginCleanliness cleanliness;
  EXPECT_NO_THROW(cleanliness = db_->GetPluginCleanliness(blankEsp));
  EXPECT_EQ(PluginCleanliness::unknown, cleanliness);
}

TEST_P(DatabaseInterfaceTest, getPluginCleanlinessShouldReturnOkAndOutputYesForAPluginWithDirtyInfo) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  PluginCleanliness cleanliness;
  EXPECT_NO_THROW(cleanliness = db_->GetPluginCleanliness(blankDifferentEsm));
  EXPECT_EQ(PluginCleanliness::dirty, cleanliness);
}

TEST_P(DatabaseInterfaceTest, getPluginCleanlinessShouldReturnOkAndOutputNoForAPluginWithADoNotCleanMessage) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  PluginCleanliness cleanliness;
  EXPECT_NO_THROW(cleanliness = db_->GetPluginCleanliness(blankEsm));
  EXPECT_EQ(PluginCleanliness::do_not_clean, cleanliness);
}

TEST_P(DatabaseInterfaceTest, writeMinimalListShouldReturnOkAndWriteToFileIfArgumentsGivenAreValid) {
  EXPECT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), false));
  EXPECT_TRUE(boost::filesystem::exists(minimalOutputPath_));
}

TEST_P(DatabaseInterfaceTest, writeMinimalListShouldReturnAFileWriteErrorIfTheFileAlreadyExistsAndTheOverwriteArgumentIsFalse) {
  ASSERT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), false));
  ASSERT_TRUE(boost::filesystem::exists(minimalOutputPath_));

  EXPECT_ANY_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), false));
}

TEST_P(DatabaseInterfaceTest, writeMinimalListShouldReturnOkAndWriteToFileIfTheArgumentsAreValidAndTheOverwriteArgumentIsTrue) {
  EXPECT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), true));
  EXPECT_TRUE(boost::filesystem::exists(minimalOutputPath_));
}

TEST_P(DatabaseInterfaceTest, writeMinimalListShouldReturnOkIfTheFileAlreadyExistsAndTheOverwriteArgumentIsTrue) {
  ASSERT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), false));
  ASSERT_TRUE(boost::filesystem::exists(minimalOutputPath_));

  EXPECT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), true));
}

TEST_P(DatabaseInterfaceTest, writeMinimalListShouldReturnAFileWriteErrorIfPathGivenExistsAndIsReadOnly) {
  ASSERT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), false));
  ASSERT_TRUE(boost::filesystem::exists(minimalOutputPath_));

  boost::filesystem::permissions(minimalOutputPath_,
                                 boost::filesystem::perms::remove_perms
                                 | boost::filesystem::perms::owner_write);

  EXPECT_ANY_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), true));
}

TEST_P(DatabaseInterfaceTest, writeMinimalListShouldWriteOnlyBashTagsAndDirtyInfo) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  EXPECT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), true));

  boost::filesystem::ifstream in(minimalOutputPath_);
  std::stringstream content;
  content << in.rdbuf();

  EXPECT_EQ(GetExpectedMinimalContent(), content.str());
}
}
}

#endif
