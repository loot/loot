'use strict';

describe('updateExists()', () => {
  it('should reject if no arguments are passed', () =>
    loot.updateExists().catch((error) =>
      error.should.be.an('error')
    )
  );

  it('should reject if one argument is passed', () =>
    loot.updateExists('1.0.0').catch((error) =>
      error.should.be.an('error')
    )
  );

  it('should reject if passed a version number has less than three parts', () =>
    loot.updateExists('1.0', 'deadbeef').catch((error) =>
      error.should.be.an('error')
    )
  );

  it('should reject if passed a version number has more than three parts', () =>
    loot.updateExists('1.0.0.0', 'deadbeef').catch((error) =>
      error.should.be.an('error')
    )
  );

  it('should reject if the version number given contains non-digit, non-period characters', () =>
    loot.updateExists('1.0a.0', 'deadbeef').catch((error) =>
      error.should.be.an('error')
    )
  );

  it('should resolve to true if the given version is less than the latest version', () =>
    loot.updateExists('0.9.1', 'deadbeef').then((result) =>
      result.should.be.true
    )
  );

  it('should resolve to false if the given version is greater than the latest version', () =>
    loot.updateExists('0.10.0', 'deadbeef').then((result) =>
      result.should.be.false
    )
  );

  it('should resolve to true if the given version equals the latest version but the short build SHAs are unequal and the latest version commit is newer', () =>
    loot.updateExists('0.9.2', 'deadbeef').then((result) =>
      result.should.be.true
    )
  );

  it('should resolve to false if the given version equals the latest version but the short build SHAs are unequal and the build commits dates are equal', () =>
    loot.updateExists('0.9.2', 'feedbac').then((result) =>
      result.should.be.false
    )
  );

  it('should resolve to false if the given version equals the latest version and the short build SHAs are equal', () =>
    loot.updateExists('0.9.2', '6b58f92').then((result) =>
      result.should.be.false
    )
  );
});
