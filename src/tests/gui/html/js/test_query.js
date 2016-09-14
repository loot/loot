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
    loot.query('test').should.be.a.Promise(); // eslint-disable-line new-cap
  });

  it('should succeed if a request name is passed', () =>
    loot.query('test').should.be.fulfilledWith('{"name":"test","args":[]}')
  );

  it('should succeed if a request name and arguments are passed', () =>
    loot.query('test', 1, false, ['a']).should.be.fulfilledWith(JSON.stringify({
      name: 'test',
      args: [
        1,
        false,
        ['a'],
      ],
    }))
  );

  it('should fail with an Error object when an error occurs', () =>
    loot.query('fail').should.be.rejectedWith(Error, { message: 'Error code: -1; {"name":"fail","args":[]}' })
  );
});
