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

#include "DynamicLootFamilies.h"

#include <iterator>

namespace
{
    using S = DynamicLootSource;

    // The families of docs/SPECIFICATION.md. Append new ones at the end.
    DynamicLootFamily const FAMILIES[] =
    {
        // Creatures, first filter
        { "VanillaWorld",         S::Creature },
        { "VanillaWorldBoss",     S::Creature },
        { "VanillaRare",          S::Creature },
        { "VanillaDungeonTrash",  S::Creature },
        { "VanillaDungeonBoss",   S::Creature },
        { "VanillaRaidTrash",     S::Creature },
        { "VanillaRaidBoss",      S::Creature },
        { "BcWorld",              S::Creature },
        { "BcWorldBoss",          S::Creature },
        { "BcRare",               S::Creature },
        { "BcDungeonTrash",       S::Creature },
        { "BcDungeonBoss",        S::Creature },
        { "BcRaidTrash",          S::Creature },
        { "BcRaidBoss",           S::Creature },
        { "WotlkWorld",           S::Creature },
        { "WotlkWorldBoss",       S::Creature },
        { "WotlkRare",            S::Creature },
        { "WotlkDungeonTrash",    S::Creature },
        { "WotlkDungeonBoss",     S::Creature },
        { "WotlkHeroicTrash",     S::Creature },
        { "WotlkHeroicBoss",      S::Creature },
        { "WotlkRaidTrash",       S::Creature },
        { "WotlkRaidBoss",        S::Creature },
        { "IcecrownRaidBoss",     S::Creature },

        // Creatures, second filter: the creature's level
        { "MobLvl_1_9",           S::LevelBracket },
        { "MobLvl_10_19",         S::LevelBracket },
        { "MobLvl_20_29",         S::LevelBracket },
        { "MobLvl_30_39",         S::LevelBracket },
        { "MobLvl_40_49",         S::LevelBracket },
        { "MobLvl_50_59",         S::LevelBracket },
        { "MobLvl_60_69",         S::LevelBracket },
        { "MobLvl_70_79",         S::LevelBracket },
        { "MobLvl_80_89",         S::LevelBracket },

        // Gathering
        { "VanillaOres",          S::Gathering },
        { "BcOres",               S::Gathering },
        { "WotlkOresLow",         S::Gathering },
        { "WotlkOresMedium",      S::Gathering },
        { "WotlkOresHigh",        S::Gathering },
        { "VanillaHerbs",         S::Gathering },
        { "BcHerbs",              S::Gathering },
        { "WotlkHerbsLow",        S::Gathering },
        { "WotlkHerbsMedium",     S::Gathering },
        { "WotlkHerbsHigh",       S::Gathering },

        // Skinning
        { "Skinning80",           S::Skinning },
        { "Skinning71",           S::Skinning },
        { "Skinning61",           S::Skinning },
        { "Skinning1",            S::Skinning },

        // Corpses gathered by another profession
        { "VanillaCorpseHerbs",   S::CorpseProfession },
        { "BcCorpseHerbs",        S::CorpseProfession },
        { "WotlkCorpseHerbs",     S::CorpseProfession },
        { "VanillaCorpseOres",    S::CorpseProfession },
        { "BcCorpseOres",         S::CorpseProfession },
        { "WotlkCorpseOres",      S::CorpseProfession },
        { "VanillaCorpseSalvage", S::CorpseProfession },
        { "BcCorpseSalvage",      S::CorpseProfession },
        { "WotlkCorpseSalvage",   S::CorpseProfession },

        // Chests
        { "WorldChest",           S::Chest },
        { "DungeonChest",         S::Chest },
        { "HeroicDungeonChest",   S::Chest },

        // Inventory containers
        { "VanillaContainer",     S::Container },
        { "BcContainer",          S::Container },
        { "WotlkContainer",       S::Container },
    };
}

uint32 DynamicLootFamilyCount()
{
    return uint32(std::size(FAMILIES));
}

DynamicLootFamily const& DynamicLootFamilyAt(uint32 index)
{
    return FAMILIES[index];
}

int32 DynamicLootFamilyIndex(std::string const& name)
{
    for (uint32 i = 0; i < std::size(FAMILIES); ++i)
        if (name == FAMILIES[i].name)
            return int32(i);
    return -1;
}
