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
    db_(nullptr),
    userlistPath_(localPath / "userlist.yaml"),
    url_("https://github.com/loot/testing-metadata.git"),
    branch_("master"),
    minimalOutputPath_(localPath / "minimal.yml"),
    generalUserlistMessage("A general userlist message.") {}

  void SetUp() {
    ApiGameOperationsTest::SetUp();

    db_ = handle_->GetDatabase();

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
      << "        util: 'TES4Edit'" << endl
      << "        udr: 4";

    return expectedContent.str();
  }

  std::string GetFileContent(const boost::filesystem::path& file) {
    boost::filesystem::ifstream stream(file);
    std::stringstream content;
    content << stream.rdbuf();

    return content.str();
  }

  void GenerateUserlist() {
    using std::endl;

    boost::filesystem::ofstream userlist(userlistPath_);
    userlist
      << "bash_tags:" << endl
      << "  - RaceRelations" << endl
      << "  - C.Lighting" << endl
      << "globals:" << endl
      << "  - type: say" << endl
      << "    content: '" << generalUserlistMessage << "'" << endl
      << "plugins:" << endl
      << "  - name: " << blankEsm << endl
      << "    after:" << endl
      << "      - " << blankDifferentEsm << endl
      << "  - name: " << blankDifferentEsp << endl
      << "    inc:" << endl
      << "      - " << blankEsp << endl;

    userlist.close();
  }

  const boost::filesystem::path userlistPath_;
  const boost::filesystem::path minimalOutputPath_;
  const std::string url_;
  const std::string branch_;
  const std::string generalUserlistMessage;

  std::shared_ptr<DatabaseInterface> db_;
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
  EXPECT_THROW(db_->LoadLists(masterlistPath.string(), ""), FileAccessError);
}

TEST_P(DatabaseInterfaceTest, loadListsShouldThrowIfAMasterlistIsPresentButAUserlistDoesNotExistAtTheGivenPath) {
  ASSERT_NO_THROW(GenerateMasterlist());
  EXPECT_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()), FileAccessError);
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

TEST_P(DatabaseInterfaceTest, writeUserMetadataShouldThrowIfTheFileAlreadyExistsAndTheOverwriteArgumentIsFalse) {
  ASSERT_NO_THROW(db_->WriteUserMetadata(minimalOutputPath_.string(), false));
  ASSERT_TRUE(boost::filesystem::exists(minimalOutputPath_));

  EXPECT_THROW(db_->WriteUserMetadata(minimalOutputPath_.string(), false), FileAccessError);
}

TEST_P(DatabaseInterfaceTest, writeUserMetadataShouldReturnOkAndWriteToFileIfTheArgumentsAreValidAndTheOverwriteArgumentIsTrue) {
  EXPECT_NO_THROW(db_->WriteUserMetadata(minimalOutputPath_.string(), true));
  EXPECT_TRUE(boost::filesystem::exists(minimalOutputPath_));
}

TEST_P(DatabaseInterfaceTest, writeUserMetadataShouldReturnOkIfTheFileAlreadyExistsAndTheOverwriteArgumentIsTrue) {
  ASSERT_NO_THROW(db_->WriteUserMetadata(minimalOutputPath_.string(), false));
  ASSERT_TRUE(boost::filesystem::exists(minimalOutputPath_));

  EXPECT_NO_THROW(db_->WriteUserMetadata(minimalOutputPath_.string(), true));
}

TEST_P(DatabaseInterfaceTest, writeUserMetadataShouldThrowIfPathGivenExistsAndIsReadOnly) {
  ASSERT_NO_THROW(db_->WriteUserMetadata(minimalOutputPath_.string(), false));
  ASSERT_TRUE(boost::filesystem::exists(minimalOutputPath_));

  boost::filesystem::permissions(minimalOutputPath_,
                                 boost::filesystem::perms::remove_perms
                                 | boost::filesystem::perms::owner_write);

  EXPECT_THROW(db_->WriteUserMetadata(minimalOutputPath_.string(), true), FileAccessError);
}

TEST_P(DatabaseInterfaceTest, writeUserMetadataShouldShouldNotWriteMasterlistMetadata) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  EXPECT_NO_THROW(db_->WriteUserMetadata(minimalOutputPath_.string(), true));

  std::string expectedContent = "bash_tags:\n  []\nglobals:\n  []\nplugins:\n  []";

  EXPECT_EQ(expectedContent, GetFileContent(minimalOutputPath_));
}

