# dynamicloot

A shared loot extension for AzerothCore 3.3.5a: modules that add items across the whole game spread their drops through it, without touching the loot tables of every creature, node or chest.

## Key features

- Every loot source sorted into families: world creatures, dungeon and raid bosses by expansion, level brackets, rares, gathering nodes, skinning, chests, containers.
- A module only says, in its own `.conf`, what drops in which family and at what chance.
- A grey boss brake and a bad luck protection shared by every module.
- Declarations checked at startup, with a report of what cannot be read.
- `tools/loot-editor/loot-editor.exe` writes a mod's loot lines by drag and drop, with the game's icons, in five languages.
- Used by [mod-spheregrid](https://github.com/Adrestie/mod-spheregrid) and [mod-stellar-tarot](https://github.com/Adrestie/mod-stellar-tarot).

## Dependencies

| Dependency | For |
|---|---|
| [AzerothCore](https://github.com/azerothcore/azerothcore-wotlk) (WotLK 3.3.5a) | the server |

## How to install

1. Download `installer.exe` from the [WoW-mods-installer releases](https://github.com/Adrestie/WoW-mods-installer/releases).
2. Stop the worldserver.
3. Run `installer.exe`, give it this folder and the folders it asks for, click **Check**, then **Install**.
4. Rebuild the worldserver.

## How to update

Same steps with the new version's folder, choosing **Update** instead of **Install**. Rebuild the worldserver.

## How to uninstall

Same steps, choosing **Remove**. The modules that use it lose their drops until it is back. Rebuild the worldserver.

## Configuration

DynamicLoot has no `.conf` of its own: each module that uses it sets its drops in its own `.conf` (`<Module>.Loot.*` lines: family, item or list, chance, quantity, and a rate on top).

## Commands

| Command | |
|---|---|
| `.dynamicloot pity` | game master: shows or sets a player's bad luck protection |
