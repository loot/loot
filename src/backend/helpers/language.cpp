/*  LOOT

    A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

    Copyright (C) 2012-2016    WrinklyNinja

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

    Language::Language(const Code code) {
        Construct(code);
    }

    Language::Language(const std::string& locale) {
        for (Code code : Codes) {
            if (locale == Language(code).GetLocale()) {
                Construct(code);
                return;
            }
        }

        Construct(Code::english);
    }

    void Language::Construct(const Code code) {
        _code = code;
        if (_code == Code::spanish) {
            _name = "Español";
            _locale = "es";
        }
        else if (_code == Code::russian) {
            _name = "Русский";
            _locale = "ru";
        }
        else if (_code == Code::french) {
            _name = "Français";
            _locale = "fr";
        }
        else if (_code == Code::chinese) {
            _name = "简体中文";
            _locale = "zh_CN";
        }
        else if (_code == Code::polish) {
            _name = "Polski";
            _locale = "pl";
        }
        else if (_code == Code::brazilian_portuguese) {
            _name = "Português do Brasil";
            _locale = "pt_BR";
        }
        else if (_code == Code::finnish) {
            _name = "suomi";
            _locale = "fi";
        }
        else if (_code == Code::german) {
            _name = "Deutsch";
            _locale = "de";
        }
        else if (_code == Code::danish) {
            _name = "Dansk";
            _locale = "da";
        }
        else if (_code == Code::korean) {
            _name = "한국어";
            _locale = "ko";
        }
        else {
            _code = Code::english;
            _name = "English";
            _locale = "en";
        }
    }

    Language::Code Language::GetCode() const {
        return _code;
    }

    std::string Language::GetName() const {
        return _name;
    }

    std::string Language::GetLocale() const {
        return _locale;
    }

    const std::vector<Language::Code> Language::Codes({
        Code::english,
        Code::spanish,
        Code::russian,
        Code::french,
        Code::chinese,
        Code::polish,
        Code::brazilian_portuguese,
        Code::finnish,
        Code::german,
        Code::danish,
        Code::korean
    });
}
