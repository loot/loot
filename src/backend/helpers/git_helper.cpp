/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

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

#include "git_helper.h"
#include "../error.h"

#include <boost/log/trivial.hpp>
#include <boost/locale.hpp>
#include <boost/format.hpp>

using namespace std;

namespace fs = boost::filesystem;
namespace lc = boost::locale;

namespace loot {
    GitHelper::GitHelper() :
        repo(nullptr),
        remote(nullptr),
        cfg(nullptr),
        obj(nullptr),
        commit(nullptr),
        ref(nullptr),
        ref2(nullptr),
        blob(nullptr),
        annotated_commit(nullptr),
        tree(nullptr),
        diff(nullptr),
        buf({0}) {
        // Init threading system and OpenSSL (for Linux builds).
        git_libgit2_init();

        checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
        clone_options = GIT_CLONE_OPTIONS_INIT;
    }

    GitHelper::~GitHelper() {
        string path;
        if (repo != nullptr)
            path = git_repository_path(repo);

        Free();

        if (!path.empty()) {
            try {
                FixRepoPermissions(path);
            }
            catch (exception&) {}
        }

        git_libgit2_shutdown();
    }

    void GitHelper::Call(int error_code) {
        if (!error_code)
            return;

        const git_error * last_error = giterr_last();
        std::string gitError;
        if (last_error == nullptr)
            gitError = to_string(error_code) + ".";
        else
            gitError = to_string(error_code) + "; " + last_error->message;
        giterr_clear();

        if (errorMessage.empty())
            errorMessage = (boost::format(lc::translate("Git operation failed. Error: %1%")) % gitError).str();

        BOOST_LOG_TRIVIAL(error) << "Git operation failed. Error: " << gitError;
        throw loot::error(loot::error::git_error, errorMessage);
    }

    void GitHelper::SetErrorMessage(const std::string& message) {
        errorMessage = message;
    }

    void GitHelper::Free() {
        git_commit_free(commit);
        git_object_free(obj);
        git_config_free(cfg);
        git_remote_free(remote);
        git_repository_free(repo);
        git_reference_free(ref);
        git_reference_free(ref2);
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
        blob = nullptr;
        annotated_commit = nullptr;
        tree = nullptr;
        diff = nullptr;
        buf = {0};

        // Also free any path strings in the checkout options.
        for (size_t i = 0; i < checkout_options.paths.count; ++i) {
            delete[] checkout_options.paths.strings[i];
            checkout_options.paths.strings[i] = nullptr;
        }
    }

