import { Jed, LocaleData } from 'jed';
import Translator from '../../../../gui/html/js/translator';

describe('Translator', () => {
  let mockedJed: Jed;

  beforeEach(() => {
    const localeData: LocaleData = {
      messages: {
        '': {
          domain: 'messages',
          lang: 'en',
          // eslint-disable-next-line @typescript-eslint/camelcase
          plural_forms: 'nplurals=2; plural=(n != 1);'
        },
        foo: ['bar'],
        'foo %1$s %2$s': ['bar is bar']
      }
    };
    mockedJed = {
      translate: (input: string) => ({
        fetch: () => {
          const data = localeData.messages[input];
          if (data instanceof Array) {
            return data[0];
          }
          return input;
        }
      })
    };
  });

  describe('#load()', () => {
    test('should return a Promise', () => {
      const l10n = new Translator();

      return l10n.load('en').then(result => {
        expect(result).toBe(undefined);
      });
    });

    /* Cannot test rejection or other locales as the URL used is invalid in the
       browser, causing a CORS error that cannot be handled in JavaScript. */
  });

  describe('#translate()', () => {
    test('should return original string if the translator has not been loaded', () => {
      const l10n = new Translator();

      expect(l10n.translate('foo')).toBe('foo');
    });

    test('should return the input string if the current locale is "en"', () => {
      const l10n = new Translator();

      l10n.load('en').then(() => {
        expect(l10n.translate('foo')).toBe('foo');
      });
    });

    test('should return the translated string if locale data has been loaded', () => {
      const l10n = new Translator();
      /* Since loading data doesn't work in the browser, hack it by setting some
         data manually. */
      l10n['jed'] = mockedJed; // eslint-disable-line dot-notation

      expect(l10n.translate('foo')).toBe('bar');
    });
  });

  describe('#translateFormatted()', () => {
    test('should return input string if the translator has not been loaded', () => {
      const l10n = new Translator();

      expect(l10n.translateFormatted('foo')).toBe('foo');
    });

    test('should return the input string if the current locale is "en"', () => {
      const l10n = new Translator();

      l10n.load('en').then(() => {
        expect(l10n.translateFormatted('foo')).toBe('foo');
      });
    });

    test('should return the translated string if locale data has been loaded', () => {
      const l10n = new Translator();
      /* Since loading data doesn't work in the browser, hack it by setting some
         data manually. */
      l10n['jed'] = mockedJed; // eslint-disable-line dot-notation

      expect(l10n.translateFormatted('foo')).toBe('bar');
    });

    test('should subsitute additional arguments into string', () => {
      const l10n = new Translator();
      /* Since loading data doesn't work in the browser, hack it by setting some
         data manually. */
      l10n['jed'] = mockedJed; // eslint-disable-line dot-notation

      expect(l10n.translateFormatted('foo %1$s %2$s', 'is not', 'bar')).toBe(
        'bar is bar'
      );
    });
  });
});
