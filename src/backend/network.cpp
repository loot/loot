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

#if _WIN32 || _WIN64
#   ifndef UNICODE
#       define UNICODE
#   endif
#   ifndef _UNICODE
#      define _UNICODE
#   endif
#   include "windows.h"
#   include "shlobj.h"
#endif
#define BUFSIZE 4096 

using namespace std;

namespace fs = boost::filesystem;

namespace boss {

    bool RunCommand(const std::string& command, std::string& output) {
        HANDLE consoleWrite = NULL;
        HANDLE consoleRead = NULL;

        SECURITY_ATTRIBUTES saAttr;

        PROCESS_INFORMATION piProcInfo; 
        STARTUPINFO siStartInfo;

        CHAR chBuf[BUFSIZE];
        DWORD dwRead;
        
        DWORD exitCode;
        
        //Init attributes.
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        //Create I/O pipes.
        if (!CreatePipe(&consoleRead, &consoleWrite, &saAttr, 0))
            throw error(error::subversion_error, "Could not create pipe for Subversion process.");

        //Create a child process.
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO); 
        siStartInfo.hStdError = consoleWrite;
        siStartInfo.hStdOutput = consoleWrite;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        siStartInfo.wShowWindow = SW_HIDE;

        const int utf16Len = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, NULL, 0);
        wchar_t * cmdLine = new wchar_t[utf16Len];
        MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, cmdLine, utf16Len);

        bool result = CreateProcess(NULL, 
            cmdLine,     // command line 
            NULL,          // process security attributes 
            NULL,          // primary thread security attributes 
            TRUE,          // handles are inherited 
            0,             // creation flags 
            NULL,          // use parent's environment 
            NULL,          // use parent's current directory 
            &siStartInfo,  // STARTUPINFO pointer 
            &piProcInfo);  // receives PROCESS_INFORMATION

        delete [] cmdLine;

        if (!result)
            throw error(error::subversion_error, "Could not create Subversion process.");

        WaitForSingleObject(piProcInfo.hProcess, INFINITE);

        if (!GetExitCodeProcess(piProcInfo.hProcess, &exitCode))
            throw error(error::subversion_error, "Could not get Subversion process exit code.");
        
        if (!ReadFile(consoleRead, chBuf, BUFSIZE, &dwRead, NULL))
            throw error(error::subversion_error, "Could not read Subversion process output.");
            
        output = string(chBuf, dwRead);
        
        return exitCode == 0;
    }

    //Gets revision + date string.
    string GetRevision(const std::string& buffer) {
        string revision, date;
        size_t pos1, pos2;
        
        pos1 = buffer.rfind("Revision: ");
        if (pos1 == string::npos)
            return "";
        
        pos2 = buffer.find('\n', pos1);

        revision = buffer.substr(pos1+10, pos2-pos1-10);

        pos1 = buffer.find("Last Changed Date: ", pos2);
        pos2 = buffer.find(' ', pos1+19);

        date = buffer.substr(pos1+19, pos2-pos1-19);

        return revision + " (" + date + ")";
    }

    std::string UpdateMasterlist(const Game& game, std::vector<std::string>& parsingErrors) {
        
        string command, output, revision;
        //First check if the working copy is set up or not.
        command = g_path_svn.string() + " info \"" + game.MasterlistPath().string() + "\"";

        revision = GetRevision(output);

        if (game.URL().empty()) {
            if (!revision.empty())
                return revision;
            else
                return "N/A";
        }
        
        if (!RunCommand(command, output)) {
            //Working copy not set up, perform a checkout.
            command = g_path_svn.string() + " co --depth empty " + game.URL().substr(0, game.URL().rfind('/')) + " \"" + game.MasterlistPath().parent_path().string() + "\\.\"";
            if (!RunCommand(command, output))
                throw error(error::subversion_error, "Subversion could not perform a checkout. Details: " + output);
        }

        //Now update masterlist.
        command = g_path_svn.string() + " update \"" + game.MasterlistPath().string() + "\"";
        if (!RunCommand(command, output))
            throw error(error::subversion_error, "Subversion could not update the masterlist. Details: " + output);

        while (true) {
            try {

                //Now get the masterlist revision. 
                command = g_path_svn.string() + " info \"" + game.MasterlistPath().string() + "\"";
                if (!RunCommand(command, output))
                    throw error(error::subversion_error, "Subversion could not read the masterlist revision number. Details: " + output);

                revision = GetRevision(output);

                //Now test masterlist to see if it parses OK.
                YAML::Node mlist = YAML::LoadFile(game.MasterlistPath().string());

                return revision;
            } catch (YAML::Exception& e) {
                //Roll back one revision if there's an error.
                parsingErrors.push_back("Masterlist revision " + revision + ": " + e.what());
                command = g_path_svn.string() + " update --revision PREV \"" + game.MasterlistPath().string() + "\"";
                if (!RunCommand(command, output))
                    throw error(error::subversion_error, "Subversion could not update the masterlist. Details: " + output);
            }
        }
    }
}
