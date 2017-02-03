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
#ifndef LOOT_METADATA_PRIORITY
#define LOOT_METADATA_PRIORITY

#include <cstdint>

#include "loot/api_decorator.h"

namespace loot {
/**
 * Represents the priority of a plugin in the load order.
 *
 * Plugins have a default implicit priority of zero. Lower priority values cause
 * plugins to be loaded earlier, and higher priority values cause plugins to be
 * loaded later.
 */
class Priority {
public:
  /**
   * Construct a Priority object with an implicit value of zero.
   * @return A Priority object.
   */
  LOOT_API Priority();

  // Take an int to prevent literals that are too large for one byte from
  // wrapping around to negative values.
  /**
   * Construct a Priority object with the given priority value.
   *
   * If the given value is zero, it is marked as being set explicitly. This
   * affects how priority metadata values get merged in PluginMetadata objects.
   * @param  value
   *         The priority value to set. The valid range of values is -127 to 127
   *         inclusive, and values passed to the constructor that lie outside
   *         this range are clamped. The input type is an int to avoid invalid
   *         values from implicitly wrapping around.
   * @return A Priority object.
   */
  LOOT_API explicit Priority(const int value);

  /**
   * Get the stored priority value.
   * @return The priority value. While the valid value range fits in 8 bits,
   *         a short is returned to avoid interpreting the value as a character.
   */
  LOOT_API short GetValue() const;

  /**
   * Check if the priority value is explicit or not.
   * @return Returns true if the value is non-zero or was explicitly set to
   *         zero, and false otherwise.
   */
  LOOT_API bool IsExplicit() const;

  /**
   * Check if this Priority object is less than another.
   * @return True if this Priority object's value is less than the given
   *         Priority object's value.
   */
  LOOT_API bool operator < (const Priority& rhs) const;

  /**
   * Check if this Priority object is greater than another.
   * @return True if this Priority object's value is greater than the given
   *         Priority object's value, false otherwise.
   */
  LOOT_API bool operator > (const Priority& rhs) const;

  /**
   * Check if this Priority object is greater than or equal to another.
   * @return True if this Priority object's value is greater than or equal to
   *         the given Priority object's value, false otherwise.
   */
  LOOT_API bool operator >= (const Priority& rhs) const;

  /**
   * Check if this Priority object is equal to another.
   * @return True if this Priority object's value is equal to the given
   *         Priority object's value, false otherwise.
   */
  LOOT_API bool operator == (const Priority& rhs) const;


  /**
   * Check if this Priority object is greater than a given priority value.
   * @return True if this Priority object's value is greater than the given
   *         value, false otherwise.
   */
  LOOT_API bool operator > (const uint8_t rhs) const;

private:
  bool isExplicitZeroValue_;
  int8_t value_;
};
}

#endif
