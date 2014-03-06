/*  LOOT

A load order optimisation tool for Oblivion, Skyrim, Fallout 3 and
    Fallout: New Vegas.

Copyright (C) 2013-2014    WrinklyNinja

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
<http://www.gnu.org/licenses/>.
*/
#ifndef __LOOT_GUI_MISC__
#define __LOOT_GUI_MISC__

#include "ids.h"
#include "../backend/metadata.h"

#include <string>
#include <vector>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>

const wxString Type[3] = {
    translate("Note"),
    translate("Warning"),
    translate("Error")
};

const wxString State[2] = {
    translate("Add"),
    translate("Remove")
};

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