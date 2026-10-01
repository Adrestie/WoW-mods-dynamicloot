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

**Specification only. No code yet.** The design is settled and written down in
[docs/SPECIFICATION.md](docs/SPECIFICATION.md): the 53 families and the level
brackets, what a module declares and how, the mechanisms, and the form of the
extension. Nothing else is in this repository yet: no sources, no SQL, no
configuration.

Original module, GPL-2.0-or-later, the licence of AzerothCore it is compiled
into.
