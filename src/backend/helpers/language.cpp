/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2015    WrinklyNinja

    This file is part of LOOT.

    LOOT is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.  If not, see
    <http://www.gnu.org/licenses/>.
    */

#include "language.h"

namespace loot {
    Language::Language(const unsigned int code) {
        Construct(code);
    }

    Language::Language(const std::string& locale) {
        if (locale == Language(Language::english).Locale())
            Construct(Language::english);
        else if (locale == Language(Language::spanish).Locale())
            Construct(Language::spanish);
        else if (locale == Language(Language::russian).Locale())
            Construct(Language::russian);
        else if (locale == Language(Language::french).Locale())
            Construct(Language::french);
        else if (locale == Language(Language::chinese).Locale())
            Construct(Language::chinese);
        else if (locale == Language(Language::polish).Locale())
            Construct(Language::polish);
        else if (locale == Language(Language::brazilian_portuguese).Locale())
            Construct(Language::brazilian_portuguese);
        else if (locale == Language(Language::finnish).Locale())
            Construct(Language::finnish);
        else if (locale == Language(Language::german).Locale())
            Construct(Language::german);
        else if (locale == Language(Language::danish).Locale())
            Construct(Language::danish);
        else if (locale == Language(Language::korean).Locale())
            Construct(Language::korean);
        else
            Construct(Language::english);
    }

    void Language::Construct(const unsigned int code) {
        _code = code;
        if (_code == Language::spanish) {
            _name = "Español";
            _locale = "es";
        }
        else if (_code == Language::russian) {
            _name = "Русский";
            _locale = "ru";
        }
        else if (_code == Language::french) {
            _name = "Français";
            _locale = "fr";
        }
        else if (_code == Language::chinese) {
            _name = "简体中文";
            _locale = "zh_CN";
        }
        else if (_code == Language::polish) {
            _name = "Polski";
            _locale = "pl";
        }
        else if (_code == Language::brazilian_portuguese) {
            _name = "Português do Brasil";
            _locale = "pt_BR";
        }
        else if (_code == Language::finnish) {
            _name = "suomi";
            _locale = "fi";
        }
        else if (_code == Language::german) {
            _name = "Deutsch";
            _locale = "de";
        }
        else if (_code == Language::danish) {
            _name = "Dansk";
            _locale = "da";
        }
        else if (_code == Language::korean) {
            _name = "한국어";
            _locale = "ko";
        }
        else {
            _code = Language::english;
            _name = "English";
            _locale = "en";
        }
    }

    unsigned int Language::Code() const {
        return _code;
    }

    std::string Language::Name() const {
        return _name;
    }

    std::string Language::Locale() const {
        return _locale;
    }

    const std::vector<unsigned int> Language::Codes({
        Language::english,
        Language::spanish,
        Language::russian,
        Language::french,
        Language::chinese,
        Language::polish,
        Language::brazilian_portuguese,
        Language::finnish,
        Language::german,
        Language::danish,
        Language::korean
    });
}
