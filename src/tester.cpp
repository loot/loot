
#include <yaml-cpp/yaml.h>
#include "metadata.h"
#include "parsers.h"
#include "game.h"
#include "error.h"
#include "globals.h"

#include <ostream>
#include <fstream>

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

int main() {

    cout << "Testing masterlist parser." << endl;

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

    return 0;
}
