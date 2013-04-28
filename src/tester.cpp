
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

#include <src/commonSupport.h>
#include <src/fileFormat.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

using namespace std;

//It would be neater if this could be done by just skipping bits of the output...
void minimisePluginList(set<boss::Plugin, boss::plugin_comp>& plugins) {
    set<boss::Plugin, boss::plugin_comp> out;
    for (set<boss::Plugin, boss::plugin_comp>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
        boss::Plugin p(it->Name());
        p.Tags(it->Tags());
        out.insert(p);
    }
    plugins.swap(out);
}

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
    cout << "Testing plugin reading." << endl;
    
    time_t start, end;
    start = time(NULL);
    
    // Set up libesm.
	common::options::setGame(boss::libespm_game);
	ifstream input(boss::libespm_options_path);
	common::readOptions(input);
	input.close();
    
    // Get a list of the plugins.
    list<boss::Plugin> plugins;
    for (fs::directory_iterator it(argv[1]); it != fs::directory_iterator(); ++it) {
        if (fs::is_regular_file(it->status()) && (it->path().extension().string() == ".esp" || it->path().extension().string() == ".esm")) {
			
			if (it->path().filename().string() == "Skyrim.esm")
				continue;  // Libespm crashes with v1.9 Skyrim.esm.
			
			cout << "Found plugin: " << it->path().string() << endl;
			boss::Plugin plugin(it->path().filename().string(),it->path().parent_path().string());
			plugins.push_back(plugin);
        }
    }
    
    cout << "Finished looking for plugins." << endl;
    
    plugins.sort(boss::plugin_comp());
    
    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt = plugins.end(); it != endIt; ++it) {
		cout << it->Name() << endl
			 << '\t' << "Is Master: " << it->IsMaster() << endl
			 << '\t' << "Masters:" << endl;
			 
		vector<std::string> masters = it->Masters();
		for(int i = 0; i < masters.size(); ++i)
			cout << '\t' << '\t' << i << ": " << masters[i] << endl;
			
		cout << '\t' << "Number of Records: " << it->FormIDs().size() << endl;
		
		cout << '\t' << "Number of Override Records: " << it->OverrideFormIDs().size() << endl;
	}
	
	end = time(NULL);
	cout << "Time taken to sort plugins: " << (end - start) << " seconds." << endl;
	
    return 0;
}
