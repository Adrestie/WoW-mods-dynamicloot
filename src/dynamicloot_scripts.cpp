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

#include "Chat.h"
#include "CommandScript.h"
#include "DynamicLootMgr.h"
#include "Player.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

// The declarations are read once the world database is loaded: a draw naming an
// item is checked against the item templates.
class DynamicLootWorldScript : public WorldScript
{
public:
    DynamicLootWorldScript() : WorldScript("DynamicLootWorldScript",
        {
            WORLDHOOK_ON_BEFORE_WORLD_INITIALIZED
        }) { }

    void OnBeforeWorldInitialized() override
    {
        sDynamicLootMgr->Load();
    }
};

// The items are added to the loot being filled, right after its template and before group
// rights and quality thresholds: nothing in the loot tables of the database changes.
class DynamicLootMiscScript : public MiscScript
{
public:
    DynamicLootMiscScript() : MiscScript("DynamicLootMiscScript",
        {
            MISCHOOK_ON_AFTER_LOOT_TEMPLATE_PROCESS
        }) { }

    void OnAfterLootTemplateProcess(Loot* loot, LootTemplate const* /*tab*/, LootStore const& store,
        Player* lootOwner, bool /*personal*/, bool /*noEmptyError*/, uint16 /*lootMode*/) override
    {
        sDynamicLootMgr->Fill(loot, store, lootOwner);
    }
};

// The bad luck protection does not outlive the session.
class DynamicLootPlayerScript : public PlayerScript
{
public:
    DynamicLootPlayerScript() : PlayerScript("DynamicLootPlayerScript",
        {
            PLAYERHOOK_ON_LOGOUT
        }) { }

    void OnPlayerLogout(Player* player) override
    {
        sDynamicLootMgr->Forget(player);
    }
};

/*
 * .dynamicloot pity            (SEC_GAMEMASTER)  the bad luck protection of the selected player, else one's own
 * .dynamicloot pity <points>   (SEC_GAMEMASTER)  sets it, to try the protection in game
 */
class DynamicLootCommandScript : public CommandScript
{
public:
    DynamicLootCommandScript() : CommandScript("DynamicLootCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable dynamiclootTable =
        {
            { "pity", HandlePityCommand, SEC_GAMEMASTER, Console::No },
        };
        static ChatCommandTable commandTable =
        {
            { "dynamicloot", dynamiclootTable },
        };
        return commandTable;
    }

    static bool HandlePityCommand(ChatHandler* handler, Optional<float> points)
    {
        Player* player = handler->getSelectedPlayerOrSelf();
        if (!player)
            return false;
        if (points)
            sDynamicLootMgr->SetPity(player, *points);
        handler->PSendSysMessage("DynamicLoot: bad luck protection of {}: {} point(s).", player->GetName(),
            sDynamicLootMgr->Pity(player));
        return true;
    }
};

void AddSC_dynamicloot_scripts()
{
    new DynamicLootWorldScript();
    new DynamicLootMiscScript();
    new DynamicLootPlayerScript();
    new DynamicLootCommandScript();
}
