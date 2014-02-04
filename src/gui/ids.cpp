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

wxString translate(const std::string& str) {
    return wxString::FromUTF8(boost::locale::translate(str).str().c_str());
}

wxString FromUTF8(const std::string& str) {
    return wxString::FromUTF8(str.c_str());
}

wxString FromUTF8(const boost::format& f) {
    return FromUTF8(f.str());
}