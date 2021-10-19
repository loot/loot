import { Octokit } from '@octokit/rest';

interface Repo {
  owner: string;
  repo: string;
}

interface Tag {
  name: string;
  commit: {
    sha: string;
  };
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

function findTag(tags: Tag[], tagName: string): Tag | undefined {
  return tags.find(element => element.name === tagName);
}

async function paginatedFindTag(
  octokit: Octokit,
  repo: Repo,
  tagName: string
): Promise<Tag | undefined> {
  const iterator = octokit.paginate.iterator(octokit.repos.listTags, {
    ...repo,
    // eslint-disable-next-line @typescript-eslint/naming-convention
    per_page: 100
  });

  for await (const page of iterator) {
    const tag = findTag(page.data, tagName);
    if (tag) {
      return tag;
    }
  }

  return undefined;
}

export default async function updateExists(
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

  try {
    const latestReleaseResponse = await octokit.repos.getLatestRelease(repo);

    const latestReleaseTagName = latestReleaseResponse.data.tag_name;
    const comparison = compare(currentVersion, latestReleaseTagName);
    if (comparison === -1) {
      return true;
    }
    if (comparison === 1) {
      return false;
    }

    const tag = await paginatedFindTag(octokit, repo, latestReleaseTagName);

    if (!tag) {
      throw new Error(
        `Couldn't find tag data for tag "${latestReleaseTagName}"`
      );
    }

    if (tag.commit.sha.startsWith(currentBuild)) {
      return false;
    }

    const gitCommitResponse = await octokit.git.getCommit({
      ...repo,
      // eslint-disable-next-line @typescript-eslint/naming-convention
      commit_sha: tag.commit.sha
    });

    const tagDate = Date.parse(gitCommitResponse.data.committer.date);

    const repoCommitResponse = await octokit.repos.getCommit({
      ...repo,
      ref: currentBuild
    });

    const dateString = repoCommitResponse.data.commit.committer?.date;
    if (!dateString) {
      throw new Error(
        `Couldn't get commit date for tag "${latestReleaseTagName}"`
      );
    }

    return tagDate > Date.parse(dateString);
  } catch (error) {
    // eslint-disable-next-line no-console, @typescript-eslint/restrict-template-expressions
    console.error(`Failed to check for LOOT updates, details: ${error}`);
    throw error;
  }
}
