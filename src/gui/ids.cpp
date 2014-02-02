/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2013-2014    WrinklyNinja

    This file is part of BOSS.

    BOSS is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#include "ids.h"
#include "../backend/globals.h"

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

wxString Language[4] = {
    translate("None Specified"),
    wxT("English"),
	wxString::FromUTF8("Español"),
    wxString::FromUTF8("Русский"),
};

wxString translate(const std::string& str) {
    return wxString::FromUTF8(boost::locale::translate(str).str().c_str());
}

wxString FromUTF8(const std::string& str) {
    return wxString::FromUTF8(str.c_str());
}

wxString FromUTF8(const boost::format& f) {
    return FromUTF8(f.str());
}

unsigned int GetLangIndex(const std::string& str) {
    if (boost::iequals(str, "eng"))
        return 1;
    else if (boost::iequals(str, "spa"))
        return 2;
    else if (boost::iequals(str, "rus"))
        return 3;
    else
        return 0;
}

std::string GetLangStringFromIndex(const unsigned int index) {
    if (index == 1)
        return "eng";
    else if (index == 2)
        return "spa";
    else if (index == 3)
        return "rus";
    else
        return "";
}

unsigned int GetLangNum(const wxString& str) {
    for (int i=0; i < 4; ++i) {
        if (str == Language[i])
            return GetLangNum(GetLangStringFromIndex(i));
    }
    return boss::g_lang_any;
}

unsigned int GetLangIndex(const wxString& str) {
    for (int i=0; i < 4; ++i) {
        if (str == Language[i])
            return i;
    }
    return 0;
}
