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

#include "editor.h"
#include "../backend/generators.h"
#include "../backend/streams.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>

using namespace std;

wxString Type[3] = {
    translate("Note"),
    translate("Warning"),
    translate("Error")
};

wxString State[2] = {
    translate("Add"),
    translate("Remove")
};

MessageList::MessageList(wxWindow * parent, wxWindowID id, const unsigned int language) : wxListView(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VIRTUAL|wxLC_SINGLE_SEL), _language(language) {
    InsertColumn(0, translate("Type"));
    InsertColumn(1, translate("Content"));
    InsertColumn(2, translate("Condition"));
    InsertColumn(3, translate("Language"));

    Bind(wxEVT_COMMAND_LIST_DELETE_ITEM, &MessageList::OnDeleteItem, this);

    SetItemCount(0);
}

std::vector<boss::Message> MessageList::GetItems() const {
    return _messages;
}

void MessageList::SetItems(const std::vector<boss::Message>& messages) {
    _messages = messages;
    SetItemCount(_messages.size());
    RefreshItems(0, _messages.size() - 1);
}

boss::Message MessageList::GetItem(long item) const {
    return _messages[item];
}

void MessageList::SetItem(long item, const boss::Message& message) {
    _messages[item] = message;
    RefreshItem(item);
}

void MessageList::AppendItem(const boss::Message& message) {
    _messages.push_back(message);
    SetItemCount(_messages.size());
    RefreshItem(_messages.size() - 1);
}

void MessageList::OnDeleteItem(wxListEvent& event) {
    _messages.erase(_messages.begin() + event.GetIndex());
    SetItemCount(_messages.size());
    RefreshItems(event.GetIndex(), _messages.size() - 1);
}

wxString MessageList::OnGetItemText(long item, long column) const {
    if (column < 0 || column > 3 || item < 0 || item > _messages.size() - 1)
        return wxString();

    if (column == 0) {
        if (_messages[item].Type() == boss::g_message_say)
            return Type[0];
        else if (_messages[item].Type() == boss::g_message_warn)
            return Type[1];
        else
            return Type[2];
    } else if (column == 1) {
        return FromUTF8(_messages[item].ChooseContent(_language).Str());
    } else if (column == 2) {
        return FromUTF8(_messages[item].Condition());
    } else {
        return Language[ GetLangIndex(boss::GetLangString(_messages[item].ChooseContent(_language).Language())) ];
    }
}

