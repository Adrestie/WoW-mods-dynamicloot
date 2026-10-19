# dynamicloot

A shared loot extension for AzerothCore 3.3.5a. Modules that add items across the
whole game -- [mod-spheregrid](https://github.com/Adrestie/mod-spheregrid),
[mod-stellar-tarot](https://github.com/Adrestie/mod-stellar-tarot) and those to
come -- spread their drops without touching the loot tables of every creature,
node or chest. DynamicLoot sorts every loot source into families (world
creatures, dungeon and raid bosses by expansion, level brackets, gathering nodes,
skinning, chests, inventory containers), places the items, and holds the grey
boss brake and a bad luck protection shared by all modules. A module that uses it
only says, in its own `.conf`, what drops in which family and at what chance.

The design is written down in [docs/SPECIFICATION.md](docs/SPECIFICATION.md):
the 54 families and the level brackets, what a module declares and how, the
mechanisms, and the form of the extension. DynamicLoot reads and checks every
module's declarations when the server starts, reports what it cannot read, and
places their loot on creatures (first filter, level brackets, rares of
dungeons, rate factor, grey boss brake), with the bad luck protection shared by
all modules (`.dynamicloot pity` shows or sets it), on herbs and veins, on
skinning and on the corpses gathered by herbalism, mining or engineering, on
the real chests and the boss chests, and on the inventory containers (closed
lists in the code). [mod-spheregrid](https://github.com/Adrestie/mod-spheregrid)
and [mod-stellar-tarot](https://github.com/Adrestie/mod-stellar-tarot) drop
their items through it, and list it in the `dependencies` of their
`installer.json`.

## The loot editor

`tools/loot-editor` holds `loot-editor.exe`, which writes a mod's loot lines
(the `<Mod>.Loot.*` settings of its `.conf`) without typing them.

* Give it the game folder (the icons come from its archives) and the
  worldserver folder (its `worldserver.conf` gives the world database; the
  families are read from the DynamicLoot installed in the server's sources),
  then **Connect**.
* Search items by name, by entry, or by range (`87001-87372`); the results
  show with their icons, framed in their quality's colour.
* **Mod (optional)**: the folder of one of your mods (the one of its
  `installer.json`). Its items show by themselves in the search: those of the
  `item_template` rows its manifest declares, else those its `data/sql` files
  insert; **Mod's items** shows them again, and the mod's loot prefix (read
  from its `.conf` template) fills the prefix when it is empty.
* Drag an item onto a family: a new draw (a die). Onto an existing die: a
  fallback, tried after the others. Click an item placed in a family to set
  its chance and quantity, move it in its die or delete it; drag it to another
  family to move it (Ctrl: copy), or back onto the search to remove it.
* **New list** makes a list (`items[0]`, renamed at will); drop items into
  it, or **Add all to the list** for a whole search. A list is dragged into
  the families like an item, and drawn without weights.
* Drop items on **NoPity** to keep the bad luck protection off them; set the
  mod's prefix and its rate at the top.
* **Import a .conf** reads the `<Mod>.Loot.*` lines of a mod already
  written, to change them. **Generate** checks everything as DynamicLoot
  would, copies the lines to the clipboard and shows them: paste them into the
  mod's `.conf` (`configs/modules`), then restart the worldserver.

The window speaks French when the game runs in French (`SET locale` of
`WTF/Config.wtf`), English otherwise; the language can be changed at the top
right. It only reads: the database, the game and the server's files are never
written. The installer leaves `tools` out of the server's sources.

`tools/loot-editor/build.cmd` builds it with PyInstaller and Pillow
(`python -m pip install pyinstaller pillow`); without building,
`python tools/loot-editor/loot_editor.py`. Its `core.py`, `manifest.py`, `window.py`,
`mpq_archive.py` and `wow_exe.py` are copies of the
[WoW-mods installer](https://github.com/Adrestie/WoW-mods-installer)'s engine
(MIT), refreshed from there unchanged; the editor itself is MIT as well.

Original module, GPL-2.0-or-later, the licence of AzerothCore it is compiled
into.
