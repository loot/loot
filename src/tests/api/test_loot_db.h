/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2014-2015    WrinklyNinja

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
#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class loot_db_test : public SkyrimTest {
        protected:
            virtual void SetUp() {
                SkyrimTest::SetUp();

                db = new loot_db(Game::tes5, dataPath.parent_path().string().c_str(), localPath.string().c_str());
            }
        };

        TEST_F(loot_db_test, settingRevisionIdStringShouldCopyIt) {
            db->setRevisionIdString("id");
            EXPECT_STREQ("id", db->getRevisionIdString());
        }

        TEST_F(loot_db_test, settingRevisionDateStringShouldCopyIt) {
            db->setRevisionDateString("date");
            EXPECT_STREQ("date", db->getRevisionDateString());
        }

        TEST_F(loot_db_test, settingPluginNamesShouldCopyThem) {
            db->setPluginNames(std::vector<PluginMetadata>({
                PluginMetadata("Blank.esm"),
                PluginMetadata("Blank.esp"),
            }));

            EXPECT_EQ(2, db->getPluginNames().size());
            EXPECT_STREQ("Blank.esm", db->getPluginNames()[0]);
            EXPECT_STREQ("Blank.esp", db->getPluginNames()[1]);
        }

        TEST_F(loot_db_test, settingPluginNamesTwiceShouldOverwriteTheFirstDataSet) {
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

        TEST_F(loot_db_test, addingNewBashTagsToTheMapShouldAppendThem) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            EXPECT_EQ(2, db->getBashTagMap().size());
            EXPECT_STREQ("C.Climate", db->getBashTagMap()[0]);
            EXPECT_STREQ("Relev", db->getBashTagMap()[1]);
        }

        TEST_F(loot_db_test, addingAnExistingBashTagToTheMapShouldNotDuplicateIt) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
                "C.Climate",
            });

            EXPECT_EQ(2, db->getBashTagMap().size());
            EXPECT_STREQ("C.Climate", db->getBashTagMap()[0]);
            EXPECT_STREQ("Relev", db->getBashTagMap()[1]);
        }

        TEST_F(loot_db_test, gettingABashTagsUidForATagThatIsNotInTheMapShouldThrow) {
            EXPECT_ANY_THROW(db->getBashTagUid("Relev"));
        }

        TEST_F(loot_db_test, gettingABashTagsUidShouldReturnItsTagMapIndex) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            EXPECT_EQ(1, db->getBashTagUid("Relev"));
        }

        TEST_F(loot_db_test, clearingAnEmptyBashTagMapShouldDoNothing) {
            EXPECT_NO_THROW(db->clearBashTagMap());
        }

        TEST_F(loot_db_test, clearingABashTagMapShouldEmptyIt) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            db->clearBashTagMap();
            EXPECT_TRUE(db->getBashTagMap().empty());
        }

        TEST_F(loot_db_test, clearingABashTagMapShouldAffectExistingReferences) {
            db->addBashTagsToMap({
                "C.Climate",
                "Relev",
            });

            auto& bashTagMap = db->getBashTagMap();
            db->clearBashTagMap();
            EXPECT_TRUE(bashTagMap.empty());
        }

        TEST_F(loot_db_test, settingAddedTagsWithNoTagMapShouldThrow) {
            EXPECT_ANY_THROW(db->setAddedTags({
                "Relev",
            }));
        }

        TEST_F(loot_db_test, gettingSetAddedTagsShouldReturnTheirUids) {
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

        TEST_F(loot_db_test, settingRemovedTagsWithNoTagMapShouldThrow) {
            EXPECT_ANY_THROW(db->setRemovedTags({
                "Relev",
            }));
        }

        TEST_F(loot_db_test, gettingSetRemovedTagsShouldReturnTheirUids) {
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

        TEST_F(loot_db_test, settingPluginMessagesShouldCopyThem) {
            db->setPluginMessages(std::list<Message>({
                Message(Message::warn, "Test 1"),
                Message(Message::error, "Test 2"),
            }));

            EXPECT_EQ(2, db->getPluginMessages().size());
            EXPECT_EQ(Message::warn, db->getPluginMessages()[0].type);
            EXPECT_STREQ("Test 1", db->getPluginMessages()[0].message);
            EXPECT_EQ(Message::error, db->getPluginMessages()[1].type);
            EXPECT_STREQ("Test 2", db->getPluginMessages()[1].message);
        }

        TEST_F(loot_db_test, settingPluginMessagesTwiceShouldOverwriteTheFirstDataSet) {
            db->setPluginMessages(std::list<Message>({
                Message(Message::warn, "Test 1"),
                Message(Message::error, "Test 2"),
            }));
            db->setPluginMessages(std::list<Message>({
                Message(Message::error, "Test 3"),
                Message(Message::warn, "Test 4"),
                Message(Message::say, "Test 5"),
            }));

            EXPECT_EQ(3, db->getPluginMessages().size());
            EXPECT_EQ(Message::error, db->getPluginMessages()[0].type);
            EXPECT_STREQ("Test 3", db->getPluginMessages()[0].message);
            EXPECT_EQ(Message::warn, db->getPluginMessages()[1].type);
            EXPECT_STREQ("Test 4", db->getPluginMessages()[1].message);
            EXPECT_EQ(Message::say, db->getPluginMessages()[2].type);
            EXPECT_STREQ("Test 5", db->getPluginMessages()[2].message);
        }

        TEST_F(loot_db_test, clearingArraysShouldEmptyPluginNamesTagIdsAndMessages) {
            db->setPluginMessages(std::list<Message>({
                Message(Message::warn, "Test 1"),
                Message(Message::error, "Test 2"),
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

        TEST_F(loot_db_test, clearingArraysShouldNotEmptyBashTagMap) {
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
