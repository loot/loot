'use strict';

/* Mock the octokat methods used. */
function GitHub() {  // eslint-disable-line no-unused-vars
  this.getRepo = (/* owner, repo */) => ({
    getRelease: (/* latest */) => (
      Promise.resolve({
        data: {
          tag_name: '0.9.2',
        },
      })
    ),
    listTags: () => (
      Promise.resolve({
        data: [{
          name: '0.9.2',
          commit: {
            sha: '6b58f92a5d41f5d7f149a1263dac78687a065ff5',
          },
        }, {
          name: '0.9.1',
          commit: {
            sha: 'dc24e10a4774903ede4e94165e7d6fa806466e4a',
          },
        }, {
          name: '0.9.0',
          commit: {
            sha: '44a0d8505d5402dd24cf0fda9540da9557866c80',
          },
        }],
      })
    ),
    getCommit: (/* sha */) => (
      Promise.resolve({
        data: {
          committer: {
            date: '2011-04-14T16:00:49Z',
          },
        },
      })
    ),
    getSingleCommit: (ref) => (
      Promise.resolve({
        data: {
          commit: {
            committer: {
              date: ref === 'deadbeef' ? '2011-04-14T16:00:00Z' : '2011-04-14T16:00:49Z',
            },
          },
        },
      })
    ),
  });
}
