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
 * The closed lists of docs/SPECIFICATION.md: no data tells a real chest from a
 * crate or a quest object, nor a lockbox from any other item that opens.
 *
 * - The real chests of the world and of dungeons, by game object entry; their
 *   family is the place's: WorldChest outside instances, DungeonChest or
 *   HeroicDungeonChest in a dungeon, nothing in a raid.
 * - The boss chests, by game object entry: a boss's chest is its loot, it
 *   belongs to the boss's family (and level bracket) and is braked and
 *   protected as the boss would be.
 * - The inventory containers, by item name, as the specification names them:
 *   resolved at start-up among the items that open (ITEM_FLAG_HAS_LOOT, a row
 *   in item_loot_template), every entry found written to the log.
 */

#ifndef DYNAMICLOOT_LISTS_H_
#define DYNAMICLOOT_LISTS_H_

#include "Define.h"

struct DynamicLootContext;

// Resolves the inventory containers by name; once the item templates and the loot tables are loaded.
void DynamicLootLoadLists();

// A chest's family, or -1 when the game object is none of the closed lists. For a boss chest the
// context becomes the boss's (boss, level, raid); bossChest says so.
int32 DynamicLootChestFamily(DynamicLootContext& context, bool& bossChest);

// An inventory container's family, or -1.
int32 DynamicLootContainerFamily(uint32 itemEntry);

#endif
