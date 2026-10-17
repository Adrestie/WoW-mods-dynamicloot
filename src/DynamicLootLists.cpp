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

#include "DynamicLootLists.h"

#include "CreatureData.h"
#include "DynamicLootFamilies.h"
#include "DynamicLootSource.h"
#include "GameObject.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "LootMgr.h"
#include "ObjectMgr.h"

#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    // The real chests, every entry of each name in the world database. Their family is the place's.
    uint32 const REAL_CHESTS[] =
    {
        // Outside dungeons and raids
        2843, 2849, 106318, 106319,                                     // Battered Chest
        2844, 2845, 2846, 2847, 3715, 4096, 105578, 105579, 105581, 111095, // Tattered Chest
        2850, 2852, 2855, 2857, 4149, 153451, 153453, 153454,           // Solid Chest
        179486, 179488, 179490,                                         // Battered Footlocker
        179492, 179494, 179496, 184741,                                 // Dented Footlocker
        179493, 179497,                                                 // Mossy Footlocker
        179487, 179489, 179491,                                         // Waterlogged Footlocker
        179498,                                                         // Scarlet Footlocker
        181798,                                                         // Fel Iron Chest
        181800,                                                         // Heavy Fel Iron Chest
        181802,                                                         // Adamantite Bound Chest
        181804,                                                         // Felsteel Chest
        // Outside and in dungeons
        75293,                                                          // Large Battered Chest
        131978, 153468, 153469,                                         // Large Mithril Bound Chest
        // Dungeons
        74447, 75295, 75296, 75297,                                     // Large Iron Bound Chest
        74448, 75298, 75299, 75300, 153462, 153463, 153464,             // Large Solid Chest
        185168, 185169,                                                 // Reinforced Fel Iron Chest
    };

    // A boss's chest is its loot. family: the raid boss family; nullptr for a dungeon boss, whose
    // family (normal or heroic) is the map's. boss: the creature whose level the chest takes.
    struct BossChest
    {
        uint32      entry;
        uint32      boss;
        char const* family;
    };

    BossChest const BOSS_CHESTS[] =
    {
        // Raids
        { 194307, 32845, "WotlkRaidBoss"    },  // Cache of Winter (Hodir)
        { 194308, 32845, "WotlkRaidBoss"    },
        { 194200, 32845, "WotlkRaidBoss"    },  // Rare Cache of Winter (Hodir, hard mode)
        { 194201, 32845, "WotlkRaidBoss"    },
        { 202238, 37813, "IcecrownRaidBoss" },  // Deathbringer's Cache (Saurfang)
        { 202239, 37813, "IcecrownRaidBoss" },
        { 202240, 37813, "IcecrownRaidBoss" },
        { 202241, 37813, "IcecrownRaidBoss" },
        { 201872, 36948, "IcecrownRaidBoss" },  // Gunship Armory (gunships, Icecrown Citadel)
        { 201873, 36948, "IcecrownRaidBoss" },
        { 201874, 36948, "IcecrownRaidBoss" },
        { 201875, 36948, "IcecrownRaidBoss" },
        { 202177, 36948, "IcecrownRaidBoss" },
        { 202178, 36948, "IcecrownRaidBoss" },
        { 202179, 36948, "IcecrownRaidBoss" },
        { 202180, 36948, "IcecrownRaidBoss" },
        { 185119, 16816, "BcRaidBoss"       },  // Dust Covered Chest (chess event, Karazhan)
        // Dungeons
        { 190586, 28070, nullptr },             // Tribunal Chest (Halls of Stone)
        { 193996, 28070, nullptr },
        { 191349, 27656, nullptr },             // Cache of Eregos (The Oculus)
        { 193603, 27656, nullptr },
        { 201710, 36954, nullptr },             // The Captain's Chest (Halls of Reflection)
        { 202212, 36954, nullptr },
        { 202336, 36954, nullptr },
        { 202337, 36954, nullptr },
        { 169243,  9039, nullptr },             // Chest of The Seven (Blackrock Depths)
        { 184465, 19219, nullptr },             // Cache of the Legion (The Mechanar)
        { 184849, 19219, nullptr },
        { 187372, 18473, nullptr },             // The Talon King's Coffer (Sethekk Halls)
        { 123329,  6906, nullptr },             // Baelog's Chest (Uldaman)
    };

    // The inventory containers, by name as docs/SPECIFICATION.md lists them.
    struct ContainerName
    {
        char const* family;
        char const* name;
    };

    ContainerName const CONTAINERS[] =
    {
        { "VanillaContainer", "Ornate Bronze Lockbox" },
        { "VanillaContainer", "Heavy Bronze Lockbox" },
        { "VanillaContainer", "Iron Lockbox" },
        { "VanillaContainer", "Strong Iron Lockbox" },
        { "VanillaContainer", "Steel Lockbox" },
        { "VanillaContainer", "Reinforced Steel Lockbox" },
        { "VanillaContainer", "Mithril Lockbox" },
        { "VanillaContainer", "Thorium Lockbox" },
        { "VanillaContainer", "Small Locked Chest" },
        { "VanillaContainer", "Sturdy Locked Chest" },
        { "VanillaContainer", "Ironbound Locked Chest" },
        { "VanillaContainer", "Reinforced Locked Chest" },
        { "VanillaContainer", "Battered Junkbox" },
        { "VanillaContainer", "Worn Junkbox" },
        { "VanillaContainer", "Sturdy Junkbox" },
        { "VanillaContainer", "Heavy Junkbox" },
        // fishing crates and chests
        { "VanillaContainer", "Battered Chest" },
        { "VanillaContainer", "Small Chest" },
        { "VanillaContainer", "Dented Crate" },
        { "VanillaContainer", "Waterlogged Crate" },
        { "VanillaContainer", "Sealed Crate" },
        { "VanillaContainer", "Heavy Crate" },
        { "VanillaContainer", "Tightly Sealed Trunk" },
        { "VanillaContainer", "Watertight Trunk" },
        { "VanillaContainer", "Iron Bound Trunk" },
        { "VanillaContainer", "Mithril Bound Trunk" },
        { "VanillaContainer", "Pirate's Footlocker" },
        { "VanillaContainer", "Curious Crate" },
        { "VanillaContainer", "Heavy Supply Crate" },
        { "BcContainer",      "Eternium Lockbox" },
        { "BcContainer",      "Khorium Lockbox" },
        { "BcContainer",      "Strong Junkbox" },
        { "WotlkContainer",   "Froststeel Lockbox" },
        { "WotlkContainer",   "Titanium Lockbox" },
        { "WotlkContainer",   "Tiny Titanium Lockbox" },
        { "WotlkContainer",   "Reinforced Junkbox" },
        { "WotlkContainer",   "Reinforced Crate" },
    };

    // Item entry -> family, resolved at start-up.
    std::unordered_map<uint32, uint32> containers;

    BossChest const* FindBossChest(uint32 entry)
    {
        for (BossChest const& chest : BOSS_CHESTS)
            if (chest.entry == entry)
                return &chest;
        return nullptr;
    }

    bool IsRealChest(uint32 entry)
    {
        for (uint32 chest : REAL_CHESTS)
            if (chest == entry)
                return true;
        return false;
    }
}

