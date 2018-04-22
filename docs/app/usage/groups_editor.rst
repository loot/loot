*********************
Editing Plugin Groups
*********************

.. image:: ../../images/groups_editor.png

LOOT assigns each plugin to one plugin group, with plugins belonging to the
``default`` group by default. Each group has a name and a list of zero or more
other groups it loads after. In this way, it's possible to concisely load groups
of plugins after other groups of plugins.

A group must be defined before plugins can belong to it, and defining and
editing groups is done in the Groups Editor, which can be accessed through the
main menu.

The groups editor consists of an interactive graph displaying all defined groups
and their load after metadata, and an input for defining new groups. Metadata
defined in the masterlist is greyed out, while user-defined metadata is not.

It's worth noting that groups are transitive, i.e. given three groups A, B and
C, if C loads after B and B loads after A, then plugins in group C will load
after plugins in group A even if no plugins in group B are installed.

The groups editor enforces a few rules:

- A group cannot load after itself.
- A group cannot load after another group if the other group does not exist.
- It's not possible to delete groups that are defined in the masterlist.
- It's not possible to remove 'load after' entries from a group if they were
  defined in the masterlist.

There are also a few rules that the groups editor cannot reasonably enforce, and
which are checked when sorting:

- Group loading must not be cyclic. For example, if group C loads after group B,
  and group B loads after group A and group A loads after group C, this will
  cause a cyclic interaction error when sorting.

- Group metadata produce "soft" sorting rules. This means that when group
  membership contradicts a "hard" sorting rule (produced from a plugin's
  masters, master flag, load after or requirement metadata), it will be ignored
  to avoid a cyclic interaction.

  For example, if:

    - A.esp belongs to group A
    - B.esp belongs to group B
    - A.esp is a master of B.esp
    - Group A loads after group B

  B.esp must load after A.esp, but their groups suggest that A.esp must load
  after B.esp, so the group metadata is ignored.

- Group metadata must not introduce cycles. For example, if:

    - A.esp belongs to group A
    - B.esp belongs to group B
    - C.esp belongs to group C
    - Group B loads after group A
    - Group C loads after goup B
    - C.esp is a master of A.esp

  This will cause a cyclic interaction error when sorting, as no group-derived
  sorting rule individually contradicts the master-derived rule, but given the
  master-derived rule, it's impossible for both group-derived rules to be
  applied without causing a cycle, and there's no meaningful way to choose one.
