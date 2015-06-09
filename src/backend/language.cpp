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

    Language::Language(const std::string& nameOrCode) {
        if (nameOrCode == Language(Language::english).Name() || nameOrCode == Language(Language::english).Locale())
            Construct(Language::english);
        else if (nameOrCode == Language(Language::spanish).Name() || nameOrCode == Language(Language::spanish).Locale())
            Construct(Language::spanish);
        else if (nameOrCode == Language(Language::russian).Name() || nameOrCode == Language(Language::russian).Locale())
            Construct(Language::russian);
        else if (nameOrCode == Language(Language::french).Name() || nameOrCode == Language(Language::french).Locale())
            Construct(Language::french);
        else if (nameOrCode == Language(Language::chinese).Name() || nameOrCode == Language(Language::chinese).Locale())
            Construct(Language::chinese);
        else if (nameOrCode == Language(Language::polish).Name() || nameOrCode == Language(Language::polish).Locale())
            Construct(Language::polish);
        else if (nameOrCode == Language(Language::brazilian_portuguese).Name() || nameOrCode == Language(Language::brazilian_portuguese).Locale())
            Construct(Language::brazilian_portuguese);
        else if (nameOrCode == Language(Language::finnish).Name() || nameOrCode == Language(Language::finnish).Locale())
            Construct(Language::finnish);
        else if (nameOrCode == Language(Language::german).Name() || nameOrCode == Language(Language::german).Locale())
            Construct(Language::german);
        else if (nameOrCode == Language(Language::danish).Name() || nameOrCode == Language(Language::danish).Locale())
            Construct(Language::danish);
        else if (nameOrCode == Language(Language::korean).Name() || nameOrCode == Language(Language::korean).Locale())
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

    const std::vector<std::string> Language::Names({
        Language(Language::english).Name(),
        Language(Language::spanish).Name(),
        Language(Language::russian).Name(),
        Language(Language::french).Name(),
        Language(Language::chinese).Name(),
        Language(Language::polish).Name(),
        Language(Language::brazilian_portuguese).Name(),
        Language(Language::finnish).Name(),
        Language(Language::german).Name(),
        Language(Language::danish).Name(),
        Language(Language::korean).Name()
    });
}
