/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2013-2016    WrinklyNinja

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

#ifndef LOOT_TESTS_API_API_GAME_OPERATIONS_TEST
#define LOOT_TESTS_API_API_GAME_OPERATIONS_TEST

#include "loot/api.h"

#include "tests/common_game_test_fixture.h"

namespace loot {
namespace test {
class ApiGameOperationsTest : public CommonGameTestFixture {
protected:
  ApiGameOperationsTest() :
    db_(nullptr),
    masterlistPath(localPath / "masterlist.yaml"),
    noteMessage("Do not clean ITM records, they are intentional and required for the mod to function."),
    warningMessage("Check you are using v2+. If not, Update. v1 has a severe bug with the Mystic Emporium disappearing."),
    errorMessage("Obsolete. Remove this and install Enhanced Weather.") {}

  virtual void SetUp() {
    CommonGameTestFixture::SetUp();

    ASSERT_FALSE(boost::filesystem::exists(masterlistPath));

    db_ = CreateDatabase(GetParam(), dataPath.parent_path().string(), localPath.string());
  }

  virtual void TearDown() {
    CommonGameTestFixture::TearDown();

    // The masterlist may have been created during the test, so delete it.
    ASSERT_NO_THROW(boost::filesystem::remove(masterlistPath));
  }

  void GenerateMasterlist() {
    using std::endl;

    boost::filesystem::ofstream masterlist(masterlistPath);
    masterlist
      << "plugins:" << endl
      << "  - name: " << blankEsm << endl
      << "    after:" << endl
      << "      - " << masterFile << endl
      << "    msg:" << endl
      << "      - type: say" << endl
      << "        content: '" << noteMessage << "'" << endl
      << "    tag:" << endl
      << "      - Actors.ACBS" << endl
      << "      - Actors.AIData" << endl
      << "      - '-C.Water'" << endl
      << "  - name: " << blankDifferentEsm << endl
      << "    after:" << endl
      << "      - " << blankMasterDependentEsm << endl
      << "    msg:" << endl
      << "      - type: warn" << endl
      << "        content: '" << warningMessage << "'" << endl
      << "    dirty:" << endl
      << "      - crc: 0x7d22f9df" << endl
      << "        utility: TES4Edit" << endl
      << "        udr: 4" << endl
      << "  - name: " << blankDifferentEsp << endl
      << "    after:" << endl
      << "      - " << blankPluginDependentEsp << endl
      << "    msg:" << endl
      << "      - type: error" << endl
      << "        content: '" << errorMessage << "'" << endl
      << "  - name: " << blankEsp << endl
      << "    after:" << endl
      << "      - " << blankDifferentMasterDependentEsp << endl
      << "  - name: " << blankDifferentMasterDependentEsp << endl
      << "    after:" << endl
      << "      - " << blankMasterDependentEsp << endl
      << "    msg:" << endl
      << "      - type: say" << endl
      << "        content: '" << noteMessage << "'" << endl
      << "      - type: warn" << endl
      << "        content: '" << warningMessage << "'" << endl
      << "      - type: error" << endl
      << "        content: '" << errorMessage << "'" << endl;

    masterlist.close();
  }

  std::shared_ptr<DatabaseInterface> db_;

  const boost::filesystem::path masterlistPath;

  const std::string noteMessage;
  const std::string warningMessage;
  const std::string errorMessage;
};
}
}

#endif
