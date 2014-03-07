/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2014    WrinklyNinja

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

namespace loot {

    struct pointers_struct {
        pointers_struct() : repo(NULL), remote(NULL), cfg(NULL), obj(NULL), commit(NULL), ref(NULL), sig(NULL), blob(NULL) {}

        void free() {
            git_commit_free(commit);
            git_object_free(obj);
            git_config_free(cfg);
            git_remote_free(remote);
            git_repository_free(repo);
            git_reference_free(ref);
            git_signature_free(sig);
            git_blob_free(blob);
        }

        git_repository * repo;
        git_remote * remote;
        git_config * cfg;
        git_object * obj;
        git_commit * commit;
        git_reference * ref;
        git_signature * sig;
        git_blob * blob;
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
        throw loot::error(loot::error::git_error, (boost::format(lc::translate("Git operation failed. Error: %1%")) % error_message).str());
    }

	int progress_cb(const char *str, int len, void *data) {
		BOOST_LOG_TRIVIAL(info) << string(str, len);
		return 0;
	}

    bool are_files_equal(const void * buf1, size_t buf1_size, const void * buf2, size_t buf2_size) {
        if (buf1_size != buf2_size)
            return false;

        size_t pos = 0;
        while (pos < buf1_size) {
            if (*((char*)buf1 + pos) != *((char*)buf2 + pos))
                return false;
            ++pos;
        }
        return true;
    }

    std::string GetMasterlistRevision(const Game& game) {
        if (!fs::exists(game.MasterlistPath().parent_path() / ".git")) {
            return "Unknown: Git repository missing";
        }
        else if (!fs::exists(game.MasterlistPath()))
            return "N/A: No masterlist present";
        else {
            /* Compares HEAD to the working dir.
                1. Get an object for the masterlist in HEAD.
                2. Get the blob for that object.
                3. Open the masterlist file in the working dir in a file buffer.
                4. Compare the file and blob buffers.
            */
            pointers_struct ptrs;
            BOOST_LOG_TRIVIAL(trace) << "Existing repository found, attempting to open it.";
            handle_error(git_repository_open(&ptrs.repo, game.MasterlistPath().parent_path().string().c_str()), ptrs);

            BOOST_LOG_TRIVIAL(trace) << "Getting HEAD masterlist object.";
            handle_error(git_revparse_single(&ptrs.obj, ptrs.repo, "HEAD:masterlist.yaml"), ptrs);

            BOOST_LOG_TRIVIAL(trace) << "Getting blob for masterlist object.";
            handle_error(git_blob_lookup(&ptrs.blob, ptrs.repo, git_object_id(ptrs.obj)), ptrs);

            BOOST_LOG_TRIVIAL(trace) << "Opening masterlist in working directory.";
            std::string mlist;
            loot::ifstream ifile(game.MasterlistPath().string().c_str(), ios::binary);
            if (ifile.fail())
                throw error(error::path_read_fail, "Couldn't open masterlist.");
            ifile.unsetf(ios::skipws); // No white space skipping!
            copy(
                istream_iterator<char>(ifile),
                istream_iterator<char>(),
                back_inserter(mlist)
                );

            BOOST_LOG_TRIVIAL(trace) << "Comparing files.";
            if (are_files_equal(git_blob_rawcontent(ptrs.blob), git_blob_rawsize(ptrs.blob), mlist.data(), mlist.length())) {
                char revision[10];
                //Need to get the HEAD object, because the individual file has a different SHA.
                git_object_free(ptrs.obj);
                BOOST_LOG_TRIVIAL(trace) << "Getting HEAD object revision SHA.";
                handle_error(git_revparse_single(&ptrs.obj, ptrs.repo, "HEAD"), ptrs);
                git_oid_tostr(revision, 10, git_object_id(ptrs.obj));
                ptrs.free();
                return string(revision);
            }
            else {
                ptrs.free();
                return "Unknown: Masterlist edited";
            }
        }
    }

