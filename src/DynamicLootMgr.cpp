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

#include "DynamicLootMgr.h"

#include "Config.h"
#include "Creature.h"
#include "DynamicLootFamilies.h"
#include "DynamicLootSource.h"
#include "Formulas.h"
#include "GameObject.h"
#include "Log.h"
#include "LootMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Random.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>

namespace
{
    std::string const LOOT_MARK = ".Loot.";

    // A boss whose level is grey for the player has its chances divided by this.
    constexpr float GREY_BOSS_DIVISOR = 5.0f;

    // The bad luck protection, in points: what a kill without a win adds.
    constexpr float PITY_WORLD_STEP = 1.0f;     // a creature outside dungeons and raids
    constexpr float PITY_RAID_STEP  = 2.5f;     // a raid boss

    std::string Trimmed(std::string const& s)
    {
        size_t a = 0, b = s.size();
        while (a < b && std::isspace(static_cast<unsigned char>(s[a])))
            ++a;
        while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1])))
            --b;
        return s.substr(a, b - a);
    }

    std::vector<std::string> Split(std::string const& s, char separator)
    {
        std::vector<std::string> out;
        std::string current;
        for (char c : s)
        {
            if (c == separator)
            {
                out.push_back(current);
                current.clear();
            }
            else
                current += c;
        }
        out.push_back(current);
        return out;
    }

    bool IsWord(std::string const& s)
    {
        return !s.empty() && std::all_of(s.begin(), s.end(), [](char c)
            { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; });
    }

    // A list's name: a word followed by a whole index, "card_tiers[0]".
    bool IsListName(std::string const& s)
    {
        size_t const open = s.find('[');
        if (open == std::string::npos || s.back() != ']' || open + 2 >= s.size())
            return false;
        std::string const index = s.substr(open + 1, s.size() - open - 2);
        return IsWord(s.substr(0, open))
            && std::all_of(index.begin(), index.end(), [](char c)
                { return std::isdigit(static_cast<unsigned char>(c)); });
    }

    bool ParseWhole(std::string const& text, unsigned long max, unsigned long& out)
    {
        if (text.empty() || !std::all_of(text.begin(), text.end(), [](char c)
                { return std::isdigit(static_cast<unsigned char>(c)); }))
            return false;
        char* end = nullptr;
        out = std::strtoul(text.c_str(), &end, 10);
        return !*end && out <= max;
    }

    bool ParseNumber(std::string const& text, double& out)
    {
        if (text.empty())
            return false;
        char* end = nullptr;
        out = std::strtod(text.c_str(), &end);
        return !*end;
    }

    // An item entry the server knows.
    bool ParseItem(std::string const& text, uint32& item, std::string& problem)
    {
        unsigned long value = 0;
        if (!ParseWhole(text, 0xFFFFFFFFul, value) || !value)
        {
            problem = "\"" + text + "\" is not an item number";
            return false;
        }
        if (!sObjectMgr->GetItemTemplate(uint32(value)))
        {
            problem = "item " + text + " does not exist";
            return false;
        }
        item = uint32(value);
        return true;
    }

    // "[item, item, ...]"
    bool ParseItems(std::string const& raw, std::vector<uint32>& out, std::string& problem)
    {
        std::string const text = Trimmed(raw);
        if (text.size() < 2 || text.front() != '[' || text.back() != ']')
        {
            problem = "not written [item, item, ...]";
            return false;
        }
        std::string const inside = Trimmed(text.substr(1, text.size() - 2));
        if (inside.empty())
        {
            problem = "an empty list";
            return false;
        }
        for (std::string const& piece : Split(inside, ','))
        {
            uint32 item = 0;
            if (!ParseItem(Trimmed(piece), item, problem))
                return false;
            out.push_back(item);
        }
        return true;
    }

    // "<list[i] or item>:<quantity>:<chance>"
    bool ParseDraw(std::string const& raw, DynamicLootModule const& module, DynamicLootDraw& draw,
        std::string& problem)
    {
        std::string const text = Trimmed(raw);
        std::vector<std::string> const parts = Split(text, ':');
        if (parts.size() != 3)
        {
            problem = "\"" + text + "\" is not <list or item>:<quantity>:<chance>";
            return false;
        }

        std::string const target = Trimmed(parts[0]);
        if (IsListName(target))
        {
            auto const it = std::find(module.listNames.begin(), module.listNames.end(), target);
            if (it == module.listNames.end())
            {
                problem = "no readable list " + target;
                return false;
            }
            draw.list = int32(it - module.listNames.begin());
        }
        else if (!ParseItem(target, draw.item, problem))
            return false;

        unsigned long quantity = 0;
        std::string const q = Trimmed(parts[1]);
        if (!ParseWhole(q, 255, quantity) || !quantity)
        {
            problem = "\"" + q + "\" is not a quantity from 1 to 255";
            return false;
        }
        draw.quantity = uint8(quantity);

        double chance = 0.0;
        std::string const c = Trimmed(parts[2]);
        if (!ParseNumber(c, chance) || chance < 0.0 || chance > 100.0)
        {
            problem = "\"" + c + "\" is not a chance from 0 to 100";
            return false;
        }
        draw.chance = float(chance);
        return true;
    }

    // "<draw>, <draw> ; <draw>": independent draws separated by ";", fallbacks by ",".
    bool ParseLine(std::string const& raw, DynamicLootModule const& module, DynamicLootLine& line,
        std::string& problem)
    {
        for (std::string const& chainText : Split(raw, ';'))
        {
            if (Trimmed(chainText).empty())
                continue;
            DynamicLootChain chain;
            for (std::string const& drawText : Split(chainText, ','))
            {
                DynamicLootDraw draw;
                if (!ParseDraw(drawText, module, draw, problem))
                    return false;
                chain.push_back(draw);
            }
            line.chains.push_back(chain);
        }
        return true;
    }

    void Unreadable(std::string const& key, std::string const& problem)
    {
        LOG_WARN("module", "DynamicLoot: {} cannot be read -- {}. Ignored.", key, problem);
    }

    // One module's settings, by name after ".Loot.": lists first, since the lines name them.
    DynamicLootModule ReadModule(std::string const& prefix, std::vector<std::string> const& names)
    {
        DynamicLootModule module;
        module.name = prefix;
        auto const keyOf = [&prefix](std::string const& name) { return prefix + LOOT_MARK + name; };
        auto const valueOf = [&keyOf](std::string const& name)
            { return sConfigMgr->GetOption<std::string>(keyOf(name), "", false); };

        for (std::string const& name : names)
        {
            if (!IsListName(name))
                continue;
            std::vector<uint32> items;
            std::string problem;
            if (!ParseItems(valueOf(name), items, problem))
            {
                Unreadable(keyOf(name), problem);
                continue;
            }
            module.listNames.push_back(name);
            module.lists.push_back(items);
        }

        for (std::string const& name : names)
        {
            if (IsListName(name))
                continue;
            std::string problem;
            if (name == "Rate")
            {
                double percent = 0.0;
                std::string const value = Trimmed(valueOf(name));
                if (!ParseNumber(value, percent) || percent < 0.0)
                    Unreadable(keyOf(name), "\"" + value + "\" is not a percentage of 0 or more; 100 is used");
                else
                    module.rate = float(percent / 100.0);
                continue;
            }
            if (name == "NoPity")
            {
                std::vector<uint32> items;
                if (!ParseItems(valueOf(name), items, problem))
                    Unreadable(keyOf(name), problem);
                else
                    module.noPity.insert(items.begin(), items.end());
                continue;
            }
            int32 const family = DynamicLootFamilyIndex(name);
            if (family < 0)
            {
                LOG_WARN("module", "DynamicLoot: {} names no family this version of DynamicLoot knows. "
                    "Ignored.", keyOf(name));
                continue;
            }
            DynamicLootLine line;
            line.family = uint32(family);
            if (!ParseLine(valueOf(name), module, line, problem))
            {
                Unreadable(keyOf(name), problem);
                continue;
            }
            module.lines.push_back(line);
        }

        // The protection leaves alone a draw that can give a NoPity item. A list mixing both kinds
        // is drawn without it: an excluded item must never come out boosted.
        std::vector<bool> mixedReported(module.lists.size(), false);
        for (DynamicLootLine& line : module.lines)
            for (DynamicLootChain& chain : line.chains)
                for (DynamicLootDraw& draw : chain)
                {
                    if (draw.list < 0)
                    {
                        draw.pity = !module.noPity.count(draw.item);
                        continue;
                    }
                    std::vector<uint32> const& list = module.lists[draw.list];
                    size_t const excluded = std::count_if(list.begin(), list.end(),
                        [&module](uint32 item) { return module.noPity.count(item) != 0; });
                    draw.pity = !excluded;
                    if (excluded && excluded < list.size() && !mixedReported[draw.list])
                    {
                        mixedReported[draw.list] = true;
                        LOG_WARN("module", "DynamicLoot: {} mixes items of NoPity with others: its draws get "
                            "no bad luck protection.", keyOf(module.listNames[draw.list]));
                    }
                }
        return module;
    }

    void AddToLoot(Loot* loot, uint32 item, uint8 count, DynamicLootModule const& module, uint32 family)
    {
        // Past the loot window's size, Loot::AddItem gives up silently.
        if (loot->items.size() >= MAX_NR_LOOT_ITEMS)
        {
            LOG_DEBUG("module", "DynamicLoot: loot full, {} from {} ({}) not added.", item, module.name,
                DynamicLootFamilyAt(family).name);
            return;
        }
        loot->AddItem(LootStoreItem(item, 0, 100.0f, false, LOOT_MODE_DEFAULT, 0, count, count));
        LOG_DEBUG("module", "DynamicLoot: {} x{} from {} ({}).", item, count, module.name,
            DynamicLootFamilyAt(family).name);
    }

    // A draw on a list is that many distinct draws in the list, one item each; on an item, that many of it.
    void Place(Loot* loot, DynamicLootModule const& module, DynamicLootDraw const& draw, uint32 family)
    {
        if (draw.list < 0)
        {
            AddToLoot(loot, draw.item, draw.quantity, module, family);
            return;
        }
        std::vector<uint32> const& list = module.lists[draw.list];
        for (uint8 i = 0; i < draw.quantity; ++i)
            AddToLoot(loot, list[urand(0, uint32(list.size()) - 1)], 1, module, family);
    }

    // The loot of an item in the bags has no world object for a source.
    Creature* CreatureSource(Loot* loot, Player* player)
    {
        if (!loot->sourceWorldObjectGUID.IsCreature())
            return nullptr;
        return ObjectAccessor::GetCreature(*player, loot->sourceWorldObjectGUID);
    }

    GameObject* GameObjectSource(Loot* loot, Player* player)
    {
        if (!loot->sourceWorldObjectGUID.IsGameObject())
            return nullptr;
        return ObjectAccessor::GetGameObject(*player, loot->sourceWorldObjectGUID);
    }

    // A boss whose level is grey for the player has its chances divided.
    float GreyBossFactor(DynamicLootContext const& context, Player* player)
    {
        return context.boss && context.level <= Acore::XP::GetGrayLevel(player->GetLevel())
            ? 1.0f / GREY_BOSS_DIVISOR : 1.0f;
    }
}

