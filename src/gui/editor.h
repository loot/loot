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

class MessageList : public wxListView {
public:
    MessageList(wxWindow * parent, wxWindowID id, const unsigned int language);

    void SetItems(const std::vector<boss::Message>& messages);
    std::vector<boss::Message> GetItems() const;

    boss::Message GetItem(long item) const;
    void SetItem(long item, const boss::Message& message);
    void AppendItem(const boss::Message& message);

    void OnDeleteItem(wxListEvent& event);
protected:
    wxString OnGetItemText(long item, long column) const;

private:
    std::vector<boss::Message> _messages;
    const unsigned int _language;
};

class Editor : public wxFrame {
public:
    Editor(wxWindow *parent, const wxString& title, const std::string userlistPath, const std::vector<boss::Plugin>& basePlugins, std::vector<boss::Plugin>& editedPlugins, const unsigned int language, const boss::Game& game);

    void OnPluginSelect(wxListEvent& event);
    void OnPluginListRightClick(wxListEvent& event);
    void OnPluginCopyName(wxCommandEvent& event);
    void OnPluginCopyMetadata(wxCommandEvent& event);
    void OnPluginClearMetadata(wxCommandEvent& event);
    void OnEnabledToggle(wxCommandEvent& event);
    void OnPriorityChange(wxSpinEvent& event);
    void OnListBookChange(wxBookCtrlEvent& event);
    void OnAddRow(wxCommandEvent& event);
    void OnEditRow(wxCommandEvent& event);
    void OnRemoveRow(wxCommandEvent& event);
    void OnRecalc(wxCommandEvent& event);
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
    const std::vector<boss::Plugin> _basePlugins;
    const boss::Game& _game;
    std::vector<boss::Plugin> _editedPlugins;
    std::vector<boss::Message> currentMessages;

    void ApplyEdits(const wxString& plugin);

    boss::Plugin GetMasterData(const wxString& plugin) const;
    boss::Plugin GetUserData(const wxString& plugin) const;
    boss::Plugin GetNewData(const wxString& plugin) const;

    boss::File RowToFile(wxListView * list, long row) const;
    boss::Tag RowToTag(wxListView * list, long row) const;
    boss::PluginDirtyInfo RowToPluginDirtyInfo(wxListView * list, long row) const;
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

    void SetMessage(const boss::Message& message);
    boss::Message GetMessage() const;

    void OnSelect(wxListEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnRemove(wxCommandEvent& event);
private:
    wxButton * addBtn;
    wxButton * editBtn;
    wxButton * removeBtn;
    wxChoice * _type;
    wxChoice * _language;
    wxListView * _content;
    wxTextCtrl * _condition;
    wxTextCtrl * _str;
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

class DirtInfoEditDialog : public wxDialog {
public:
    DirtInfoEditDialog(wxWindow * parent, const wxString& title);

    void SetValues(const wxString& crc, unsigned int itm, unsigned int udr, unsigned int nav, const wxString& utility);
    wxString GetCRC() const;
    wxString GetUtility() const;
    unsigned int GetITMs() const;
    unsigned int GetUDRs() const;
    unsigned int GetDeletedNavmeshes() const;
private:
    wxTextCtrl * _crc;
    wxSpinCtrl * _itm;
    wxSpinCtrl * _udr;
    wxSpinCtrl * _nav;
    wxTextCtrl * _utility;
};

#endif