    std::string UpdateMasterlist(Game& game, std::list<Message>& parsingErrors, std::list<Plugin>& plugins, std::list<Message>& messages) {
        pointers_struct ptrs;

        BOOST_LOG_TRIVIAL(trace) << "Checking for a Git repository.";

        //Checking for a ".git" folder.
        if (fs::exists(game.MasterlistPath().parent_path() / ".git")) {
            //Repository exists. Open it.
            BOOST_LOG_TRIVIAL(info) << "Existing repository found, attempting to open it.";
            handle_error(git_repository_open(&ptrs.repo, game.MasterlistPath().parent_path().string().c_str()), ptrs);

            BOOST_LOG_TRIVIAL(trace) << "Attempting to get info on the repository remote.";

            //Now get remote info.
            handle_error(git_remote_load(&ptrs.remote, ptrs.repo, "origin"), ptrs);

            BOOST_LOG_TRIVIAL(trace) << "Getting the remote URL.";

            //Get the remote URL.
            const char * url = git_remote_url(ptrs.remote);

            BOOST_LOG_TRIVIAL(trace) << "Checking to see if remote URL matches URL in settings.";

            //Check if the repo URLs match.
            BOOST_LOG_TRIVIAL(info) << "Remote URL given: " << game.RepoURL();
            BOOST_LOG_TRIVIAL(info) << "Remote URL in repository settings: " << url;
            if (url != game.RepoURL()) {
                BOOST_LOG_TRIVIAL(trace) << "URLs do not match, setting repository URL to URL in settings.";
                //The URLs don't match. Change the remote URL to match the one LOOT has.
                handle_error(git_remote_set_url(ptrs.remote, game.RepoURL().c_str()), ptrs);

                //Now save change.
                handle_error(git_remote_save(ptrs.remote), ptrs);
            }
        } else {
            BOOST_LOG_TRIVIAL(info) << "Repository doesn't exist, initialising a new repository.";
            //Repository doesn't exist. Set up a repository.
            handle_error(git_repository_init(&ptrs.repo, game.MasterlistPath().parent_path().string().c_str(), false), ptrs);

            BOOST_LOG_TRIVIAL(info) << "Setting the new repository's remote to: " << game.RepoURL();

            //Now set the repository's remote.
            handle_error(git_remote_create(&ptrs.remote, ptrs.repo, "origin", game.RepoURL().c_str()), ptrs);
        }

        //WARNING: This is generally a very bad idea, since it makes HTTPS a little bit pointless, but in this case because we're only reading data and not really concerned about its integrity, it's acceptable. A better solution would be to figure out why GitHub's certificate appears to be invalid to OpenSSL.
#ifndef _WIN32
		git_remote_check_cert(ptrs.remote, 0);
#endif

        BOOST_LOG_TRIVIAL(trace) << "Fetching updates from remote.";

        //Now pull from the remote repository. This involves a fetch followed by a merge. First perform the fetch.

        //Fetch from remote.
        BOOST_LOG_TRIVIAL(trace) << "Fetching from remote.";
        handle_error(git_remote_fetch(ptrs.remote), ptrs);

        const git_transfer_progress * stats = git_remote_stats(ptrs.remote);
        BOOST_LOG_TRIVIAL(info) << "Received " << stats->indexed_objects << " of " << stats->total_objects << " objects in " << stats->received_bytes << " bytes.";

        //Now do a forced checkout of masterlist.yaml from the desired branch.
        BOOST_LOG_TRIVIAL(trace) << "Setting up checkout parameters.";

        char * paths[] = { "masterlist.yaml" };

        git_checkout_opts opts = GIT_CHECKOUT_OPTS_INIT;
        opts.checkout_strategy = GIT_CHECKOUT_FORCE;  //Make sure the existing file gets overwritten.
        opts.paths.strings = paths;
        opts.paths.count = 1;

        //Next, we need to do a looping checkout / parsing check / roll-back.
        /* Here's what to do:
        0. Create a git_signature using git_signature_default.
        1. Get the git_object for the desired masterlist revision, using git_revparse_single.
        2. Get the git_oid for that object, using git_object_id.
        3. Get the git_reference for the HEAD reference using git_reference_lookup.
        5. Generate a short string for the git_oid, to display in the log.
        6. (Re)create a HEAD reference to point directly to the desired masterlist revision,
        using its git_oid and git_reference_create.
        7. Perform the checkout of HEAD.
        */

        //Apparently I'm using libgit2's head, not v0.20.0, so I don't need this...
        //LOG_INFO("Creating a Git signature.");
        //handle_error(git_signature_default(&ptrs.sig, ptrs.repo), ptrs);

        bool parsingFailed = false;
        unsigned int rollbacks = 0;
        char revision[10];
        do {
            string filespec = "refs/remotes/origin/" + game.RepoBranch() + "~" + IntToString(rollbacks);
            BOOST_LOG_TRIVIAL(trace) << "Getting the Git object for the tree at " << filespec;
            handle_error(git_revparse_single(&ptrs.obj, ptrs.repo, filespec.c_str()), ptrs);

            BOOST_LOG_TRIVIAL(trace) << "Getting the Git object ID.";
            const git_oid * oid = git_object_id(ptrs.obj);

            BOOST_LOG_TRIVIAL(trace) << "Generating hex string for Git object ID.";
            git_oid_tostr(revision, 10, oid);

            BOOST_LOG_TRIVIAL(trace) << "Recreating HEAD as a direct reference (overwriting it) to the desired revision.";
            handle_error(git_reference_create(&ptrs.ref, ptrs.repo, "HEAD", oid, 1), ptrs);

            BOOST_LOG_TRIVIAL(trace) << "Performing a Git checkout of HEAD.";
            handle_error(git_checkout_head(ptrs.repo, &opts), ptrs);

            BOOST_LOG_TRIVIAL(trace) << "Tree hash is: " << revision;
            BOOST_LOG_TRIVIAL(trace) << "Freeing pointers.";
            git_object_free(ptrs.obj);
            git_reference_free(ptrs.ref);

            BOOST_LOG_TRIVIAL(trace) << "Testing masterlist parsing.";

            //Now try parsing the masterlist.
            list<loot::Message> messages;
            list<loot::Plugin> plugins;
            try {
                loot::ifstream in(game.MasterlistPath());
                YAML::Node mlist = YAML::Load(in);
                in.close();

                if (mlist["globals"])
                    messages = mlist["globals"].as< list<loot::Message> >();
                if (mlist["plugins"])
                    plugins = mlist["plugins"].as< list<loot::Plugin> >();

                for (list<loot::Plugin>::iterator it=plugins.begin(), endIt=plugins.end(); it != endIt; ++it) {
                    it->EvalAllConditions(game, g_lang_any);
                }

                for (list<loot::Message>::iterator it=messages.begin(), endIt=messages.end(); it != endIt; ++it) {
                    it->EvalCondition(game, g_lang_any);
                }

                parsingFailed = false;

            } catch (std::exception& e) {
                parsingFailed = true;
                rollbacks++;

                //Roll back one revision if there's an error.
                BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. Masterlist revision " + string(revision) + ": " + e.what();

                parsingErrors.push_back(loot::Message(loot::g_message_error, boost::locale::translate("Masterlist revision").str() + " " + string(revision) + ": " + e.what() + " " + boost::locale::translate("Rolled back to the previous revision.").str()));
            }
        } while (parsingFailed);

        //Finally, free memory.
        ptrs.free();

        return string(revision);
    }
}
