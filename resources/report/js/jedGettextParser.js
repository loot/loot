/* The text-encoding module is only strictly required for Node.js, and may not
   be required in recent browsers, so don't specify it for them. */
'use strict';
(function(root, factory) {
    if (typeof define === 'function' && define.amd) {
        // AMD. Register as an anonymous module.
        define([], factory);
    } else if (typeof exports === 'object') {
        // Node. Does not work with strict CommonJS, but
        // only CommonJS-like environments that support module.exports,
        // like Node.
        module.exports = factory(require('text-encoding'));
    } else {
        // Browser globals
        root.jedGettextParser = factory();
    }
}(this, function(textEncoding) {
    /* Return what this module exports. */

    if (textEncoding) {
        var TextDecoder = textEncoding.TextDecoder;
    } else {
        var TextDecoder = window.TextDecoder;
    }

    function Parser() {
        this._littleEndian;
        this._dataView;
        this._encoding;

        this._originalOffset;
        this._translationOffset;
    }
    Parser.prototype._MAGIC = 0x950412de;

    Parser.prototype._getEndianness = function() {
        /* MO files can be big or little endian, independent of the source or current platform. Use DataView's optional get*** argument to set the endianness if necessary. */
        if (this._dataView.getUint32(0, true) == this._MAGIC) {
            this._littleEndian = true;
        } else if (this._dataView.getUint32(0, false) == this._MAGIC){
            this._littleEndian = false;
        } else {
            throw new Error('Not a gettext binary message catalog file.');
        }
    }

    Parser.prototype._readTranslationPair = function(originalOffset, translationOffset) {
        var length, position, idBytes, strBytes;
        /* Get original byte array, that forms the key. */
        length = this._dataView.getUint32(originalOffset, this._littleEndian);
        position = this._dataView.getUint32(originalOffset + 4, this._littleEndian);
        try {
            idBytes = new Uint8Array(this._dataView.buffer, position, length);
        } catch(e) {
            throw new Error('The given ArrayBuffer data is corrupt or incomplete.');
        }

        /* Get translation byte array, that forms the value. */
        length = this._dataView.getUint32(translationOffset, this._littleEndian);
        position = this._dataView.getUint32(translationOffset + 4, this._littleEndian);
        try {
            strBytes = new Uint8Array(this._dataView.buffer, position, length);
        } catch(e) {
            throw new Error('The given ArrayBuffer data is corrupt or incomplete.');
        }

        return {
            id: idBytes,
            str: strBytes
        };
    }

    Parser.prototype._parseHeader = function() {
        /* Read translation header. This is stored as a msgstr where the msgid
           is '', so it's the first entry in the translation block, since
           strings are sorted. Assume that the header is in UTF-8. We only want
           the language, encoding and plural forms values, which should all be
           ASCII anyway.
           */
        var msgBytes = this._readTranslationPair(this._originalOffset, this._translationOffset);

        var language, pluralForms;
        if (msgBytes.id.byteLength == 0) {
            var decoder = new TextDecoder();
            var str = decoder.decode(msgBytes.str);

            var headers = {};
            str.split("\n").forEach(function(line){
                /* Header format is like HTTP headers. */
                var parts = line.split(':');
                var key = parts.shift().trim();
                var value = parts.join(':').trim();
                headers[key] = value;
            });

            /* Get encoding if not given. */
            if (!this._encoding) {
                var pos = headers['Content-Type'].indexOf('charset=');

                if (pos != -1 && pos + 8 < headers['Content-Type'].length) {
                    /* TextDecoder expects a lowercased encoding name. */
                    this._encoding = headers['Content-Type'].substring(pos + 8).toLowerCase();
                }
                if (!this._encoding) {
                    this._encoding = 'utf-8';
                }
            }

            /* Get language from header. */
            language = headers['Language'];

            /* Get plural forms from header. */
            pluralForms = headers['Plural-Forms'];
        }

        return {
            '': {
                'domain': '',
                'lang': language,
                'plural_forms': pluralForms,
            }
        }
    }

    Parser.prototype._splitPlurals = function(msgid, msgstr) {
        /* Need to handle plurals. Don't need to handle contexts, because Jed
           expects the context-msgid strings to be its keys. However, plural
           translations must be split into an array of strings. Jed only wants
           the first part of a plural as its key. */
        return {
            id: msgid.split('\u0000')[0],
            str: msgstr.split('\u0000')
        }
    }

    Parser.prototype.parse = function(buffer, encoding) {

        /* A mo file can be empty apart from its magic, revision, strings count,
           offsets and hash table size, but fields for these must all exist, so
           verify the file is large enough. */
        if (buffer.byteLength < 28) {
            throw new Error('The given ArrayBuffer is too small to hold a valid .mo file.');
        }

        this._dataView = new DataView(buffer);
        this._encoding = encoding;

        this._getEndianness();

        /* Get size and offsets. Skip the revision and hash table, they're
           unnecessary. */
        var stringsCount = this._dataView.getUint32(8, this._littleEndian);
        this._originalOffset = this._dataView.getUint32(12, this._littleEndian);
        this._translationOffset = this._dataView.getUint32(16, this._littleEndian);

        /* Parse header for info, and use it to create the Jed locale_data
           object 'header'. */
        var jedLocaleData = this._parseHeader();

        /* Create a TextDecoder for encoding conversion. */
        try {
            var decoder = new TextDecoder(this._encoding);
        } catch(e) {
            throw new Error("The encoding label provided ('" + this._encoding + "') is invalid.");
        }

        /* Now get translations. */
        var originalOffset = this._originalOffset + 8;
        var translationOffset = this._translationOffset + 8;
        for (var i = 1; i < stringsCount; ++i) {
            var msgBytes = this._readTranslationPair(originalOffset, translationOffset);
            var msg = this._splitPlurals( decoder.decode(msgBytes.id), decoder.decode(msgBytes.str) );

            jedLocaleData[msg.id] = [].concat(msg.str);

            originalOffset += 8;
            translationOffset += 8;
        }

        return jedLocaleData;
    }

    return {

        mo: {
            parse: function(buffer, options) {

                /* Leave the encoding undefined if no options are given. */
                options = options || { domain: 'messages' };
                options.domain = options.domain || 'messages';

                if (buffer && buffer.byteLength == 0) {
                    throw new Error('Given ArrayBuffer is empty.');
                }

                if (!buffer || Object.prototype.toString.call(buffer) != '[object ArrayBuffer]') {
                    throw new Error('First argument must be an ArrayBuffer.');
                }

                var parser = new Parser();
                var messages = parser.parse(buffer, options.encoding);

                messages[''].domain = options.domain;
                var locale_data = {};
                locale_data[options.domain] = messages;
                return locale_data;
            }
        }
    };
}));