DynamicLootMgr* DynamicLootMgr::instance()
{
    static DynamicLootMgr instance;
    return &instance;
}

void DynamicLootMgr::Load()
{
    _modules.clear();

    // Every <Module>.Loot.<name> setting, whatever file holds it, by module.
    std::map<std::string, std::vector<std::string>> byModule;
    for (std::string const& key : sConfigMgr->GetKeysByString(""))
    {
        size_t const mark = key.find(LOOT_MARK);
        if (mark == std::string::npos || mark + LOOT_MARK.size() >= key.size())
            continue;
        std::string const prefix = key.substr(0, mark);
        if (IsWord(prefix))
            byModule[prefix].push_back(key.substr(mark + LOOT_MARK.size()));
    }

    uint32 lines = 0;
    for (auto& [prefix, names] : byModule)
    {
        std::sort(names.begin(), names.end());
        _modules.push_back(ReadModule(prefix, names));
        DynamicLootModule const& module = _modules.back();
        lines += uint32(module.lines.size());
        LOG_INFO("module", "DynamicLoot: {}: {} line(s), {} list(s), rate {} %, {} item(s) without bad "
            "luck protection.", module.name, module.lines.size(), module.lists.size(), module.rate * 100.0f,
            module.noPity.size());
    }

    _byFamily.assign(DynamicLootFamilyCount(), {});
    for (uint32 m = 0; m < _modules.size(); ++m)
        for (uint32 l = 0; l < _modules[m].lines.size(); ++l)
            _byFamily[_modules[m].lines[l].family].emplace_back(m, l);

    LOG_INFO("module", "DynamicLoot: {} families known; {} module(s) declare loot, {} line(s) read.",
        DynamicLootFamilyCount(), _modules.size(), lines);
}

