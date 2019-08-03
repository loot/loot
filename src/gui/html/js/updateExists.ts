import * as Octokit from '@octokit/rest';

interface Repo {
  owner: string;
  repo: string;
}

const versionRegex = /^(\d+)\.(\d+)\.(\d+)$/;

function compare(lhs: string, rhs: string): number {
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

function findTag(
  tags: Octokit.ReposListTagsResponseItem[],
  tagName: string
): Octokit.ReposListTagsResponseItem {
  return tags.find(element => element.name === tagName);
}

async function paginatedFindTag(
  octokit: Octokit,
  repo: Repo,
  tagName: string
): Promise<Octokit.ReposListTagsResponseItem> {
  const options = octokit.repos.listTags.endpoint.merge({
    ...repo,
    // eslint-disable-next-line @typescript-eslint/camelcase
    per_page: 100
  });

  // The version of Chromium used by LOOT natively supports async iterators, so
  // disable the syntax restriction lint.
  // eslint-disable-next-line no-restricted-syntax
  for await (const page of octokit.paginate.iterator(options)) {
    const tag = findTag(page.data, tagName);
    if (tag) {
      return tag;
    }
  }

  return undefined;
}

export default function updateExists(
  currentVersion: string,
  currentBuild: string
): Promise<boolean> {
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

        return octokit.git
          .getCommit({ ...repo, commit_sha: tag.commit.sha }) // eslint-disable-line @typescript-eslint/camelcase
          .then(commitResponse =>
            Date.parse(commitResponse.data.committer.date)
          )
          .then(tagDate =>
            octokit.repos
              .getCommit({ ...repo, ref: currentBuild })
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
