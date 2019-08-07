import { Jed } from 'jed';
import jedGettextParser from 'jed-gettext-parser';

export default class Translator {
  private jed?: Jed;

  public load(locale: string): Promise<void> {
    const defaultTranslationData = {
      messages: {
        '': {
          domain: 'messages',
          lang: 'en',
          // eslint-disable-next-line @typescript-eslint/camelcase
          plural_forms: 'nplurals=2; plural=(n != 1);'
        }
      }
    };

    let translationDataPromise;
    if (locale === 'en') {
      /* Just resolve to an empty data set. */
      translationDataPromise = Promise.resolve(defaultTranslationData);
    } else {
      const url = `http://loot/l10n/${locale}/LC_MESSAGES/loot.mo`;
      translationDataPromise = fetch(url)
        .then(response => {
          if (response.ok) {
            return response.arrayBuffer();
          }
          throw new Error(response.statusText);
        })
        .then(jedGettextParser.mo.parse);
    }

    return translationDataPromise
      .catch(error => {
        console.error(`Error loading translation data: ${error.message}`); // eslint-disable-line no-console
        return defaultTranslationData;
      })
      .then(result => {
        this.jed = new Jed({
          // eslint-disable-next-line @typescript-eslint/camelcase
          locale_data: result,
          domain: 'messages'
        });
      });
  }

  public translate(text: string): string {
    return this.translateFormatted(text);
  }

  public translateFormatted(
    text: string,
    ...substitutions: (string | object)[]
  ): string {
    if (text === undefined) {
      return '';
    }
    if (this.jed === undefined) {
      return text;
    }
    return this.jed.translate(text).fetch(...substitutions);
  }
}
