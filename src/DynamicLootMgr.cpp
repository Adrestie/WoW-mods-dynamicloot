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
#include "DynamicLootFamilies.h"
#include "Log.h"
#include "ObjectMgr.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>

namespace
{
    std::string const LOOT_MARK = ".Loot.";

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
        return module;
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

    LOG_INFO("module", "DynamicLoot: {} families known; {} module(s) declare loot, {} line(s) read.",
        DynamicLootFamilyCount(), _modules.size(), lines);
}
