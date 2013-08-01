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
#include "streams.h"
#include "helpers.h"

#include <boost/log/trivial.hpp>

#include <git2.h>

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
        if (!CreatePipe(&consoleRead, &consoleWrite, &saAttr, 0)) {
            BOOST_LOG_TRIVIAL(error) << "Could not create pipe for Subversion process.";
            throw error(error::subversion_error, "Could not create pipe for Subversion process.");
        }

        //Create a child process.
        BOOST_LOG_TRIVIAL(trace) << "Creating a child process.";
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

        if (!result) {
            BOOST_LOG_TRIVIAL(error) << "Could not create Subversion process.";
            throw error(error::subversion_error, "Could not create Subversion process.");
        }

        WaitForSingleObject(piProcInfo.hProcess, INFINITE);

        if (!GetExitCodeProcess(piProcInfo.hProcess, &exitCode)) {
            BOOST_LOG_TRIVIAL(error) << "Could not get Subversion process exit code.";
            throw error(error::subversion_error, "Could not get Subversion process exit code.");
        }

        if (!ReadFile(consoleRead, chBuf, BUFSIZE, &dwRead, NULL)) {
            BOOST_LOG_TRIVIAL(error) << "Could not read Subversion process output.";
            throw error(error::subversion_error, "Could not read Subversion process output.");
        }

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

    //Gets repository URL string.
    string GetURL(const std::string& buffer) {
        size_t pos1, pos2;

        pos1 = buffer.rfind("Repository Root: ");
        if (pos1 == string::npos)
            return "";

        pos2 = buffer.find('\n', pos1);

        return buffer.substr(pos1+17, pos2-pos1-17);
    }

    struct pointers_struct {
        pointers_struct() : repo(NULL), remote(NULL), cfg(NULL), obj(NULL), commit(NULL) {}

        void free() {
            git_commit_free(commit);
            git_object_free(obj);
            git_config_free(cfg);
            git_remote_free(remote);
            git_repository_free(repo);
        }

        git_repository * repo;
        git_remote * remote;
        git_config * cfg;
        git_object * obj;
        git_commit * commit;
    };

    //Returns false for errors, true for OK.
    void handle_error(int error_code, pointers_struct& pointers) {
        if (!error_code)
            return;

        const git_error * error = giterr_last();
        std::string error_message = "An error occurred during a Git operation. Error code: " + IntToString(error_code) + ".";
        if (error != NULL)
            error_message += string(" Message: ") + error->message;

        BOOST_LOG_TRIVIAL(error) << error_message;

        pointers.free();
        giterr_clear();
        throw boss::error(boss::error::git_error, error_message);
    }

    std::string UpdateMasterlist(Game& game, std::vector<std::string>& parsingErrors, std::list<Plugin>& plugins, std::list<Message>& messages) {

        //First need to decide how the masterlist is updated: using Git or Subversion?
        //Look at the update URL to decide.

        if (!boost::iends_with(game.URL(), ".git")) {  //Subversion
            string command, output, revision;
            //First check if the working copy is set up or not.
            command = g_path_svn.string() + " info \"" + game.MasterlistPath().string() + "\"";

            BOOST_LOG_TRIVIAL(trace) << "Checking to see if the working copy is set up or not for the masterlist at \"" + game.MasterlistPath().string() + "\"";
            bool success = RunCommand(command, output);

            revision = GetRevision(output);

            if (game.URL().empty()) {
                if (!revision.empty())
                    return revision;
                else
                    return "N/A";
            }

            if (!success) {
                BOOST_LOG_TRIVIAL(trace) << "Working copy is not set up, checking out repository.";
                //Working copy not set up, perform a checkout.
                command = g_path_svn.string() + " co --depth empty " + game.URL().substr(0, game.URL().rfind('/')) + " \"" + game.MasterlistPath().parent_path().string() + "\\.\"";
                if (!RunCommand(command, output)) {
                    BOOST_LOG_TRIVIAL(error) << "Subversion could not perform a checkout. Details: " << output;
                    throw error(error::subversion_error, "Subversion could not perform a checkout. Details: " + output);
                }
            } else {
                //A working copy exists, but we need to make sure that it points to the right repository.
                BOOST_LOG_TRIVIAL(trace) << "Comparing working copy repository URL with BOSS's URL";

                command = g_path_svn.string() + " info \"" + game.MasterlistPath().string() + "\"";

                if (!RunCommand(command, output)) {
                    BOOST_LOG_TRIVIAL(error) << "Subversion could not get the repository URL. Details: " << output;
                    throw error(error::subversion_error, "Subversion could not get the repository URL. Details: " + output);
                }

                string url = GetURL(output);

                //Now compare URLs.
                if (url != game.URL()) {
                    BOOST_LOG_TRIVIAL(trace) << "URLs do not match: relocating the working copy.";

                    command = g_path_svn.string() + " relocate " + game.URL();

                     if (!RunCommand(command, output)) {
                    BOOST_LOG_TRIVIAL(error) << "Subversion could not relocate the working copy. Details: " << output;
                    throw error(error::subversion_error, "Subversion could not relocate the working copy. Details: " + output);
                    }
                }
            }

            //Now update masterlist.
            BOOST_LOG_TRIVIAL(trace) << "Performing Subversion update of masterlist.";
            command = g_path_svn.string() + " update \"" + game.MasterlistPath().string() + "\"";
            if (!RunCommand(command, output)) {
                BOOST_LOG_TRIVIAL(error) << "Subversion could not update the masterlist. Details: " << output;
                throw error(error::subversion_error, "Subversion could not update the masterlist. Details: " + output);
            }

            bool parsingFailed = false;
            do {
                //Now get the masterlist revision.
                BOOST_LOG_TRIVIAL(trace) << "Getting the new masterlist version.";
                command = g_path_svn.string() + " info \"" + game.MasterlistPath().string() + "\"";
                if (!RunCommand(command, output)) {
                    BOOST_LOG_TRIVIAL(error) << "Subversion could not read the masterlist revision number. Details: " << output;
                    throw error(error::subversion_error, "Subversion could not read the masterlist revision number. Details: " + output);
                }

                BOOST_LOG_TRIVIAL(trace) << "Reading the masterlist version from the svn info output.";
                std::string newRevision = GetRevision(output);

                //Check if revision has changed. If it hasn't, exit early.
                if (newRevision == revision)
                    return revision;
                else
                    revision = newRevision;

                try {
                    //Now test masterlist to see if it parses OK.
                    BOOST_LOG_TRIVIAL(trace) << "Testing the new masterlist to see if it parses OK.";
                    YAML::Node mlist = YAML::LoadFile(game.MasterlistPath().string());

                    if (mlist["globals"])
                        messages = mlist["globals"].as< list<boss::Message> >();
                    if (mlist["plugins"])
                        plugins = mlist["plugins"].as< list<boss::Plugin> >();

                    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
                        it->EvalAllConditions(game, g_lang_any);
                    }

                    for (list<boss::Message>::iterator it=messages.begin(), endIt=messages.end(); it != endIt; ++it) {
                        it->EvalCondition(game, g_lang_any);
                    }

                    parsingFailed = false;

                } catch (exception& e) {
                    parsingFailed = true;

                    //Roll back one revision if there's an error.
                    BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. Masterlist revision " + revision + ": " + e.what();
                    parsingErrors.push_back("Masterlist revision " + revision + ": " + e.what());


                    command = g_path_svn.string() + " update --revision PREV \"" + game.MasterlistPath().string() + "\"";
                    if (!RunCommand(command, output)) {
                        BOOST_LOG_TRIVIAL(error) << "Subversion could not update the masterlist. Details: " << output;
                        throw error(error::subversion_error, "Subversion could not update the masterlist. Details: " + output);
                    }
                }
            } while (parsingFailed);

            return revision;
        } else {  //Git.
            /*  List of operations (porcelain commands shown, will need to implement using plumbing in the API though):

                1. Check if there is already a Git repository in the game's BOSS subfolder.

                    Since the masterlists will each be in the root of a separate repository, just check if there is a `.git` folder present.

                2a. If there is, compare its remote URL with the URL that BOSS is currently set to use.

                    The current remote can be gotten using `git config --get remote.origin.url`.


                3a. If the URLs are different, then update the remote URL to the one given by BOSS.

                    The remote URL can be changed using `git remote set-url origin <URL>`

                2b. If there isn't already a Git repo present, initialise one using the URL given (remembering to split the URL so that it ends in `.git`).

                    `git init`
                    `git remote add origin <URL>`


                3b. Now set up sparse checkout support, so that even if the repository has other files, the user's BOSS install only gets the masterlist added to it.

                    `git config core.sparseCheckout true`
                    `echo masterlist.yaml >> .git/info/sparse-checkout`

                4.  Now update the repository.

                    `git reset --hard HEAD` is required to undo any roll-backs that were done in the local repository.
                    `git pull origin master`

                5. Get the masterlist's commit hash.

                6. Test the masterlist to see if it parses OK (and evals conditions OK).

                7a. If it has errors, checkout one revision earlier.

                    `git checkout HEAD~1 masterlist.yaml`

                8a. Go back to step (5).

                7b. If it doesn't have errors, finish.

            */
            pointers_struct ptrs;
            const git_transfer_progress * stats = NULL;
            std::string httpURL;

            //If the URL is a HTTPS URL, convert it to a HTTP URL, because the build of libgit2 BOSS uses doesn't support HTTPS.
            BOOST_LOG_TRIVIAL(trace) << "Checking URL type.";
            httpURL = game.URL();
            if (boost::istarts_with(httpURL, "https")) {
                BOOST_LOG_TRIVIAL(info) << "HTTPS URL found. Converting to a HTTP URL.";
                httpURL.erase(4, 1);
            }

            BOOST_LOG_TRIVIAL(trace) << "Checking for a Git repository.";

            //Checking for a ".git" folder.
            if (fs::exists(game.MasterlistPath().parent_path() / ".git")) {
                //Repository exists. Open it.
                BOOST_LOG_TRIVIAL(trace) << "Existing repository found, attempting to open it.";
                handle_error(git_repository_open(&ptrs.repo, game.MasterlistPath().parent_path().string().c_str()), ptrs);

                BOOST_LOG_TRIVIAL(trace) << "Attempting to get info on the repository remote.";

                //Now get remote info.
                handle_error(git_remote_load(&ptrs.remote, ptrs.repo, "origin"), ptrs);

                BOOST_LOG_TRIVIAL(trace) << "Getting the remote URL.";

                //Get the remote URL.
                const char * url = git_remote_url(ptrs.remote);

                BOOST_LOG_TRIVIAL(trace) << "Checking to see if remote URL matches URL in settings.";

                //Check if the URLs match.
                if (url != httpURL) {
                    BOOST_LOG_TRIVIAL(trace) << "URLs do not match, setting repository URL to URL in settings.";
                    //The URLs don't match. Change the remote URL to match the one BOSS has.
                    handle_error(git_remote_set_url(ptrs.remote, httpURL.c_str()), ptrs);

                    //Now save change.
                    handle_error(git_remote_save(ptrs.remote), ptrs);
                }
            } else {
                BOOST_LOG_TRIVIAL(trace) << "Repository doesn't exist, initialising a new repository.";
                //Repository doesn't exist. Set up a repository.
                handle_error(git_repository_init(&ptrs.repo, game.MasterlistPath().parent_path().string().c_str(), false), ptrs);

                BOOST_LOG_TRIVIAL(trace) << "Setting the new repository's remote.";

                //Now set the repository's remote.
                handle_error(git_remote_create(&ptrs.remote, ptrs.repo, "origin", httpURL.c_str()), ptrs);

                BOOST_LOG_TRIVIAL(trace) << "Getting the repository config.";

                handle_error(git_repository_config(&ptrs.cfg, ptrs.repo), ptrs);

                BOOST_LOG_TRIVIAL(trace) << "Setting the repository up for sparse checkouts.";

                handle_error(git_config_set_bool(ptrs.cfg, "core.sparseCheckout", true), ptrs);

                //Now add the masterlist file to the list of files to be checked out. We can actually just overwrite anything that was there previously, since it's only one file.

                BOOST_LOG_TRIVIAL(trace) << "Adding the masterlist to the list of files to be checked out.";

                boss::ofstream out(game.MasterlistPath().parent_path() / ".git/info/sparse-checkout");

                out << "masterlist.yaml";

                out.close();

            }

            BOOST_LOG_TRIVIAL(trace) << "Fetching updates from remote.";

            //Now pull from the remote repository. This involves a fetch followed by a merge. First perform the fetch.

            stats = git_remote_stats(ptrs.remote);

            //Open a connection to the remote repository.

            BOOST_LOG_TRIVIAL(trace) << "Connecting to remote.";

            handle_error(git_remote_connect(ptrs.remote, GIT_DIRECTION_FETCH), ptrs);

            // Download the files needed. Skipping progress info for now, see <http://libgit2.github.com/libgit2/ex/v0.19.0/network/fetch.html> for an example of how to do that. It uses pthreads, but Boost.Thread should work fine.

            BOOST_LOG_TRIVIAL(trace) << "Downloading changes from remote.";

            handle_error(git_remote_download(ptrs.remote, NULL, NULL), ptrs);

            BOOST_LOG_TRIVIAL(info) << "Received " << stats->indexed_objects << " of " << stats->total_objects << " objects in " << stats->received_bytes << " bytes.";

            bool exitEarly = false;
            if (stats->received_bytes == 0)  //No update received.
                exitEarly = true;

            // Disconnect from the remote repository.

            BOOST_LOG_TRIVIAL(trace) << "Disconnecting from remote.";

            git_remote_disconnect(ptrs.remote);

            // Update references in case they've changed.

            BOOST_LOG_TRIVIAL(trace) << "Updating references for remote.";

            handle_error(git_remote_update_tips(ptrs.remote), ptrs);

            // Now start the merging. Not entirely sure what's going on here, but it looks like libgit2's merge API is incomplete, you can create some git_merge_head objects, but can't do anything with them...

            // Thankfully, we don't really need a merge, we just need to replace whatever's in the working directory with the relevant file from FETCH_HEAD, which was updated in the fetching step before.

            // The porcelain equivalent is `git checkout FETCH_HEAD masterlist.yaml`

            BOOST_LOG_TRIVIAL(trace) << "Setting up checkout parameters.";

            char * paths[] = { "masterlist.yaml" };

            git_checkout_opts opts = GIT_CHECKOUT_OPTS_INIT;
            opts.checkout_strategy = GIT_CHECKOUT_FORCE;  //Make sure the existing file gets overwritten.
            opts.paths.strings = paths;
            opts.paths.count = 1;

            //Next, we need to do a looping checkout / parsing check / roll-back.

            git_object_free(ptrs.obj);  //Free object since it will be reallocated in loop.

            bool parsingFailed = false;
            unsigned int rollbacks = 0;
            char revision[10];
            do {
                BOOST_LOG_TRIVIAL(trace) << "Getting the Git object for the tree at FETCH_HEAD - " << rollbacks << ".";

                //Get the commit hash so that we can report the revision if there is an error.
                string filespec = "FETCH_HEAD~" + IntToString(rollbacks);
                git_object * mlistObj;

                list<boss::Message> messages;
                list<boss::Plugin> plugins;
                handle_error(git_revparse_single(&ptrs.obj, ptrs.repo, filespec.c_str()), ptrs);

                BOOST_LOG_TRIVIAL(trace) << "Checking out the tree at FETCH_HEAD - " << rollbacks << ".";

                //Now we can do the checkout.
                handle_error(git_checkout_tree(ptrs.repo, ptrs.obj, &opts), ptrs);

                BOOST_LOG_TRIVIAL(trace) << "Getting the hash for the tree.";

                const git_oid * mlistOid = git_object_id(ptrs.obj);

                BOOST_LOG_TRIVIAL(trace) << "Converting and recording the first 10 hex characters of the hash.";

                git_oid_tostr(&revision[0], 10, mlistOid);

                BOOST_LOG_TRIVIAL(trace) << "Freeing the masterlist object.";

                git_object_free(ptrs.obj);

                if (exitEarly) {
                    ptrs.free();
                    return string(revision);
                }

                BOOST_LOG_TRIVIAL(trace) << "Testing masterlist parsing.";

                //Now try parsing the masterlist.
                try {
                    YAML::Node mlist = YAML::LoadFile(game.MasterlistPath().string());

                    if (mlist["globals"])
                        messages = mlist["globals"].as< list<boss::Message> >();
                    if (mlist["plugins"])
                        plugins = mlist["plugins"].as< list<boss::Plugin> >();

                    for (list<boss::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
                        it->EvalAllConditions(game, g_lang_any);
                    }

                    for (list<boss::Message>::iterator it=messages.begin(), endIt=messages.end(); it != endIt; ++it) {
                        it->EvalCondition(game, g_lang_any);
                    }

                    parsingFailed = false;

                } catch (std::exception& e) {
                    parsingFailed = true;
                    rollbacks++;

                    //Roll back one revision if there's an error.
                    BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. Masterlist revision " + string(revision) + ": " + e.what();

                    parsingErrors.push_back("Masterlist revision " + string(revision) + ": " + e.what());
                }
            } while (parsingFailed);

            //Finally, free memory.
            ptrs.free();

            return string(revision);
        }
    }
}
