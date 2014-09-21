'use strict';

(function (root, factory) {
    if (typeof define === 'function' && define.amd) {
        // AMD. Register as an anonymous module.
        define(['jed', 'jedGettextParser'], factory);
    } else {
        // Browser globals
        root.amdWeb = factory(root.jed, root.jedGettextParser);
    }
}(this, function (jed, jedGettextParser) {
    //use b in some fashion.

    // Just return a value to define the module export.
    // This example returns an object, but the module
    // can return a function as the exported value.
    return {

        loadLocaleData: function(locale) {

            var url = 'loot://l10n/' + locale + '/LC_MESSAGES/loot.mo';

            return new Promise(function(resolve, reject){
                var xhr = new XMLHttpRequest();
                xhr.open('GET', url);
                xhr.responseType = 'arraybuffer';
                xhr.addEventListener('readystatechange', function(evt){
                    if (evt.target.readyState == 4) {
                        /* Status is 0 for local file URL loading. */
                        if (evt.target.status >= 200 && evt.target.status < 400) {
                            resolve(jedGettextParser.mo.parse(evt.target.response));
                        } else {
                            reject(new Error('XHR Error'));
                        }
                    }
                }, false);
                xhr.send();
            });
        },

        translateStaticText: function(l10n) {
            document.getElementById('hideVersionNumbers').nextElementSibling.textContent = l10n.translate("Hide version numbers").fetch();
        },

        getJedInstance: function(locale) {
            return this.loadLocaleData(locale).then(function(result){
                return new jed({
                    'locale_data': result,
                    'domain': 'messages'
                });
            });
        }

    };
}));


