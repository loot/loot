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

#ifndef LOOT_TEST_API_LOOT_DB_INT
#define LOOT_TEST_API_LOOT_DB_INT

#include "api/loot_db.h"
#include "backend/game/game_settings.h"
#include "tests/backend/base_game_test.h"

namespace loot {
    namespace test {
        class loot_db_test : public BaseGameTest {
        protected:
            loot_db_test() :
                db(nullptr) {}

            virtual void SetUp() {
                BaseGameTest::SetUp();

                db = new loot_db(static_cast<unsigned int>(GetParam()), dataPath.parent_path().string().c_str(), localPath.string().c_str());
            }

            inline virtual void TearDown() {
                BaseGameTest::TearDown();

                delete db;
            }

            loot_db * db;
        };

        // Pass an empty first argument, as it's a prefix for the test instantation,
        // but we only have the one so no prefix is necessary.
        INSTANTIATE_TEST_CASE_P(,
                                loot_db_test,
                                ::testing::Values(
                                    GameType::tes4,
                                    GameType::tes5,
                                    GameType::fo3,
                                    GameType::fonv,
                                    GameType::fo4));

        TEST_P(loot_db_test, settingRevisionIdStringShouldCopyIt) {
            db->setRevisionIdString("id");
            EXPECT_STREQ("id", db->getRevisionIdString());
        }

        TEST_P(loot_db_test, settingRevisionDateStringShouldCopyIt) {
            db->setRevisionDateString("date");
            EXPECT_STREQ("date", db->getRevisionDateString());
        }

        TEST_P(loot_db_test, settingPluginNamesShouldCopyThem) {
            db->setPluginNames(std::vector<PluginMetadata>({
                PluginMetadata("Blank.esm"),
                PluginMetadata("Blank.esp"),
            }));

            EXPECT_EQ(2, db->getPluginNames().size());
            EXPECT_STREQ("Blank.esm", db->getPluginNames()[0]);
            EXPECT_STREQ("Blank.esp", db->getPluginNames()[1]);
        }

        TEST_P(loot_db_test, settingPluginNamesTwiceShouldOverwriteTheFirstDataSet) {
            db->setPluginNames(std::vector<PluginMetadata>({
                PluginMetadata("Blank.esm"),
                PluginMetadata("Blank.esp"),
            }));
            db->setPluginNames(std::vector<PluginMetadata>({
                PluginMetadata("Blank - Different.esm"),
                PluginMetadata("Blank - Different.esp"),
            }));

            EXPECT_EQ(2, db->getPluginNames().size());
            EXPECT_STREQ("Blank - Different.esm", db->getPluginNames()[0]);
            EXPECT_STREQ("Blank - Different.esp", db->getPluginNames()[1]);
        }

        TEST_P(loot_db_test, addingNewBashTagsToTheMapShouldAppendThem) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            EXPECT_EQ(2, db->getBashTagMap().size());
            EXPECT_STREQ("C.Climate", db->getBashTagMap()[0]);
            EXPECT_STREQ("Relev", db->getBashTagMap()[1]);
        }

