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
#include "../backend/metadata.h"

#include <string>
#include <vector>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>

class Editor : public wxFrame {
public:
    Editor(wxWindow *parent, const wxString& title, const std::string userlistPath, const std::vector<boss::Plugin>& basePlugins, std::vector<boss::Plugin>& editedPlugins, const unsigned int language);

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
    
    wxButton * addBtn;
    wxButton * editBtn;
    wxButton * removeBtn;
    wxButton * applyBtn;
    wxButton * cancelBtn;
    wxListView * pluginList;
    wxListView * reqsList;
    wxListView * incsList;
    wxListView * loadAfterList;
    wxListView * messageList;
    wxListView * tagsList;
    wxNotebook * listBook;
    wxCheckBox * enableUserEditsBox;
    wxSpinCtrl * prioritySpin;
    wxStaticText * pluginText;

    const std::string _userlistPath;
    const std::vector<boss::Plugin> _basePlugins;
    std::vector<boss::Plugin> _editedPlugins;
    const unsigned int _language;
    std::vector<boss::Message> currentMessages;

    void ApplyEdits(const wxString& plugin);
    
    boss::Plugin GetMasterData(const wxString& plugin) const;
    boss::Plugin GetUserData(const wxString& plugin) const;
    boss::Plugin GetNewData(const wxString& plugin) const;

    boss::File RowToFile(wxListView * list, long row) const;
    boss::Message RowToMessage(wxListView * list, long row) const;
    boss::Tag RowToTag(wxListView * list, long row) const;
};

class FileEditDialog : public wxDialog {
public:
    FileEditDialog(wxWindow *parent, const wxString& title);

    void SetValues(const wxString& name, const wxString& display, const wxString& condition);
    wxString GetName() const;
    wxString GetDisplayName() const;
    wxString GetCondition() const;
private:
    wxTextCtrl * _name;
    wxTextCtrl * _display;
    wxTextCtrl * _condition;
};

class MessageEditDialog : public wxDialog {
public:
    MessageEditDialog(wxWindow *parent, const wxString& title);

    void SetValues(int type, const wxString& content, const wxString& condition, int language);
    wxString GetType() const;
    wxString GetContent() const;
    wxString GetCondition() const;
    wxString GetLanguage() const;
private:
    wxChoice * _type;
    wxTextCtrl * _content;
    wxTextCtrl * _condition;
    wxChoice * _language;
};

class TagEditDialog : public wxDialog {
public:
    TagEditDialog(wxWindow *parent, const wxString& title);

    void SetValues(int state, const wxString& name, const wxString& condition);
    wxString GetState() const;
    wxString GetName() const;
    wxString GetCondition() const;
private:
    wxChoice * _state;
    wxTextCtrl * _name;
    wxTextCtrl * _condition;
};

#endif
