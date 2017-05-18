'use strict';

/* Mock the octokat methods used. */
function Octokat() {  // eslint-disable-line no-unused-vars
  this.repos = (/* owner, repo */) => ({
    releases: {
      latest: {
        fetch() {
          return Promise.resolve({
            tagName: '0.9.2',
          });
        },
      },
    },
    tags: {
      fetch() {
        return Promise.resolve([{
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
        }]);
      },
    },
  });
}
