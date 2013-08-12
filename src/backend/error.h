/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012-2013    WrinklyNinja

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
#include <string>

namespace boss {

    class error : public std::exception {
    public:
        error(const unsigned int code_arg, const std::string& what_arg) : _code(code_arg), _what(what_arg) {}
        ~error() throw() {};

        unsigned int code() const { return _code; }
        const char * what() const throw() { return _what.c_str(); }


        static const unsigned int ok                    = 0;
        static const unsigned int liblo_error           = 1;
        static const unsigned int path_write_fail       = 2;
        static const unsigned int path_read_fail        = 3;
        static const unsigned int condition_eval_fail   = 4;
        static const unsigned int regex_eval_fail       = 5;
        static const unsigned int no_mem                = 6;
        static const unsigned int invalid_args          = 7;
        static const unsigned int no_tag_map            = 8;
        static const unsigned int path_not_found        = 9;
        static const unsigned int no_game_detected      = 10;
        static const unsigned int subversion_error      = 11;
        static const unsigned int git_error             = 12;
        static const unsigned int windows_error         = 13;
        static const unsigned int sorting_error         = 14;
    private:
        std::string _what;
        unsigned int _code;
    };

}

#endif
