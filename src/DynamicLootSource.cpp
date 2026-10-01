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

#include "DynamicLootSource.h"

#include "AreaDefines.h"
#include "Creature.h"
#include "DBCStores.h"
#include "DynamicLootFamilies.h"
#include "Map.h"
#include "Player.h"
#include "SharedDefines.h"

#include <string>

namespace
{
    char const* const EXPANSIONS[] = { "Vanilla", "Bc", "Wotlk" };

    // A family index by name; the names below are all in the table.
    uint32 Family(std::string const& name)
    {
        return uint32(DynamicLootFamilyIndex(name));
    }
}

void DynamicLootFillCreature(DynamicLootContext& context, Player* player, Creature* creature)
{
    context.player   = player;
    context.creature = creature;
    context.level    = creature->GetLevel();
    context.rank     = uint8(creature->GetCreatureTemplate()->rank);
    context.boss     = creature->IsDungeonBoss() || creature->isWorldBoss()
                       || context.rank == CREATURE_ELITE_WORLDBOSS;
    context.map      = creature->GetMapId();

    if (Map* map = creature->GetMap())
    {
        context.dungeon = map->IsDungeon();
        context.raid    = map->IsRaid();
        context.heroic  = map->IsHeroic();
        if (MapEntry const* entry = map->GetEntry())
            context.expansion = entry->Expansion();
    }
}

uint32 DynamicLootCreatureFamily(DynamicLootContext const& c)
{
    std::string const expansion = EXPANSIONS[c.expansion < 3 ? c.expansion : 2];

    // Outside dungeons and raids: a world boss or a rare before the World family.
    if (!c.dungeon)
    {
        if (c.rank == CREATURE_ELITE_WORLDBOSS || (c.creature && c.creature->isWorldBoss()))
            return Family(expansion + "WorldBoss");
        if (c.rank == CREATURE_ELITE_RARE || c.rank == CREATURE_ELITE_RAREELITE)
            return Family(expansion + "Rare");
        return Family(expansion + "World");
    }

    if (c.raid)
    {
        if (c.boss && c.map == MAP_ICECROWN_CITADEL)
            return Family("IcecrownRaidBoss");
        return Family(expansion + (c.boss ? "RaidBoss" : "RaidTrash"));
    }

    // WotLK dungeons alone tell heroic from normal.
    if (c.expansion == 2 && c.heroic)
        return Family(std::string("WotlkHeroic") + (c.boss ? "Boss" : "Trash"));
    return Family(expansion + (c.boss ? "DungeonBoss" : "DungeonTrash"));
}

uint32 DynamicLootLevelBracket(uint32 level)
{
    if (level >= 80)
        return Family("MobLvl_80_89");
    uint32 const low = level < 10 ? 1 : level / 10 * 10;
    uint32 const high = level < 10 ? 9 : low + 9;
    return Family("MobLvl_" + std::to_string(low) + "_" + std::to_string(high));
}
