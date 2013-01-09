
#include <yaml-cpp/yaml.h>
#include "metadata.h"
#include "parsers.h"

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

    YAML::Node test = YAML::LoadFile("masterlist.yaml");

    list<boss::Message> globalMessages;
    if (test["globals"]) {
        YAML::Node globals = test["globals"];
        for (YAML::const_iterator it=globals.begin(); it != globals.end(); ++it) {
            globalMessages.push_back(it->as<boss::Message>());
        }
    }

    set<boss::Plugin, boss::plugin_comp> pluginData;
    if (test["plugins"]) {
        YAML::Node plugins = test["plugins"];
        for (YAML::const_iterator it=plugins.begin(); it != plugins.end(); ++it) {
            pluginData.insert(it->as<boss::Plugin>());
        }
    }

    cout << "Testing masterlist generator." << endl;

    minimisePluginList(pluginData);

    ofstream out("minimal.yaml");
    YAML::Emitter yout;
    yout.SetIndent(2);
    yout << YAML::BeginMap
         << YAML::Key << "globals" << YAML::Value << globalMessages
         << YAML::Key << "plugins" << YAML::Value << pluginData
         << YAML::EndMap;

    out << yout.c_str();
    out.close();

    return 0;
}
