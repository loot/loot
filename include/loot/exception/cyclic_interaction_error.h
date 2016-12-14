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

#ifndef LOOT_EXCEPTION_CYCLIC_INTERACTION_ERROR
#define LOOT_EXCEPTION_CYCLIC_INTERACTION_ERROR

#include <stdexcept>

namespace loot {
/**
 * @brief An exception class thrown if a cyclic interaction is detected when
 *        sorting a load order.
 */
class CyclicInteractionError : public std::runtime_error {
public:
  CyclicInteractionError(const std::string& firstPlugin, const std::string& lastPlugin, const std::string& backCycle) : 
    std::runtime_error("Cyclic interaction detected between plugins \"" + firstPlugin + "\" and \"" + lastPlugin + "\". Back cycle: " + backCycle),
    firstPlugin_(firstPlugin), 
    lastPlugin_(lastPlugin), 
    backCycle_(backCycle) {}

  std::string getFirstPlugin() {
    return firstPlugin_;
  }

  std::string getLastPlugin() {
    return lastPlugin_;
  }

  std::string getBackCycle() {
    return backCycle_;
  }

private:
  const std::string firstPlugin_;
  const std::string lastPlugin_;
  const std::string backCycle_;
};
}

#endif
