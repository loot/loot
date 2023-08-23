.. _groups_editor:

*********************
Editing Plugin Groups
*********************

.. image:: ../../images/groups_editor.png

What Are Plugin Groups?
=======================

LOOT assigns each plugin to one plugin group, with plugins belonging to the
``default`` group by default. Each group has a name and a list of zero or more
other groups it loads after. In this way, it's possible to concisely load groups
of plugins after other groups of plugins.

Group load order is transitive, i.e. given three groups A, B and
C, if C loads after B and B loads after A, then plugins in group C will load
after plugins in group A even if no plugins in group B are installed.

The Groups Editor
=================

A group must be defined before plugins can belong to it, and defining and
editing groups is done in the Groups Editor, which can be accessed through the
Game menu.

The groups editor consists of an interactive graph displaying all defined groups
and their load after metadata, and a sidebar containing input for defining new
groups, renaming the currently selected group, a list of plugins in the
currently selected group and a dropdown combo box for adding plugins to the
currently selected group.

- Groups are displayed as circular nodes in the graph, labelled with their
  names.

  - Groups that load after no other groups are displayed in blue.
  - Groups that no other groups load after are displayed in green.
  - The ``default`` group is displayed in orange.

- Load after metadata is displayed as lines (edges/vertices) between nodes,
  pointing from the earlier group to the later group.
- Metadata defined in the masterlist is greyed out, while user-defined metadata
  is not.

If any group definitions reference another group that does not exist, the groups
editor will create the missing group as user metadata. This is to help when
there is user metadata that says the user-defined group B must load after the
masterlist-defined group A, but then group A is removed in a masterlist update.
In that case, just open up the groups editor and link group A back into the
graph as it was before.

New load after metadata can be added by double-clicking on one group node and
dragging a line from it to any other group nodes.

Clicking on a group will cause any installed plugins in that group to be listed
in the sidebar. Right-clicking the list will display a context menu that
contains an action to copy the listed plugin names to the clipboard.

Right-clicking a load after metadata line will remove that load after metadata,
and right-clicking a group will remove it. Masterlist metadata cannot be
removed. A group cannot be removed if any installed plugins belong to it.

The graph can be zoomed in and out of using your mouse's scroll wheel.
Left-clicking and dragging an empty space will move the whole graph, while
left-clicking and dragging a node will move it.

Rules For Using Groups
======================

The groups editor enforces a few rules:

- A group cannot load after itself.
- A group cannot load after another group if the other group does not exist.
- It's not possible to delete groups that are defined in the masterlist.
- It's not possible to remove 'load after' entries from a group if they were
  defined in the masterlist.

Another rule that the groups editor cannot enforce is that **group metadata must
not introduce cycles**. A simple example of cyclic groups is where group ``B``
loads after group ``A``, and group ``A`` loads after group ``B``.

A more complex example involving other types of metadata is where

- ``A.esp`` is in the ``early`` group
- ``B.esp`` is in the ``mid`` group
- ``C.esp`` is in the ``late`` group
- ``A.esp`` has ``C.esp`` as a master
- The ``late`` group loads after the ``mid`` group, which loads after the
  ``early`` group.

This will cause a cyclic interaction error when sorting, because the groups say
that the load order should be

1. ``A.esp``
2. ``B.esp``
3. ``C.esp``

but ``A.esp`` must load after ``C.esp`` to satisfy its dependency.

Cycle Avoidance
===============

Groups must not introduce cycles, but in practice this can be quite hard to
ensure. LOOT helps by avoiding cycles that have an "obvious" solution.

- If group membership contradicts where a plugin's masters, master flag, load
  after or requirement metadata say that plugin should load relative to another
  plugin, the plugins' groups' relationship will not be enforced. For example,
  if:

    - ``dependent.esp`` belongs to group ``early``
    - ``master.esp`` belongs to group ``late``
    - ``master.esp`` is a master of ``dependent.esp``
    - The ``late`` group loads after the ``early`` group.

  ``dependent.esp`` must load after ``master.esp`` due to the former being a
  master of the latter, but their groups suggest that ``master.esp`` must load
  after ``dependent.esp``, so the group metadata is ignored for that pair of
  plugins.

- In addition, if one of a pair of plugins with contradictory groups is the
  ``default`` group, that plugin will also have its group metadata ignored for
  all plugins in all groups that load between ``default`` and the other plugin's
  group.

  For example, if:

    - ``A.esp`` is in the ``default`` group
    - ``B.esp`` is in the ``mid`` group
    - ``C.esp`` is in the ``late`` group
    - ``A.esp`` has ``C.esp`` as a master
    - The ``late`` group loads after the ``mid`` group, which loads after the
      ``default`` group.

  This will not cause a cycle, as:

    - ``A.esp``'s group is ignored for ``C.esp`` as their groups contradict
      ``C.esp`` being a master of ``A.esp``
    - ``A.esp``'s group is ignored for ``B.esp`` as ``B.esp`` is in the ``mid``
      group, which loads between ``default`` and ``late``.

  The sorted load order is therefore:

    1. ``B.esp``
    2. ``C.esp``
    3. ``A.esp``

  This is very similar to the example given in the previous section which *did*
  cause a cycle: the only difference is that the ``early`` group is now
  ``default``.
