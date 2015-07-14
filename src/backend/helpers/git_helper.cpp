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
    }

    bool GitHelper::IsRepository(const boost::filesystem::path& path) {
        return git_repository_open_ext(NULL, path.string().c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0;
    }

    // Removes the read-only flag from some files in git repositories created by libgit2.
    void GitHelper::FixRepoPermissions(const boost::filesystem::path& path) {
        BOOST_LOG_TRIVIAL(trace) << "Recursively setting write permission on directory: " << path;
        for (fs::recursive_directory_iterator it(path); it != fs::recursive_directory_iterator(); ++it) {
            BOOST_LOG_TRIVIAL(trace) << "Setting write permission for: " << it->path();
            fs::permissions(it->path(), fs::add_perms | fs::owner_write | fs::group_write | fs::others_write);
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

    bool IsFileDifferent(const boost::filesystem::path& repoRoot, const std::string& filename) {
        GitHelper git;

        if (!git.IsRepository(repoRoot)) {
            BOOST_LOG_TRIVIAL(info) << "Unknown masterlist revision: Git repository missing.";
            throw error(error::ok, lc::translate("Unknown: Git repository missing"));
        }

        BOOST_LOG_TRIVIAL(debug) << "Existing repository found, attempting to open it.";
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
