/*  LOOT

    A load order optimisation tool for
    Morrowind, Oblivion, Skyrim, Skyrim Special Edition, Skyrim VR,
    Fallout 3, Fallout: New Vegas, Fallout 4 and Fallout 4 VR.

    Copyright (C) 2021    Oliver Hamlet

    This file is part of LOOT.

    LOOT is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    LOOT is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LOOT.  If not, see
    <https://www.gnu.org/licenses/>.
    */

#ifndef LOOT_GUI_QT_GROUPS_EDITOR_GRAPH_VIEW
#define LOOT_GUI_QT_GROUPS_EDITOR_GRAPH_VIEW

#include <loot/metadata/group.h>

#include <QtWidgets/QGraphicsView>
#include <set>

#include "gui/state/game/group_node_positions.h"

namespace loot {
class Node;

class GraphView : public QGraphicsView {
  Q_OBJECT
  Q_PROPERTY(QColor masterColor MEMBER masterColor READ getMasterColor)
  Q_PROPERTY(QColor userColor MEMBER userColor READ getUserColor)
  Q_PROPERTY(
      QColor backgroundColor MEMBER backgroundColor READ getBackgroundColor)

public:
  explicit GraphView(QWidget *parent = nullptr);

  void setGroups(const std::vector<Group> &masterlistGroups,
                 const std::vector<Group> &userGroups,
                 const std::set<std::string> &installedPluginGroups,
                 const std::vector<GroupNodePosition> &nodePositions);

  bool addGroup(const std::string &name);
  void renameGroup(const std::string &oldName, const std::string &newName);
  void setGroupContainsInstalledPlugins(const std::string &name,
                                        bool containsInstalledPlugins);
  void autoLayout();
  void registerUserLayoutChange();

  std::vector<Group> getUserGroups() const;
  std::vector<GroupNodePosition> getNodePositions() const;
  bool hasUnsavedLayoutChanges() const;
  bool isUserGroup(const std::string &name) const;

  void handleGroupRemoved(const QString &name);
  void handleGroupSelected(const QString &name);

  QColor getMasterColor() const;
  QColor getUserColor() const;
  QColor getBackgroundColor() const;

signals:
  void groupRemoved(const QString name);
  void groupSelected(const QString &name);

protected:
#if QT_CONFIG(wheelevent)
  void wheelEvent(QWheelEvent *event) override;
#endif

private:
  static constexpr qreal SCALE_CONSTANT = qreal(1.2);

  QColor masterColor;
  QColor userColor;
  QColor backgroundColor;
  bool hasUnsavedLayoutChanges_{false};

  void doLayout(const std::vector<GroupNodePosition> &nodePositions);
};
}

#endif
