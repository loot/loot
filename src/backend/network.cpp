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

    struct git_handler {
    public:
        git_handler() : repo(NULL), remote(NULL), cfg(NULL), obj(NULL), commit(NULL), ref(NULL), sig(NULL), blob(NULL) {}

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

        void call(int error_code) {
            if (!error_code)
                return;

            const git_error * error = giterr_last();
            std::string error_message;
            if (error == NULL)
                error_message = IntToString(error_code) + ".";
            else
                error_message = IntToString(error_code) + "; " + error->message;
            free();
            giterr_clear();

            if (message.empty())
                message = "Git operation failed.";
            BOOST_LOG_TRIVIAL(error) << "Git operation failed. Error: " << error_message;
            throw loot::error(loot::error::git_error, (boost::format(lc::translate("Git operation failed. Error: %1%")) % error_message).str());
        }

        void call(int error_code, std::string& error_message) {
            message = error_message;
            call(error_code);
        }

        git_repository * repo;
        git_remote * remote;
        git_config * cfg;
        git_object * obj;
        git_commit * commit;
        git_reference * ref;
        git_signature * sig;
        git_blob * blob;

        std::string message;
    };

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
            git_handler git;
            BOOST_LOG_TRIVIAL(debug) << "Existing repository found, attempting to open it.";
            git.call(git_repository_open(&git.repo, game.MasterlistPath().parent_path().string().c_str()));

            BOOST_LOG_TRIVIAL(trace) << "Getting HEAD masterlist object.";
            git.call(git_revparse_single(&git.obj, git.repo, "HEAD:masterlist.yaml"));

            BOOST_LOG_TRIVIAL(trace) << "Getting blob for masterlist object.";
            git.call(git_blob_lookup(&git.blob, git.repo, git_object_id(git.obj)));

            BOOST_LOG_TRIVIAL(debug) << "Opening masterlist in working directory.";
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

            BOOST_LOG_TRIVIAL(debug) << "Comparing files.";
            if (are_files_equal(git_blob_rawcontent(git.blob), git_blob_rawsize(git.blob), mlist.data(), mlist.length())) {
                char revision[10];
                //Need to get the HEAD object, because the individual file has a different SHA.
                git_object_free(git.obj);
                git.obj = NULL;  //Just to be safe.
                BOOST_LOG_TRIVIAL(trace) << "Getting HEAD object revision SHA.";
                git.call(git_revparse_single(&git.obj, git.repo, "HEAD"));
                git_oid_tostr(revision, 10, git_object_id(git.obj));
                git.free();
                return string(revision);
            }
            else {
                git.free();
                return "Unknown: Masterlist edited";
            }
        }
    }

    std::pair<std::string, std::string> UpdateMasterlist(Game& game, std::list<Message>& parsingErrors, std::list<Plugin>& plugins, std::list<Message>& messages) {
        git_handler git;

        BOOST_LOG_TRIVIAL(debug) << "Checking for a Git repository.";

        //Checking for a ".git" folder.
        if (fs::exists(game.MasterlistPath().parent_path() / ".git")) {
            //Repository exists. Open it.
            BOOST_LOG_TRIVIAL(info) << "Existing repository found, attempting to open it.";
            git.call(git_repository_open(&git.repo, game.MasterlistPath().parent_path().string().c_str()));

            BOOST_LOG_TRIVIAL(trace) << "Attempting to get info on the repository remote.";

            //Now get remote info.
            git.call(git_remote_load(&git.remote, git.repo, "origin"));

            BOOST_LOG_TRIVIAL(trace) << "Getting the remote URL.";

            //Get the remote URL.
            const char * url = git_remote_url(git.remote);

            BOOST_LOG_TRIVIAL(info) << "Checking to see if remote URL matches URL in settings.";

            //Check if the repo URLs match.
            BOOST_LOG_TRIVIAL(info) << "Remote URL given: " << game.RepoURL();
            BOOST_LOG_TRIVIAL(info) << "Remote URL in repository settings: " << url;
            if (url != game.RepoURL()) {
                BOOST_LOG_TRIVIAL(info) << "URLs do not match, setting repository URL to URL in settings.";
                //The URLs don't match. Change the remote URL to match the one LOOT has.
                git.call(git_remote_set_url(git.remote, game.RepoURL().c_str()));

                //Now save change.
                git.call(git_remote_save(git.remote));
            }
        } else {
            BOOST_LOG_TRIVIAL(info) << "Repository doesn't exist, initialising a new repository.";
            //Repository doesn't exist. Set up a repository.
            git.call(git_repository_init(&git.repo, game.MasterlistPath().parent_path().string().c_str(), false));

            BOOST_LOG_TRIVIAL(info) << "Setting the new repository's remote to: " << game.RepoURL();

            //Now set the repository's remote.
            git.call(git_remote_create(&git.remote, git.repo, "origin", game.RepoURL().c_str()));
        }

        //WARNING: This is generally a very bad idea, since it makes HTTPS a little bit pointless, but in this case because we're only reading data and not really concerned about its integrity, it's acceptable. A better solution would be to figure out why GitHub's certificate appears to be invalid to OpenSSL.
#ifndef _WIN32
		git_remote_check_cert(git.remote, 0);
#endif

        BOOST_LOG_TRIVIAL(trace) << "Fetching updates from remote.";

        //Now pull from the remote repository. This involves a fetch followed by a merge. First perform the fetch.

        //Fetch from remote.
        BOOST_LOG_TRIVIAL(trace) << "Fetching from remote.";
        git.call(git_remote_fetch(git.remote));

        const git_transfer_progress * stats = git_remote_stats(git.remote);
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
        //git.call(git_signature_default(&git.sig, git.repo));

        bool parsingFailed = false;
        unsigned int rollbacks = 0;
        string revision, date;
        do {
            string filespec = "refs/remotes/origin/" + game.RepoBranch() + "~" + IntToString(rollbacks);
            BOOST_LOG_TRIVIAL(info) << "Getting the Git object for the tree at " << filespec;
            git.call(git_revparse_single(&git.obj, git.repo, filespec.c_str()));

            BOOST_LOG_TRIVIAL(trace) << "Getting the Git object ID.";
            const git_oid * oid = git_object_id(git.obj);

            BOOST_LOG_TRIVIAL(trace) << "Generating hex string for Git object ID.";
            char sha1[10];
            git_oid_tostr(sha1, 10, oid);
            revision = sha1;

            BOOST_LOG_TRIVIAL(trace) << "Getting date for Git object ID.";
            git.call(git_commit_lookup(&git.commit, git.repo, oid));
            git_time_t time = git_commit_time(git.commit);
            boost::locale::date_time dateTime(time);
            stringstream out;
            out << boost::locale::as::ftime("%Y-%m-%d") << dateTime;
            date = out.str();

            BOOST_LOG_TRIVIAL(trace) << "Recreating HEAD as a direct reference (overwriting it) to the desired revision.";
            git.call(git_reference_create(&git.ref, git.repo, "HEAD", oid, 1));

            BOOST_LOG_TRIVIAL(trace) << "Performing a Git checkout of HEAD.";
            git.call(git_checkout_head(git.repo, &opts));

            BOOST_LOG_TRIVIAL(debug) << "Tree hash is: " << revision;
            BOOST_LOG_TRIVIAL(trace) << "Freeing pointers.";
            git_object_free(git.obj);
            git_reference_free(git.ref);
            git_commit_free(git.commit);
            git.obj = NULL;
            git.ref = NULL;
            git.commit = NULL;

            BOOST_LOG_TRIVIAL(debug) << "Testing masterlist parsing.";

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
                    it->EvalAllConditions(game, Language::any);
                }

                for (list<loot::Message>::iterator it=messages.begin(), endIt=messages.end(); it != endIt; ++it) {
                    it->EvalCondition(game, Language::any);
                }

                parsingFailed = false;

            } catch (std::exception& e) {
                parsingFailed = true;
                rollbacks++;

                //Roll back one revision if there's an error.
                BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. Masterlist revision " + string(revision) + ": " + e.what();

                parsingErrors.push_back(loot::Message(loot::Message::error, boost::locale::translate("Masterlist revision").str() + " " + string(revision) + ": " + e.what() + " " + boost::locale::translate("Rolled back to the previous revision.").str()));
            }
        } while (parsingFailed);

        //Finally, free memory.
        git.free();

        return pair<string, string>(revision, date);
    }
}
