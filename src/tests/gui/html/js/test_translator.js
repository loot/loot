'use strict';

describe('Translator', () => {
  describe('#Tranlsator()', () => {
    it('should set locale to "en" if no locale is given', () => {
      const l10n = new loot.Translator();
      l10n.locale.should.equal('en');
    });

    it('should set the locale to the given value', () => {
      const l10n = new loot.Translator('de');
      l10n.locale.should.equal('de');
    });
  });

  describe('#load()', () => {
    it('should return a Promise', () => {
      const l10n = new loot.Translator();

      l10n.load().should.be.a.Promise(); // eslint-disable-line new-cap
    });

    it('should be fulfilled for a locale of "en"', () => {
      const l10n = new loot.Translator('en');

      return l10n.load().should.be.fulfilled();
    });

    /* Cannot test rejection or other locales as the URL uses is invalid in the
       browser, causing a CORS error that cannot be handled in JavaScript. */
  });

  describe('#translate()', () => {
    let l10n;

    beforeEach(() => {
      l10n = new loot.Translator();
    });

    it('should return original string if the translator has not been loaded', () => {
      l10n.translate('foo').should.equal('foo');
    });

    it('should return an empty string if nothing is passed', () =>
      l10n.load().then(() => {
        l10n.translate().should.equal('');
      })
    );

    it('should return the input string if the current locale is "en"', () =>
      l10n.load().then(() => {
        l10n.translate('foo').should.equal('foo');
      })
    );

    it('should return the translated string if locale data has been loaded', () => {
      /* Since loading data doesn't work in the browser, hack it by setting some
         data manually. */
      l10n.jed = new window.Jed({
        locale_data: {
          messages: {
            '': {
              domain: 'messages',
              lang: 'en',
              plural_forms: 'nplurals=2; plural=(n != 1);',
            },
            foo: ['bar'],
          },
        },
        domain: 'messages',
      });

      l10n.translate('foo').should.equal('bar');
    });
  });

  describe('#translateFormatted()', () => {
    let l10n;

    beforeEach(() => {
      l10n = new loot.Translator();
    });

    it('should return original string if the translator has not been loaded', () => {
      l10n.translateFormatted('foo').should.equal('foo');
    });

    it('should return an empty string if nothing is passed', () =>
      l10n.load().then(() => {
        l10n.translateFormatted().should.equal('');
      })
    );

    it('should return the input string if the current locale is "en"', () =>
      l10n.load().then(() => {
        l10n.translateFormatted('foo').should.equal('foo');
      })
    );

    it('should return the translated string if locale data has been loaded', () => {
      /* Since loading data doesn't work in the browser, hack it by setting some
         data manually. */
      l10n.jed = new window.Jed({
        locale_data: {
          messages: {
            '': {
              domain: 'messages',
              lang: 'en',
              plural_forms: 'nplurals=2; plural=(n != 1);',
            },
            foo: ['bar'],
          },
        },
        domain: 'messages',
      });

      l10n.translateFormatted('foo').should.equal('bar');
    });

    it('should subsitute additional arguments into string', () => {
      /* Since loading data doesn't work in the browser, hack it by setting some
         data manually. */
      l10n.jed = new window.Jed({
        locale_data: {
          messages: {
            '': {
              domain: 'messages',
              lang: 'en',
              plural_forms: 'nplurals=2; plural=(n != 1);',
            },
            'foo %1$s %2$s': ['%2$s is bar'],
          },
        },
        domain: 'messages',
      });

      l10n.translateFormatted('foo %1$s %2$s', 'is not', 'bar').should.equal('bar is bar');
    });
  });
});
