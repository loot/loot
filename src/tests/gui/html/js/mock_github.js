/* eslint-disable class-methods-use-this */

'use strict';

/* Mock the GitHub library methods used. */
class Repo {
  getRelease(/* latest */) {
    return Promise.resolve({
      data: {
        tag_name: '0.9.2'
      }
    });
  }

  listTags() {
    return Promise.resolve({
      data: [
        {
          name: '0.9.2',
          commit: {
            sha: '6b58f92a5d41f5d7f149a1263dac78687a065ff5'
          }
        },
        {
          name: '0.9.1',
          commit: {
            sha: 'dc24e10a4774903ede4e94165e7d6fa806466e4a'
          }
        },
        {
          name: '0.9.0',
          commit: {
            sha: '44a0d8505d5402dd24cf0fda9540da9557866c80'
          }
        }
      ]
    });
  }

  getCommit(/* sha */) {
    return Promise.resolve({
      data: {
        committer: {
          date: '2011-04-14T16:00:49Z'
        }
      }
    });
  }

  getSingleCommit(ref) {
    return Promise.resolve({
      data: {
        commit: {
          committer: {
            date:
              ref === 'deadbeef'
                ? '2011-04-14T16:00:00Z'
                : '2011-04-14T16:00:49Z'
          }
        }
      }
    });
  }
}

class GitHub {
  getRepo(/* owner, repo */) {
    return new Repo();
  }
}

window.GitHub = GitHub;
