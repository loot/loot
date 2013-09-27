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
#include <boost/locale.hpp>
#include <boost/format.hpp>

#include <git2.h>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace boss {

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

    void handle_error(int error_code, pointers_struct& pointers) {
        if (!error_code)
            return;

        const git_error * error = giterr_last();
        std::string error_message;
        if (error == NULL)
            error_message = IntToString(error_code) + ".";
        else
            error_message = IntToString(error_code) + "; " + error->message;
        pointers.free();
        giterr_clear();

        BOOST_LOG_TRIVIAL(error) << "Git operation failed. Error: " << error_message;
        throw boss::error(boss::error::git_error, (boost::format(lc::translate("Git operation failed. Error: %1%")) % error_message).str());
    }

    std::string UpdateMasterlist(Game& game, std::list<Message>& parsingErrors, std::list<Plugin>& plugins, std::list<Message>& messages) {

        /*  List of Git operations (porcelain commands shown, will need to implement using plumbing in the API though):

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
            if (url != game.URL()) {
                BOOST_LOG_TRIVIAL(trace) << "URLs do not match, setting repository URL to URL in settings.";
                //The URLs don't match. Change the remote URL to match the one BOSS has.
                handle_error(git_remote_set_url(ptrs.remote, game.URL().c_str()), ptrs);

                //Now save change.
                handle_error(git_remote_save(ptrs.remote), ptrs);
            }
        } else {
            BOOST_LOG_TRIVIAL(trace) << "Repository doesn't exist, initialising a new repository.";
            //Repository doesn't exist. Set up a repository.
            handle_error(git_repository_init(&ptrs.repo, game.MasterlistPath().parent_path().string().c_str(), false), ptrs);

            BOOST_LOG_TRIVIAL(trace) << "Setting the new repository's remote.";

            //Now set the repository's remote.
            handle_error(git_remote_create(&ptrs.remote, ptrs.repo, "origin", game.URL().c_str()), ptrs);

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

        //WARNING: This is generally a very bad idea, since it makes HTTPS a little bit pointless, but in this case because we're only reading data and not really concerned about its integrity, it's acceptable. A better solution would be to figure out why GitHub's certificate appears to be invalid to OpenSSL.
        git_remote_check_cert(ptrs.remote, 0);

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

                parsingErrors.push_back(boss::Message(boss::g_message_error, boost::locale::translate("Masterlist revision").str() + " " + string(revision) + ": " + e.what()));
            }
        } while (parsingFailed);

        //Finally, free memory.
        ptrs.free();

        return string(revision);
    }
}
