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
    <https://www.gnu.org/licenses/>.
    */

#ifndef LOOT_ERROR_CATEGORIES
#define LOOT_ERROR_CATEGORIES

#include <system_error>

#include "loot/api_decorator.h"

namespace loot {
/** @brief Get the error category that can be used to identify system_error
 *         exceptions that are due to libloadorder errors.
 *  @returns A reference to the static object of unspecified runtime type,
             derived from std::error_category.
 */
LOOT_API const std::error_category& libloadorder_category();

/** @brief Get the error category that can be used to identify system_error
 *         exceptions that are due to libgit2 errors.
 *  @returns A reference to the static object of unspecified runtime type,
             derived from std::error_category.
 */
LOOT_API const std::error_category& libgit2_category();
}

#endif
