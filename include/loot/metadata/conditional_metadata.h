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
#ifndef LOOT_METADATA_CONDITIONAL_METADATA
#define LOOT_METADATA_CONDITIONAL_METADATA

#include <string>

#include "loot/api_decorator.h"

namespace loot {
/**
 * A base class for metadata that can be conditional based on the result of
 * evaluating a condition string.
 */
class ConditionalMetadata {
public:
  /**
   * Construct a ConditionalMetadata object with an empty condition string.
   * @return A ConditionalMetadata object.
   */
  LOOT_API ConditionalMetadata();

  /**
   * Construct a ConditionalMetadata object with a given condition string.
   * @param  condition
   *         A condition string, as defined in the LOOT metadata syntax
   *         documentation.
   * @return A ConditionalMetadata object.
   */
  LOOT_API ConditionalMetadata(const std::string& condition);

  /**
   * Check if the condition string is non-empty.
   * @return True if the condition string is not empty, false otherwise.
   */
  LOOT_API bool IsConditional() const;

  /**
   * Check if the condition string is syntactically valid.
   *
   * Throws a ``ConditionSyntaxError`` if the condition string's syntax is not
   * valid.
   */
  LOOT_API void ParseCondition() const;

  /**
   * Get the condition string.
   * @return The object's condition string.
   */
  LOOT_API std::string GetCondition() const;
private:
  std::string condition_;
};
}
#endif
