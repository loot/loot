import Translator from '../../../../gui/html/js/translator.js';

describe('Translator', () => {
  let localeData;

  beforeEach(() => {
    localeData = {
      messages: {
        '': {
          domain: 'messages',
          lang: 'en',
          plural_forms: 'nplurals=2; plural=(n != 1);'
        },
        foo: ['foo'],
        'foo %1$s %2$s': ['bar is bar']
      }
    };
    window.Jed = jest.fn().mockImplementation(() => ({
      translate: jest.fn().mockImplementation(input => ({
        fetch: () => localeData.messages[input][0]
      }))
    }));
  });

  describe('#Translator()', () => {
    test('should set locale to "en" if no locale is given', () => {
      const l10n = new Translator();
      expect(l10n.locale).toBe('en');
    });

    test('should set the locale to the given value', () => {
      const l10n = new Translator('de');
      expect(l10n.locale).toBe('de');
    });
  });

  describe('#load()', () => {
    test('should return a Promise', () => {
      const l10n = new Translator();

      return l10n.load().then(result => {
        expect(result).toBe(undefined);
      });
    });

    test('should be fulfilled for a locale of "en"', () => {
      const l10n = new Translator('en');

      return l10n.load();
    });

    /* Cannot test rejection or other locales as the URL uses is invalid in the
       browser, causing a CORS error that cannot be handled in JavaScript. */
  });

  describe('#translate()', () => {
    let l10n;

    beforeEach(() => {
      l10n = new Translator();
    });

    test('should return original string if the translator has not been loaded', () => {
      expect(l10n.translate('foo')).toBe('foo');
    });

    test('should return an empty string if nothing is passed', () =>
      l10n.load().then(() => {
        expect(l10n.translate()).toBe('');
      }));

    test('should return the input string if the current locale is "en"', () =>
      l10n.load().then(() => {
        expect(l10n.translate('foo')).toBe('foo');
      }));

    test('should return the translated string if locale data has been loaded', () => {
      /* Since loading data doesn't work in the browser, hack it by setting some
         data manually. */
      l10n.jed = window.Jed();
      localeData.messages.foo = ['bar'];

      expect(l10n.translate('foo')).toBe('bar');
    });
  });

  describe('#translateFormatted()', () => {
    let l10n;

    beforeEach(() => {
      l10n = new Translator();
    });

    test('should return original string if the translator has not been loaded', () => {
      expect(l10n.translateFormatted('foo')).toBe('foo');
    });

    test('should return an empty string if nothing is passed', () =>
      l10n.load().then(() => {
        expect(l10n.translateFormatted()).toBe('');
      }));

    test('should return the input string if the current locale is "en"', () =>
      l10n.load().then(() => {
        expect(l10n.translateFormatted('foo')).toBe('foo');
      }));

    test('should return the translated string if locale data has been loaded', () => {
      /* Since loading data doesn't work in the browser, hack it by setting some
         data manually. */
      l10n.jed = window.Jed();
      localeData.messages.foo = ['bar'];

      expect(l10n.translateFormatted('foo')).toBe('bar');
    });

    test('should subsitute additional arguments into string', () => {
      /* Since loading data doesn't work in the browser, hack it by setting some
         data manually. */
      l10n.jed = window.Jed();

      expect(l10n.translateFormatted('foo %1$s %2$s', 'is not', 'bar')).toBe(
        'bar is bar'
      );
    });
  });
});
