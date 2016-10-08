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
    <https://www.gnu.org/licenses/>.
    */

#include "backend/helpers/git_helper.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>

#include "loot/error.h"
#include "loot/error_categories.h"
#include "loot/exception/git_state_error.h"

using boost::locale::translate;
using std::string;

namespace fs = boost::filesystem;

namespace loot {
GitHelper::GitData::GitData() :
  repo(nullptr),
  remote(nullptr),
  config(nullptr),
  object(nullptr),
  commit(nullptr),
  reference(nullptr),
  reference2(nullptr),
  blob(nullptr),
  annotated_commit(nullptr),
  tree(nullptr),
  diff(nullptr),
  buffer({0}) {
  // Init threading system and OpenSSL (for Linux builds).
  git_libgit2_init();

  checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
  clone_options = GIT_CLONE_OPTIONS_INIT;
}

GitHelper::GitData::~GitData() {
  string path;
  if (repo != nullptr)
    path = git_repository_path(repo);

  git_commit_free(commit);
  git_object_free(object);
  git_config_free(config);
  git_remote_free(remote);
  git_repository_free(repo);
  git_reference_free(reference);
  git_reference_free(reference2);
  git_blob_free(blob);
  git_annotated_commit_free(annotated_commit);
  git_tree_free(tree);
  git_diff_free(diff);
  git_buf_free(&buffer);

  // Also free any path strings in the checkout options.
  for (size_t i = 0; i < checkout_options.paths.count; ++i) {
    delete[] checkout_options.paths.strings[i];
  }

  if (!path.empty()) {
    try {
      FixRepoPermissions(path);
    } catch (std::exception&) {}
  }

  git_libgit2_shutdown();
}

void GitHelper::Call(int error_code) {
  if (!error_code)
    return;

  const git_error * last_error = giterr_last();
  std::string gitError;
  if (last_error == nullptr)
    gitError = std::to_string(error_code) + ".";
  else
    gitError = std::to_string(error_code) + "; " + last_error->message;
  giterr_clear();

  if (errorMessage_.empty())
    errorMessage_ = (boost::format(translate("Git operation failed. Error: %1%")) % gitError).str();

  BOOST_LOG_TRIVIAL(error) << "Git operation failed. Error: " << gitError;
  throw std::system_error(error_code, libgit2_category(), errorMessage_);
}

void GitHelper::SetErrorMessage(const std::string& message) {
  errorMessage_ = message;
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

int GitHelper::DiffFileCallback(const git_diff_delta *delta, float progress, void * payload) {
  BOOST_LOG_TRIVIAL(trace) << "Checking diff for: " << delta->old_file.path;
  DiffPayload * gdp = (DiffPayload*)payload;
  if (strcmp(delta->old_file.path, gdp->fileToFind) == 0) {
    BOOST_LOG_TRIVIAL(warning) << "Edited masterlist found.";
    gdp->fileFound = true;
  }

  return 0;
}

// Clones a repository and opens it.
void GitHelper::Clone(const boost::filesystem::path& path, const std::string& url) {
  if (data_.repo != nullptr)
    throw GitStateError("Cannot clone repository that has already been opened.");

  SetErrorMessage(translate("An error occurred while trying to clone the remote masterlist repository."));
  // Clone the remote repository.
  BOOST_LOG_TRIVIAL(info) << "Repository doesn't exist, cloning the remote repository.";

  fs::path tempPath = path.parent_path() / fs::unique_path();

  //Delete temporary folder in case it already exists.
  fs::remove_all(tempPath);

  if (!fs::is_empty(path)) {
      // Directory is non-empty. Delete the masterlist file and
      // .git folder, then move any remaining files to a temporary
      // folder while the repo is cloned, before moving them back.
    BOOST_LOG_TRIVIAL(trace) << "Repo path not empty, renaming folder.";

    // Clear any read-only flags first.
    FixRepoPermissions(path);

    // Now move to temp path.
    fs::rename(path, tempPath);

    // Recreate the game folder so that we don't inadvertently
    // cause any other errors (everything past LOOT init assumes
    // it exists).
    fs::create_directory(path);
  }

  // Perform the clone.
  Call(git_clone(&data_.repo, url.c_str(), path.string().c_str(), &data_.clone_options));

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
  if (data_.repo == nullptr)
    throw GitStateError("Cannot fetch updates for repository that has not been opened.");

  BOOST_LOG_TRIVIAL(trace) << "Fetching updates from remote.";
  SetErrorMessage(translate("An error occurred while trying to update the masterlist. This could be due to a server-side error. Try again in a few minutes."));

  // Get the origin remote.
  Call(git_remote_lookup(&data_.remote, data_.repo, remote.c_str()));

  // Now fetch any updates.
  git_fetch_options fetch_options = GIT_FETCH_OPTIONS_INIT;
  Call(git_remote_fetch(data_.remote, nullptr, &fetch_options, nullptr));

  // Log some stats on what was fetched either during update or clone.
  const git_transfer_progress * stats = git_remote_stats(data_.remote);
  BOOST_LOG_TRIVIAL(info) << "Received " << stats->indexed_objects << " of " << stats->total_objects << " objects in " << stats->received_bytes << " bytes.";

  git_remote_free(data_.remote);
  data_.remote = nullptr;
}

void GitHelper::CheckoutNewBranch(const std::string& remote, const std::string& branch) {
  if (data_.repo == nullptr)
    throw GitStateError("Cannot fetch updates for repository that has not been opened.");
  else if (data_.commit != nullptr)
    throw GitStateError("Cannot fetch repository updates, commit memory already allocated.");
  else if (data_.object != nullptr)
    throw GitStateError("Cannot fetch repository updates, object memory already allocated.");
  else if (data_.reference != nullptr)
    throw GitStateError("Cannot fetch repository updates, reference memory already allocated.");

  BOOST_LOG_TRIVIAL(trace) << "Looking up commit referred to by the remote branch \"" << branch << "\".";
  Call(git_revparse_single(&data_.object, data_.repo, (remote + "/" + branch).c_str()));
  const git_oid * commit_id = git_object_id(data_.object);

  // Create a branch.
  BOOST_LOG_TRIVIAL(trace) << "Creating the new branch.";
  Call(git_commit_lookup(&data_.commit, data_.repo, commit_id));
  Call(git_branch_create(&data_.reference, data_.repo, branch.c_str(), data_.commit, 0));

  // Set upstream.
  BOOST_LOG_TRIVIAL(trace) << "Setting the upstream for the new branch.";
  Call(git_branch_set_upstream(data_.reference, (remote + "/" + branch).c_str()));

  // Check if HEAD points to the desired branch and set it to if not.
  if (!git_branch_is_head(data_.reference)) {
    BOOST_LOG_TRIVIAL(trace) << "Setting HEAD to follow branch: " << branch;
    Call(git_repository_set_head(data_.repo, (string("refs/heads/") + branch).c_str()));
  }

  BOOST_LOG_TRIVIAL(trace) << "Performing a Git checkout of HEAD.";
  Call(git_checkout_head(data_.repo, &data_.checkout_options));

  // Free tree and commit pointers. Reference pointer is still used below.
  git_object_free(data_.object);
  git_commit_free(data_.commit);
  git_reference_free(data_.reference);
  data_.commit = nullptr;
  data_.object = nullptr;
  data_.reference = nullptr;
}

void GitHelper::CheckoutRevision(const std::string& revision) {
  if (data_.repo == nullptr)
    throw GitStateError("Cannot checkout revision for repository that has not been opened.");
  else if (data_.object != nullptr)
    throw GitStateError("Cannot fetch repository updates, object memory already allocated.");

// Get an object ID for 'HEAD^'.
  Call(git_revparse_single(&data_.object, data_.repo, revision.c_str()));
  const git_oid * oid = git_object_id(data_.object);

  // Detach HEAD to HEAD~1. This will roll back HEAD by one commit each time it is called.
  Call(git_repository_set_head_detached(data_.repo, oid));

  // Checkout the new HEAD.
  BOOST_LOG_TRIVIAL(trace) << "Performing a Git checkout of HEAD.";
  Call(git_checkout_head(data_.repo, &data_.checkout_options));

  git_object_free(data_.object);
  data_.object = nullptr;
}

std::string GitHelper::GetHeadShortId() {
  if (data_.repo == nullptr)
    throw GitStateError("Cannot checkout revision for repository that has not been opened.");
  else if (data_.object != nullptr)
    throw GitStateError("Cannot fetch repository updates, object memory already allocated.");
  else if (data_.reference != nullptr)
    throw GitStateError("Cannot fetch repository updates, reference memory already allocated.");
  else if (data_.buffer.ptr != nullptr)
    throw GitStateError("Cannot fetch repository updates, buffer memory already allocated.");

  BOOST_LOG_TRIVIAL(trace) << "Getting the Git object for HEAD.";
  Call(git_repository_head(&data_.reference, data_.repo));
  Call(git_reference_peel(&data_.object, data_.reference, GIT_OBJ_COMMIT));

  BOOST_LOG_TRIVIAL(trace) << "Generating hex string for Git object ID.";
  Call(git_object_short_id(&data_.buffer, data_.object));
  string revision = data_.buffer.ptr;

  git_reference_free(data_.reference);
  git_object_free(data_.object);
  git_buf_free(&data_.buffer);
  data_.reference = nullptr;
  data_.object = nullptr;
  data_.buffer = {0};

  return revision;
}

GitHelper::GitData& GitHelper::GetData() {
  return data_;
}

bool GitHelper::IsFileDifferent(const boost::filesystem::path& repoRoot, const std::string& filename) {
  if (!IsRepository(repoRoot)) {
    BOOST_LOG_TRIVIAL(info) << "Unknown masterlist revision: Git repository missing.";
    throw GitStateError("Cannot check if the \"" + filename + "\" working copy is edited, Git repository missing.");
  }

  BOOST_LOG_TRIVIAL(debug) << "Existing repository found, attempting to open it.";
  GitHelper git;
  git.Call(git_repository_open(&git.data_.repo, repoRoot.string().c_str()));

  // Perform a git diff, then iterate the deltas to see if one exists for the masterlist.
  BOOST_LOG_TRIVIAL(trace) << "Getting the tree for the HEAD revision.";
  git.Call(git_revparse_single(&git.data_.object, git.data_.repo, "HEAD^{tree}"));
  git.Call(git_tree_lookup(&git.data_.tree, git.data_.repo, git_object_id(git.data_.object)));

  BOOST_LOG_TRIVIAL(trace) << "Performing git diff.";
  git.Call(git_diff_tree_to_workdir_with_index(&git.data_.diff, git.data_.repo, git.data_.tree, NULL));

  BOOST_LOG_TRIVIAL(trace) << "Iterating over git diff deltas.";
  GitHelper::DiffPayload payload;
  payload.fileFound = false;
  payload.fileToFind = filename.c_str();
  git.Call(git_diff_foreach(git.data_.diff, &git.DiffFileCallback, NULL, NULL, NULL, &payload));

  return payload.fileFound;
}
}