Editor::Editor(wxWindow *parent, const wxString& title, const std::string userlistPath, const std::vector<boss::Plugin>& basePlugins, std::vector<boss::Plugin>& editedPlugins, const unsigned int language) : wxFrame(parent, wxID_ANY, title), _userlistPath(userlistPath), _basePlugins(basePlugins), _editedPlugins(editedPlugins) {

    //Initialise child windows.
    listBook = new wxNotebook(this, BOOK_Lists);

    wxPanel * reqsTab = new wxPanel(listBook);
    wxPanel * incsTab = new wxPanel(listBook);
    wxPanel * loadAfterTab = new wxPanel(listBook);
    wxPanel * messagesTab = new wxPanel(listBook);
    wxPanel * tagsTab = new wxPanel(listBook);

    //Initialise controls.
    pluginText = new wxStaticText(this, wxID_ANY, "");
    prioritySpin = new wxSpinCtrl(this, wxID_ANY, "0");
    prioritySpin->SetRange(-10,10);
    enableUserEditsBox = new wxCheckBox(this, wxID_ANY, translate("Enable User Changes"));
    
    addBtn = new wxButton(this, BUTTON_AddRow, translate("Add File"));
    editBtn = new wxButton(this, BUTTON_EditRow, translate("Edit File"));
    removeBtn = new wxButton(this, BUTTON_RemoveRow, translate("Remove File"));
    applyBtn = new wxButton(this, BUTTON_Apply, translate("Save Changes"));
    cancelBtn = new wxButton(this, BUTTON_Cancel, translate("Cancel"));

    pluginList = new wxListView(this, LIST_Plugins, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    reqsList = new wxListView(reqsTab, LIST_Reqs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    incsList = new wxListView(incsTab, LIST_Incs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    loadAfterList = new wxListView(loadAfterTab, LIST_LoadAfter, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    tagsList = new wxListView(tagsTab, LIST_BashTags, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    messageList = new MessageList(messagesTab, LIST_Messages, language);

    //Tie together notebooks and panels.
    listBook->AddPage(reqsTab, translate("Requirements"), true);
    listBook->AddPage(incsTab, translate("Incompatibilities"));
    listBook->AddPage(loadAfterTab, translate("Load After"));
    listBook->AddPage(messagesTab, translate("Messages"));
    listBook->AddPage(tagsTab, translate("Bash Tags"));

    //Set up list columns.
    pluginList->AppendColumn(translate("Plugins"));
    
    reqsList->AppendColumn(translate("Filename"));
    reqsList->AppendColumn(translate("Display Name"));
    reqsList->AppendColumn(translate("Condition"));
    
    incsList->AppendColumn(translate("Filename"));
    incsList->AppendColumn(translate("Display Name"));
    incsList->AppendColumn(translate("Condition"));
    
    loadAfterList->AppendColumn(translate("Filename"));
    loadAfterList->AppendColumn(translate("Display Name"));
    loadAfterList->AppendColumn(translate("Condition"));

    tagsList->AppendColumn(translate("Add/Remove"));
    tagsList->AppendColumn(translate("Bash Tag"));
    tagsList->AppendColumn(translate("Condition"));

    //Initialise control states.
    addBtn->Enable(false);
    editBtn->Enable(false);
    removeBtn->Enable(false);
    prioritySpin->Enable(false);
    enableUserEditsBox->Enable(false);

    //Make plugin name bold text.
    wxFont font = pluginText->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    pluginText->SetFont(font);

    //Set up event handling.
    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &Editor::OnPluginSelect, this, LIST_Plugins);
    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_Reqs);
    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_Incs);
    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_LoadAfter);
    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_Messages);
    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_BashTags);
    Bind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, &Editor::OnListBookChange, this, BOOK_Lists);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Editor::OnQuit, this, BUTTON_Apply);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Editor::OnQuit, this, BUTTON_Cancel);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Editor::OnAddRow, this, BUTTON_AddRow);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Editor::OnEditRow, this, BUTTON_EditRow);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Editor::OnRemoveRow, this, BUTTON_RemoveRow);

    //Set up layout.
    wxBoxSizer * bigBox = new wxBoxSizer(wxHORIZONTAL);

    bigBox->Add(pluginList, 1, wxEXPAND|wxALL, 10);

    wxBoxSizer * mainBox = new wxBoxSizer(wxVERTICAL);

    mainBox->Add(pluginText, 0, wxTOP|wxBOTTOM, 10);

    
    wxBoxSizer * hbox1 = new wxBoxSizer(wxHORIZONTAL);
    hbox1->Add(enableUserEditsBox, 0, wxALIGN_LEFT|wxRIGHT, 10);
    hbox1->AddStretchSpacer(1);
    hbox1->Add(new wxStaticText(this, wxID_ANY, translate("Priority: ")), 0, wxALIGN_RIGHT|wxLEFT|wxRIGHT, 5);
    hbox1->Add(prioritySpin, 0, wxALIGN_RIGHT);

    mainBox->Add(hbox1, 0, wxEXPAND|wxALIGN_RIGHT|wxTOP|wxBOTTOM, 5);

    wxBoxSizer * tabBox1 = new wxBoxSizer(wxVERTICAL);
    tabBox1->Add(reqsList, 1, wxEXPAND);
    reqsTab->SetSizer(tabBox1);

    wxBoxSizer * tabBox2 = new wxBoxSizer(wxVERTICAL);
    tabBox2->Add(incsList, 1, wxEXPAND);
    incsTab->SetSizer(tabBox2);
    
    wxBoxSizer * tabBox3 = new wxBoxSizer(wxVERTICAL);
    tabBox3->Add(loadAfterList, 1, wxEXPAND);
    loadAfterTab->SetSizer(tabBox3);

    wxBoxSizer * tabBox4 = new wxBoxSizer(wxVERTICAL);
    tabBox4->Add(messageList, 1, wxEXPAND);
    messagesTab->SetSizer(tabBox4);

    wxBoxSizer * tabBox5 = new wxBoxSizer(wxVERTICAL);
    tabBox5->Add(tagsList, 1, wxEXPAND);
    tagsTab->SetSizer(tabBox5);

    mainBox->Add(listBook, 1, wxEXPAND|wxTOP|wxBOTTOM, 10);

    wxBoxSizer * hbox2 = new wxBoxSizer(wxHORIZONTAL);
    hbox2->Add(addBtn, 0, wxRIGHT, 5);
    hbox2->Add(editBtn, 0, wxLEFT|wxRIGHT, 5);
    hbox2->Add(removeBtn, 0, wxLEFT, 5);
    mainBox->Add(hbox2, 0, wxALIGN_RIGHT);
    
    mainBox->AddSpacer(30);

    wxBoxSizer * hbox6 = new wxBoxSizer(wxHORIZONTAL);
    hbox6->Add(applyBtn, 0, wxRIGHT, 5);
    hbox6->Add(cancelBtn, 0, wxLEFT, 5);
    mainBox->Add(hbox6, 0, wxALIGN_RIGHT);

    bigBox->Add(mainBox, 2, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10);

    //Fill pluginList with the contents of basePlugins.
    for (int i=0, max=_basePlugins.size(); i < max; ++i) {
        pluginList->InsertItem(i, FromUTF8(_basePlugins[i].Name()));
    }
    pluginList->SetColumnWidth(0, wxLIST_AUTOSIZE);
        
    SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));

    SetSizerAndFit(bigBox);
    Layout();
}

