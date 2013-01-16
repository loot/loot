/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012    WrinklyNinja

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

#ifndef __BOSS_ERROR__
#define __BOSS_ERROR__

#include <exception>

namespace boss {

    const unsigned int OK                          = 0;
    const unsigned int ERROR_LIBLO_ERROR           = 1;
    const unsigned int ERROR_PATH_WRITE_FAIL       = 2;
    const unsigned int ERROR_PATH_READ_FAIL        = 3;
    const unsigned int ERROR_CONDITION_EVAL_FAIL   = 4;
    const unsigned int ERROR_REGEX_EVAL_FAIL       = 5;
    const unsigned int ERROR_NO_MEM                = 6;
    const unsigned int ERROR_INVALID_ARGS          = 7;
    const unsigned int ERROR_NO_TAG_MAP            = 8;
    const unsigned int ERROR_PATH_NOT_FOUND        = 9;

    class error : public std::exception {
    public:
        error(const int code_arg, const std::string& what_arg) : _code(code_arg), _what(what_arg) {}
        ~error() throw() {};

        unsigned int code() const { return _code; }
        const char * what() const throw() { return _what.c_str(); }
    private:
        std::string _what;
        unsigned int _code;
    };

}

#endif
