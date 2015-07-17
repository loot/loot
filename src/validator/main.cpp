/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2014-2015    WrinklyNinja

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

#ifdef TRAVIS
#pragma message("This is a Travis build, so defining BOOST_NO_CXX11_SCOPED_ENUMS to avoid boost::filesystem::copy_file() linking errors.")
#define BOOST_NO_CXX11_SCOPED_ENUMS
#endif

#include "backend/globals.h"
#include "backend/metadata_list.h"

#include <boost/log/core.hpp>

int main(int argc, char **argv) {
    //Set the locale to get encoding conversions working correctly.
    std::locale::global(boost::locale::generator().generate(""));
    boost::filesystem::path::imbue(std::locale());

    //Disable logging or else stdout will get overrun.
    boost::log::core::get()->set_logging_enabled(false);

    // Print help text if -h, --help or invalid args are given (including no args).
    if (argc != 2 || (strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
        std::cout << std::endl
            << "Usage: metadata-validator <arg>" << std::endl << std::endl
            << "Arguments:" << std::endl << std::endl
            << "  " << "<file>" << "         " << "The metadata file to validate." << std::endl
            << "  " << "-v, --version" << "  " << "Prints version information for this utility." << std::endl
            << "  " << "-h, --help" << "     " << "Prints this help text." << std::endl << std::endl;
        return 1;
    }

    // Print version info if -v or --version are given.
    if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0)) {
        std::cout << std::endl << "LOOT Metadata Validator" << std::endl
            << "v" << loot::g_version_major << "." << loot::g_version_minor
            << "." << loot::g_version_patch << std::endl
            << "build revision " << loot::g_build_revision << std::endl << std::endl;
        return 0;
    }

    try {
        std::cout << std::endl << "Validating metadata file: " << argv[1] << std::endl << std::endl;
        // Test YAML parsing.
        loot::MetadataList metadata;
        metadata.Load(argv[1]);

        // Test condition parsing.
        for (auto &plugin : metadata.Plugins()) {
            plugin.ParseAllConditions();
        }
        for (auto &message : metadata.messages) {
            message.ParseCondition();
        }
    }
    catch (std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl << std::endl;
        return 1;
    }
    std::cout << "SUCCESS!" << std::endl << std::endl;

    return 0;
}