void DynamicLootMgr::Fill(Loot* loot, LootStore const& store, Player* player)
{
    // Every loot of the server comes through here: the store is tested first.
    bool const creatureStore   = &store == &LootTemplates_Creature;
    bool const skinningStore   = &store == &LootTemplates_Skinning;
    bool const gameObjectStore = &store == &LootTemplates_Gameobject;
    if ((!creatureStore && !skinningStore && !gameObjectStore) || !loot || !player || _modules.empty())
        return;

    if (creatureStore)
        FillCreature(loot, player);
    else if (skinningStore)
        FillSkinning(loot, player);
    else
        FillGameObject(loot, player);
}

void DynamicLootMgr::FillCreature(Loot* loot, Player* player)
{
    Creature* creature = CreatureSource(loot, player);
    if (!creature)
        return;

    DynamicLootContext context;
    DynamicLootFillCreature(context, player, creature);

    float const factor = GreyBossFactor(context, player);

    // The protection feeds on world creatures (world bosses and rares included) and raid bosses;
    // nothing in dungeons, nor on raid trash. It is played only where it feeds.
    float step = 0.0f;
    if (!context.dungeon)
        step = PITY_WORLD_STEP;
    else if (context.raid && context.boss)
        step = PITY_RAID_STEP;
    float const bonus = step > 0.0f ? Pity(player) : 0.0f;

    // A creature belongs to one family of the first filter and to one level bracket.
    Outcome outcome;
    PlayFamily(loot, DynamicLootCreatureFamily(context), factor, bonus, outcome);
    PlayFamily(loot, DynamicLootLevelBracket(context.level), factor, bonus, outcome);
    Settle(player, outcome, step);
}

