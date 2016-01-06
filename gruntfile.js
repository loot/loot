'use strict';
module.exports = (grunt) => {
  grunt.initConfig({
    connect: {
      server: {
        options: {
          base: '',
          port: 9999,
        },
      },
    },
    'saucelabs-mocha': {
      all: {
        options: {
          urls: ['http://127.0.0.1:9999/src/tests/gui/html/js/test.html'],
          build: process.env.TRAVIS_JOB_ID,
          throttled: 3,
          browsers: [{
            browserName: 'chrome',
            platform: 'Windows 10',
            version: '47',
          }],
          testname: 'LOOT UI JS Tests',
        },
      },
    },
    watch: {},
  });

  // Loading dependencies
  for (const key in grunt.file.readJSON('package.json').devDependencies) {
    if (key !== 'grunt' && key.indexOf('grunt') === 0) {
      grunt.loadNpmTasks(key);
    }
  }

  grunt.registerTask('dev', ['connect', 'watch']);
  grunt.registerTask('test', ['connect', 'saucelabs-mocha']);
};
