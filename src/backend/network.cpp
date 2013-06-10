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

    unsigned int UpdateMasterlist(const Game& game) {
        //First check if the working copy is set up or not.
        int ret = system(("resources\\svn\\svn.exe info " + game.MasterlistPath().parent_path().string() + " > svn.log").c_str());
        
        if (ret != 0) {
            //Working copy not set up, perform a checkout.
            ret = system(("resources\\svn\\svn.exe co --depth empty " + game.URL().substr(0, game.URL().rfind('/')) + " " + game.MasterlistPath().parent_path().string() + "/. > svn.log").c_str());
        }

        //Now update masterlist.
        ret = system(("resources\\svn\\svn.exe update " + game.MasterlistPath().string() + " > svn.log").c_str());

        //Now test masterlist to see if it parses OK.
        ret = 1;
        while (ret == 1) {
            try {
                YAML::Node mlist = YAML::LoadFile(game.MasterlistPath().string());
                ret = 0;
            } catch (YAML::Exception& e) {
                //Roll back one revision if there's an error.
                ret = system(("resources\\svn\\svn.exe update --revision PREV " + game.MasterlistPath().string() + " > svn.log").c_str());
                ret = 1;
            }
        }

        //Now get the masterlist revision. Can either create a pipe using the Win32 API (http://msdn.microsoft.com/en-us/library/ms682499.aspx), or output to a file, read it, then delete it.
        ret = system(("resources\\svn\\svn.exe info " + game.MasterlistPath().string() + " > svn.log").c_str());

        ifstream in("svn.log");

        int revision;
        while (in.good()) {
            string line;
            getline(in, line);

            if (line.substr(0, 10) == "Revision: ")
                revision = atoi(line.substr(10).c_str());
        }
        in.close();

        fs::remove("svn.log");

        return revision;
    }
}