void Editor::OnPluginSelect(wxListEvent& event) {
    //Create Plugin object for selected plugin.
    wxString selectedPlugin = pluginList->GetItemText(event.GetIndex());
    wxString currentPlugin = pluginText->GetLabelText();
    
    //Check if the selected plugin is the same as the current plugin.
    if (selectedPlugin != currentPlugin) {
        //Apply any current edits.
        if (!currentPlugin.empty())
            ApplyEdits(currentPlugin);

        boss::Plugin plugin = GetMasterData(selectedPlugin);
        plugin.Merge(GetUserData(selectedPlugin), true);

        //Now fill editor fields with new plugin's info and update control states.
        pluginText->SetLabelText(FromUTF8(plugin.Name()));

        prioritySpin->SetValue(plugin.Priority());

        enableUserEditsBox->SetValue(plugin.Enabled());

        loadAfterList->DeleteAllItems();
        reqsList->DeleteAllItems();
        incsList->DeleteAllItems();
        messageList->DeleteAllItems();
        tagsList->DeleteAllItems();

        set<boss::File> files = plugin.LoadAfter();
        int i=0;
        for (set<boss::File>::const_iterator it=files.begin(), endit=files.end(); it != endit; ++it) {
            loadAfterList->InsertItem(i, FromUTF8(it->Name()));
            loadAfterList->SetItem(i, 1, FromUTF8(it->DisplayName()));
            loadAfterList->SetItem(i, 2, FromUTF8(it->Condition()));
            ++i;
        }

        files = plugin.Reqs();
        i=0;
        for (set<boss::File>::const_iterator it=files.begin(), endit=files.end(); it != endit; ++it) {
            reqsList->InsertItem(i, FromUTF8(it->Name()));
            reqsList->SetItem(i, 1, FromUTF8(it->DisplayName()));
            reqsList->SetItem(i, 2, FromUTF8(it->Condition()));
            ++i;
        }

        files = plugin.Incs();
        i=0;
        for (set<boss::File>::const_iterator it=files.begin(), endit=files.end(); it != endit; ++it) {
            incsList->InsertItem(i, FromUTF8(it->Name()));
            incsList->SetItem(i, 1, FromUTF8(it->DisplayName()));
            incsList->SetItem(i, 2, FromUTF8(it->Condition()));
            ++i;
        }

        list<boss::Message> messages = plugin.Messages();
        vector<boss::Message> vec(messages.begin(), messages.end());
        messageList->SetItems(vec);

        set<boss::Tag> tags = plugin.Tags();
        i=0;
        for (set<boss::Tag>::const_iterator it=tags.begin(), endit=tags.end(); it != endit; ++it) {
            if (it->IsAddition())
                tagsList->InsertItem(i, State[0]);
            else
                tagsList->InsertItem(i, State[1]);
            tagsList->SetItem(i, 1, FromUTF8(it->Name()));
            tagsList->SetItem(i, 2, FromUTF8(it->Condition()));
            ++i;
        }

        //Set control states.
        prioritySpin->Enable(true);
        enableUserEditsBox->Enable(true);
        addBtn->Enable(true);
        editBtn->Enable(false);
        removeBtn->Enable(false);
    }
}

