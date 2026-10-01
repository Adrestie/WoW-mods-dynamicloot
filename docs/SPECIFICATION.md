# dynamicloot — the shared loot extension

**Status as of 2026-10-01: steps 1 (the families), 2 (what a module declares), 3 (the mechanisms) and
4 (the form) validated: the specification is complete. No code yet.**

Starting point: the sections of Spheregrid (`SphereGrid.Loot.<section>`), to which **World boss** and
**Rare** are added for each expansion (decided on 2026-09-30), whose gathering is grouped by tiers, and
which corpses and containers complete (decided on 2026-10-01). 53 families.

"Trash": any creature of the place that is not a boss. "World": anything outside dungeons and raids. The
expansion is the map's.

## Creatures — 24 families

| Family | Who |
|---|---|
| VanillaWorld | creatures outside dungeons and raids, Vanilla maps |
| VanillaWorldBoss | **(new)** world bosses, Vanilla maps |
| VanillaRare | **(new)** rare creatures outside dungeons and raids, Vanilla maps |
| VanillaDungeonTrash | non-boss creatures of Vanilla dungeons |
| VanillaDungeonBoss | bosses of Vanilla dungeons |
| VanillaRaidTrash | non-boss creatures of Vanilla raids |
| VanillaRaidBoss | bosses of Vanilla raids |
| BcWorld | creatures outside dungeons and raids, BC maps |
| BcWorldBoss | **(new)** world bosses, BC maps |
| BcRare | **(new)** rare creatures outside dungeons and raids, BC maps |
| BcDungeonTrash | non-boss creatures of BC dungeons (normal and heroic) |
| BcDungeonBoss | bosses of BC dungeons (normal and heroic) |
| BcRaidTrash | non-boss creatures of BC raids |
| BcRaidBoss | bosses of BC raids |
| WotlkWorld | creatures outside dungeons and raids, WotLK maps |
| WotlkWorldBoss | **(new)** world bosses, WotLK maps |
| WotlkRare | **(new)** rare creatures outside dungeons and raids, WotLK maps |
| WotlkDungeonTrash | non-boss creatures of WotLK dungeons in normal |
| WotlkDungeonBoss | bosses of WotLK dungeons in normal |
| WotlkHeroicTrash | non-boss creatures of WotLK dungeons in heroic |
| WotlkHeroicBoss | bosses of WotLK dungeons in heroic |
| WotlkRaidTrash | non-boss creatures of WotLK raids (10, 25 and heroic) |
| WotlkRaidBoss | bosses of WotLK raids outside Icecrown Citadel |
| IcecrownRaidBoss | bosses of Icecrown Citadel |

A world boss or a rare is read **before** the World family of its expansion: otherwise it would fall into
it, as it does in Spheregrid today.

## Creatures — second filter: level brackets (decided on 2026-10-01)

A second filter, independent of the first: the **creature's level**, whether boss, rare, elite or normal,
wherever it stands. A creature therefore belongs to one family of the first filter **and** to one level
bracket.

| Family | Level |
|---|---|
| MobLvl_1_9 | 1 to 9 |
| MobLvl_10_19 | 10 to 19 |
| MobLvl_20_29 | 20 to 29 |
| MobLvl_30_39 | 30 to 39 |
| MobLvl_40_49 | 40 to 49 |
| MobLvl_50_59 | 50 to 59 |
| MobLvl_60_69 | 60 to 69 |
| MobLvl_70_79 | 70 to 79 |
| MobLvl_80_89 | 80 and above (80 to 89, so any creature of level 80 or more) |

Names in the "MobLvl_X_Y" form (decided on 2026-10-01).

## Gathering — 10 families (revised on 2026-10-01)

A node is recognised by the skill its lock requires (Lock.dbc) and by the expansion of its map.

**Mining**

| Family | Condition | Nodes |
|---|---|---|
| VanillaOres | any mining not classed elsewhere | Copper, Tin, Silver, Iron, Gold, Mithril, Truesilver, Dark Iron, Small and Rich Thorium, "Ooze Covered" variants, Hakkari Thorium, Incendicite, Indurium, Lesser Bloodstone |
| BcOres | 275 or more, outside WotLK maps | Fel Iron, Nethercite, Adamantite, Rich Adamantite, Khorium, Ancient Gem Vein |
| WotlkOresLow | 350 or 375, WotLK map | Cobalt Deposit, Rich Cobalt Deposit |
| WotlkOresMedium | 400 or 425 | Saronite Deposit, Rich Saronite Deposit |
| WotlkOresHigh | 450 | Titanium Vein |

