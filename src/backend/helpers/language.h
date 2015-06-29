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

#ifndef __LOOT_LANGUAGE__
#define __LOOT_LANGUAGE__

#include <string>
#include <vector>

namespace loot {
    //Language class for simpler language support.
    class Language {
    public:
        Language(const unsigned int code);
        Language(const std::string& locale);

        unsigned int Code() const;
        std::string Name() const;
        std::string Locale() const;

        static const unsigned int any; // This shouldn't be used as a selectable language, just for when picking any string in a message.
        static const unsigned int english;
        static const unsigned int spanish;
        static const unsigned int russian;
        static const unsigned int french;
        static const unsigned int chinese;
        static const unsigned int polish;
        static const unsigned int brazilian_portuguese;
        static const unsigned int finnish;
        static const unsigned int german;
        static const unsigned int danish;
        static const unsigned int korean;

        static const std::vector<unsigned int> Codes;
    private:
        unsigned int _code;
        std::string _name;
        std::string _locale;

        void Construct(const unsigned int code);
    };
}

#endif