void Editor::OnListBookChange(wxBookCtrlEvent& event) {
    if (event.GetSelection() == 0 || event.GetSelection() == 1) {
        addBtn->SetLabel(translate("Add File"));
        editBtn->SetLabel(translate("Edit File"));
        removeBtn->SetLabel(translate("Remove File"));
    } else if (event.GetSelection() == 2) {
        addBtn->SetLabel(translate("Add Plugin"));
        editBtn->SetLabel(translate("Edit Plugin"));
        removeBtn->SetLabel(translate("Remove Plugin"));
    } else if (event.GetSelection() == 3) {
        addBtn->SetLabel(translate("Add Message"));
        editBtn->SetLabel(translate("Edit Message"));
        removeBtn->SetLabel(translate("Remove Message"));
    } else if (event.GetSelection() == 4) {
        addBtn->SetLabel(translate("Add Bash Tag"));
        editBtn->SetLabel(translate("Edit Bash Tag"));
        removeBtn->SetLabel(translate("Remove Bash Tag"));
    }
    editBtn->Enable(false);
    removeBtn->Enable(false);

    reqsList->Select(reqsList->GetFirstSelected(), false);
    incsList->Select(incsList->GetFirstSelected(), false);
    loadAfterList->Select(loadAfterList->GetFirstSelected(), false);
    messageList->Select(messageList->GetFirstSelected(), false);
    tagsList->Select(tagsList->GetFirstSelected(), false);
    
    Layout();
}

void Editor::OnAddRow(wxCommandEvent& event) {
    if (listBook->GetSelection() < 3) {
        FileEditDialog * rowDialog = new FileEditDialog(this, translate("BOSS: Add File/Plugin"));

        if (rowDialog->ShowModal() == wxID_OK) {

            if (rowDialog->GetName().empty()) {
                wxMessageBox(
                    translate("Error: No filename specified. Row will not be added."),
                    translate("BOSS: Error"),
                    wxOK | wxICON_ERROR,
                    this);
                return;
            }

            wxListView * list;
            if (listBook->GetSelection() == 0)
                list = reqsList;
            else if (listBook->GetSelection() == 1)
                list = incsList;
            else
                list = loadAfterList;

            long i = list->GetItemCount();
            list->InsertItem(i, rowDialog->GetName());
            list->SetItem(i, 1, rowDialog->GetDisplayName());
            list->SetItem(i, 2, rowDialog->GetCondition());
        }
    } else if (listBook->GetSelection() == 3) {
        MessageEditDialog * rowDialog = new MessageEditDialog(this, translate("BOSS: Add Message"));

        if (rowDialog->ShowModal() == wxID_OK) {

            if (rowDialog->GetMessage().Content().empty()) {
                wxMessageBox(
                    translate("Error: No content specified. Row will not be added."),
                    translate("BOSS: Error"),
                    wxOK | wxICON_ERROR,
                    this);
                return;
            }

            messageList->AppendItem(rowDialog->GetMessage());
        }
    } else if (listBook->GetSelection() == 4) {
        TagEditDialog * rowDialog = new TagEditDialog(this, translate("BOSS: Add Tag"));

        if (rowDialog->ShowModal() == wxID_OK) {

            if (rowDialog->GetName().empty()) {
                wxMessageBox(
                    translate("Error: No Bash Tag specified. Row will not be added."),
                    translate("BOSS: Error"),
                    wxOK | wxICON_ERROR,
                    this);
                return;
            }
            
            long i = tagsList->GetItemCount();
            tagsList->InsertItem(i, rowDialog->GetState());
            tagsList->SetItem(i, 1, rowDialog->GetName());
            tagsList->SetItem(i, 2, rowDialog->GetCondition());
        }
    }
}