**Herbalism**

| Family | Condition | Plants |
|---|---|---|
| VanillaHerbs | any herbalism not classed elsewhere | Peacebloom to Black Lotus |
| BcHerbs | above 300, or 300 on a BC map, outside WotLK maps | Felweed, Dreaming Glory, Ragveil, Terocone, Flame Cap, Ancient Lichen, Netherbloom, Netherdust Bush, Nightmare Vine, Mana Thistle |
| WotlkHerbsLow | 350 (WotLK map), 360 or 375 (WotLK map) | Goldclover, Firethorn (the Fire Leaf node), Tiger Lily |
| WotlkHerbsMedium | 385, 400 (WotLK map) or 425 | Talandra's Rose, Adder's Tongue, Lichbloom |
| WotlkHerbsHigh | 435 or 450 | Icethorn, Frost Lotus |

**Left out of the logic**, recognised by their entry:

- the Frozen Herb, its three nodes (190174 at 300, 190173 at 400, 190175 at 415) -- the one at 400 shares
  the lock of Adder's Tongue;
- the Pure Saronite Deposit (195036, Ulduar), which shares the lock of titanium (decided on 2026-10-01).

**Quest objects stay** (decided on 2026-10-01): their skill is 0, so they fall into VanillaOres or
VanillaHerbs whatever their expansion (Chunk of Saronite, Crystalsong Carrot, Nethervine Crystal…).

WotLK families are written "Wotlk" everywhere, creatures included (decided on 2026-10-01).

## Skinning — 4 families

Skinning80 (level 80), Skinning71 (71 to 79), Skinning61 (61 to 70), Skinning1 (1 to 60), by the creature's
level.

## Corpses gathered by another profession — 9 families (decided on 2026-10-01)

The corpses that the creature template gives to herbalism, mining or engineering (`type_flags`), by
profession and by expansion of the map:

| | Vanilla | BC | WotLK |
|---|---|---|---|
| Herbalism | VanillaCorpseHerbs | BcCorpseHerbs | WotlkCorpseHerbs |
| Mining | VanillaCorpseOres | BcCorpseOres | WotlkCorpseOres |
| Engineering | VanillaCorpseSalvage | BcCorpseSalvage | WotlkCorpseSalvage |

Names validated on 2026-10-01. Today 141 creatures, all of level 61 or more: the Vanilla families are
empty.

## Chests — 3 families (revised on 2026-10-01)

Only the **real chests** of the world and of dungeons; neither crates nor quest or event objects
(ignored). No data tells them apart: a **closed list** of entries.

| Family | Where | Chests |
|---|---|---|
| WorldChest | outside dungeons and raids | Battered, Tattered and Solid Chest; Large Battered Chest; Large Mithril Bound Chest; Battered, Dented, Mossy, Waterlogged and Scarlet footlockers; Fel Iron, Heavy Fel Iron, Adamantite Bound and Felsteel Chest |
| DungeonChest | dungeon in normal | Large Iron Bound, Large Solid, Large Battered and Large Mithril Bound Chest; Reinforced Fel Iron Chest |
| HeroicDungeonChest | dungeon in heroic | the same, when the instance is heroic |

**RaidChest is gone**: in a raid, a boss's chest is its loot.

**A boss's chest is its loot**: it belongs to the boss's family -- raid for a raid boss, dungeon (normal or
heroic) for a dungeon boss. Closed list:

| Place | Chests |
|---|---|
| Raid | Cache of Winter (Hodir), Deathbringer's Cache (Saurfang), Gunship Armory (gunships, Icecrown Citadel), Dust Covered Chest (chess event, Karazhan) |
| Dungeon | Tribunal Chest (Halls of Stone), Cache of Eregos (The Oculus), The Captain's Chest (Halls of Reflection), Chest of The Seven (Blackrock Depths), Cache of the Legion (The Mechanar), The Talon King's Coffer (Sethekk Halls), Baelog's Chest (Uldaman) |

