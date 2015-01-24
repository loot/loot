/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2015    WrinklyNinja

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
    // Removes the read-only flag from some files in git repositories created by libgit2.
    void FixRepoPermissions(const fs::path& path) {
        BOOST_LOG_TRIVIAL(trace) << "Recursively setting write permission on directory: " << path;
        for (fs::recursive_directory_iterator it(path); it != fs::recursive_directory_iterator(); ++it) {
            BOOST_LOG_TRIVIAL(trace) << "Setting write permission for: " << it->path();
            fs::permissions(it->path(), fs::add_perms | fs::owner_write | fs::group_write | fs::others_write);
        }
    }

    struct git_handler {
    public:
        git_handler() :
            repo(nullptr),
            remote(nullptr),
            cfg(nullptr),
            obj(nullptr),
            commit(nullptr),
            ref(nullptr),
            ref2(nullptr),
            sig(nullptr),
            blob(nullptr),
            annotated_commit(nullptr),
            tree(nullptr),
            diff(nullptr),
            buf({0}) {
            // Init threading system and OpenSSL (for Linux builds).
            git_libgit2_init();
        }

        ~git_handler() {
            string path;
            if (repo != nullptr)
                path = git_repository_path(repo);

            free();

            if (!path.empty()) {
                try {
                    FixRepoPermissions(path);
                }
                catch (exception&) {}
            }

            git_libgit2_shutdown();
        }

        void free() {
            git_commit_free(commit);
            git_object_free(obj);
            git_config_free(cfg);
            git_remote_free(remote);
            git_repository_free(repo);
            git_reference_free(ref);
            git_reference_free(ref2);
            git_signature_free(sig);
            git_blob_free(blob);
            git_annotated_commit_free(annotated_commit);
            git_tree_free(tree);
            git_diff_free(diff);
            git_buf_free(&buf);

            commit = nullptr;
            obj = nullptr;
            cfg = nullptr;
            remote = nullptr;
            repo = nullptr;
            ref = nullptr;
            ref2 = nullptr;
            sig = nullptr;
            blob = nullptr;
            annotated_commit = nullptr;
            tree = nullptr;
            diff = nullptr;
            buf = {0};
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
        git_annotated_commit * annotated_commit;
        git_tree * tree;
        git_diff * diff;
        git_buf buf;

        std::string ui_message;
    };

    struct git_diff_payload {
    public:
        bool fileFound;
        const char * fileToFind;
    };

    bool isRepository(const fs::path& path) {
        return git_repository_open_ext(NULL, path.string().c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0;
    }

    int diffFileCallback(const git_diff_delta *delta, float progress, void * payload) {
        BOOST_LOG_TRIVIAL(trace) << "Checking diff for: " << delta->old_file.path;
        git_diff_payload * gdp = (git_diff_payload*)payload;
        if (strcmp(delta->old_file.path, gdp->fileToFind) == 0) {
            BOOST_LOG_TRIVIAL(warning) << "Edited masterlist found.";
            gdp->fileFound = true;
        }

        return 0;
    }

    bool IsMasterlistDifferent(git_handler& git, const std::string& filename) {
        if (git.obj) {
            BOOST_LOG_TRIVIAL(error) << "Object memory already allocated!";
            throw error(error::git_error, lc::translate("Object memory already allocated!"));
        }
        else if (git.tree) {
            BOOST_LOG_TRIVIAL(error) << "Tree memory already allocated!";
            throw error(error::git_error, lc::translate("Tree memory already allocated!"));
        }
        else if (git.diff) {
            BOOST_LOG_TRIVIAL(error) << "Diff memory already allocated!";
            throw error(error::git_error, lc::translate("Diff memory already allocated!"));
        }
        else if (!git.repo) {
            BOOST_LOG_TRIVIAL(error) << "Repository handle not open!";
            throw error(error::git_error, lc::translate("Repository handle not open!"));
        }

        // Perform a git diff, then iterate the deltas to see if one exists for the masterlist.
        BOOST_LOG_TRIVIAL(trace) << "Getting the tree for the HEAD revision.";
        git.call(git_revparse_single(&git.obj, git.repo, "HEAD^{tree}"));
        git.call(git_tree_lookup(&git.tree, git.repo, git_object_id(git.obj)));

        BOOST_LOG_TRIVIAL(trace) << "Performing git diff.";
        git.call(git_diff_tree_to_workdir_with_index(&git.diff, git.repo, git.tree, NULL));

        BOOST_LOG_TRIVIAL(trace) << "Iterating over git diff deltas.";
        git_diff_payload payload;
        payload.fileFound = false;
        payload.fileToFind = filename.c_str();
        git.call(git_diff_foreach(git.diff, &diffFileCallback, NULL, NULL, &payload));

        // Clean up memory
        git_object_free(git.obj);
        git.obj = nullptr;
        git_tree_free(git.tree);
        git.tree = nullptr;
        git_diff_free(git.diff);
        git.diff = nullptr;

        return payload.fileFound;
    }

    void Masterlist::GetGitInfo(const boost::filesystem::path& path, bool shortID) {
        // Compare HEAD and working copy, and get revision info.
        git_handler git;
        git.ui_message = (boost::format(lc::translate("An error occurred while trying to read the local masterlist's version. If this error happens again, try deleting the \".git\" folder in %1%.")) % path.parent_path().string()).str();

        if (!isRepository(path.parent_path())) {
            BOOST_LOG_TRIVIAL(info) << "Unknown masterlist revision: Git repository missing.";
            throw error(error::ok, lc::translate("Unknown: Git repository missing"));
        }
        else if (!fs::exists(path)) {
            BOOST_LOG_TRIVIAL(info) << "Unknown masterlist revision: No masterlist present.";
            throw error(error::ok, lc::translate("N/A: No masterlist present"));
        }

        BOOST_LOG_TRIVIAL(debug) << "Existing repository found, attempting to open it.";
        git.call(git_repository_open(&git.repo, path.parent_path().string().c_str()));

        //Need to get the HEAD object, because the individual file has a different SHA.
        BOOST_LOG_TRIVIAL(info) << "Getting the Git object for the tree at HEAD.";
        git.call(git_revparse_single(&git.obj, git.repo, "HEAD"));

        BOOST_LOG_TRIVIAL(trace) << "Generating hex string for Git object ID.";
        if (shortID) {
            git.call(git_object_short_id(&git.buf, git.obj));
            revision = git.buf.ptr;
        }
        else {
            char c_rev[GIT_OID_HEXSZ + 1];
            revision = git_oid_tostr(c_rev, GIT_OID_HEXSZ + 1, git_object_id(git.obj));
        }

        BOOST_LOG_TRIVIAL(trace) << "Getting date for Git object.";
        const git_oid * oid = git_object_id(git.obj);
        git.call(git_commit_lookup(&git.commit, git.repo, oid));
        git_time_t time = git_commit_time(git.commit);
        boost::locale::date_time dateTime(time);
        stringstream out;
        out << boost::locale::as::ftime("%Y-%m-%d") << dateTime;
        date = out.str();

        // Free object memory.
        git_object_free(git.obj);
        git.obj = nullptr;

        BOOST_LOG_TRIVIAL(trace) << "Diffing masterlist HEAD and working copy.";
        if (IsMasterlistDifferent(git, path.filename().string())) {
            revision += string(" ") + lc::translate("(edited)").str();
            date += string(" ") + lc::translate("(edited)").str();
        }
    }

    bool Masterlist::Update(const Game& game) {
        return Update(game.MasterlistPath(), game.RepoURL(), game.RepoBranch());
    }

    bool Masterlist::Update(const boost::filesystem::path& path, const std::string& repoURL, const std::string& repoBranch) {
        git_handler git;
        fs::path repo_path = path.parent_path();
        string filename = path.filename().string();

        // First initialise some stuff that isn't specific to a repository.
        BOOST_LOG_TRIVIAL(debug) << "Creating a reflog signature to use.";
        // Create a reflog signature for any changes.
        git.call(git_signature_new(&git.sig, "LOOT", "loot@placeholder.net", 0, 0));

        BOOST_LOG_TRIVIAL(debug) << "Setting up checkout options.";
        char * paths = new char[filename.length() + 1];
        strcpy(paths, filename.c_str());
        git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
        checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
        checkout_opts.paths.strings = &paths;
        checkout_opts.paths.count = 1;

        // Now try to access the repository if it exists, or clone one if it doesn't.
        BOOST_LOG_TRIVIAL(trace) << "Attempting to open the Git repository at: " << repo_path;
        if (!isRepository(repo_path)) {
            git.ui_message = lc::translate("An error occurred while trying to clone the remote masterlist repository.");
            // Clone the remote repository.
            BOOST_LOG_TRIVIAL(info) << "Repository doesn't exist, cloning the remote repository.";

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
            }

            //First set up clone options.

            git_clone_options clone_options = GIT_CLONE_OPTIONS_INIT;
            clone_options.checkout_opts = checkout_opts;
            clone_options.bare = 0;
            clone_options.checkout_branch = repoBranch.c_str();
            clone_options.signature = git.sig;

            //Now perform the clone.
            git.call(git_clone(&git.repo, repoURL.c_str(), repo_path.string().c_str(), &clone_options));

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
                FixRepoPermissions(temp_path);
                fs::remove_all(temp_path);
            }
        }
        else {
            // Repository exists: check settings are correct, then pull updates.
            git.ui_message = (boost::format(lc::translate("An error occurred while trying to access the local masterlist repository. If this error happens again, try deleting the \".git\" folder in %1%.")) % repo_path.string()).str();

            // Open the repository.
            BOOST_LOG_TRIVIAL(info) << "Existing repository found, attempting to open it.";
            git.call(git_repository_open(&git.repo, repo_path.string().c_str()));

            // Check that the repository's remote settings match LOOT's.
            BOOST_LOG_TRIVIAL(info) << "Checking to see if remote URL matches URL in settings.";
            git.call(git_remote_lookup(&git.remote, git.repo, "origin"));
            const char * url = git_remote_url(git.remote);

            BOOST_LOG_TRIVIAL(info) << "Remote URL given: " << repoURL;
            BOOST_LOG_TRIVIAL(info) << "Remote URL in repository settings: " << url;
            if (url != repoURL) {
                BOOST_LOG_TRIVIAL(info) << "URLs do not match, setting repository URL to URL in settings.";
                // The URLs don't match. Change the remote URL to match the one LOOT has.
                git.call(git_remote_set_url(git.remote, repoURL.c_str()));

                // Now save change.
                git.call(git_remote_save(git.remote));
            }

            // Now fetch updates from the remote.
            BOOST_LOG_TRIVIAL(trace) << "Fetching updates from remote.";
            git.ui_message = lc::translate("An error occurred while trying to update the masterlist. This could be due to a server-side error. Try again in a few minutes.");

            git.call(git_remote_fetch(git.remote, nullptr, git.sig, nullptr));

            // Print some stats on what was fetched either during update or clone.
            const git_transfer_progress * stats = git_remote_stats(git.remote);
            BOOST_LOG_TRIVIAL(info) << "Received " << stats->indexed_objects << " of " << stats->total_objects << " objects in " << stats->received_bytes << " bytes.";

            // Check that a branch with the correct name exists.
            git.ui_message = (boost::format(lc::translate("An error occurred while trying to access the local masterlist repository. If this error happens again, try deleting the \".git\" folder in %1%.")) % repo_path.string()).str();
            int ret = git_branch_lookup(&git.ref, git.repo, repoBranch.c_str(), GIT_BRANCH_LOCAL);
            if (ret == GIT_ENOTFOUND) {
                // Branch doesn't exist. Create a new branch using the remote branch's latest commit.

                BOOST_LOG_TRIVIAL(trace) << "Looking up commit referred to by the remote branch \"" << repoBranch << "\".";
                git.call(git_revparse_single(&git.obj, git.repo, (string("origin/") + repoBranch).c_str()));
                const git_oid * commit_id = git_object_id(git.obj);

                BOOST_LOG_TRIVIAL(trace) << "Creating the new branch.";
                // Create a branch.
                git.call(git_commit_lookup(&git.commit, git.repo, commit_id));
                git.call(git_branch_create(&git.ref, git.repo, repoBranch.c_str(), git.commit, 0, git.sig, NULL));

                // Set upstream. Don't really know if this is necessary or not.
                git.call(git_branch_set_upstream(git.ref, (string("origin/") + repoBranch).c_str()));

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
                BOOST_LOG_TRIVIAL(trace) << "Setting HEAD to follow branch: " << repoBranch;
                git.call(git_repository_set_head(git.repo, (string("refs/heads/") + repoBranch).c_str(), git.sig, (string("Updated HEAD to ") + repoBranch).c_str()));
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
                git.call(git_reference_lookup(&git.ref2, git.repo, (string("refs/remotes/origin/") + repoBranch).c_str()));
                git.call(git_annotated_commit_from_ref(&git.annotated_commit, git.repo, git.ref2));
                git.call(git_merge_analysis(&analysis, &pref, git.repo, (const git_annotated_commit **)&git.annotated_commit, 1));

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
                    // No merge is required, but HEAD might be ahead of the remote branch. Check
                    // to see if that's the case, and move HEAD back to match the remote branch
                    // if so.
                    BOOST_LOG_TRIVIAL(trace) << "Local branch is up-to-date with remote branch.";

                    BOOST_LOG_TRIVIAL(trace) << "Checking to see if local and remote branch heads are equal.";
                    // Get the local branch and remote branch head commit IDs.

                    // Local branch.
                    git.call(git_reference_peel(&git.obj, git.ref, GIT_OBJ_COMMIT));
                    const git_oid * local_commit_id = git_object_id(git.obj);
                    git_object_free(git.obj);
                    git.obj = nullptr;

                    // Remote branch.
                    git.call(git_reference_peel(&git.obj, git.ref2, GIT_OBJ_COMMIT));
                    const git_oid * remote_commit_id = git_object_id(git.obj);
                    git_reference_free(git.ref2);
                    git.ref2 = nullptr;
                    git_object_free(git.obj);
                    git.obj = nullptr;

                    if (local_commit_id->id != remote_commit_id->id) {
                        BOOST_LOG_TRIVIAL(trace) << "Branch heads are not equal, updating local HEAD.";
                        // Commit IDs don't match, update HEAD, and continue with normal update procedure.
                        git.call(git_reference_set_target(&git.ref2, git.ref, remote_commit_id, git.sig, "Setting branch reference."));
                        git_reference_free(git.ref2);
                        git.ref2 = nullptr;
                    }
                    else {
                        // HEAD matches the remote branch. If the masterlist in
                        // HEAD also matches the masterlist file, no further
                        // action needs to be taken. Otherwise, a checkout
                        // must be performed and the checked-out file parsed.
                        BOOST_LOG_TRIVIAL(trace) << "Branch heads are equal.";

                        BOOST_LOG_TRIVIAL(trace) << "Diffing HEAD and filesystem masterlists.";
                        if (!IsMasterlistDifferent(git, filename)) {
                            return false;
                        }
                    }
                }
                else {
                    // The local repository can't be easily merged. It's best just to delete and re-clone it.
                    FixRepoPermissions(repo_path / ".git");
                    git.free();
                    fs::remove_all(repo_path / ".git");
                    return this->Update(path, repoURL, repoBranch);
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
        git.ui_message = (boost::format(lc::translate("An error occurred while trying to read information on the updated masterlist. If this error happens again, try deleting the \".git\" folder in %1%.")) % repo_path.string()).str();
        do {
            // Get some descriptive info about what was checked out.

            BOOST_LOG_TRIVIAL(trace) << "Getting the Git object for HEAD.";
            git.call(git_repository_head(&git.ref, git.repo));
            git.call(git_reference_peel(&git.obj, git.ref, GIT_OBJ_COMMIT));

            BOOST_LOG_TRIVIAL(trace) << "Generating hex string for Git object ID.";
            git.call(git_object_short_id(&git.buf, git.obj));
            revision = git.buf.ptr;

            BOOST_LOG_TRIVIAL(trace) << "Getting date for Git object.";
            const git_oid * oid = git_object_id(git.obj);
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
            git_buf_free(&git.buf);
            git_commit_free(git.commit);
            git.ref = nullptr;
            git.obj = nullptr;
            git.commit = nullptr;
            git.buf = {0};

            //Now try parsing the masterlist.
            BOOST_LOG_TRIVIAL(debug) << "Testing masterlist parsing.";
            try {
                this->MetadataList::Load(path);

                for (auto &plugin : plugins) {
                    plugin.ParseAllConditions();
                }
                for (auto &plugin : regexPlugins) {
                    plugin.ParseAllConditions();
                }
                for (auto &message : messages) {
                    message.ParseCondition();
                }

                parsingFailed = false;
            }
            catch (std::exception& e) {
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