void Editor::OnEditRow(wxCommandEvent& event) {
    if (listBook->GetSelection() < 3) {
        FileEditDialog * rowDialog = new FileEditDialog(this, translate("BOSS: Edit File/Plugin"));

        wxListView * list;
        if (listBook->GetSelection() == 0)
            list = reqsList;
        else if (listBook->GetSelection() == 1)
            list = incsList;
        else
            list = loadAfterList;

        long i = list->GetFirstSelected();

        rowDialog->SetValues(list->GetItemText(i, 0), list->GetItemText(i, 1), list->GetItemText(i, 2));

        if (rowDialog->ShowModal() == wxID_OK) {

            if (rowDialog->GetName().empty()) {
                wxMessageBox(
                    translate("Error: No filename specified. Row will not be edited."),
                    translate("BOSS: Error"),
                    wxOK | wxICON_ERROR,
                    this);
                return;
            }
            
            list->SetItem(i, 0, rowDialog->GetName());
            list->SetItem(i, 1, rowDialog->GetDisplayName());
            list->SetItem(i, 2, rowDialog->GetCondition());
        }
    } else if (listBook->GetSelection() == 3) {
        MessageEditDialog * rowDialog = new MessageEditDialog(this, translate("BOSS: Edit Message"));

        long i = messageList->GetFirstSelected();

        rowDialog->SetMessage(messageList->GetItem(i));

        if (rowDialog->ShowModal() == wxID_OK) {

            if (rowDialog->GetMessage().Content().empty()) {
                wxMessageBox(
                    translate("Error: No content specified. Row will not be edited."),
                    translate("BOSS: Error"),
                    wxOK | wxICON_ERROR,
                    this);
                return;
            }

            messageList->SetItem(i, rowDialog->GetMessage());
        }
    } else if (listBook->GetSelection() == 4) {
        TagEditDialog * rowDialog = new TagEditDialog(this, translate("BOSS: Edit Tag"));

        long i = tagsList->GetFirstSelected();

        int stateNo;
        if (tagsList->GetItemText(i, 0) == State[0])
            stateNo = 0;
        else
            stateNo = 1;

        rowDialog->SetValues(stateNo, tagsList->GetItemText(i, 1), tagsList->GetItemText(i, 2));

        if (rowDialog->ShowModal() == wxID_OK) {

            if (rowDialog->GetName().empty()) {
                wxMessageBox(
                    translate("Error: No Bash Tag specified. Row will not be edited."),
                    translate("BOSS: Error"),
                    wxOK | wxICON_ERROR,
                    this);
                return;
            }
            
            tagsList->SetItem(i, 0, rowDialog->GetState());
            tagsList->SetItem(i, 1, rowDialog->GetName());
            tagsList->SetItem(i, 2, rowDialog->GetCondition());
        }
    }
}

void Editor::OnRemoveRow(wxCommandEvent& event) {
    wxListView * list;
    if (listBook->GetSelection() == 0)
        list = reqsList;
    else if (listBook->GetSelection() == 1)
        list = incsList;
    else if (listBook->GetSelection() == 2)
        list = loadAfterList;
    else if (listBook->GetSelection() == 3)
        list = messageList;
    else
        list = tagsList;

    list->DeleteItem(list->GetFirstSelected());

    editBtn->Enable(false);
    removeBtn->Enable(false);
}

