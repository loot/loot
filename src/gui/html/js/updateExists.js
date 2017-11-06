'use strict';
(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define(['node_modules/github-api/dist/GitHub.bundle.min.js'], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.updateExists = factory(root.GitHub);
  }
}(this, (GitHub) => {
  const versionRegex = /^(\d+)\.(\d+)\.(\d+)$/;

  function compare(lhs, rhs) {
    if (!versionRegex.test(lhs) || !versionRegex.test(rhs)) {
      throw new Error(`versions to compare are of unexpected format: ${lhs}, ${rhs}`);
    }

    const lhsNumbers = lhs.split('.');
    const rhsNumbers = rhs.split('.');

    for (let i = 0; i < lhsNumbers.length; i += 1) {
      const lhsNumber = Number(lhsNumbers[i]);
      const rhsNumber = Number(rhsNumbers[i]);

      if (lhsNumber < rhsNumber) {
        return -1;
      }

      if (rhsNumber < lhsNumber) {
        return 1;
      }
    }

    return 0;
  }

  return (currentVersion, currentBuild) => {
    if (currentVersion === undefined || currentBuild === undefined) {
      return Promise.reject(new Error('Invalid arguments, both version and build must be given'));
    }

    const repo = (new GitHub()).getRepo('loot', 'loot');

    return repo.getRelease('latest').then((response) => {
      const latestReleaseTagName = response.data.tag_name;
      const comparison = compare(currentVersion, latestReleaseTagName);
      if (comparison === -1) {
        return true;
      } else if (comparison === 1) {
        return false;
      }

      return repo.listTags().then((tagsResponse) => (
        tagsResponse.data.find((element) => element.name === latestReleaseTagName)
      )).then((tag) => {
        if (tag.commit.sha.startsWith(currentBuild)) {
          return false;
        }

        return repo.getCommit(tag.commit.sha)
          .then((commitResponse) => Date.parse(commitResponse.data.committer.date))
          .then((tagDate) => (
            repo.getSingleCommit(currentBuild)
              .then((commitResponse) => Date.parse(commitResponse.data.commit.committer.date))
              .then((buildCommitDate) => tagDate > buildCommitDate)
          ));
      });
    }).catch((error) => {
      if (!error.message) {
        console.error(error);
      } else {
        throw error;
      }
    });
  };
}));
