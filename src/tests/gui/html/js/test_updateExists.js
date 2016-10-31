'use strict';

describe('updateExists()', () => {
  it('should reject if no arguments are passed', () => {
    return loot.updateExists().should.be.rejected();
  });

  it('should reject if one argument is passed', () => {
    return loot.updateExists('1.0.0').should.be.rejected();
  });

  it('should reject if passed a version number has less than three parts', () => {
    return loot.updateExists('1.0', 'deadbeef').should.be.rejected();
  });

  it('should reject if passed a version number has more than three parts', () => {
    return loot.updateExists('1.0.0.0', 'deadbeef').should.be.rejected();
  });

  it('should reject if the version number given contains non-digit, non-period characters', () => {
    return loot.updateExists('1.0a.0', 'deadbeef').should.be.rejected();
  });

  it('should resolve to true if the given version is less than the latest version', () => {
    return loot.updateExists('0.9.1', 'deadbeef').should.be.fulfilledWith(true);
  });

  it('should resolve to false if the given version is greater than the latest version', () => {
    return loot.updateExists('0.10.0', 'deadbeef').should.be.fulfilledWith(false);
  });

  it('should resolve to true if the given version equals the latest version but the short build SHAs are unequal', () => {
    return loot.updateExists('0.9.2', 'deadbeef').should.be.fulfilledWith(true);
  });

  it('should resolve to false if the given version equals the latest version and the short build SHAs are equal', () => {
    return loot.updateExists('0.9.2', '6b58f92').should.be.fulfilledWith(false);
  });
});