void Editor::OnRowSelect(wxListEvent& event) {
    if (event.GetId() == LIST_Reqs) {

        //Create File object, search the masterlist vector for the plugin and search its reqs for this object.
        boss::File file = RowToFile(reqsList, event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        vector<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;

        set<boss::File> reqs = plugin.Reqs();

        if (reqs.find(file) == reqs.end()) {
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }
        
    } else if (event.GetId() == LIST_Incs) {

        boss::File file = RowToFile(incsList, event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        vector<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;

        set<boss::File> incs = plugin.Incs();

        if (incs.find(file) == incs.end()) {
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    } else if (event.GetId() == LIST_LoadAfter) {

        boss::File file = RowToFile(loadAfterList, event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        vector<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;

        set<boss::File> loadAfter = plugin.LoadAfter();

        if (loadAfter.find(file) == loadAfter.end()) {
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    } else if (event.GetId() == LIST_Messages) {

        boss::Message message = messageList->GetItem(event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        vector<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;

        list<boss::Message> messages = plugin.Messages();

        if (find(messages.begin(), messages.end(), message) == messages.end()) {
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    } else {

        boss::Tag tag = RowToTag(tagsList, event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        vector<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;

        set<boss::Tag> tags = plugin.Tags();

        if (tags.find(tag) == tags.end()) {
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    }
}


void Editor::OnQuit(wxCommandEvent& event) {
    if (event.GetId() == BUTTON_Apply) {

        //Apply any current edits.
        wxString currentPlugin = pluginText->GetLabelText();
        if (!currentPlugin.empty())
            ApplyEdits(currentPlugin);
        
        //Save edits to userlist.
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << YAML::BeginMap
             << YAML::Key << "plugins" << YAML::Value << _editedPlugins
             << YAML::EndMap;

        boss::ofstream out(_userlistPath.c_str());
        out << yout.c_str();
        out.close();
    }
    Close();
}

void Editor::ApplyEdits(const wxString& plugin) {
    boss::Plugin initial = GetMasterData(plugin);
    boss::Plugin edited = GetNewData(plugin);
    
    boss::Plugin diff = edited.DiffMetadata(initial);
    
    vector<boss::Plugin>::iterator it = std::find(_editedPlugins.begin(), _editedPlugins.end(), diff);

    if (it != _editedPlugins.end())
        *it = diff;
    else
        _editedPlugins.push_back(diff);
}

boss::Plugin Editor::GetMasterData(const wxString& plugin) const {
    boss::Plugin p;
    boss::Plugin p_in(string(plugin.ToUTF8()));

    vector<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), p_in);

    if (it != _basePlugins.end())
        p = *it;

    return p;
}

boss::Plugin Editor::GetUserData(const wxString& plugin) const {
    boss::Plugin p;
    boss::Plugin p_in(string(plugin.ToUTF8()));

    vector<boss::Plugin>::const_iterator it = std::find(_editedPlugins.begin(), _editedPlugins.end(), p_in);

    if (it != _editedPlugins.end())
        p = *it;

    return p;
}

boss::Plugin Editor::GetNewData(const wxString& plugin) const {
    boss::Plugin p(string(plugin.ToUTF8()));

    p.Priority(prioritySpin->GetValue());
    p.Enabled(enableUserEditsBox->IsChecked());

    set<boss::File> files;
    for (int i=0,max=reqsList->GetItemCount(); i < max; ++i) {
        files.insert(RowToFile(reqsList, i));
    }
    p.Reqs(files);
    files.clear();
    
    for (int i=0,max=incsList->GetItemCount(); i < max; ++i) {
        files.insert(RowToFile(incsList, i));
    }
    p.Incs(files);
    files.clear();
    
    for (int i=0,max=loadAfterList->GetItemCount(); i < max; ++i) {
        files.insert(RowToFile(loadAfterList, i));
    }
    p.LoadAfter(files);

    set<boss::Tag> tags;
    for (int i=0,max=tagsList->GetItemCount(); i < max; ++i) {
        tags.insert(RowToTag(tagsList, i));
    }
    p.Tags(tags);

    vector<boss::Message> vec = messageList->GetItems();
    list<boss::Message> messages(vec.begin(), vec.end());
    p.Messages(messages);

    return p;
}

boss::File Editor::RowToFile(wxListView * list, long row) const {
    return boss::File(
        string(list->GetItemText(row, 0).ToUTF8()),
        string(list->GetItemText(row, 1).ToUTF8()),
        string(list->GetItemText(row, 2).ToUTF8())
    );
}

boss::Tag Editor::RowToTag(wxListView * list, long row) const {
    string name = string(list->GetItemText(row, 1).ToUTF8());

    if (list->GetItemText(row, 0) == State[1])
        return boss::Tag(
            name,
            false,
            string(list->GetItemText(row, 2).ToUTF8())
        );
    else
        return boss::Tag(
            name,
            true,
            string(list->GetItemText(row, 2).ToUTF8())
        );
}

FileEditDialog::FileEditDialog(wxWindow *parent, const wxString& title) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {

    _name = new wxTextCtrl(this, wxID_ANY);
    _display = new wxTextCtrl(this, wxID_ANY);
    _condition = new wxTextCtrl(this, wxID_ANY);

    wxSizerFlags leftItem(0);
	leftItem.Left();

	wxSizerFlags rightItem(1);
	rightItem.Right().Expand();

    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer * GridSizer = new wxFlexGridSizer(2, 5, 5);
    GridSizer->AddGrowableCol(1,1);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Filename:")), leftItem);
	GridSizer->Add(_name, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Displayed Name:")), leftItem);
	GridSizer->Add(_display, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Condition:")), leftItem);
	GridSizer->Add(_condition, rightItem);

    bigBox->Add(GridSizer, 0, wxEXPAND|wxALL, 10);

    bigBox->AddSpacer(10);
    bigBox->AddStretchSpacer(1);

    //Need to add 'OK' and 'Cancel' buttons.
	wxSizer * sizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 15);

    SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));
	SetSizerAndFit(bigBox);
}

void FileEditDialog::SetValues(const wxString& name, const wxString& display, const wxString& condition) {

    _name->SetValue(name);
    _display->SetValue(display);
    _condition->SetValue(condition);
}

wxString FileEditDialog::GetName() const {
    return _name->GetValue();
}

wxString FileEditDialog::GetDisplayName() const {
    return _display->GetValue();
}

wxString FileEditDialog::GetCondition() const {
    return _condition->GetValue();
}

MessageEditDialog::MessageEditDialog(wxWindow *parent, const wxString& title) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {

    //Initialise controls.
    _type = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, Type);
    _language = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, Language);

    _condition = new wxTextCtrl(this, wxID_ANY);
    _str = new wxTextCtrl(this, wxID_ANY);

    _content = new wxListView(this, LIST_MessageContent, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);

    addBtn = new wxButton(this, BUTTON_AddContent, translate("Add Content"));
    editBtn = new wxButton(this, BUTTON_EditContent, translate("Edit Content"));
    removeBtn = new wxButton(this, BUTTON_RemoveContent, translate("Remove Content"));

    _content->InsertColumn(0, translate("Language"));
    _content->InsertColumn(1, translate("String"));

    //Set up event handling.
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MessageEditDialog::OnAdd, this, BUTTON_AddContent);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MessageEditDialog::OnEdit, this, BUTTON_EditContent);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MessageEditDialog::OnRemove, this, BUTTON_RemoveContent);
    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &MessageEditDialog::OnSelect, this, LIST_MessageContent);

    wxSizerFlags leftItem(0);
    leftItem.Left();
    
	wxSizerFlags rightItem(1);
    rightItem.Right();
    
    wxSizerFlags wholeItem(0);
    wholeItem.Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 10);
    
    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer * GridSizer = new wxFlexGridSizer(2, 5, 5);
    GridSizer->AddGrowableCol(1,1);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Type:")), leftItem);
	GridSizer->Add(_type, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Condition:")), leftItem);
	GridSizer->Add(_condition, rightItem);

    bigBox->Add(GridSizer, wholeItem);

    bigBox->Add(_content, wholeItem);

    wxFlexGridSizer * GridSizer2 = new wxFlexGridSizer(2, 5, 5);
    GridSizer2->AddGrowableCol(1,1);

	GridSizer2->Add(new wxStaticText(this, wxID_ANY, translate("Language:")), leftItem);
	GridSizer2->Add(_language, rightItem);

	GridSizer2->Add(new wxStaticText(this, wxID_ANY, translate("Content:")), leftItem);
	GridSizer2->Add(_str, rightItem);

    bigBox->Add(GridSizer2, wholeItem);

    wxBoxSizer * hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(addBtn, 0, wxRIGHT, 5);
    hbox->Add(editBtn, 0, wxLEFT|wxRIGHT, 5);
    hbox->Add(removeBtn, 0, wxLEFT, 5);
    bigBox->Add(hbox, 0, wxALIGN_RIGHT|wxRIGHT, 10);

    bigBox->AddSpacer(10);
    bigBox->AddStretchSpacer(1);

    //Need to add 'OK' and 'Cancel' buttons.
	wxSizer * sizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 15);

    //Set defaults.
    _type->SetSelection(0);

    SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));
	SetSizerAndFit(bigBox);
}

