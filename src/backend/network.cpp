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
            throw error(ERROR_SUBVERSION_ERROR, "Could not create pipe for Subversion process.");

        //Create a child process.
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO); 
        siStartInfo.hStdError = consoleWrite;
        siStartInfo.hStdOutput = consoleWrite;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

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
            throw error(ERROR_SUBVERSION_ERROR, "Could not create Subversion process.");

        WaitForSingleObject(piProcInfo.hProcess, INFINITE);

        if (!GetExitCodeProcess(piProcInfo.hProcess, &exitCode))
            throw error(ERROR_SUBVERSION_ERROR, "Could not get Subversion process exit code.");
        
        if (!ReadFile(consoleRead, chBuf, BUFSIZE, &dwRead, NULL))
            throw error(ERROR_SUBVERSION_ERROR, "Could not read Subversion process output.");
            
        output = string(chBuf, dwRead);
        
        return exitCode == 0;
    }

    std::string UpdateMasterlist(const Game& game, std::vector<std::string>& parsingErrors) {
        
        string command, output;
        //First check if the working copy is set up or not.
        command = svn_path.string() + " info \"" + game.MasterlistPath().parent_path().string() + "\"";
        
        if (!RunCommand(command, output)) {
            //Working copy not set up, perform a checkout.
            command = svn_path.string() + " co --depth empty " + game.URL().substr(0, game.URL().rfind('/')) + " \"" + game.MasterlistPath().parent_path().string() + "\\.\"";
            if (!RunCommand(command, output))
                throw error(ERROR_SUBVERSION_ERROR, "Subversion could not perform a checkout. Details: " + output);
        }

        //Now update masterlist.
        command = svn_path.string() + " update \"" + game.MasterlistPath().string() + "\"";
        if (!RunCommand(command, output))
            throw error(ERROR_SUBVERSION_ERROR, "Subversion could not update the masterlist. Details: " + output);

        //Now test masterlist to see if it parses OK.
        bool good = false;
        while (!good) {
            try {
                YAML::Node mlist = YAML::LoadFile(game.MasterlistPath().string());
                good = true;
            } catch (YAML::Exception& e) {
                //Roll back one revision if there's an error.
                parsingErrors.push_back(e.what());
                command = svn_path.string() + " update --revision PREV \"" + game.MasterlistPath().string() + "\"";
                if (!RunCommand(command, output))
                    throw error(ERROR_SUBVERSION_ERROR, "Subversion could not update the masterlist. Details: " + output);
            }
        }

        //Now get the masterlist revision. Can either create a pipe using the Win32 API (http://msdn.microsoft.com/en-us/library/ms682499.aspx), or output to a file, read it, then delete it.
        command = svn_path.string() + " info \"" + game.MasterlistPath().string() + "\"";
        if (!RunCommand(command, output))
            throw error(ERROR_SUBVERSION_ERROR, "Subversion could not read the masterlist revision number. Details: " + output);

        size_t pos1 = output.rfind("Revision: ");
        size_t pos2 = output.find('\n', pos1);

        string revision = output.substr(pos1+10, pos2-pos1-10);

        pos1 = output.find("Last Changed Date: ", pos2);
        pos2 = output.find(' ', pos1+19);

        string date = output.substr(pos1+19, pos2-pos1-19);

        return revision + " (" + date + ")";
    }
}
