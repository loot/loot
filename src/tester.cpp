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

const char * const masterlist_path = "masterlist.yaml";
const char * const userlist_path = "userlist.yaml";



int main(int argc, char *argv[]) {

    /* Stuff still missing from a normal execution of BOSS:

      - Reading of settings file.
      - Detecting game to run for.
      - Error handling.
      - Masterlist updating.
      - Writing of log file.

      Need to include libespm setup into game setup, once the former is made to be not global.
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

    YAML::Node mlist, ulist;
    list<boss::Message> messages, mlist_messages, ulist_messages;
    list<boss::Plugin> mlist_plugins, ulist_plugins;

    if (fs::exists(masterlist_path)) {
        cout << "Parsing masterlist..." << endl;

        mlist = YAML::LoadFile(masterlist_path);
        if (mlist["globals"])
            mlist_messages = mlist["globals"].as< list<boss::Message> >();
        if (mlist["plugins"])
            mlist_plugins = mlist["plugins"].as< list<boss::Plugin> >();

        end = time(NULL);
        cout << "Time taken to parse masterlist: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }

    if (fs::exists(userlist_path)) {
        cout << "Parsing userlist..." << endl;

        ulist = YAML::LoadFile(userlist_path);
        if (ulist["globals"])
            ulist_messages = ulist["globals"].as< list<boss::Message> >();
        if (ulist["plugins"])
            ulist_plugins = ulist["plugins"].as< list<boss::Plugin> >();

        end = time(NULL);
        cout << "Time taken to parse userlist: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }

    if (fs::exists(masterlist_path) || fs::exists(userlist_path)) {
        cout << "Merging plugin lists..." << endl;

        //Merge all global message lists.
        messages = mlist_messages;
        messages.insert(messages.end(), ulist_messages.begin(), ulist_messages.end());

        //Merge plugin list, masterlist and userlist plugin data.
        for (list<boss::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
            //Check if there is already a plugin in the 'plugins' list or not.
            list<boss::Plugin>::iterator pos = std::find(mlist_plugins.begin(), mlist_plugins.end(), *it);

            if (pos != mlist_plugins.end()) {
                //Need to merge plugins.
                it->Merge(*pos);
            }

            pos = std::find(ulist_plugins.begin(), ulist_plugins.end(), *it);

            if (pos != ulist_plugins.end()) {
                //Need to merge plugins.
                it->Merge(*pos);
            }
        }

        end = time(NULL);
        cout << "Time taken to merge lists: " << (end - start) << " seconds." << endl;
        start = time(NULL);
    }
    
    cout << "Evaluating plugin list..." << endl;

    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
        try {
        it->EvalAllConditions(game);
        } catch (boss::error& e) {
            cout << e.what() << endl;
        }
    }

    end = time(NULL);
	cout << "Time taken to evaluate plugin list: " << (end - start) << " seconds." << endl;
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

    end = time(NULL);
	cout << "Time taken to sort plugins: " << (end - start) << " seconds." << endl;

    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it)
        cout << it->Name() << endl;
    
	cout << "Tester finished." << endl;
    start = time(NULL);
    return 0;
}
