/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2013    WrinklyNinja

    This file is part of BOSS.

    BOSS is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#include "backend/legacy-parser.h"
#include "backend/generators.h"
#include "backend/streams.h"
#include "backend/metadata.h"

using namespace std;

int main(int argc, char * argv[]) {
    boost::filesystem::path v2masterlist("masterlist.txt");
    list<boss::Plugin> test_plugins;
    list<boss::Message> globalMessages;

    boost::log::core::get()->set_logging_enabled(false);

    try {
        Loadv2Masterlist(v2masterlist, test_plugins, globalMessages);
    } catch (exception& e) {
        cout << "An error was encountered during masterlist parsing. Message: " << e.what() << endl;
        return 1;
    }

    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << YAML::BeginMap
         << YAML::Key << "globals" << YAML::Value << globalMessages
         << YAML::Key << "plugins" << YAML::Value << test_plugins
         << YAML::EndMap;

    boost::filesystem::path v3masterlist = v2masterlist.string() + ".yaml";
    boss::ofstream out(v3masterlist);
    out << yout.c_str();
    out.close();

    return 0;
}
