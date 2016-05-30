'use strict';

(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define(['bower_components/Jed/jed',
           'bower_components/jed-gettext-parser/jedGettextParser'],
           factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.Translator = factory(root.Jed, root.jedGettextParser);
  }
}(this, (Jed, jedGettextParser) => class {
  /* Returns a Promise */
  constructor(locale) {
    this.locale = locale || 'en';
    this.jed = undefined;
  }

  load() {
    const defaultTranslationData = {
      messages: {
        '': {
          domain: 'messages',
          lang: 'en',
          plural_forms: 'nplurals=2; plural=(n != 1);',
        },
      },
    };

    let translationDataPromise;
    if (this.locale === 'en') {
      /* Just resolve to an empty data set. */
      translationDataPromise = Promise.resolve(defaultTranslationData);
    } else {
      translationDataPromise = new Promise((resolve, reject) => {
        const url = `loot://l10n/${this.locale}/LC_MESSAGES/loot.mo`;
        const xhr = new XMLHttpRequest();
        xhr.open('GET', url);
        xhr.responseType = 'arraybuffer';
        xhr.addEventListener('readystatechange', (evt) => {
          if (evt.target.readyState === 4) {
            /* Status is 0 for local file URL loading. */
            if (evt.target.status >= 200 && evt.target.status < 400) {
              resolve(jedGettextParser.mo.parse(evt.target.response));
            } else {
              reject(new Error(evt.target.statusText));
            }
          }
        }, false);
        xhr.send();
      });
    }

    return translationDataPromise.catch((error) => {
      console.log(`Error loading translation data: ${error.message}`); // eslint-disable-line no-console
      return defaultTranslationData;
    }).then((result) => {
      this.jed = new Jed({
        locale_data: result,
        domain: 'messages',
      });
    });
  }

  translate(text, ...substitutions) {
    if (text === undefined) {
      return '';
    }
    if (this.jed === undefined) {
      return text;
    }
    const func = this.jed.translate(text);
    return func.fetch.apply(func, substitutions);
  }
}));
