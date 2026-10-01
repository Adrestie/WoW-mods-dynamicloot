/*
 * This file is part of dynamicloot.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * The loot families. Only DynamicLoot defines them; a module sets its loot on
 * them by name. New families are appended to the table: an index never changes
 * meaning, and a module's line on a family this version does not know is only
 * a warning.
 */

#ifndef DYNAMICLOOT_FAMILIES_H_
#define DYNAMICLOOT_FAMILIES_H_

#include "Define.h"

#include <string>

// Where a family's loot comes from.
enum class DynamicLootSource : uint8
{
    Creature,           // a creature killed (first filter)
    LevelBracket,       // a creature killed (second filter: its level)
    Gathering,          // a herb or a vein
    Skinning,           // a skinned creature
    CorpseProfession,   // a corpse gathered by herbalism, mining or engineering
    Chest,              // a real chest of the world or of a dungeon
    Container           // an item opened from the bags
};

struct DynamicLootFamily
{
    char const*       name;
    DynamicLootSource source;
};

// Number of families this version knows.
uint32 DynamicLootFamilyCount();

// The family at this index (0 .. count - 1).
DynamicLootFamily const& DynamicLootFamilyAt(uint32 index);

// Index of the family of that exact name, or -1.
int32 DynamicLootFamilyIndex(std::string const& name);

#endif