Ignored, as event or quest chests: Scarab Coffers (Ahn'Qiraj), Tanzar's Trunk (Zul'Aman), Dark Coffer,
Relic Coffer, Secret Safe, Doan's Strongbox, Malor's Strongbox, Fengus's Chest, Knot Thimblejack's Cache,
Ancient Treasure, Old Treasure Chest, Witch Doctor's Chest, Benedict's Chest, Captain's Chest, Hidden
Strongbox, Stolen Chest, Wicker Chest, Worn Wooden Chest, Rusted Prisoner's Footlocker.

## Inventory containers — 3 families (decided on 2026-10-01)

The lockboxes that really give items, the crates and the chests opened from the bags: VanillaContainer,
BcContainer, WotlkContainer. Closed list (validated on 2026-10-01):

| Family | Containers |
|---|---|
| VanillaContainer | Ornate Bronze, Heavy Bronze, Iron, Strong Iron, Steel, Reinforced Steel, Mithril and Thorium Lockbox; Small, Sturdy, Ironbound and Reinforced Locked Chest; Battered, Worn, Sturdy and Heavy Junkbox; fishing crates and chests: Battered Chest, Small Chest, Dented, Waterlogged, Sealed and Heavy Crate, Tightly Sealed, Watertight, Iron Bound and Mithril Bound Trunk; Pirate's Footlocker, Curious Crate, Heavy Supply Crate |
| BcContainer | Eternium Lockbox, Khorium Lockbox, Strong Junkbox |
| WotlkContainer | Froststeel Lockbox, Titanium Lockbox, Tiny Titanium Lockbox, Reinforced Junkbox, Reinforced Crate |

Left out: the Brooding Darkwater Clam (a clam, neither crate nor chest), test items and items without
content, quest or event rewards (Cenarion Circle Cache, Chest of Spoils, Mysterious Lockbox, Cache of the
Ley-Guardian, Alchemist's Cache, Keg-Shaped Treasure Chest, Crate of Meat…) and the Tarot's crate (Crate of
Goods, 87600).

## Step 2 — what a module declares (decided on 2026-10-01)

**Independent lines.** Each line targets ONE family (of the first filter, or a MobLvl_X_Y bracket). They
are not combinations of conditions: a creature matching several lines can give the items of each.

**Modules never deprive one another.** The same creature can give a card AND a Nexus, if their modules
declare it.

**Fallbacks.** Within one declaration, dice rolled one after the other, stopping at the first success:
"25 % a Nexus; otherwise 50 % another Nexus (of the same level or not)". A fallback guarantees nothing:
with 25 % A, then 25 % B, then 25 % C, all three dice can fail and the player gets nothing.

**Lists.** An item can be drawn at random from a list, without weights (every item has the same chance).
The module declares its lists in its `.conf`, as arrays, and its drops by naming these arrays. The same
item can be in several arrays. Example:

```
card_tiers[0] = [card_id_A, card_id_B, card_id_C]
card_tiers[1] = [card_id_C, card_id_D, card_id_E]
VanillaContainer = card_tiers[0]:1:1
```

-- Vanilla containers have a 1 % chance to give a card drawn at random among A, B and C
(`<list>:<quantity>:<chance>`).

**The syntax** (decided on 2026-10-01):

- `,` chains fallbacks; `;` separates independent draws within one family.
- A draw targets a list (`name[i]`) or directly an item entry: `<list or item>:<quantity>:<chance>`.
- The quantity of a draw on a list is that many distinct draws in the list, each giving one item
  (`card_tiers[0]:3:5`: three cards, each drawn on its own); on an item, that many of the item.
- Lists and drops carry the module's prefix, in **that module's** `.conf`: for the Tarot,
  `mod-stellar-tarot.conf`.

```
StellarTarot.Loot.card_tiers[0] = [87001, 87002, 87003]
StellarTarot.Loot.VanillaContainer = card_tiers[0]:1:1
SphereGrid.Loot.WotlkRaidBoss = nexus[4]:1:25 , nexus[3]:1:50 ; stones[3]:1:100
SphereGrid.Loot.MobLvl_80_89 = 85104:1:2
```

The third line: 25 % a level 4 Nexus, otherwise 50 % a level 3 Nexus; and, independently, a level 3 stone
for sure. The fourth: an item named directly.

## Step 3 — the mechanisms

Decided on 2026-10-01:

- **Rate multipliers live in the declaring module**, in its `.conf`: cards must be raisable without
  touching the Nexus.
- **The grey boss brake and the bad luck protection are held by the extension.**
- **One rate factor per module, global**: no distinction within the module (neither by list nor by item).
  Example: `StellarTarot.Loot.Rate = 100`.
- **The grey boss brake applies to every module**: a boss whose level is grey for the player has its
  chances divided by 5 (Spheregrid's rule).
- **The bad luck protection is global, across all modules**: one counter per player; any item won through
  the extension -- a card, a Nexus, or the item of a module to come -- resets it to zero. It rises as in
  Spheregrid: +1 point per world creature killed without a win, +2.5 per raid boss, nothing in dungeons;
  it is kept in memory and lost at logout (decided on 2026-10-01).
- **Its points are added to the chance of every draw**, except for the items that **the declaring module
  excludes** itself in its `.conf` (as Spheregrid excludes its Prismatic Nexus today). Validated syntax:
  `SphereGrid.Loot.NoPity = [85105]`. An excluded item that drops still resets the counter: any item won
  does.

## Step 4 — the form (validated on 2026-10-01)

Decided on 2026-10-01:

- **Name**: repository `WoW-mods-dynamicloot`, folder and module `dynamicloot`.
- **DynamicLoot is a module of its own, installed once.** It determines the families, places the loot
  (corpses, gathering nodes, chests, containers) and holds the grey boss brake and the bad luck
  protection: all this logic is written once, in its code.
- **The modules that use it (Spheregrid, Stellar Tarot, those to come) only set their loot**: what drops,
  in which families, at what chance, their rate factor and their exclusions, in their own `.conf` (steps 2
  and 3). They have no loot code.
- **A module declares itself to the extension when the server starts**, through its `<Module>.Loot.…`
  settings. AzerothCore loads the `.conf` of every module into one configuration (`LoadModulesConfigs`);
  at startup, DynamicLoot reads every setting of that form there (`GetKeysByString`,
  `src/common/Configuration/Config.h`). No code link between DynamicLoot and the other modules. The `.conf`
  reader accepts the syntax of step 2 (brackets in names; `[ ]`, `:`, `,` and `;` in values); a key written
  twice in one file: only the first counts.
- **The closed lists** (real chests, inventory containers, boss chests, exclusions) **live in the
  extension's code**.
- **The families are extensible, by DynamicLoot only**: more can be added later (fishing, pickpocketing,
  battlegrounds…) without breaking anything in the modules that already use it. A module does not create
  families; it sets its loot on the ones DynamicLoot defines. A module's line targeting a family that the
  installed version of DynamicLoot does not know: a **warning** in the log when the server starts, and the
  line is ignored; the rest of the module's loot works.
- **DynamicLoot missing**:
  - when the server starts, each module that needs it reports it with a **warning** in the log, not an
    error. Its C++ looks directly whether `dynamicloot` is among the server's modules
    (`Acore::Module::GetEnableModulesList()`, `src/server/game/Modules/ModuleMgr.h`), without looking at
    its settings. The module works; none of its loot drops;
  - the installer also reports it when it installs a module that needs it;
  - removing DynamicLoot while modules that need it stay installed: the installer warns, then goes on;
    these modules only lose their loot.
- **The dependency in the manifest**: a `dependencies` key in the module's `installer.json`, the list of the
  modules it needs, each named as the `module` field of its own manifest: `"dependencies": ["dynamicloot"]`.
  At installation, a dependency missing from the server gives a warning, and the installation goes on. When
  a module is removed, the installer looks among the installed modules (each keeps its `installer.json` in
  the server sources) for those that list it in their `dependencies`, names them in a warning, and removes
  it anyway. The key serves any future dependency between modules.
- **Migration**: once DynamicLoot is written, Spheregrid and the Tarot lose their current loot system (for
  the Tarot, its source and pool tables), replaced by their settings in their `.conf`.
- Finding: the extension is C++ (the Lua engine has no event when a loot is filled), and AzerothCore
  compiles every module into one library (`modules/CMakeLists.txt`): two copies of the same C++ code cannot
  live side by side there. Hence a module of its own, installed once.

## Proposed on 2026-09-30, not retained to date

BcHeroicTrash / BcHeroicBoss; WotLK raids by difficulty (10, 25, heroic); world elites; battlegrounds and
Wintergrasp; open water fishing and fish schools; pickpocketing; chests by expansion.
