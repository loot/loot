describe('query()', () => {
  it('should throw if no arguments are passed', () => {
    (() => {
      loot.query();
    }).should.throw();
  });

  it('should return a promise', () => {
    loot.query('test').should.be.a('promise'); // eslint-disable-line new-cap
  });

  it('should succeed if a request name is passed', () =>
    loot
      .query('discardUnappliedChanges')
      .then(result => result.should.be.empty));

  it('should succeed if a request name and arguments are passed', () =>
    loot
      .query('copyContent', { messages: {} })
      .then(result => result.should.be.empty));

  it('should fail with an Error object when an error occurs', () =>
    loot.query('copyContent').catch(error => error.should.be.an('error')));
});
