/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012    WrinklyNinja

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

#include <yaml-cpp/yaml.h>
#include "metadata.h"
#include "parsers.h"
#include "game.h"
#include "error.h"
#include "globals.h"

#include <ostream>
#include <fstream>
#include <stdint.h>
#include <ctime>

#include <algorithm>
#include <iterator>

#include <src/commonSupport.h>
#include <src/fileFormat.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace fs = boost::filesystem;

using namespace std;

// Temporary hack.
const char * const libespm_options_path = "libespm.cfg";
const char * const libespm_game = "Skyrim";



int main(int argc, char *argv[]) {

 /*   cout << "Testing masterlist parser." << endl;

    YAML::Node test = YAML::LoadFile("masterlist-example.yaml");

    list<boss::Message> globalMessages = test["globals"].as< list<boss::Message> >();
    list<boss::Plugin> pluginData = test["plugins"].as< list<boss::Plugin> >();

    cout << "Testing masterlist generator." << endl;

  //  minimisePluginList(pluginData);

    ofstream out("generated.yaml");
    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << YAML::BeginMap
         << YAML::Key << "globals" << YAML::Value << globalMessages
         << YAML::Key << "plugins" << YAML::Value << pluginData
         << YAML::EndMap;

    out << yout.c_str();
    out.close();

    cout << "Testing game settings structure." << endl;

    boss::Game game(boss::GAME_TES5, "/media/oliver/6CF05918F058EA3A/Program Files (x86)/Steam/steamapps/common/skyrim");

    for (list<boss::Plugin>::iterator it=pluginData.begin(), endIt=pluginData.end(); it != endIt; ++it) {
        try {
        it->EvalAllConditions(game);
        } catch (boss::error& e) {
            cout << e.what() << endl;
        }
    }

    ofstream out2("evaled.yaml");
    YAML::Emitter yout2;
    yout2.SetIndent(2);
    yout2 << YAML::BeginMap
         << YAML::Key << "globals" << YAML::Value << globalMessages
         << YAML::Key << "plugins" << YAML::Value << pluginData
         << YAML::EndMap;

    out2 << yout2.c_str();
    out2.close();
 */   
    cout << "Setting up libespm and BOSS..." << endl;
    
    
    // Set up libesm.
	common::options::setGame(libespm_game);
	ifstream input(libespm_options_path);
	common::readOptions(input);
	input.close();
	
	//Set up BOSS vars.
	boss::Game game(boss::GAME_TES5);

    cout << "Reading plugins in Data folder..." << endl;
    time_t start, end;
    start = time(NULL);
    
    // Get a list of the plugins.
    list<boss::Plugin> plugins;
    for (fs::directory_iterator it(game.DataPath()); it != fs::directory_iterator(); ++it) {
        if (fs::is_regular_file(it->status()) && (it->path().extension().string() == ".esp" || it->path().extension().string() == ".esm")) {

            string filename = it->path().filename().string();
			if (filename == "Skyrim.esm"
             || filename == "Tamriel Compendium.esp"
             || filename == "Tamriel Compendium - Skill Books.esp")
				continue;  // Libespm crashes with these plugins.
			
			cout << "Reading plugin: " << filename << endl;
			boss::Plugin plugin(filename, it->path().parent_path().string());
            plugins.push_back(plugin);
        }
    }

    end = time(NULL);
	cout << "Time taken to read plugins: " << (end - start) << " seconds." << endl;
    start = time(NULL);

    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it) {
		cout << it->Name() << endl
             << '\t' << "Number of records: " << it->FormIDs().size() << endl
			 << '\t' << "Is Master: " << it->IsMaster() << endl
			 << '\t' << "Masters:" << endl;
			 
		vector<std::string> masters = it->Masters();
		for(int i = 0; i < masters.size(); ++i)
			cout << '\t' << '\t' << i << ": " << masters[i] << endl;

        cout << '\t' << "Conflicts with:" << endl;
        for (list<boss::Plugin>::iterator jt=plugins.begin(), endJt = plugins.end(); jt != endJt; ++jt) {
            if (*jt != *it && !jt->IsChildOf(*it)) {
                size_t overlap = jt->OverlapFormIDs(*it).size();
                if (overlap > 0)
                    cout << '\t' << '\t' << jt->Name() << " (" << overlap << " records)" << endl;
            }
        }
	}
    
    end = time(NULL);
	cout << "Time taken to print plugins' details: " << (end - start) << " seconds." << endl;
    start = time(NULL);

    cout << "Sorting plugins..." << endl;

    plugins.sort();
	
	cout << "Time taken to sort plugins: " << (end - start) << " seconds." << endl;

    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it)
        cout << it->Name() << endl;
    
	cout << "Tester finished." << endl;
    start = time(NULL);
    return 0;
}
