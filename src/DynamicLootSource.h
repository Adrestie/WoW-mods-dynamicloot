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
 * What a loot source is: which families a creature belongs to. The expansion
 * is the map's (Map.dbc), never the creature's.
 */

#ifndef DYNAMICLOOT_SOURCE_H_
#define DYNAMICLOOT_SOURCE_H_

#include "Define.h"

class Creature;
class Player;

struct DynamicLootContext
{
    Player*   player    = nullptr;
    Creature* creature  = nullptr;
    uint32    level     = 0;
    uint8     rank      = 0;        // CreatureEliteType
    bool      boss      = false;    // dungeon, raid or world boss
    uint32    map       = 0;
    uint32    expansion = 0;        // 0 Vanilla, 1 BC, 2 WotLK
    bool      dungeon   = false;    // any instance, raids included
    bool      raid      = false;
    bool      heroic    = false;
};

// Fills the context of a creature killed by (or looted for) player.
void DynamicLootFillCreature(DynamicLootContext& context, Player* player, Creature* creature);

// The creature's family of the first filter (an index of DynamicLootFamilyAt).
uint32 DynamicLootCreatureFamily(DynamicLootContext const& context);

// The creature's level bracket (an index of DynamicLootFamilyAt).
uint32 DynamicLootLevelBracket(uint32 level);

#endif
