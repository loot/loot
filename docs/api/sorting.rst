************************
LOOT's Sorting Algorithm
************************

LOOT's sorting algorithm consists of four stages:

.. contents::
  :local:

Load plugin data
================

In this first stage, the plugins to be sorted are parsed and their FormIDs
stored. Parsing is multithreaded by dividing the plugins into buckets with
roughly equal total file sizes, and loading each bucket's plugins in a separate
thread. The number of buckets created is equal to the number of concurrent
threads that are hardware-supported (e.g. a dual-core CPU without hyperthreading
may report that it supports two threads).

When parsing plugins, all subrecords are skipped over for efficiency, apart from
the subrecords of the ``TES4`` header record.

Create plugin graph vertices
=================================

Once loaded, a directed graph is created and the plugins are added to it in
lexicographical order as vertices. Any metadata a plugin has in the masterlist
and userlist are then merged into its vertex's data store.

Create plugin graph edges
==============================

In this section, the terms *vertex* and *plugin* are used interchangeably, and
the iteration order 'for each plugin' is the order in which the vertices were
added to the graph.

For each plugin:

1. If the plugin is a master file, add edges going to all non-master files. If
   the plugin is a non-master file, add edges coming from all master files.
2. Add edges coming from all the plugin's masters. Missing masters have no edges
   added.
3. Add edges coming from all the plugin's requirements. Missing requirements
   have no edges added.
4. Add edges coming from all the plugin's load after files that are installed
   plugins.

At this point, all explicit interdependencies have been graphed. Plugin priority
metadata values must now be propagated down the dependency trees to ensure that
priority edges are added correctly later in the process. To do this:

1. Create a list of all vertices with a global or non-global priority value
   greater than zero.
2. Sort the list in order of decreasing priority value.
3. For each vertex, perform a depth-first search, setting priorities at each
   vertex visited until equal or larger values are encountered.

Now that the priorities have been propagated, the priority edges can be added.
For each plugin, if it has a global priority value of zero, overrides no records
and loads no archive, skip it, otherwise iterate over all other plugins and:

* If the other plugin's global and non-global priority values equal the
  plugin's own values, or if both plugins have a global priority of zero and
  have no FormIDs in common, skip the other plugin.
* Otherwise, add an edge from the plugin with lower global priority to the
  plugin with higher global priority, if that edge does not cause a cycle. A
  cycle is caused if a circular dependency is introduced, for example for two
  vertices A and B, A -> B -> A is a cycle.

  If the global priorities are equal, compare the non-global priorities
  instead.

Plugin overlap edges are then added. Two plugins overlap if they contain the
same FormID, i.e. if they both edit the same record or if one edits a record the
other plugin adds.

For each plugin, skip it if it overrides no records, otherwise iterate over all
other plugins.

* If the plugin and other plugin override the same number of records, or do not
  overlap, skip the other plugin.
* Otherwise, add an edge from the plugin which overrides more records to the
  plugin that overrides fewer records, unless that edge would cause a cycle.

Finally, tie-break edges are added to ensure that sorting is consistent. For
each plugin, iterate over all other plugins and add an edge between each pair of
plugins in the direction given by the tie-break comparison function, unless that
edge would cause a cycle.

The tie-break comparison function compares current plugin load order positions,
falling back to plugin names.

* If both plugins have positions in the current load order, the function
  preserves their existing relative order.
* If one plugin has a position and the other does not, the edge added goes from
  the plugin with a position to the plugin without a position.
* If neither plugin has a load order position, a case-insensitive
  lexicographical comparison of their filenames without file extensions is used
  to decide their order.

Topologically sort the plugin graph
===================================

Note that edges for explicit interdependencies are the only edges allowed to
create cycles: this is because the first step of this stage is to check the
plugin graph for cycles, and throw an error if any are encountered, so that
metadata (or indeed plugin data) that cause them can be corrected.

Once the graph is confirmed to be cycle-free, a topological sort is performed on
the graph, outputting a list of plugins in their newly-sorted load order.