void DynamicLootLoadLists()
{
    containers.clear();

    // Only the items that open: a name can also be a quest item or a test item.
    std::unordered_map<std::string, std::vector<uint32>> byName;
    for (auto const& [entry, item] : *sObjectMgr->GetItemTemplateStore())
        if (item.HasFlag(ITEM_FLAG_HAS_LOOT) && LootTemplates_Item.HaveLootFor(entry))
            byName[item.Name1].push_back(entry);

    for (ContainerName const& container : CONTAINERS)
    {
        auto const it = byName.find(container.name);
        if (it == byName.end())
        {
            LOG_WARN("module", "DynamicLoot: no item that opens is named \"{}\" ({}).", container.name,
                container.family);
            continue;
        }
        std::string entries;
        for (uint32 entry : it->second)
        {
            containers[entry] = uint32(DynamicLootFamilyIndex(container.family));
            entries += (entries.empty() ? "" : ", ") + std::to_string(entry);
        }
        LOG_INFO("module", "DynamicLoot: {}: \"{}\" = {}.", container.family, container.name, entries);
    }

    for (BossChest const& chest : BOSS_CHESTS)
        if (!sObjectMgr->GetCreatureTemplate(chest.boss))
            LOG_WARN("module", "DynamicLoot: the boss of chest {}, creature {}, does not exist: the chest "
                "gives its loot without the grey boss brake.", chest.entry, chest.boss);

    LOG_INFO("module", "DynamicLoot: {} real chest(s), {} boss chest(s), {} inventory container(s).",
        std::size(REAL_CHESTS), std::size(BOSS_CHESTS), containers.size());
}

int32 DynamicLootChestFamily(DynamicLootContext& c, bool& bossChest)
{
    bossChest = false;
    if (!c.gameObject)
        return -1;
    uint32 const entry = c.gameObject->GetEntry();

    if (BossChest const* chest = FindBossChest(entry))
    {
        bossChest = true;
        CreatureTemplate const* boss = sObjectMgr->GetCreatureTemplate(chest->boss);
        c.boss  = true;
        c.level = boss ? boss->maxlevel : 0;    // 0: no level, no brake
        if (chest->family)
        {
            // A raid boss chest can stand on a transport (the gunships): its family is written here.
            c.dungeon = c.raid = true;
            return DynamicLootFamilyIndex(chest->family);
        }
        return int32(DynamicLootCreatureFamily(c));
    }

    if (!IsRealChest(entry) || c.raid)
        return -1;      // in a raid, a chest is a boss's
    if (!c.dungeon)
        return DynamicLootFamilyIndex("WorldChest");
    return DynamicLootFamilyIndex(c.heroic ? "HeroicDungeonChest" : "DungeonChest");
}

int32 DynamicLootContainerFamily(uint32 itemEntry)
{
    auto const it = containers.find(itemEntry);
    return it == containers.end() ? -1 : int32(it->second);
}
