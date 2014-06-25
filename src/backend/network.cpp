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
        git_handler() : repo(nullptr), remote(nullptr), cfg(nullptr), obj(nullptr), commit(nullptr), ref(nullptr), sig(nullptr), blob(nullptr) {}

        ~git_handler() {
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

            const git_error * last_error = giterr_last();
            std::string error_message;
            if (last_error == nullptr)
                error_message = to_string(error_code) + ".";
            else
                error_message = to_string(error_code) + "; " + last_error->message;
            giterr_clear();

            if (ui_message.empty())
                ui_message = (boost::format(lc::translate("Git operation failed. Error: %1%")) % error_message).str();

            BOOST_LOG_TRIVIAL(error) << "Git operation failed. Error: " << error_message;
            throw loot::error(loot::error::git_error, ui_message);
        }

        git_repository * repo;
        git_remote * remote;
        git_config * cfg;
        git_object * obj;
        git_commit * commit;
        git_reference * ref;
        git_signature * sig;
        git_blob * blob;

        std::string ui_message;
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

    bool isRepository(const fs::path& path) {
        return git_repository_open_ext(NULL, path.string().c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0;
    }

    std::pair<string, string> GetMasterlistRevision(const Game& game) {
        if (!fs::exists(game.MasterlistPath().parent_path() / ".git")) {
            return pair<string, string>("Unknown: Git repository missing", "Unknown: Git repository missing");
        }
        else if (!fs::exists(game.MasterlistPath()))
            return pair<string, string>("N/A: No masterlist present", "N/A: No masterlist present");
        else {
            /* Compares HEAD to the working dir.
                1. Get an object for the masterlist in HEAD.
                2. Get the blob for that object.
                3. Open the masterlist file in the working dir in a file buffer.
                4. Compare the file and blob buffers.
            */
            git_handler git;
            git.ui_message = "An error occurred while trying to read the local masterlist's version. If this error happens again, try deleting the \".git\" folder in \"%LOCALAPPDATA%\\LOOT\\" + game.FolderName() + "\".";
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

                string revision, date;
                //Need to get the HEAD object, because the individual file has a different SHA.
                git_object_free(git.obj);
                git.obj = nullptr;  //Just to be safe.
                BOOST_LOG_TRIVIAL(info) << "Getting the Git object for the tree at HEAD.";
                git.call(git_revparse_single(&git.obj, git.repo, "HEAD"));

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

                return pair<string, string>(revision, date);
            }
            else {
                return pair<string, string>("Unknown: Masterlist edited", "Unknown: Masterlist edited");
            }
        }
    }

    std::pair<std::string, std::string> UpdateMasterlist(Game& game, std::list<Message>& parsingErrors, std::list<Plugin>& plugins, std::list<Message>& messages) {
        git_handler git;
        fs::path repo_path = game.MasterlistPath().parent_path();
        string repo_branch = game.RepoBranch();

        // First initialise some stuff that isn't specific to a repository.
        BOOST_LOG_TRIVIAL(debug) << "Creating a reflog signature to use.";
        // Create a reflog signature for any changes.
        git.call(git_signature_new(&git.sig, "LOOT", "loot@placeholder.net", 0, 0));

        BOOST_LOG_TRIVIAL(debug) << "Setting up checkout options.";
        char * paths[] = { "masterlist.yaml" };
        git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
        checkout_opts.paths.strings = paths;
        checkout_opts.paths.count = 1;

        // Now try to access the repository if it exists, or clone one if it doesn't.
        git.ui_message = "An error occurred while trying to access the local masterlist repository.";

        if (!isRepository(repo_path)) {
            // Clone the remote repository.
            BOOST_LOG_TRIVIAL(info) << "Repository doesn't exist, cloning the remote repository.";

            bool wasEmpty = true;
            if (!fs::is_empty(repo_path)) {
                // Now, libgit2 doesn't support cloning into non-empty folders. Rename the folder 
                // temporarily, and move its contents back in afterwards, skipping any that then conflict.
                BOOST_LOG_TRIVIAL(trace) << "Repo path not empty, renaming folder.";
                fs::rename(repo_path, repo_path.string() + ".temp");
                wasEmpty = false;
            }

            //First set up clone options.

            git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;
            clone_options.checkout_opts = checkout_opts;
            clone_options.bare = 0;
            clone_options.checkout_branch = repo_branch.c_str();
            clone_options.signature = git.sig;
#ifndef _WIN32
            //OpenSSL doesn't seem to like GitHub's certificate.
            clone_options.ignore_cert_errors = 1;
#endif

            //Now perform the clone.
            git.call(git_clone(&git.repo, game.RepoURL().c_str(), repo_path.string().c_str(), &clone_options));

            if (!wasEmpty) {
                //Move contents back in.
                BOOST_LOG_TRIVIAL(trace) << "Repo path wasn't empty, moving previous files back in.";
                for (fs::directory_iterator it(repo_path.string() + ".temp"); it != fs::directory_iterator(); ++it) {
                    if (!fs::exists(repo_path / it->path().filename())) {
                        //No conflict, OK to move back in.
                        fs::rename(it->path(), repo_path / it->path().filename());
                    }
                }
                //Delete temporary folder.
                fs::remove_all(repo_path.string() + ".temp");
            }
        }
        else {
            // Repository exists: check settings are correct, then pull updates.

            // Open the repository.
            BOOST_LOG_TRIVIAL(info) << "Existing repository found, attempting to open it.";
            git.call(git_repository_open(&git.repo, repo_path.string().c_str()));

            // Check that the repository's remote settings match LOOT's.
            BOOST_LOG_TRIVIAL(info) << "Checking to see if remote URL matches URL in settings.";
            git.call(git_remote_load(&git.remote, git.repo, "origin"));
            const char * url = git_remote_url(git.remote);

            BOOST_LOG_TRIVIAL(info) << "Remote URL given: " << game.RepoURL();
            BOOST_LOG_TRIVIAL(info) << "Remote URL in repository settings: " << url;
            if (url != game.RepoURL()) {
                BOOST_LOG_TRIVIAL(info) << "URLs do not match, setting repository URL to URL in settings.";
                // The URLs don't match. Change the remote URL to match the one LOOT has.
                git.call(git_remote_set_url(git.remote, game.RepoURL().c_str()));

                // Now save change.
                git.call(git_remote_save(git.remote));
            }

            // Now fetch updates from the remote.
            BOOST_LOG_TRIVIAL(trace) << "Fetching updates from remote.";
            git.ui_message = "An error occurred while trying to update the masterlist. This could be due to a server-side error. Try again in a few minutes.";

            git.call(git_remote_fetch(git.remote, git.sig, nullptr));

            // Print some stats on what was fetched either during update or clone.
            const git_transfer_progress * stats = git_remote_stats(git.remote);
            BOOST_LOG_TRIVIAL(info) << "Received " << stats->indexed_objects << " of " << stats->total_objects << " objects in " << stats->received_bytes << " bytes.";

            // Check that a branch with the correct name exists.
            int ret = git_branch_lookup(&git.ref, git.repo, repo_branch.c_str(), GIT_BRANCH_LOCAL);
            if (ret == GIT_ENOTFOUND) {
                // Branch doesn't exist. Create a new branch using the remote branch's latest commit.

                BOOST_LOG_TRIVIAL(trace) << "Looking up commit referred to by the remote branch \"" << repo_branch << "\".";
                git.call(git_revparse_single(&git.obj, git.repo, (string("origin/") + repo_branch).c_str()));
                const git_oid * commit_id = git_object_id(git.obj);

                BOOST_LOG_TRIVIAL(trace) << "Creating the new branch.";
                
                // Create a branch.
                git.call(git_commit_lookup(&git.commit, git.repo, commit_id));
                git.call(git_branch_create(&git.ref, git.repo, repo_branch.c_str(), git.commit, 0, git.sig, NULL));

                // Set upstream. Don't really know if this is necessary or not.
                git.call(git_branch_set_upstream(git.ref, (string("origin/") + repo_branch).c_str()));

                BOOST_LOG_TRIVIAL(trace) << "Setting the upstream for the new branch.";
                
                // Free tree and commit pointers. Reference pointer is still used below.
                git_object_free(git.obj);
                git_commit_free(git.commit);
                git.commit = nullptr;
                git.obj = nullptr;

                BOOST_LOG_TRIVIAL(trace) << "Done creating the new branch.";
            }
            else if (ret != 0)
                git.call(ret);  // Handle other errors.

            // Check if HEAD points to the desired branch and set it to if not.
            if (!git_branch_is_head(git.ref)) {
                BOOST_LOG_TRIVIAL(trace) << "Setting HEAD to follow branch: " << repo_branch;
                git.call(git_repository_set_head(git.repo, (string("refs/heads/") + repo_branch).c_str(), git.sig, (string("Updated HEAD to ") + repo_branch).c_str()));
            }

            if (ret == 0) {
                /* The branch did exist, and is now pointed at by HEAD.
                   Need to merge the remote branch into it.
                  
                   1. git_merge_head_from_fetchhead to get the remote branch object to merge with.
                   2. git_merge to merge the remote branch object into HEAD, which is set to be
                      the desired branch.Docs say a commit may be necessary after this.
                */
            }

            // Free branch pointer.
            git_reference_free(git.ref);
            git.ref = nullptr;

            BOOST_LOG_TRIVIAL(trace) << "Performing a Git checkout of HEAD.";
            git.call(git_checkout_head(git.repo, &checkout_opts));

        }

        // Now whether the repository was cloned or updated, the working directory contains
        // the latest masterlist. Try parsing it: on failure, detach the HEAD back one commit
        // and try again.

        bool parsingFailed = false;
        string revision, date;
        git.ui_message = "An error occurred while trying to read information on the updated masterlist. If this error happens again, try deleting the \".git\" folder in \"%LOCALAPPDATA%\\LOOT\\" + game.FolderName() + "\".";
        do {
            // Get some descriptive info about what was checked out.

            BOOST_LOG_TRIVIAL(trace) << "Getting the Git object for HEAD.";
            git.call(git_repository_head(&git.ref, git.repo));
            git.call(git_reference_peel(&git.obj, git.ref, GIT_OBJ_COMMIT));
            const git_oid * oid = git_object_id(git.obj);

            BOOST_LOG_TRIVIAL(trace) << "Generating hex string for the Git object.";
            // git_object_short_id seems to be unstable, or I'm just not freeing memory right somewhere, I can't tell.
            /*git_buf buffer;
            git.call(git_object_short_id(&buffer, git.obj));
            revision = string(buffer.ptr, buffer.size);
            git_buf_free(&buffer);*/
            char sha1[10];
            git_oid_tostr(sha1, 10, oid);
            revision = sha1;

            BOOST_LOG_TRIVIAL(trace) << "Getting date for Git object ID.";
            git.call(git_commit_lookup(&git.commit, git.repo, oid));
            git_time_t time = git_commit_time(git.commit);
            // Now convert into a nice text format.
            boost::locale::date_time dateTime(time);
            stringstream out;
            out << boost::locale::as::ftime("%Y-%m-%d") << dateTime;
            date = out.str();

            BOOST_LOG_TRIVIAL(debug) << "Set HEAD to commit (date): " << revision << " (" << date << ").";
            BOOST_LOG_TRIVIAL(trace) << "Freeing pointers.";
            git_reference_free(git.ref);
            git_object_free(git.obj);
            git_commit_free(git.commit);
            git.ref = nullptr;
            git.obj = nullptr;
            git.commit = nullptr;

            //Now try parsing the masterlist.
            BOOST_LOG_TRIVIAL(debug) << "Testing masterlist parsing.";
            try {
                loot::ifstream in(game.MasterlistPath());
                YAML::Node mlist = YAML::Load(in);
                in.close();

                if (mlist["globals"])
                    messages = mlist["globals"].as< list<loot::Message> >();
                if (mlist["plugins"])
                    plugins = mlist["plugins"].as< list<loot::Plugin> >();

                for (auto &plugin: plugins) {
                    plugin.EvalAllConditions(game, Language::any);
                }

                for (auto &message: messages) {
                    message.EvalCondition(game, Language::any);
                }

                parsingFailed = false;

            } catch (std::exception& e) {
                parsingFailed = true;

                //Roll back one revision if there's an error.
                BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. Masterlist revision " + string(revision) + ": " + e.what();

                // Get an object ID for 'HEAD~1'.
                git.call(git_revparse_single(&git.obj, git.repo, "HEAD~1"));
                const git_oid * oid = git_object_id(git.obj);
                git_object_free(git.obj);
                git.obj = nullptr;

                // Detach HEAD to HEAD~1. This will roll back HEAD by one commit each time it is called.
                git.call(git_repository_set_head_detached(git.repo, oid, git.sig, "Updated HEAD to HEAD~1"));

                // Checkout the new HEAD.
                BOOST_LOG_TRIVIAL(trace) << "Performing a Git checkout of HEAD.";
                git.call(git_checkout_head(git.repo, &checkout_opts));

                parsingErrors.push_back(loot::Message(loot::Message::error, boost::locale::translate("Masterlist revision").str() + " " + string(revision) + ": " + e.what() + " " + boost::locale::translate("Rolled back to the previous revision.").str()));
            }
        } while (parsingFailed);

        return pair<string, string>(revision, date);
    }
}
