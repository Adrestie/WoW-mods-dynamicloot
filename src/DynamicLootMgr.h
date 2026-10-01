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
 * What the modules declare. AzerothCore loads the .conf of every module into
 * one configuration; at start-up DynamicLoot reads every setting written
 * <Module>.Loot.<name> there, whatever file holds it:
 *
 *   <Module>.Loot.<list>[<i>] = [item, item, ...]     a list, drawn without weights
 *   <Module>.Loot.Rate = <percent>                    the module's rate factor (100 = as written)
 *   <Module>.Loot.NoPity = [item, ...]                items the bad luck protection leaves alone
 *   <Module>.Loot.<Family> = <draws>                  what that family drops
 *
 * <draws>: independent draws separated by ";", each a chain of fallbacks
 * separated by "," (the first success stops the chain), each fallback
 * <list[i] or item>:<quantity>:<chance in percent>.
 *
 * A setting that cannot be read is a warning at start-up and is ignored.
 */

#ifndef DYNAMICLOOT_MGR_H_
#define DYNAMICLOOT_MGR_H_

#include "Define.h"

#include <string>
#include <unordered_set>
#include <vector>

struct DynamicLootDraw
{
    uint32 item     = 0;    // the item, when the draw names one
    int32  list     = -1;   // else the index of one of the module's lists
    uint8  quantity = 1;
    float  chance   = 0.0f; // percent
};

// Fallbacks played in order, the first success stopping the chain.
using DynamicLootChain = std::vector<DynamicLootDraw>;

// Every chain of a family, each played on its own.
struct DynamicLootLine
{
    uint32 family = 0;
    std::vector<DynamicLootChain> chains;
};

struct DynamicLootModule
{
    std::string name;                           // the prefix before ".Loot."
    float rate = 1.0f;                          // Rate / 100
    std::unordered_set<uint32> noPity;
    std::vector<std::string> listNames;         // "card_tiers[0]"...
    std::vector<std::vector<uint32>> lists;     // same order as listNames
    std::vector<DynamicLootLine> lines;
};

class DynamicLootMgr
{
public:
    static DynamicLootMgr* instance();

    // Reads every module's declarations from the loaded configuration.
    void Load();

    std::vector<DynamicLootModule> const& Modules() const { return _modules; }

private:
    std::vector<DynamicLootModule> _modules;
};

#define sDynamicLootMgr DynamicLootMgr::instance()

#endif
