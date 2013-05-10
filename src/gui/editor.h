/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2013    WrinklyNinja

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
#include "../metadata.h"

#include <string>
#include <vector>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>

class Editor : public wxFrame {
public:
    Editor(wxWindow *parent, const wxString& title);

    void SetList(const std::vector<boss::Plugin>& basePlugins, const std::vector<boss::Plugin>& editedPlugins);
    void IsSorted(bool sorted);

    void OnPluginSelect(wxListEvent& event);
    void OnEnabledToggle(wxCommandEvent& event);
    void OnPriorityChange(wxSpinEvent& event);
    void OnListBookChange(wxBookCtrlEvent& event);
    void OnAddRow(wxCommandEvent& event);
    void OnEditRow(wxCommandEvent& event);
    void OnRemoveRow(wxCommandEvent& event);
    void OnRecalc(wxCommandEvent& event);
    void OnRowSelect(wxListEvent& event);
    void OnQuit(wxCommandEvent& event);
    DECLARE_EVENT_TABLE()
private:

    bool IsCurrentPluginEdited() const;
    boss::Plugin GetOriginal(const boss::Plugin& plugin, bool withEdits) const;
    void ApplyCurrentPluginEdits();
    
    wxButton * addBtn;
    wxButton * editBtn;
    wxButton * removeBtn;
    wxButton * recalcBtn;
    wxButton * applyBtn;
    wxButton * cancelBtn;
    wxListCtrl * pluginList;
    wxListCtrl * reqsList;
    wxListCtrl * incsList;
    wxListCtrl * loadAfterList;
    wxListCtrl * messageList;
    wxListCtrl * tagsList;
    wxNotebook * listBook;
    wxCheckBox * enableUserEditsBox;
    wxSpinCtrl * prioritySpin;
    wxStaticText * pluginText;

    std::vector<boss::Plugin> _basePlugins, _editedPlugins;

    boss::Plugin currentPlugin;

    //boss::Game& _game;
   // YAML::Node& _settings;  //BOSS Settings.
};

class FileEditDialog : public wxDialog {
public:
    FileEditDialog(wxWindow *parent, const wxString& title);

    void SetValues(const std::string& name, const std::string& display, const std::string& condition);
    std::vector<std::string> GetValues() const;
private:
    wxTextCtrl * _name;
    wxTextCtrl * _display;
    wxTextCtrl * _condition;
};

class MessageEditDialog : public wxDialog {
public:
    MessageEditDialog(wxWindow *parent, const wxString& title);

    void SetValues(const std::string& type, const std::string& content, const std::string& condition, const std::string& language);
    std::vector<std::string> GetValues() const;
private:
    wxChoice * _type;
    wxTextCtrl * _content;
    wxTextCtrl * _condition;
    wxChoice * _language;
};

class TagEditDialog : public wxDialog {
public:
    TagEditDialog(wxWindow *parent, const wxString& title);

    void SetValues(const std::string& addRemove, const std::string& name, const std::string& condition);
    std::vector<std::string> GetValues() const;
private:
    wxChoice * _addRemove;
    wxTextCtrl * _name;
    wxTextCtrl * _condition;
};

#endif
