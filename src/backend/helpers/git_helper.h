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

#ifndef __LOOT_GIT_HELPER__
#define __LOOT_GIT_HELPER__

#include <string>

#include <boost/filesystem.hpp>

#include <git2.h>

namespace loot {
    class GitHelper {
    public:
        GitHelper();
        ~GitHelper();

        void Call(int error_code);
        void SetErrorMessage(const std::string& message);
        void Free();

        static bool IsRepository(const boost::filesystem::path& path);

        static bool IsFileDifferent(const boost::filesystem::path& repoRoot, const std::string& filename);

        // Clones a repository and opens it. Sets 'repo'.
        void Clone(const boost::filesystem::path& path, const std::string& url);

        // Fetch from remote.
        void Fetch(const std::string& remote);

        // Create and checkout a new remote-tracking branch.
        void CheckoutNewBranch(const std::string& remote, const std::string& branch);

        void CheckoutRevision(const std::string& revision);

        std::string GetHeadShortId();

        git_repository * repo;
        git_remote * remote;
        git_config * cfg;
        git_object * obj;
        git_commit * commit;
        git_reference * ref;
        git_reference * ref2;
        git_blob * blob;
        git_annotated_commit * annotated_commit;
        git_tree * tree;
        git_diff * diff;
        git_buf buf;

        git_checkout_options checkout_options;
        git_clone_options clone_options;

        struct git_diff_payload {
            bool fileFound;
            const char * fileToFind;
        };

        static int diff_file_cb(const git_diff_delta *delta, float progress, void * payload);
    private:
        std::string errorMessage;

        // Removes the read-only flag from some files in git repositories
        // created by libgit2.
        static void FixRepoPermissions(const boost::filesystem::path& path);
    };
}
#endif