    bool GitHelper::IsRepository(const boost::filesystem::path& path) {
        return git_repository_open_ext(NULL, path.string().c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0;
    }

    // Removes the read-only flag from some files in git repositories created by libgit2.
    void GitHelper::FixRepoPermissions(const boost::filesystem::path& path) {
        BOOST_LOG_TRIVIAL(trace) << "Recursively setting write permission on directory: " << path;
        for (fs::recursive_directory_iterator it(path); it != fs::recursive_directory_iterator(); ++it) {
            if ((it->status().permissions() & (fs::owner_write | fs::group_write | fs::others_write)) == 0) {
                BOOST_LOG_TRIVIAL(trace) << "Setting write permission for: " << it->path();
                fs::permissions(it->path(), fs::add_perms | fs::owner_write);
            }
        }
    }

    int GitHelper::diff_file_cb(const git_diff_delta *delta, float progress, void * payload) {
        BOOST_LOG_TRIVIAL(trace) << "Checking diff for: " << delta->old_file.path;
        git_diff_payload * gdp = (git_diff_payload*)payload;
        if (strcmp(delta->old_file.path, gdp->fileToFind) == 0) {
            BOOST_LOG_TRIVIAL(warning) << "Edited masterlist found.";
            gdp->fileFound = true;
        }

        return 0;
    }

    // Clones a repository and opens it.
    void GitHelper::Clone(const boost::filesystem::path& path, const std::string& url) {
        if (this->repo != nullptr)
            throw error(error::git_error, "Cannot clone repository that has already been opened.");

        this->SetErrorMessage(lc::translate("An error occurred while trying to clone the remote masterlist repository."));
        // Clone the remote repository.
        BOOST_LOG_TRIVIAL(info) << "Repository doesn't exist, cloning the remote repository.";

        fs::path tempPath = fs::temp_directory_path() / fs::unique_path();

        //Delete temporary folder in case it already exists.
        fs::remove_all(tempPath);

        if (!fs::is_empty(path)) {
            // Directory is non-empty. Delete the masterlist file and
            // .git folder, then move any remaining files to a temporary
            // folder while the repo is cloned, before moving them back.
            BOOST_LOG_TRIVIAL(trace) << "Repo path not empty, renaming folder.";

            // Clear any read-only flags first.
            this->FixRepoPermissions(path);

            // Now move to temp path.
            fs::rename(path, tempPath);

            // Recreate the game folder so that we don't inadvertently
            // cause any other errors (everything past LOOT init assumes
            // it exists).
            fs::create_directory(path);
        }

        // Perform the clone.
        this->Call(git_clone(&this->repo, url.c_str(), path.string().c_str(), &this->clone_options));

        if (fs::exists(tempPath)) {
            //Move contents back in.
            BOOST_LOG_TRIVIAL(trace) << "Repo path wasn't empty, moving previous files back in.";
            for (fs::directory_iterator it(tempPath); it != fs::directory_iterator(); ++it) {
                if (!fs::exists(path / it->path().filename())) {
                    //No conflict, OK to move back in.
                    fs::rename(it->path(), path / it->path().filename());
                }
            }
            //Delete temporary folder.
            fs::remove_all(tempPath);
        }
    }

    void GitHelper::Fetch(const std::string& remote) {
        if (this->repo == nullptr)
            throw error(error::git_error, "Cannot fetch updates for repository that has not been opened.");

        BOOST_LOG_TRIVIAL(trace) << "Fetching updates from remote.";
        this->SetErrorMessage(lc::translate("An error occurred while trying to update the masterlist. This could be due to a server-side error. Try again in a few minutes."));

        // Get the origin remote.
        this->Call(git_remote_lookup(&this->remote, this->repo, remote.c_str()));

        // Now fetch any updates.
        git_fetch_options fetch_options = GIT_FETCH_OPTIONS_INIT;
        this->Call(git_remote_fetch(this->remote, nullptr, &fetch_options, nullptr));

        // Log some stats on what was fetched either during update or clone.
        const git_transfer_progress * stats = git_remote_stats(this->remote);
        BOOST_LOG_TRIVIAL(info) << "Received " << stats->indexed_objects << " of " << stats->total_objects << " objects in " << stats->received_bytes << " bytes.";

        git_remote_free(this->remote);
        this->remote = nullptr;
    }

    void GitHelper::CheckoutNewBranch(const std::string& remote, const std::string& branch) {
        if (this->repo == nullptr)
            throw error(error::git_error, "Cannot fetch updates for repository that has not been opened.");
        else if (this->commit != nullptr)
            throw error(error::git_error, "Cannot fetch repository updates, commit memory already allocated.");
        else if (this->obj != nullptr)
            throw error(error::git_error, "Cannot fetch repository updates, object memory already allocated.");
        else if (this->ref != nullptr)
            throw error(error::git_error, "Cannot fetch repository updates, reference memory already allocated.");

        BOOST_LOG_TRIVIAL(trace) << "Looking up commit referred to by the remote branch \"" << branch << "\".";
        this->Call(git_revparse_single(&this->obj, this->repo, (remote + "/" + branch).c_str()));
        const git_oid * commit_id = git_object_id(this->obj);

        // Create a branch.
        BOOST_LOG_TRIVIAL(trace) << "Creating the new branch.";
        this->Call(git_commit_lookup(&this->commit, this->repo, commit_id));
        this->Call(git_branch_create(&this->ref, this->repo, branch.c_str(), this->commit, 0));

        // Set upstream.
        BOOST_LOG_TRIVIAL(trace) << "Setting the upstream for the new branch.";
        this->Call(git_branch_set_upstream(this->ref, (remote + "/" + branch).c_str()));

        // Check if HEAD points to the desired branch and set it to if not.
        if (!git_branch_is_head(this->ref)) {
            BOOST_LOG_TRIVIAL(trace) << "Setting HEAD to follow branch: " << branch;
            this->Call(git_repository_set_head(this->repo, (string("refs/heads/") + branch).c_str()));
        }

        BOOST_LOG_TRIVIAL(trace) << "Performing a Git checkout of HEAD.";
        this->Call(git_checkout_head(this->repo, &this->checkout_options));

        // Free tree and commit pointers. Reference pointer is still used below.
        git_object_free(this->obj);
        git_commit_free(this->commit);
        git_reference_free(this->ref);
        this->commit = nullptr;
        this->obj = nullptr;
        this->ref = nullptr;
    }

    void GitHelper::CheckoutRevision(const std::string& revision) {
        if (this->repo == nullptr)
            throw error(error::git_error, "Cannot checkout revision for repository that has not been opened.");
        else if (this->obj != nullptr)
            throw error(error::git_error, "Cannot fetch repository updates, object memory already allocated.");

        // Get an object ID for 'HEAD^'.
        this->Call(git_revparse_single(&this->obj, this->repo, revision.c_str()));
        const git_oid * oid = git_object_id(this->obj);

        // Detach HEAD to HEAD~1. This will roll back HEAD by one commit each time it is called.
        this->Call(git_repository_set_head_detached(this->repo, oid));

        // Checkout the new HEAD.
        BOOST_LOG_TRIVIAL(trace) << "Performing a Git checkout of HEAD.";
        this->Call(git_checkout_head(this->repo, &this->checkout_options));

        git_object_free(this->obj);
        this->obj = nullptr;
    }

    std::string GitHelper::GetHeadShortId() {
        if (this->repo == nullptr)
            throw error(error::git_error, "Cannot checkout revision for repository that has not been opened.");
        else if (this->obj != nullptr)
            throw error(error::git_error, "Cannot fetch repository updates, object memory already allocated.");
        else if (this->ref != nullptr)
            throw error(error::git_error, "Cannot fetch repository updates, reference memory already allocated.");
        else if (this->buf.ptr != nullptr)
            throw error(error::git_error, "Cannot fetch repository updates, buffer memory already allocated.");

        BOOST_LOG_TRIVIAL(trace) << "Getting the Git object for HEAD.";
        this->Call(git_repository_head(&this->ref, this->repo));
        this->Call(git_reference_peel(&this->obj, this->ref, GIT_OBJ_COMMIT));

        BOOST_LOG_TRIVIAL(trace) << "Generating hex string for Git object ID.";
        this->Call(git_object_short_id(&this->buf, this->obj));
        string revision = this->buf.ptr;

        git_reference_free(this->ref);
        git_object_free(this->obj);
        git_buf_free(&this->buf);
        this->ref = nullptr;
        this->obj = nullptr;
        this->buf = {0};

        return revision;
    }

    bool GitHelper::IsFileDifferent(const boost::filesystem::path& repoRoot, const std::string& filename) {
        if (!IsRepository(repoRoot)) {
            BOOST_LOG_TRIVIAL(info) << "Unknown masterlist revision: Git repository missing.";
            throw error(error::ok, lc::translate("Unknown: Git repository missing"));
        }

        BOOST_LOG_TRIVIAL(debug) << "Existing repository found, attempting to open it.";
        GitHelper git;
        git.Call(git_repository_open(&git.repo, repoRoot.string().c_str()));

        // Perform a git diff, then iterate the deltas to see if one exists for the masterlist.
        BOOST_LOG_TRIVIAL(trace) << "Getting the tree for the HEAD revision.";
        git.Call(git_revparse_single(&git.obj, git.repo, "HEAD^{tree}"));
        git.Call(git_tree_lookup(&git.tree, git.repo, git_object_id(git.obj)));

        BOOST_LOG_TRIVIAL(trace) << "Performing git diff.";
        git.Call(git_diff_tree_to_workdir_with_index(&git.diff, git.repo, git.tree, NULL));

        BOOST_LOG_TRIVIAL(trace) << "Iterating over git diff deltas.";
        GitHelper::git_diff_payload payload;
        payload.fileFound = false;
        payload.fileToFind = filename.c_str();
        git.Call(git_diff_foreach(git.diff, &git.diff_file_cb, NULL, NULL, NULL, &payload));

        return payload.fileFound;
    }
}
