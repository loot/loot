/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012-2013    WrinklyNinja

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

#include "network.h"
#include "error.h"
#include "parsers.h"

#include <iostream>

using namespace std;

namespace fs = boost::filesystem;

namespace boss {

    unsigned int UpdateMasterlist(const Game& game, std::vector<std::string>& parsingErrors) {
        string command;
        //First check if the working copy is set up or not.
        command = svn_path.string() + " info " + game.MasterlistPath().parent_path().string() + " > " + svn_log_path.string();
        
        if (system(command.c_str()) != 0) {
            //Working copy not set up, perform a checkout.
            command = svn_path.string() + " co --depth empty " + game.URL().substr(0, game.URL().rfind('/')) + " " + game.MasterlistPath().parent_path().string() + "/. > " + svn_log_path.string();
            if (system(command.c_str()) != 0)
                throw error(ERROR_SUBVERSION_ERROR, "Subversion could not perform a checkout. See " + svn_log_path.string() + " in the BOSS folder for details.");
        }

        //Now update masterlist.
        command = svn_path.string() + " update " + game.MasterlistPath().string() + " > " + svn_log_path.string();
        if (system(command.c_str()) != 0)
            throw error(ERROR_SUBVERSION_ERROR, "Subversion could not update the masterlist. See " + svn_log_path.string() + " in the BOSS folder for details.");

        //Now test masterlist to see if it parses OK.
        bool good = false;
        while (!good) {
            try {
                YAML::Node mlist = YAML::LoadFile(game.MasterlistPath().string());
                good = true;
            } catch (YAML::Exception& e) {
                //Roll back one revision if there's an error.
                parsingErrors.push_back(e.what());
                command = svn_path.string() + " update --revision PREV " + game.MasterlistPath().string() + " > " + svn_log_path.string();
                if (system(command.c_str()) != 0)
                    throw error(ERROR_SUBVERSION_ERROR, "Subversion could not update the masterlist. See " + svn_log_path.string() + " in the BOSS folder for details.");
            }
        }

        //Now get the masterlist revision. Can either create a pipe using the Win32 API (http://msdn.microsoft.com/en-us/library/ms682499.aspx), or output to a file, read it, then delete it.
        command = svn_path.string() + " info " + game.MasterlistPath().parent_path().string() + " > " + svn_log_path.string();
        if (system(command.c_str()) != 0)
            throw error(ERROR_SUBVERSION_ERROR, "Subversion could not read the masterlist revision number. See " + svn_log_path.string() + " in the BOSS folder for details.");

        ifstream in(svn_log_path.string().c_str());

        int revision = 0;
        while (in.good()) {
            string line;
            getline(in, line);

            if (line.substr(0, 10) == "Revision: ")
                revision = atoi(line.substr(10).c_str());
        }
        in.close();

        fs::remove(svn_log_path);

        return revision;
    }
}