void MessageEditDialog::SetMessage(const boss::Message& message) {

    if (message.Type() == boss::g_message_say)
        _type->SetSelection(0);
    else if (message.Type() == boss::g_message_warn)
        _type->SetSelection(1);
    else
        _type->SetSelection(2);

    _condition->SetValue(FromUTF8(message.Condition()));

    vector<boss::MessageContent> contents = message.Content();
    for (size_t i=0, max=contents.size(); i < max; ++i) {
        _content->InsertItem(i, Language[ GetLangIndex(boss::GetLangString(contents[i].Language())) ]);
        _content->SetItem(i, 1, FromUTF8(contents[i].Str()));
    }
}

boss::Message MessageEditDialog::GetMessage() const {

    unsigned int type;
    string condition;
    if (_type->GetSelection() == 0)
        type = boss::g_message_say;
    else if (_type->GetSelection() == 1)
        type = boss::g_message_warn;
    else
        type = boss::g_message_error;

    condition = string(_condition->GetValue().ToUTF8());

    vector<boss::MessageContent> contents;
    for (size_t i=0, max=_content->GetItemCount(); i < max; ++i) {

        string str = string(_content->GetItemText(i, 1).ToUTF8());
        unsigned int lang = GetLangNum(_content->GetItemText(i, 0));
        
        contents.push_back(boss::MessageContent(str, lang));
    }
    
    return boss::Message(type, contents, condition);
}

