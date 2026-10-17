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
#include "GameObject.h"
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

    std::string Expansion(DynamicLootContext const& c)
    {
        return EXPANSIONS[c.expansion < 3 ? c.expansion : 2];
    }

    void FillMap(DynamicLootContext& context, Map* map)
    {
        if (!map)
            return;
        context.dungeon = map->IsDungeon();
        context.raid    = map->IsRaid();
        context.heroic  = map->IsHeroic();
        if (MapEntry const* entry = map->GetEntry())
            context.expansion = entry->Expansion();
    }

    // Left out of the gathering logic: the three Frozen Herb nodes (the one at 400 shares the lock
    // of Adder's Tongue) and the Pure Saronite Deposit of Ulduar (the lock of titanium).
    uint32 const GATHERING_LEFT_OUT[] = { 190173, 190174, 190175, 195036 };

    uint32 MiningFamily(uint32 skill, bool wotlkMap)
    {
        if (skill == 450)
            return Family("WotlkOresHigh");
        if (skill == 400 || skill == 425)
            return Family("WotlkOresMedium");
        if ((skill == 350 || skill == 375) && wotlkMap)
            return Family("WotlkOresLow");
        if (skill >= 275 && !wotlkMap)
            return Family("BcOres");
        return Family("VanillaOres");   // any mining not classed elsewhere, quest objects (skill 0) too
    }

    uint32 HerbalismFamily(uint32 skill, bool bcMap, bool wotlkMap)
    {
        if (skill == 435 || skill == 450)
            return Family("WotlkHerbsHigh");
        if (skill == 385 || skill == 425 || (skill == 400 && wotlkMap))
            return Family("WotlkHerbsMedium");
        if (skill == 360 || ((skill == 350 || skill == 375) && wotlkMap))
            return Family("WotlkHerbsLow");
        if (!wotlkMap && (skill > 300 || (skill == 300 && bcMap)))
            return Family("BcHerbs");
        return Family("VanillaHerbs");  // any herbalism not classed elsewhere, quest objects too
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
    FillMap(context, creature->GetMap());
}

void DynamicLootFillGameObject(DynamicLootContext& context, Player* player, GameObject* gameObject)
{
    context.player     = player;
    context.gameObject = gameObject;
    context.map        = gameObject->GetMapId();
    FillMap(context, gameObject->GetMap());
}

uint32 DynamicLootCreatureFamily(DynamicLootContext const& c)
{
    std::string const expansion = Expansion(c);

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
        if (c.map == MAP_ICECROWN_CITADEL)
            return Family(c.boss ? "IcecrownRaidBoss" : "IcecrownRaidTrash");
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

int32 DynamicLootDungeonRareFamily(DynamicLootContext const& c)
{
    if (!c.dungeon || c.raid)
        return -1;
    if (c.rank != CREATURE_ELITE_RARE && c.rank != CREATURE_ELITE_RAREELITE)
        return -1;
    return DynamicLootFamilyIndex(Expansion(c) + "Rare");
}

uint32 DynamicLootSkinningFamily(DynamicLootContext const& c)
{
    uint32 const flags = c.creature ? c.creature->GetCreatureTemplate()->type_flags : 0;
    if (flags & CREATURE_TYPE_FLAG_SKIN_WITH_HERBALISM)
        return Family(Expansion(c) + "CorpseHerbs");
    if (flags & CREATURE_TYPE_FLAG_SKIN_WITH_MINING)
        return Family(Expansion(c) + "CorpseOres");
    if (flags & CREATURE_TYPE_FLAG_SKIN_WITH_ENGINEERING)
        return Family(Expansion(c) + "CorpseSalvage");

    if (c.level >= 80)
        return Family("Skinning80");
    if (c.level >= 71)
        return Family("Skinning71");
    if (c.level >= 61)
        return Family("Skinning61");
    return Family("Skinning1");
}

int32 DynamicLootGatheringFamily(DynamicLootContext const& c)
{
    if (!c.gameObject)
        return -1;
    for (uint32 entry : GATHERING_LEFT_OUT)
        if (c.gameObject->GetEntry() == entry)
            return -1;

    // The skill lives in the lock (Lock.dbc). A skill of 0 is a real node: the first ores and
    // herbs ask for the profession without a level.
    uint32 const lockId = c.gameObject->GetGOInfo()->GetLockId();
    LockEntry const* lock = lockId ? sLockStore.LookupEntry(lockId) : nullptr;
    if (!lock)
        return -1;
    for (uint8 i = 0; i < MAX_LOCK_CASE; ++i)
    {
        if (lock->Type[i] != LOCK_KEY_SKILL)
            continue;
        if (lock->Index[i] == LOCKTYPE_MINING)
            return int32(MiningFamily(lock->Skill[i], c.expansion == 2));
        if (lock->Index[i] == LOCKTYPE_HERBALISM)
            return int32(HerbalismFamily(lock->Skill[i], c.expansion == 1, c.expansion == 2));
    }
    return -1;
}
