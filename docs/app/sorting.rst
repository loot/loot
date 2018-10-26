**********************
How LOOT Sorts Plugins
**********************

LOOT sorts plugins according to a set of rules that dictate how two plugins load
relative to one another. An example rule could be "``Cutting Room Floor.esp``
must load after ``Skyrim.esm``". LOOT derives rules from various data sources:
one source for the example rule could be the plugins' master flag fields:
``Skyrim.esm`` has its master flag set, and ``Cutting Room Floor.esp`` does not.
Another source could be the plugins' master fields: ``Cutting Room Floor.esp``
has ``Skyrim.esm`` as a master.

LOOT represents these rules in a graph, where each point (or *vertex*)
represents a plugin, and each line (or *edge*) joins two points, going from the
plugin that loads earlier to the plugin that loads later. Visualised, a simple
set of rules on a small load order looks like this:

.. image:: ../images/plugin_graph.png

It may seem like there are a lot of edges in that image, but a more realistic
load order graph could contain ~ 20 times as many vertices and ~ 1000 times as
many edges.

Building The Plugin Graph
=========================

The plugin graph is constructed in a specific way, partly to ensure that sorting is
stable (i.e. it doesn't give you different results every time).

Hard Rules
----------

First the 'hard' rules, which **must** be followed, are applied. For each
plugin, going in lexicographical (like alphabetical, but extended to cover
digits and other symbols) order:

1. If the plugin has its master flag set, edges are added going from it to every
   other plugin that does not have its master flag set.
2. If the plugin has any masters, edges are added going from each of them to
   the plugin.
3. If the plugin has any requirements metadata, edges are added going from each
   required plugin to the plugin.
4. If the plugin has any "load after" metadata, edges are added going from each
   "load after" plugin to the plugin.

Some games, like the various versions of Skyrim and Fallout 4, hardcode the
positions of some plugins. To take this into account when sorting, LOOT adds
hard rules for them. For each plugin that has a hardcoded position, going from
the first/earliest hardcoded position to the last/latest position, an edge is
added going from that plugin to every other plugin, unless that other plugin is
hardcoded to load in an earlier position.

In the example graph image above, all the edges apart from the one between
``Cutting Room Floor.esp`` and ``Bashed Patch, 0.esp`` could be due to hard
rules:

- ``Skyrim.esm`` is a master of all the other plugins.
- ``Skyrim.esm``, ``Update.esm``, ``Dawnguard.esm``, ``HearthFires.esm`` and
  ``Dragonborn.esm`` are all hardcoded to load in that order.
- All of the  ``*.esm`` plugins have their master flag set, and both of the
  ``*.esp`` plugins do not have it set.

Group Rules
-----------

At this point, all hard rules have been applied. Group rules are applied next.
This part has the most complex logic, though the ideas behind it are relatively
simple. To summarise, each plugin belongs to a group, and groups can load after
other groups. If group C loads after group B, and group B loads after group A,
this means that all the plugins in group C load after all the plugins in groups
A and B.

However, group rules are 'soft' rules, so can be ignored to avoid cyclic
interactions. A cyclic interaction occurs when following the rules results in a
load order that loops back on itself, e.g. the two rules "B loads after A" and
"A loads after B" are cyclic. If one of those rules is a hard rule and the other
is a group rule, LOOT will ignore the group rule to avoid the cycle. There are
also a few other cases in which LOOT can avoid a cycle involving group rules,
which are detailed in :ref:`groups_editor`.

It's not always possible for LOOT to choose which plugin's group metadata to
ignore, and it's often impractical to know all of the hard and group rules that
a plugin may be involved in, so plugin grouping is a relatively common source of
cyclic interaction errors.

Anyway, after applying all the hard rules, LOOT applies all the group rules it
can for each plugin in lexicographical order, avoiding cycles by ignoring those
groups that it needs to.

In the example graph image above, the edge from ``Cutting Room Floor.esp`` to
``Bashed Patch, 0.esp`` is due to a group rule, because ``Bashed Patch, 0.esp``
is in a group that loads after ``Cutting Room Floor.esp``'s group.

Overlap Rules
-------------

Overlap rules are applied after group rules, and have lower priority. They are
also soft rules, and are ignored as necessary to avoid cyclic interactions. Two
plugins are said to overlap if they both contain a copy of a record. They don't
necessarily have to make any edits to the record for there to be an overlap, it
just needs to be in both plugins.

If two plugins overlap, and one overrides more records than the other, then the
rule is to load the plugin that overrides fewer records after the other plugin.
This is done to help maximise the effect that each plugin has. If the two
plugins override the same number of records, the overlap is ignored and no rule
exists.

Each pair of plugins is checked in lexicographical order for overlap, and all
overlap rules are applied, unless adding a rule would cause a cycle.

Tie Breaks
----------

At this point LOOT might be ready to calculate a load order from the graph, but
to ensure a stable sort, it needs to make sure there is only one possible path
through the graph that visits every plugin. For example, going back to the image
above, if there was no edge between ``Cutting Room Floor.esp`` and
``Bashed Patch, 0.esp``, the load order could be::

    Skyrim.esm
    ...
    Cutting Room Floor.esp
    Bashed Patch, 0.esp

or it could be::

    Skyrim.esm
    ...
    Bashed Patch, 0.esp
    Cutting Room Floor.esp

as there would be no way to decide which plugin to put last. This could mean
that LOOT's sorting would be unstable, picking a different result each time,
which isn't good.

To avoid this, LOOT loops through each pair of plugins (again, in
lexicographical order) and checks if there is a path (i.e. a series of edges
added by rules) going from one to the other. If there is no path, LOOT adds a
tie-break edge.

The direction of the tie-break edge, i.e. which plugin loads after the other, is
decided by a few pieces of data:

1. If one plugin has a position defined in the current load order and the other
   plugin does not (e.g. it is newly-installed), then the latter loads after the
   former.
2. If both plugins have positions in the current load order, they retain their
   existing relative order, i.e. the plugin that currently loads later still
   loads later.
3. If neither plugin has a position defined in the current load order, their
   lowercased filenames are lexicographically compared, ignoring their file
   extensions. The plugin with the filename that sorts later loads after the
   other plugin. For example, ``A.esp`` and ``B.esp`` would load in that order.

It's possible for that logic to be unable to decide how to break the tie: in
that case, no tie-break edge is added, as another edge added between another
pair may also break the unresolved tie. Such cases are highly unlikely to occur
though, and pretty much have to be intentionally created.

Topological Sort
================

At this point the plugin graph is now complete. Before calculating a load order
from the graph, the graph is checked for cycles: if one is found, a sorting
error occurs. If no cycles are found, then the graph is topologically sorted,
which produces a path through the graph's vertices that visits each vertex
exactly once. This path is the calculated load order.

The topological sort of the example graph is::

    Skyrim.esm
    Update.esm
    Dawnguard.esm
    HearthFires.esm
    Dragonborn.esm
    Cutting Room Floor.esp
    Bashed Patch, 0.esp
