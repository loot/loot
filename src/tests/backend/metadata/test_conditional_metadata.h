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

#ifndef LOOT_TEST_BACKEND_METADATA_CONDITIONAL_METADATA
#define LOOT_TEST_BACKEND_METADATA_CONDITION_GRAMMAR

#include "backend/error.h"
#include "backend/metadata/conditional_metadata.h"
#include "tests/fixtures.h"

namespace loot {
    namespace test {
        class ConditionalMetadataTest : public SkyrimTest {};

        TEST_F(ConditionalMetadataTest, ConstructorAndDataAccess) {
            ConditionalMetadata cm;
            EXPECT_EQ("", cm.Condition());

            cm = ConditionalMetadata("condition");
            EXPECT_EQ("condition", cm.Condition());
        }

        TEST_F(ConditionalMetadataTest, IsConditional) {
            ConditionalMetadata cm;
            EXPECT_FALSE(cm.IsConditional());

            cm = ConditionalMetadata("condition");
            EXPECT_TRUE(cm.IsConditional());
        }

        TEST_F(ConditionalMetadataTest, EvalCondition) {
            Game game(Game::tes5);
            game.SetGamePath(dataPath.parent_path());
            ASSERT_NO_THROW(game.Init(false, localPath));

            ConditionalMetadata cm;
            EXPECT_TRUE(cm.EvalCondition(game));

            cm = ConditionalMetadata("condition");
            EXPECT_THROW(cm.EvalCondition(game), error);

            cm = ConditionalMetadata("file(\"Blank.esm\")");
            EXPECT_TRUE(cm.EvalCondition(game));

            cm = ConditionalMetadata("file(\"Blank.missing.esm\")");
            EXPECT_FALSE(cm.EvalCondition(game));
        }

        TEST_F(ConditionalMetadataTest, ParseCondition) {
            ConditionalMetadata cm;
            EXPECT_NO_THROW(cm.ParseCondition());

            cm = ConditionalMetadata("condition");
            EXPECT_THROW(cm.ParseCondition(), error);

            // Check that invalid regex also throws.
            cm = ConditionalMetadata("regex(\"RagnvaldBook(Farengar(+Ragnvald)?)?\\.esp\")");
            EXPECT_THROW(cm.ParseCondition(), error);

            cm = ConditionalMetadata("file(\"Blank.esm\")");
            EXPECT_NO_THROW(cm.ParseCondition());

            cm = ConditionalMetadata("file(\"Blank.missing.esm\")");
            EXPECT_NO_THROW(cm.ParseCondition());
        }
    }
}

#endif
