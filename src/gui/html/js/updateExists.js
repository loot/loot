import Octokit from '@octokit/rest';

const versionRegex = /^(\d+)\.(\d+)\.(\d+)$/;

function compare(lhs, rhs) {
  if (!versionRegex.test(lhs) || !versionRegex.test(rhs)) {
    throw new Error(
      `versions to compare are of unexpected format: ${lhs}, ${rhs}`
    );
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

export default function updateExists(currentVersion, currentBuild) {
  if (currentVersion === undefined || currentBuild === undefined) {
    return Promise.reject(
      new Error('Invalid arguments, both version and build must be given')
    );
  }

  const octokit = new Octokit();

  const repo = {
    owner: 'loot',
    repo: 'loot'
  };

  return octokit.repos
    .getLatestRelease(repo)
    .then(response => {
      const latestReleaseTagName = response.data.tag_name;
      const comparison = compare(currentVersion, latestReleaseTagName);
      if (comparison === -1) {
        return true;
      } else if (comparison === 1) {
        return false;
      }

      return octokit.repos
        .getTags(repo)
        .then(tagsResponse =>
          tagsResponse.data.find(
            element => element.name === latestReleaseTagName
          )
        )
        .then(tag => {
          if (tag.commit.sha.startsWith(currentBuild)) {
            return false;
          }

          return octokit.gitdata
            .getCommit({ ...repo, commit_sha: tag.commit.sha })
            .then(commitResponse =>
              Date.parse(commitResponse.data.committer.date)
            )
            .then(tagDate =>
              octokit.repos
                .getCommit({ ...repo, sha: currentBuild })
                .then(commitResponse =>
                  Date.parse(commitResponse.data.commit.committer.date)
                )
                .then(buildCommitDate => tagDate > buildCommitDate)
            );
        });
    })

    .catch(error => {
      console.error(`Failed to check for LOOT updates, details: ${error}`); // eslint-disable-line no-console
      throw error;
    });
}
