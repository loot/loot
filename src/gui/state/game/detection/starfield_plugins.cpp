#include "starfield_plugins.h"
#include <algorithm>
#include <string>
#include <vector>

namespace gui {
namespace state {
namespace game {
namespace detection {

StarfieldPlugins::StarfieldPlugins() {
    initializeOfficialPlugins();
}

void StarfieldPlugins::initializeOfficialPlugins() {
    // Base game
    officialPlugins.insert("Starfield.esm");
    
    // Official DLCs and Creation Club content
    officialPlugins.insert("BlueprintShips-StarfieldCommunity.esm");
    officialPlugins.insert("BlueprintShips-SFTAOldEarth.esm");
    officialPlugins.insert("BlueprintShips-SFTAConstellations.esm");
    officialPlugins.insert("BlueprintShips-SFTADeepCore.esm");
    officialPlugins.insert("BlueprintShips-SFTARyujinIndustries.esm");
    officialPlugins.insert("BlueprintShips-SFTAFreestarCollective.esm");
    officialPlugins.insert("BlueprintShips-SFTAHouseVaRuun.esm");
    officialPlugins.insert("BlueprintShips-SFTAUnitedColonies.esm");
    officialPlugins.insert("BlueprintShips-SFTACrimsonFleet.esm");
    officialPlugins.insert("BlueprintShips-SFTASpacers.esm");
    
    // Pattern-based recognition for future SFTA plugins
    officialPluginPatterns.push_back("blueprintships-sfta");
    officialPluginPatterns.push_back("starfield");
}

bool StarfieldPlugins::isOfficialPlugin(const std::string& pluginName) const {
    std::string lowerName = pluginName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    // Check exact matches first
    for (const auto& official : officialPlugins) {
        std::string lowerOfficial = official;
        std::transform(lowerOfficial.begin(), lowerOfficial.end(), lowerOfficial.begin(), ::tolower);
        if (lowerName == lowerOfficial) {
            return true;
        }
    }
    
    // Check pattern matches
    for (const auto& pattern : officialPluginPatterns) {
        if (lowerName.find(pattern) != std::string::npos && 
            (lowerName.ends_with(".esm") || lowerName.ends_with(".esp"))) {
            return true;
        }
    }
    
    return false;
}

bool StarfieldPlugins::isValidPlugin(const std::string& pluginName) const {
    std::string lowerName = pluginName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    // Check file extension
    if (!lowerName.ends_with(".esm") && !lowerName.ends_with(".esp") && !lowerName.ends_with(".esl")) {
        return false;
    }
    
    // All official plugins are valid
    if (isOfficialPlugin(pluginName)) {
        return true;
    }
    
    // Additional validation for user plugins could be added here
    return true;
}

bool StarfieldPlugins::isActivePlugin(const std::string& pluginName) const {
    // This would typically check against the plugins.txt file
    // For now, assume all valid plugins are potentially active
    return isValidPlugin(pluginName);
}

bool StarfieldPlugins::isSFTAPlugin(const std::string& pluginName) const {
    std::string lowerName = pluginName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    return lowerName.find("blueprintships-sfta") != std::string::npos ||
           lowerName.find("sfta") != std::string::npos;
}

std::vector<std::string> StarfieldPlugins::getOfficialPlugins() const {
    return std::vector<std::string>(officialPlugins.begin(), officialPlugins.end());
}

std::vector<std::string> StarfieldPlugins::getSFTAPlugins() const {
    std::vector<std::string> sftaPlugins;
    
    for (const auto& plugin : officialPlugins) {
        if (isSFTAPlugin(plugin)) {
            sftaPlugins.push_back(plugin);
        }
    }
    
    return sftaPlugins;
}

} // namespace detection
} // namespace game
} // namespace state
} // namespace gui