// Skinning, and the corpses gathered by herbalism, mining or engineering: the skinning store holds
// them all. Outside the protection, which only a win touches here.
void DynamicLootMgr::FillSkinning(Loot* loot, Player* player)
{
    Creature* creature = CreatureSource(loot, player);
    if (!creature)
        return;

    DynamicLootContext context;
    DynamicLootFillCreature(context, player, creature);

    Outcome outcome;
    PlayFamily(loot, DynamicLootSkinningFamily(context), GreyBossFactor(context, player), 0.0f, outcome);
    Settle(player, outcome, 0.0f);
}

// A herb or a vein. A node has no level, hence no brake; outside the protection, which only a win
// touches here.
void DynamicLootMgr::FillGameObject(Loot* loot, Player* player)
{
    GameObject* gameObject = GameObjectSource(loot, player);
    if (!gameObject)
        return;

    DynamicLootContext context;
    DynamicLootFillGameObject(context, player, gameObject);

    int32 const family = DynamicLootGatheringFamily(context);
    if (family < 0)
        return;

    Outcome outcome;
    PlayFamily(loot, uint32(family), 1.0f, 0.0f, outcome);
    Settle(player, outcome, 0.0f);
}

void DynamicLootMgr::PlayFamily(Loot* loot, uint32 family, float factor, float bonus, Outcome& outcome)
{
    if (family >= _byFamily.size())
        return;

    for (auto const& [m, l] : _byFamily[family])
    {
        DynamicLootModule const& module = _modules[m];
        // A module at rate 0 is off: the protection cannot bring it back.
        if (module.rate <= 0.0f)
            continue;
        for (DynamicLootChain const& chain : module.lines[l].chains)
            for (DynamicLootDraw const& draw : chain)
            {
                if (draw.chance <= 0.0f)
                    continue;   // never: on to the next fallback
                // The points are added after the rate factor and the grey boss brake: a bonus,
                // which neither divides.
                if (draw.pity)
                    outcome.possible = true;
                if (roll_chance_f(draw.chance * module.rate * factor + (draw.pity ? bonus : 0.0f)))
                {
                    Place(loot, module, draw, family);
                    outcome.won = true;
                    break;      // the first success stops the chain
                }
            }
    }
}

void DynamicLootMgr::Settle(Player* player, Outcome const& outcome, float step)
{
    if (!outcome.won && (step <= 0.0f || !outcome.possible))
        return;

    std::lock_guard<std::mutex> guard(_pityLock);
    if (outcome.won)
    {
        // Any item won resets it, wherever it comes from.
        if (_pity.erase(player->GetGUID()))
            LOG_DEBUG("module", "DynamicLoot: {} won, bad luck protection back to 0.", player->GetName());
        return;
    }
    float& points = _pity[player->GetGUID()];
    points += step;
    LOG_DEBUG("module", "DynamicLoot: {} won nothing, bad luck protection at {} point(s).", player->GetName(),
        points);
}

float DynamicLootMgr::Pity(Player* player)
{
    std::lock_guard<std::mutex> guard(_pityLock);
    auto const it = _pity.find(player->GetGUID());
    return it == _pity.end() ? 0.0f : it->second;
}

void DynamicLootMgr::SetPity(Player* player, float points)
{
    std::lock_guard<std::mutex> guard(_pityLock);
    if (points <= 0.0f)
        _pity.erase(player->GetGUID());
    else
        _pity[player->GetGUID()] = points;
}

void DynamicLootMgr::Forget(Player* player)
{
    std::lock_guard<std::mutex> guard(_pityLock);
    _pity.erase(player->GetGUID());
}