        TEST_P(loot_db_test, addingAnExistingBashTagToTheMapShouldNotDuplicateIt) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
                "C.Climate",
            });

            EXPECT_EQ(2, db->getBashTagMap().size());
            EXPECT_STREQ("C.Climate", db->getBashTagMap()[0]);
            EXPECT_STREQ("Relev", db->getBashTagMap()[1]);
        }

        TEST_P(loot_db_test, gettingABashTagsUidForATagThatIsNotInTheMapShouldThrow) {
            EXPECT_ANY_THROW(db->getBashTagUid("Relev"));
        }

        TEST_P(loot_db_test, gettingABashTagsUidShouldReturnItsTagMapIndex) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            EXPECT_EQ(1, db->getBashTagUid("Relev"));
        }

        TEST_P(loot_db_test, clearingAnEmptyBashTagMapShouldDoNothing) {
            EXPECT_NO_THROW(db->clearBashTagMap());
        }

        TEST_P(loot_db_test, clearingABashTagMapShouldEmptyIt) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            db->clearBashTagMap();
            EXPECT_TRUE(db->getBashTagMap().empty());
        }

        TEST_P(loot_db_test, clearingABashTagMapShouldAffectExistingReferences) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            auto& bashTagMap = db->getBashTagMap();
            db->clearBashTagMap();
            EXPECT_TRUE(bashTagMap.empty());
        }

        TEST_P(loot_db_test, settingAddedTagsWithNoTagMapShouldThrow) {
            EXPECT_ANY_THROW(db->setAddedTags({
                "Relev",
            }));
        }

        TEST_P(loot_db_test, gettingSetAddedTagsShouldReturnTheirUids) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            db->setAddedTags({
                "Relev",
            });

            EXPECT_EQ(std::vector<unsigned int>({
                1,
            }), db->getAddedTagIds());
        }

        TEST_P(loot_db_test, settingAddedTagsShouldReplaceExistingTags) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            db->setAddedTags({
                "Relev",
            });

            db->setAddedTags({
                "C.Climate",
            });

            EXPECT_EQ(std::vector<unsigned int>({
                0,
            }), db->getAddedTagIds());
        }

        TEST_P(loot_db_test, settingRemovedTagsWithNoTagMapShouldThrow) {
            EXPECT_ANY_THROW(db->setRemovedTags({
                "Relev",
            }));
        }

        TEST_P(loot_db_test, gettingSetRemovedTagsShouldReturnTheirUids) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            db->setRemovedTags({
                "Relev",
            });

            EXPECT_EQ(std::vector<unsigned int>({
                1,
            }), db->getRemovedTagIds());
        }

        TEST_P(loot_db_test, settingRemovedTagsShouldReplaceExistingTags) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            db->setRemovedTags({
                "Relev",
            });

            db->setRemovedTags({
                "C.Climate",
            });

            EXPECT_EQ(std::vector<unsigned int>({
                0,
            }), db->getRemovedTagIds());
        }

        TEST_P(loot_db_test, settingPluginMessagesShouldCopyThem) {
            db->setPluginMessages(std::list<Message>({
                Message(Message::Type::warn, "Test 1"),
                Message(Message::Type::error, "Test 2"),
            }));

            EXPECT_EQ(2, db->getPluginMessages().size());
            EXPECT_EQ(static_cast<unsigned int>(Message::Type::warn), db->getPluginMessages()[0].type);
            EXPECT_STREQ("Test 1", db->getPluginMessages()[0].message);
            EXPECT_EQ(static_cast<unsigned int>(Message::Type::error), db->getPluginMessages()[1].type);
            EXPECT_STREQ("Test 2", db->getPluginMessages()[1].message);
        }

        TEST_P(loot_db_test, settingPluginMessagesTwiceShouldOverwriteTheFirstDataSet) {
            db->setPluginMessages(std::list<Message>({
                Message(Message::Type::warn, "Test 1"),
                Message(Message::Type::error, "Test 2"),
            }));
            db->setPluginMessages(std::list<Message>({
                Message(Message::Type::error, "Test 3"),
                Message(Message::Type::warn, "Test 4"),
                Message(Message::Type::say, "Test 5"),
            }));

            EXPECT_EQ(3, db->getPluginMessages().size());
            EXPECT_EQ(static_cast<unsigned int>(Message::Type::error), db->getPluginMessages()[0].type);
            EXPECT_STREQ("Test 3", db->getPluginMessages()[0].message);
            EXPECT_EQ(static_cast<unsigned int>(Message::Type::warn), db->getPluginMessages()[1].type);
            EXPECT_STREQ("Test 4", db->getPluginMessages()[1].message);
            EXPECT_EQ(static_cast<unsigned int>(Message::Type::say), db->getPluginMessages()[2].type);
            EXPECT_STREQ("Test 5", db->getPluginMessages()[2].message);
        }

        TEST_P(loot_db_test, clearingArraysShouldEmptyPluginNamesTagIdsAndMessages) {
            db->setPluginMessages(std::list<Message>({
                Message(Message::Type::warn, "Test 1"),
                Message(Message::Type::error, "Test 2"),
            }));
            db->setPluginNames(std::vector<PluginMetadata>({
                PluginMetadata("Blank.esm"),
                PluginMetadata("Blank.esp"),
            }));

            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });
            db->setAddedTags({
                "Relev",
            });
            db->setRemovedTags({
                "Relev",
            });

            ASSERT_FALSE(db->getPluginMessages().empty());
            ASSERT_FALSE(db->getPluginNames().empty());
            ASSERT_FALSE(db->getAddedTagIds().empty());
            ASSERT_FALSE(db->getRemovedTagIds().empty());
            ASSERT_FALSE(db->getBashTagMap().empty());

            EXPECT_NO_THROW(db->clearArrays());

            EXPECT_TRUE(db->getPluginMessages().empty());
            EXPECT_TRUE(db->getPluginNames().empty());
            EXPECT_TRUE(db->getAddedTagIds().empty());
            EXPECT_TRUE(db->getRemovedTagIds().empty());
        }

        TEST_P(loot_db_test, clearingArraysShouldNotEmptyBashTagMap) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });
            ASSERT_FALSE(db->getBashTagMap().empty());

            EXPECT_NO_THROW(db->clearArrays());

            EXPECT_FALSE(db->getBashTagMap().empty());
        }
    }
}

#endif
