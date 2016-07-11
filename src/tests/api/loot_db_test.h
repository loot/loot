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
<http://www.gnu.org/licenses/>.
*/

#ifndef LOOT_TESTS_API_LOOT_DB_TEST
#define LOOT_TESTS_API_LOOT_DB_TEST

#include "api/loot_db.h"

#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class loot_db_test :
  public ::testing::TestWithParam<unsigned int>,
  public CommonGameTestFixture {
protected:
  loot_db_test() :
    CommonGameTestFixture(GetParam()),
    db_(nullptr) {}

  virtual void SetUp() {
    setUp();

    db_ = new loot_db(static_cast<unsigned int>(GetParam()), dataPath.parent_path().string().c_str(), localPath.string().c_str());
  }

  inline virtual void TearDown() {
    tearDown();

    delete db_;
  }

  loot_db * db_;
};

// Pass an empty first argument, as it's a prefix for the test instantation,
// but we only have the one so no prefix is necessary.
INSTANTIATE_TEST_CASE_P(,
                        loot_db_test,
                        ::testing::Values(
                          static_cast<unsigned int>(GameType::tes4),
                          static_cast<unsigned int>(GameType::tes5),
                          static_cast<unsigned int>(GameType::fo3),
                          static_cast<unsigned int>(GameType::fonv),
                          static_cast<unsigned int>(GameType::fo4)));

TEST_P(loot_db_test, settingRevisionIdStringShouldCopyIt) {
  db_->setRevisionIdString("id");
  EXPECT_STREQ("id", db_->getRevisionIdString());
}

TEST_P(loot_db_test, settingRevisionDateStringShouldCopyIt) {
  db_->setRevisionDateString("date");
  EXPECT_STREQ("date", db_->getRevisionDateString());
}

TEST_P(loot_db_test, settingPluginNamesShouldCopyThem) {
  db_->setPluginNames(std::vector<PluginMetadata>({
      PluginMetadata("Blank.esm"),
      PluginMetadata("Blank.esp"),
  }));

  EXPECT_EQ(2, db_->getPluginNames().size());
  EXPECT_STREQ("Blank.esm", db_->getPluginNames()[0]);
  EXPECT_STREQ("Blank.esp", db_->getPluginNames()[1]);
}

TEST_P(loot_db_test, settingPluginNamesTwiceShouldOverwriteTheFirstDataSet) {
  db_->setPluginNames(std::vector<PluginMetadata>({
      PluginMetadata("Blank.esm"),
      PluginMetadata("Blank.esp"),
  }));
  db_->setPluginNames(std::vector<PluginMetadata>({
      PluginMetadata("Blank - Different.esm"),
      PluginMetadata("Blank - Different.esp"),
  }));

  EXPECT_EQ(2, db_->getPluginNames().size());
  EXPECT_STREQ("Blank - Different.esm", db_->getPluginNames()[0]);
  EXPECT_STREQ("Blank - Different.esp", db_->getPluginNames()[1]);
}

TEST_P(loot_db_test, addingNewBashTagsToTheMapShouldAppendThem) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });

  EXPECT_EQ(2, db_->getBashTagMap().size());
  EXPECT_STREQ("C.Climate", db_->getBashTagMap()[0]);
  EXPECT_STREQ("Relev", db_->getBashTagMap()[1]);
}

TEST_P(loot_db_test, addingAnExistingBashTagToTheMapShouldNotDuplicateIt) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
      "C.Climate",
  });

  EXPECT_EQ(2, db_->getBashTagMap().size());
  EXPECT_STREQ("C.Climate", db_->getBashTagMap()[0]);
  EXPECT_STREQ("Relev", db_->getBashTagMap()[1]);
}

TEST_P(loot_db_test, gettingABashTagsUidForATagThatIsNotInTheMapShouldThrow) {
  EXPECT_ANY_THROW(db_->getBashTagUid("Relev"));
}

TEST_P(loot_db_test, gettingABashTagsUidShouldReturnItsTagMapIndex) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });

  EXPECT_EQ(1, db_->getBashTagUid("Relev"));
}

TEST_P(loot_db_test, clearingAnEmptyBashTagMapShouldDoNothing) {
  EXPECT_NO_THROW(db_->clearBashTagMap());
}

TEST_P(loot_db_test, clearingABashTagMapShouldEmptyIt) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });

  db_->clearBashTagMap();
  EXPECT_TRUE(db_->getBashTagMap().empty());
}

TEST_P(loot_db_test, clearingABashTagMapShouldAffectExistingReferences) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });

  auto& bashTagMap = db_->getBashTagMap();
  db_->clearBashTagMap();
  EXPECT_TRUE(bashTagMap.empty());
}

TEST_P(loot_db_test, settingAddedTagsWithNoTagMapShouldThrow) {
  EXPECT_ANY_THROW(db_->setAddedTags({
      "Relev",
  }));
}

