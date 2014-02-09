/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2013-2014    WrinklyNinja

    This file is part of BOSS.

    BOSS is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see
    <http://www.gnu.org/licenses/>.
*/
#ifndef __BOSS_GUI_EDITOR__
#define __BOSS_GUI_EDITOR__

#include "ids.h"
#include "misc.h"
#include "../backend/metadata.h"

#include <string>
#include <vector>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>

/* Have two versions of the editor: one mini editor that only contains 
   controls for editing the load order related metadata, and a full editor 
   that contains controls for editing all the metadata.
  
   Both editors need to convert between metadata and control data, and to
   keep track of what edits have been made.

   Have a immutable plugin list to keep track of non-user-edit metadata,
   and a mutable plugin list to keep track of user-edit metadata. When 
   recording edits, diff the metadata obtained from the control values with
   the metadata in the immutable plugin entry to get the new user-edit metadata.

   If a metadata type's corresponding control is not present (ie. in the mini
   editor), then use the immutable plugin entry's metadata for that type.
*/

class MiniEditor : public wxDialog {
public:
    MiniEditor(wxWindow *parent, const wxString& title, const std::list<boss::Plugin>& plugins, const boss::Game& game);

    void OnPluginSelect(wxListEvent& event);
    void OnRowSelect(wxListEvent& event);
    void OnRemoveRow(wxCommandEvent& event);
    void OnFilterToggle(wxCommandEvent& event);

    const std::list<boss::Plugin>& GetEditedPlugins() const;
private:
    wxButton * removeBtn;
    wxListView * pluginList;
    wxListView * loadAfterList;
    wxCheckBox * filterCheckbox;
    wxSpinCtrl * prioritySpin;
    wxStaticText * pluginText;

    void MiniEditor::ApplyEdits(const wxString& plugin);

    const std::list<boss::Plugin> _basePlugins;
    std::list<boss::Plugin> _editedPlugins;
    const boss::Game& _game;
};

class Editor : public wxFrame {
public:
    Editor(wxWindow *parent, const wxString& title, const std::string userlistPath, const std::list<boss::Plugin>& basePlugins, std::list<boss::Plugin>& editedPlugins, const unsigned int language, const boss::Game& game);

    void OnPluginSelect(wxListEvent& event);
    void OnPluginListRightClick(wxListEvent& event);
    void OnPluginCopyName(wxCommandEvent& event);
    void OnPluginCopyMetadata(wxCommandEvent& event);
    void OnPluginClearMetadata(wxCommandEvent& event);
    void OnListBookChange(wxBookCtrlEvent& event);
    void OnAddRow(wxCommandEvent& event);
    void OnEditRow(wxCommandEvent& event);
    void OnRemoveRow(wxCommandEvent& event);
    void OnRowSelect(wxListEvent& event);
    void OnQuit(wxCommandEvent& event);
private:

    wxMenu * pluginMenu;
    wxButton * addBtn;
    wxButton * editBtn;
    wxButton * removeBtn;
    wxButton * applyBtn;
    wxButton * cancelBtn;
    wxListView * pluginList;
    wxListView * reqsList;
    wxListView * incsList;
    wxListView * loadAfterList;
    MessageList * messageList;
    wxListView * tagsList;
    wxListView * dirtyList;
    wxNotebook * listBook;
    wxCheckBox * enableUserEditsBox;
    wxSpinCtrl * prioritySpin;
    wxStaticText * pluginText;

    const std::string _userlistPath;
    const std::list<boss::Plugin> _basePlugins;
    std::list<boss::Plugin> _editedPlugins;
    const boss::Game& _game;

    void ApplyEdits(const wxString& plugin);

    boss::Plugin GetMasterData(const wxString& plugin) const;
    boss::Plugin GetUserData(const wxString& plugin) const;
    boss::Plugin GetNewData(const wxString& plugin) const;

    boss::File RowToFile(wxListView * list, long row) const;
    boss::Tag RowToTag(wxListView * list, long row) const;
    boss::PluginDirtyInfo RowToPluginDirtyInfo(wxListView * list, long row) const;
};
#endif
