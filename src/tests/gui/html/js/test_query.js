'use strict';

/* Mock the window.cefQuery method */
window.cefQuery = (obj) => {
  if (obj.request === '{"name":"fail","args":[]}') {
    obj.onFailure(-1, obj.request);
  } else {
    obj.onSuccess(obj.request);
  }
};

describe('query()', () => {
  it('should throw if no arguments are passed', () => {
    (() => { loot.query(); }).should.throw();
  });

  it('should return a promise', () => {
    loot.query('test').should.be.a('promise'); // eslint-disable-line new-cap
  });

  it('should succeed if a request name is passed', () =>
    loot.query('test').then((result) =>
      result.should.equal('{"name":"test","args":[]}')
    )
  );

  it('should succeed if a request name and arguments are passed', () =>
    loot.query('test', 1, false, ['a']).then((result) =>
      result.should.equal(JSON.stringify({
        name: 'test',
        args: [
          1,
          false,
          ['a'],
        ],
      }))
    )
  );

  it('should fail with an Error object when an error occurs', () =>
    loot.query('fail').catch((error) => {
      error.should.be.an('error');
      return error.message.should.equal('{"name":"fail","args":[]}');
    })
  );
});
