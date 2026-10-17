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

**In progress.** The design is settled and written down in
[docs/SPECIFICATION.md](docs/SPECIFICATION.md): the 54 families and the level
brackets, what a module declares and how, the mechanisms, and the form of the
extension. So far DynamicLoot reads and checks every module's declarations when
the server starts, reports what it cannot read, and places their loot on
creatures (first filter and level brackets, rate factor, grey boss brake), with
the bad luck protection shared by all modules (`.dynamicloot pity` shows or sets
it), on herbs and veins, on skinning and on the corpses gathered by herbalism,
mining or engineering, on the real chests and the boss chests, and on the
inventory containers (closed lists in the code). Still to come: the
`dependencies` key of the installer, and moving Spheregrid and the Stellar Tarot
over to DynamicLoot.

Original module, GPL-2.0-or-later, the licence of AzerothCore it is compiled
into.