TEST_P(loot_db_test, gettingSetAddedTagsShouldReturnTheirUids) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });

  db_->setAddedTags({
      "Relev",
  });

  EXPECT_EQ(std::vector<unsigned int>({
      1,
  }), db_->getAddedTagIds());
}

TEST_P(loot_db_test, settingAddedTagsShouldReplaceExistingTags) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });

  db_->setAddedTags({
      "Relev",
  });

  db_->setAddedTags({
      "C.Climate",
  });

  EXPECT_EQ(std::vector<unsigned int>({
      0,
  }), db_->getAddedTagIds());
}

TEST_P(loot_db_test, settingRemovedTagsWithNoTagMapShouldThrow) {
  EXPECT_ANY_THROW(db_->setRemovedTags({
      "Relev",
  }));
}

TEST_P(loot_db_test, gettingSetRemovedTagsShouldReturnTheirUids) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });

  db_->setRemovedTags({
      "Relev",
  });

  EXPECT_EQ(std::vector<unsigned int>({
      1,
  }), db_->getRemovedTagIds());
}

TEST_P(loot_db_test, settingRemovedTagsShouldReplaceExistingTags) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });

  db_->setRemovedTags({
      "Relev",
  });

  db_->setRemovedTags({
      "C.Climate",
  });

  EXPECT_EQ(std::vector<unsigned int>({
      0,
  }), db_->getRemovedTagIds());
}

TEST_P(loot_db_test, settingPluginMessagesShouldCopyThem) {
  db_->setPluginMessages(std::list<Message>({
      Message(Message::Type::warn, "Test 1"),
      Message(Message::Type::error, "Test 2"),
  }));

  EXPECT_EQ(2, db_->getPluginMessages().size());
  EXPECT_EQ(static_cast<unsigned int>(Message::Type::warn), db_->getPluginMessages()[0].type);
  EXPECT_STREQ("Test 1", db_->getPluginMessages()[0].message);
  EXPECT_EQ(static_cast<unsigned int>(Message::Type::error), db_->getPluginMessages()[1].type);
  EXPECT_STREQ("Test 2", db_->getPluginMessages()[1].message);
}

TEST_P(loot_db_test, settingPluginMessagesTwiceShouldOverwriteTheFirstDataSet) {
  db_->setPluginMessages(std::list<Message>({
      Message(Message::Type::warn, "Test 1"),
      Message(Message::Type::error, "Test 2"),
  }));
  db_->setPluginMessages(std::list<Message>({
      Message(Message::Type::error, "Test 3"),
      Message(Message::Type::warn, "Test 4"),
      Message(Message::Type::say, "Test 5"),
  }));

  EXPECT_EQ(3, db_->getPluginMessages().size());
  EXPECT_EQ(static_cast<unsigned int>(Message::Type::error), db_->getPluginMessages()[0].type);
  EXPECT_STREQ("Test 3", db_->getPluginMessages()[0].message);
  EXPECT_EQ(static_cast<unsigned int>(Message::Type::warn), db_->getPluginMessages()[1].type);
  EXPECT_STREQ("Test 4", db_->getPluginMessages()[1].message);
  EXPECT_EQ(static_cast<unsigned int>(Message::Type::say), db_->getPluginMessages()[2].type);
  EXPECT_STREQ("Test 5", db_->getPluginMessages()[2].message);
}

TEST_P(loot_db_test, clearingArraysShouldEmptyPluginNamesTagIdsAndMessages) {
  db_->setPluginMessages(std::list<Message>({
      Message(Message::Type::warn, "Test 1"),
      Message(Message::Type::error, "Test 2"),
  }));
  db_->setPluginNames(std::vector<PluginMetadata>({
      PluginMetadata("Blank.esm"),
      PluginMetadata("Blank.esp"),
  }));

  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });
  db_->setAddedTags({
      "Relev",
  });
  db_->setRemovedTags({
      "Relev",
  });

  ASSERT_FALSE(db_->getPluginMessages().empty());
  ASSERT_FALSE(db_->getPluginNames().empty());
  ASSERT_FALSE(db_->getAddedTagIds().empty());
  ASSERT_FALSE(db_->getRemovedTagIds().empty());
  ASSERT_FALSE(db_->getBashTagMap().empty());

  EXPECT_NO_THROW(db_->clearArrays());

  EXPECT_TRUE(db_->getPluginMessages().empty());
  EXPECT_TRUE(db_->getPluginNames().empty());
  EXPECT_TRUE(db_->getAddedTagIds().empty());
  EXPECT_TRUE(db_->getRemovedTagIds().empty());
}

TEST_P(loot_db_test, clearingArraysShouldNotEmptyBashTagMap) {
  db_->addBashTagsToMap({
      "C.Climate",
      "Relev",
  });
  ASSERT_FALSE(db_->getBashTagMap().empty());

  EXPECT_NO_THROW(db_->clearArrays());

  EXPECT_FALSE(db_->getBashTagMap().empty());
}
}
}

#endif