void MessageEditDialog::OnSelect(wxListEvent& event) {
    _language->SetSelection(GetLangIndex(_content->GetItemText(event.GetIndex(), 0)));
    _str->SetValue(_content->GetItemText(event.GetIndex(), 1));
}

void MessageEditDialog::OnAdd(wxCommandEvent& event) {
    long i = _content->GetItemCount();
    _content->InsertItem(i, Language[_language->GetSelection()]);
    _content->SetItem(i, 1, _str->GetValue());
}

void MessageEditDialog::OnEdit(wxCommandEvent& event) {
    if (_content->GetFirstSelected() == -1) {
        wxMessageBox(
            FromUTF8("Error: No content row selected."),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return;
    }
    long i = _content->GetFirstSelected();
    _content->SetItem(i, 0, Language[_language->GetSelection()]);
    _content->SetItem(i, 1, _str->GetValue());
}

void MessageEditDialog::OnRemove(wxCommandEvent& event) {
    if (_content->GetFirstSelected() == -1) {
        wxMessageBox(
            FromUTF8("Error: No content row selected."),
            translate("BOSS: Error"),
            wxOK | wxICON_ERROR,
            NULL);
        return;
    }
    _content->DeleteItem(_content->GetFirstSelected());
}

TagEditDialog::TagEditDialog(wxWindow *parent, const wxString& title) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {

    //Initialise controls.
    _state = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, State);
    
    _name = new wxTextCtrl(this, wxID_ANY);
    _condition = new wxTextCtrl(this, wxID_ANY);

    wxSizerFlags leftItem(0);
	leftItem.Left();

	wxSizerFlags rightItem(1);
	rightItem.Right().Expand();

    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer * GridSizer = new wxFlexGridSizer(2, 5, 5);
    GridSizer->AddGrowableCol(1,1);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Add/Remove:")), leftItem);
	GridSizer->Add(_state, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Name:")), leftItem);
	GridSizer->Add(_name, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Condition:")), leftItem);
	GridSizer->Add(_condition, rightItem);

    bigBox->Add(GridSizer, 0, wxEXPAND|wxALL, 10);

    bigBox->AddSpacer(10);
    bigBox->AddStretchSpacer(1);

    //Need to add 'OK' and 'Cancel' buttons.
	wxSizer * sizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 15);

    //Set defaults.
    _state->SetSelection(0);

    SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));
	SetSizerAndFit(bigBox);
}

void TagEditDialog::SetValues(int state, const wxString& name, const wxString& condition) {

    _state->SetSelection(state);
    _name->SetValue(name);
    _condition->SetValue(condition);
}

wxString TagEditDialog::GetState() const {
    return State[_state->GetSelection()];
}

wxString TagEditDialog::GetName() const {
    return _name->GetValue();
}

wxString TagEditDialog::GetCondition() const {
    return _condition->GetValue();
}
