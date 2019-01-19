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

function findTag(tags, tagName) {
  return tags.find(element => element.name === tagName);
}

async function paginatedFindTag(octokit, repo, tagName) {
  let response = await octokit.repos.listTags({ ...repo, per_page: 100 });
  let tag = findTag(response.data, tagName);

  while (!tag && octokit.hasNextPage(response)) {
    response = await octokit.getNextPage(response); // eslint-disable-line no-await-in-loop
    tag = findTag(response.data, tagName);
  }

  return tag;
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
      }
      if (comparison === 1) {
        return false;
      }

      return paginatedFindTag(octokit, repo, latestReleaseTagName).then(tag => {
        if (!tag) {
          throw new Error(
            `Couldn't find tag data for tag "${latestReleaseTagName}"`
          );
        }

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
