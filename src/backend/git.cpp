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

#include "error.h"
#include "parsers.h"
#include "streams.h"
#include "helpers.h"
#include "game.h"

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
        git_handler() : repo(nullptr), remote(nullptr), cfg(nullptr), obj(nullptr), commit(nullptr), ref(nullptr), ref2(nullptr), sig(nullptr), blob(nullptr), merge_head(nullptr), tree(nullptr), diff(nullptr) {}

        ~git_handler() {
            git_commit_free(commit);
            git_object_free(obj);
            git_config_free(cfg);
            git_remote_free(remote);
            git_repository_free(repo);
            git_reference_free(ref);
            git_reference_free(ref2);
            git_signature_free(sig);
            git_blob_free(blob);
            git_merge_head_free(merge_head);
            git_tree_free(tree);
            git_diff_free(diff);
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
        git_reference * ref2;
        git_signature * sig;
        git_blob * blob;
        git_merge_head * merge_head;
        git_tree * tree;
        git_diff * diff;


        std::string ui_message;
    };

    bool isRepository(const fs::path& path) {
        return git_repository_open_ext(NULL, path.string().c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0;
    }

    // Removes the read-only flag from some files in git repositories created by libgit2.
    void FixRepoPermissions(const fs::path& path) {
        BOOST_LOG_TRIVIAL(trace) << "Recursively setting write permission on directory: " << path;
        for (fs::recursive_directory_iterator it(path); it != fs::recursive_directory_iterator(); ++it) {
            BOOST_LOG_TRIVIAL(trace) << "Setting write permission for: " << it->path();
            fs::permissions(it->path(), fs::add_perms | fs::owner_write);
        }
    }

    int diffFileCallback(const git_diff_delta *delta, float progress, void * payload) {
        BOOST_LOG_TRIVIAL(trace) << "Checking diff for: " << delta->old_file.path;
        if (strcmp(delta->old_file.path, "masterlist.yaml") == 0) {
            BOOST_LOG_TRIVIAL(warning) << "Edited masterlist found.";
            *(bool*)payload = true;
        }

        return 0;
    }

    void Masterlist::GetGitInfo(const boost::filesystem::path& path) {
        if (!isRepository(path.parent_path())) {
            revision = "Unknown: Git repository missing";
            date = "Unknown: Git repository missing";
        }
        else if (!fs::exists(path)) {
            revision = "N/A: No masterlist present";
            date = "N/A: No masterlist present";
        }
        else {
            // Perform a git diff, then iterate the deltas to see if one exists for masterlist.yaml.
            git_handler git;
            git.ui_message = "An error occurred while trying to read the local masterlist's version. If this error happens again, try deleting the \".git\" folder in " + path.parent_path().string() + ".";
            BOOST_LOG_TRIVIAL(debug) << "Existing repository found, attempting to open it.";
            git.call(git_repository_open(&git.repo, path.parent_path().string().c_str()));

            BOOST_LOG_TRIVIAL(trace) << "Getting the tree for the HEAD revision.";
            git.call(git_revparse_single(&git.obj, git.repo, "HEAD^{tree}"));
            git.call(git_tree_lookup(&git.tree, git.repo, git_object_id(git.obj)));

            BOOST_LOG_TRIVIAL(trace) << "Performing git diff.";
            git.call(git_diff_tree_to_workdir_with_index(&git.diff, git.repo, git.tree, NULL));

            BOOST_LOG_TRIVIAL(trace) << "Iterating over git diff deltas.";
            bool isMasterlistDifferent = false;
            git.call(git_diff_foreach(git.diff, &diffFileCallback, NULL, NULL, &isMasterlistDifferent));

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

            if (isMasterlistDifferent) {
                revision += " (edited)";
                date += " (edited)";
            }
        }
    }

    bool Masterlist::Update(Game& game, const unsigned int language) {
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
        BOOST_LOG_TRIVIAL(trace) << "Attempting to open the Git repository at: " << repo_path;
        if (!isRepository(repo_path)) {
            git.ui_message = "An error occurred while trying to clone the remote masterlist repository.";
            // Clone the remote repository.
            BOOST_LOG_TRIVIAL(info) << "Repository doesn't exist, cloning the remote repository.";

            bool wasEmpty = true;
            fs::path temp_path = repo_path.string() + ".temp";
            if (!fs::is_empty(repo_path)) {
                // Clear any read-only flags first.
                FixRepoPermissions(repo_path);
                // Now, libgit2 doesn't support cloning into non-empty folders. Rename the folder 
                // temporarily, and move its contents back in afterwards, skipping any that then conflict.
                BOOST_LOG_TRIVIAL(trace) << "Repo path not empty, renaming folder.";
                // If the temp path already exists, it needs to be deleted.
                if (fs::exists(temp_path)) {
                    FixRepoPermissions(temp_path);
                    fs::remove_all(temp_path);
                }
                // There's no point moving the .git folder, so delete that.
                fs::remove_all(repo_path / ".git");
                // Now move to temp path.
                fs::rename(repo_path, temp_path);
                // Recreate the game folder so that we don't inadvertently cause any other errors (everything past LOOT init assumes it exists).
                fs::create_directory(repo_path);
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

            if (fs::exists(temp_path)) {
                //Move contents back in.
                BOOST_LOG_TRIVIAL(trace) << "Repo path wasn't empty, moving previous files back in.";
                for (fs::directory_iterator it(temp_path); it != fs::directory_iterator(); ++it) {
                    if (!fs::exists(repo_path / it->path().filename())) {
                        //No conflict, OK to move back in.
                        fs::rename(it->path(), repo_path / it->path().filename());
                    }
                }
                //Delete temporary folder.
                fs::remove_all(temp_path);
            }
        }
        else {
            // Repository exists: check settings are correct, then pull updates.
            git.ui_message = "An error occurred while trying to access the local masterlist repository. If this error happens again, try deleting the \".git\" folder in \"%LOCALAPPDATA%\\LOOT\\" + game.FolderName() + "\".";

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
            git.ui_message = "An error occurred while trying to access the local masterlist repository. If this error happens again, try deleting the \".git\" folder in \"%LOCALAPPDATA%\\LOOT\\" + game.FolderName() + "\".";
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
                   Need to merge the remote branch into it. Just do a fast-forward merge because
                   that's all that should be necessary as the local repo shouldn't get changed by
                   the user.
                */

                BOOST_LOG_TRIVIAL(trace) << "Checking that local and remote branches can be merged by fast-forward.";
                git_merge_analysis_t analysis;
                git_merge_preference_t pref;
                git.call(git_reference_lookup(&git.ref2, git.repo, (string("refs/remotes/origin/") + repo_branch).c_str()));
                git.call(git_merge_head_from_ref(&git.merge_head, git.repo, git.ref2));
                git.call(git_merge_analysis(&analysis, &pref, git.repo, (const git_merge_head **)&git.merge_head, 1));

                if ((analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) != 0) {
                    BOOST_LOG_TRIVIAL(trace) << "Local branch can be fast-forwarded to remote branch.";
                    // The remote branch reference points to a particular commit. We just want to
                    // update the local branch reference to point to the same commit.

                    // Get the commit object ID.
                    git.call(git_reference_peel(&git.obj, git.ref2, GIT_OBJ_COMMIT));
                    const git_oid * commit_id = git_object_id(git.obj);
                    git_reference_free(git.ref2);
                    git.ref2 = nullptr;
                    git_object_free(git.obj);
                    git.obj = nullptr;

                    // Set the reference target.
                    git.call(git_reference_set_target(&git.ref2, git.ref, commit_id, git.sig, "Setting branch reference."));

                    git_reference_free(git.ref2);
                    git.ref2 = nullptr;
                }
                else if ((analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) != 0) {
                    // No update necessary, so exit early to skip unnecessary masterlist parsing.
                    BOOST_LOG_TRIVIAL(trace) << "Local branch is up-to-date with remote branch.";

                    BOOST_LOG_TRIVIAL(trace) << "Performing a Git checkout of HEAD.";
                    git.call(git_checkout_head(git.repo, &checkout_opts));

                    return false;
                }
                else {
                    // The local repository can't be easily merged. It's best just to delete and re-clone it.
                    FixRepoPermissions(repo_path / ".git");
                    fs::remove_all(repo_path / ".git");
                    return this->Update(game, language);
                    //throw error(error::git_error, "Local repository has been edited, an automatic fast-forward merge update is not possible.");
                }

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
        std::string parsingError;
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
                this->MetadataList::Load(game.MasterlistPath());

                for (auto &plugin: plugins) {
                    plugin.EvalAllConditions(game, language);
                }

                for (auto &message: messages) {
                    message.EvalCondition(game, language);
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

                if (parsingError.empty())
                    parsingError = boost::locale::translate("Masterlist revision").str() + " " + string(revision) + ": " + e.what() + ". " + boost::locale::translate("Rolled back to the previous revision.").str();
            }
        } while (parsingFailed);

        if (!parsingError.empty())
            throw error(error::ok, parsingError);  //Throw an OK because the process still completed in a successful state.

        return true;
    }
}