TEST_P(DatabaseInterfaceTest, writeUserMetadataShouldShouldWriteUserMetadata) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(boost::filesystem::copy(masterlistPath, userlistPath_));

  boost::filesystem::ofstream masterlist(masterlistPath);
  masterlist << "bash_tags:\n  []\nglobals:\n  []\nplugins:\n  []";
  masterlist.close();

  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  EXPECT_NO_THROW(db_->WriteUserMetadata(minimalOutputPath_.string(), true));

  EXPECT_FALSE(GetFileContent(minimalOutputPath_).empty());
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldThrowIfTheMasterlistPathGivenIsInvalid) {
  EXPECT_THROW(db_->UpdateMasterlist(";//\?", url_, branch_), std::invalid_argument);
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldThrowIfTheMasterlistPathGivenIsEmpty) {
  EXPECT_THROW(db_->UpdateMasterlist("", url_, branch_), std::invalid_argument);
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldThrowIfTheRepositoryUrlGivenCannotBeFound) {
  EXPECT_THROW(db_->UpdateMasterlist(masterlistPath.string(), "https://github.com/loot/oblivion-does-not-exist.git", branch_), std::system_error);
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldThrowIfTheRepositoryUrlGivenIsEmpty) {
  EXPECT_THROW(db_->UpdateMasterlist(masterlistPath.string(), "", branch_), std::invalid_argument);
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldThrowIfTheRepositoryBranchGivenCannotBeFound) {
  EXPECT_THROW(db_->UpdateMasterlist(masterlistPath.string(), url_, "missing-branch"), std::system_error);
}

TEST_P(DatabaseInterfaceTest, updateMasterlistShouldThrowIfTheRepositoryBranchGivenIsEmpty) {
  EXPECT_THROW(db_->UpdateMasterlist(masterlistPath.string(), url_, ""), std::invalid_argument);
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

TEST_P(DatabaseInterfaceTest, getKnownBashTagsShouldReturnAllBashTagsListedInLoadedMetadata) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());

  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  auto tags = db_->GetKnownBashTags();

  std::set<std::string> expectedTags({
    "RaceRelations",
    "C.Lighting",
    "Actors.ACBS",
    "C.Climate",
  });
  EXPECT_EQ(expectedTags, tags);
}

TEST_P(DatabaseInterfaceTest, getGeneralMessagesShouldGetGeneralMessagesFromTheMasterlistAndUserlist) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  auto messages = db_->GetGeneralMessages();

  std::vector<Message> expectedMessages({
    Message(MessageType::say, generalMasterlistMessage),
    Message(MessageType::say, generalUserlistMessage),
  });
  EXPECT_EQ(expectedMessages, messages);
}

TEST_P(DatabaseInterfaceTest, getPluginMetadataShouldReturnAnEmptyPluginMetadataObjectIfThePluginHasNoMetadata) {
  auto metadata = db_->GetPluginMetadata(blankEsm);

  EXPECT_TRUE(metadata.HasNameOnly());
}

TEST_P(DatabaseInterfaceTest, getPluginMetadataShouldReturnMergedMasterAndUserMetadataForTheGivenPlugin) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  auto metadata = db_->GetPluginMetadata(blankEsm);

  std::set<File> expectedLoadAfter({
    File(masterFile),
    File(blankDifferentEsm),
  });
  EXPECT_EQ(expectedLoadAfter, metadata.LoadAfter());
}

TEST_P(DatabaseInterfaceTest, getPluginUserMetadataShouldReturnAnEmptyPluginMetadataObjectIfThePluginHasNoUserMetadata) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  auto metadata = db_->GetPluginUserMetadata(blankDifferentEsm);

  EXPECT_TRUE(metadata.HasNameOnly());
}

TEST_P(DatabaseInterfaceTest, getPluginUserMetadataShouldReturnOnlyUserMetadataForTheGivenPlugin) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  auto metadata = db_->GetPluginUserMetadata(blankEsm);

  std::set<File> expectedLoadAfter({
    File(blankDifferentEsm),
  });
  EXPECT_EQ(expectedLoadAfter, metadata.LoadAfter());
}

TEST_P(DatabaseInterfaceTest, setPluginUserMetadataShouldReplaceExistingUserMetadataWithTheGivenMetadata) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  PluginMetadata newMetadata(blankDifferentEsp);
  newMetadata.Reqs(std::set<File>({File(masterFile)}));

  db_->SetPluginUserMetadata(newMetadata);

  auto metadata = db_->GetPluginUserMetadata(blankDifferentEsp);

  std::set<File> expectedLoadAfter({
    File(blankDifferentEsm),
  });
  EXPECT_TRUE(metadata.Incs().empty());
  EXPECT_EQ(newMetadata.Reqs(), metadata.Reqs());
}

TEST_P(DatabaseInterfaceTest, setPluginUserMetadataShouldNotAffectExistingMasterlistMetadata) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  PluginMetadata newMetadata(blankEsm);
  newMetadata.Reqs(std::set<File>({File(masterFile)}));

  db_->SetPluginUserMetadata(newMetadata);

  auto metadata = db_->GetPluginMetadata(blankEsm);

  std::set<File> expectedLoadAfter({
    File(masterFile),
  });
  EXPECT_EQ(expectedLoadAfter, metadata.LoadAfter());
}

