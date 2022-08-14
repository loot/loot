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

#ifndef LOOT_GUI_QT_GROUPS_EDITOR_GROUPS_EDITOR_DIALOG
#define LOOT_GUI_QT_GROUPS_EDITOR_GROUPS_EDITOR_DIALOG

#include <QtGui/QCloseEvent>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <set>

#include "gui/qt/groups_editor/graph_view.h"
#include "gui/qt/plugin_item_model.h"

namespace loot {
class GroupsEditorDialog : public QDialog {
  Q_OBJECT
public:
  GroupsEditorDialog(QWidget *parent, PluginItemModel *pluginItemModel);

  void setGroups(const std::vector<Group> &masterlistGroups,
                 const std::vector<Group> &userGroups,
                 const std::set<std::string> &installedPluginGroups,
                 const std::vector<GroupNodePosition> &nodePositions);

  std::vector<Group> getUserGroups() const;
  std::vector<GroupNodePosition> getNodePositions() const;

private:
  GraphView *graphView{new GraphView(this)};
  QLabel *groupPluginsTitle{new QLabel(this)};
  QListWidget *groupPluginsList{new QListWidget(this)};
  QPushButton *autoArrangeButton{new QPushButton(this)};
  QLabel *groupNameInputLabel{new QLabel(this)};
  QLineEdit *groupNameInput{new QLineEdit(this)};
  QPushButton *addGroupButton{new QPushButton(this)};

  PluginItemModel *pluginItemModel{nullptr};

  void setupUi();
  void translateUi();

  void closeEvent(QCloseEvent *event) override;

  bool askShouldDiscardChanges();

private slots:
  void on_graphView_groupSelected(const QString &name);
  void on_groupNameInput_textChanged(const QString &text);
  void on_addGroupButton_clicked();
  void on_autoArrangeButton_clicked();
  void on_dialogButtons_accepted();
  void on_dialogButtons_rejected();
};
}

#endif
