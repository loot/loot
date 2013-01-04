
#include <yaml-cpp/yaml.h>
#include "metadata.cpp"

#include <ostream>

using namespace std;

int main() {

    cout << "Testing masterlist parser." << endl;

    YAML::Node test = YAML::LoadFile("masterlist-example.yaml");

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

    for (list<boss::Message>::const_iterator it=globalMessages.begin(), endIt=globalMessages.end(); it != endIt; ++it) {
        cout << "Message data:" << endl
             << "\tCondition: " << it->condition << endl
             << "\tType: " << it->type << endl
             << "\tLanguage: " << it->language << endl
             << "\tContent: " << it->content << endl;
    }

    for (set<boss::Plugin, boss::plugin_comp>::const_iterator it=pluginData.begin(), endIt=pluginData.end(); it != endIt; ++it) {
        cout << it->name << endl;
        for (list<boss::Message>::const_iterator jt=it->messages.begin(), endJt=it->messages.end(); jt != endJt; ++jt) {
            cout << "\tMessage:" << endl
                 << "\t\tCondition: " << jt->condition << endl
                 << "\t\tType: " << jt->type << endl
                 << "\t\tLanguage: " << jt->language << endl
                 << "\t\tContent: " << jt->content << endl;
        }
        for (set<boss::Tag, boss::tag_comp>::const_iterator jt=it->tags.begin(), endJt=it->tags.end(); jt != endJt; ++jt) {
            cout << "\tTag:" << endl
                 << "\t\tCondition: " << jt->condition << endl
                 << "\t\tName: " << jt->name << endl;
        }
    }

    return 0;
}