TEST_P(DatabaseInterfaceTest, discardPluginUserMetadataShouldDiscardAllUserMetadataForTheGivenPlugin) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  db_->DiscardPluginUserMetadata(blankEsm);

  auto metadata = db_->GetPluginUserMetadata(blankEsm);
  EXPECT_TRUE(metadata.HasNameOnly());

}

TEST_P(DatabaseInterfaceTest, discardPluginUserMetadataShouldNotDiscardMasterlistMetadataForTheGivenPlugin) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  db_->DiscardPluginUserMetadata(blankEsm);

  auto metadata = db_->GetPluginMetadata(blankEsm);

  std::set<File> expectedLoadAfter({
    File(masterFile),
  });
  EXPECT_EQ(expectedLoadAfter, metadata.LoadAfter());
}

TEST_P(DatabaseInterfaceTest, discardPluginUserMetadataShouldNotDiscardUserMetadataForOtherPlugins) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  db_->DiscardPluginUserMetadata(blankEsm);

  auto metadata = db_->GetPluginUserMetadata(blankDifferentEsp);

  EXPECT_FALSE(metadata.HasNameOnly());
}

TEST_P(DatabaseInterfaceTest, discardPluginUserMetadataShouldNotDiscardGeneralMessages) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  db_->DiscardPluginUserMetadata(blankEsm);

  auto messages = db_->GetGeneralMessages();

  std::vector<Message> expectedMessages({
    Message(MessageType::say, generalMasterlistMessage),
    Message(MessageType::say, generalUserlistMessage),
  });
  EXPECT_EQ(expectedMessages, messages);
}

TEST_P(DatabaseInterfaceTest, discardPluginUserMetadataShouldNotDiscardKnownBashTags) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  db_->DiscardPluginUserMetadata(blankEsm);

  auto tags = db_->GetKnownBashTags();

  std::set<std::string> expectedTags({
    "RaceRelations",
    "C.Lighting",
    "Actors.ACBS",
    "C.Climate",
  });
  EXPECT_EQ(expectedTags, tags);
}

TEST_P(DatabaseInterfaceTest, discardAllUserMetadataShouldDiscardAllUserMetadataAndNoMasterlistMetadata) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(GenerateUserlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), userlistPath_.string()));

  db_->DiscardAllUserMetadata();

  auto metadata = db_->GetPluginUserMetadata(blankEsm);
  EXPECT_TRUE(metadata.HasNameOnly());

  metadata = db_->GetPluginUserMetadata(blankDifferentEsp);
  EXPECT_TRUE(metadata.HasNameOnly());

  metadata = db_->GetPluginMetadata(blankEsm);

  std::set<File> expectedLoadAfter({
    File(masterFile),
  });
  EXPECT_EQ(expectedLoadAfter, metadata.LoadAfter());

  auto messages = db_->GetGeneralMessages();

  std::vector<Message> expectedMessages({
    Message(MessageType::say, generalMasterlistMessage),
  });
  EXPECT_EQ(expectedMessages, messages);

  auto tags = db_->GetKnownBashTags();

  std::set<std::string> expectedTags({
    "Actors.ACBS",
    "C.Climate",
  });
  EXPECT_EQ(expectedTags, tags);
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

TEST_P(DatabaseInterfaceTest, writeMinimalListShouldThrowIfTheFileAlreadyExistsAndTheOverwriteArgumentIsFalse) {
  ASSERT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), false));
  ASSERT_TRUE(boost::filesystem::exists(minimalOutputPath_));

  EXPECT_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), false), FileAccessError);
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

TEST_P(DatabaseInterfaceTest, writeMinimalListShouldThrowIfPathGivenExistsAndIsReadOnly) {
  ASSERT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), false));
  ASSERT_TRUE(boost::filesystem::exists(minimalOutputPath_));

  boost::filesystem::permissions(minimalOutputPath_,
                                 boost::filesystem::perms::remove_perms
                                 | boost::filesystem::perms::owner_write);

  EXPECT_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), true), FileAccessError);
}

TEST_P(DatabaseInterfaceTest, writeMinimalListShouldWriteOnlyBashTagsAndDirtyInfo) {
  ASSERT_NO_THROW(GenerateMasterlist());
  ASSERT_NO_THROW(db_->LoadLists(masterlistPath.string(), ""));

  EXPECT_NO_THROW(db_->WriteMinimalList(minimalOutputPath_.string(), true));

  EXPECT_EQ(GetExpectedMinimalContent(), GetFileContent(minimalOutputPath_));
}
}
}

#endif
