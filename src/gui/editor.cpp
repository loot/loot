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

#include <wx/statline.h>
#include <algorithm>

BEGIN_EVENT_TABLE( Editor, wxFrame )
    EVT_LIST_ITEM_SELECTED( LIST_Plugins, Editor::OnPluginSelect )
    EVT_SPINCTRL( SPIN_Priority, Editor::OnPriorityChange )
    EVT_NOTEBOOK_PAGE_CHANGED( BOOK_Lists, Editor::OnListBookChange )
	EVT_BUTTON ( BUTTON_Apply, Editor::OnQuit )
	EVT_BUTTON ( BUTTON_Cancel, Editor::OnQuit )
    EVT_CHECKBOX ( wxID_ANY, Editor::OnEnabledToggle )
END_EVENT_TABLE()

using namespace std;

Editor::Editor(const wxString title, wxFrame *parent) : wxFrame(parent, wxID_ANY, title) {

    //Initialise child windows.
    listBook = new wxNotebook(this, BOOK_Lists);

    wxPanel * reqsTab = new wxPanel(listBook);
    wxPanel * incsTab = new wxPanel(listBook);
    wxPanel * loadAfterTab = new wxPanel(listBook);
    wxPanel * messagesTab = new wxPanel(listBook);
    wxPanel * tagsTab = new wxPanel(listBook);

    //Initialise controls.
    pluginText = new wxStaticText(this, wxID_ANY, "");
    prioritySpin = new wxSpinCtrl(this, SPIN_Priority, "0");
    prioritySpin->SetRange(-10,10);
    enableUserEditsBox = new wxCheckBox(this, wxID_ANY, translate("Enable User Changes"));
    
    addBtn = new wxButton(this, BUTTON_AddFile, translate("Add File"));
    editBtn = new wxButton(this, BUTTON_EditFile, translate("Edit File"));
    removeBtn = new wxButton(this, BUTTON_RemoveFile, translate("Remove File"));
    recalcBtn = new wxButton(this, BUTTON_Recalc, translate("Recalculate Load Order"));
    applyBtn = new wxButton(this, BUTTON_Apply, translate("Apply Load Order"));
    cancelBtn = new wxButton(this, BUTTON_Cancel, translate("Cancel"));

    pluginList = new wxListCtrl(this, LIST_Plugins, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    reqsList = new wxListCtrl(reqsTab, LIST_Reqs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    incsList = new wxListCtrl(incsTab, LIST_Incs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    loadAfterList = new wxListCtrl(loadAfterTab, LIST_LoadAfter, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    messageList = new wxListCtrl(messagesTab, LIST_Messages, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    tagsList = new wxListCtrl(tagsTab, LIST_BashTags, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);

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

    messageList->AppendColumn(translate("Type"));
    messageList->AppendColumn(translate("Content"));
    messageList->AppendColumn(translate("Condition"));
    messageList->AppendColumn(translate("Language"));

    tagsList->AppendColumn(translate("Add/Remove"));
    tagsList->AppendColumn(translate("Bash Tag"));
    tagsList->AppendColumn(translate("Condition"));
    
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
    hbox6->Add(recalcBtn, 0, wxRIGHT, 5);
    hbox6->Add(applyBtn, 0, wxLEFT|wxRIGHT, 5);
    hbox6->Add(cancelBtn, 0, wxLEFT, 5);
    mainBox->Add(hbox6, 0, wxALIGN_RIGHT);

    bigBox->Add(mainBox, 2, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 10);
    
    SetBackgroundColour(wxColour(255,255,255));

    SetSizerAndFit(bigBox);
}

void Editor::SetList(const std::vector<boss::Plugin>& basePlugins, const std::vector<boss::Plugin>& editedPlugins) {
    _basePlugins = basePlugins;
    _editedPlugins = editedPlugins;
    
    //Fill pluginList with the contents of basePlugins.
    for (int i=0, max=_basePlugins.size(); i < max; ++i) {
        pluginList->InsertItem(i, _basePlugins[i].Name());
    }
    pluginList->SetColumnWidth(0, wxLIST_AUTOSIZE);
    Layout();
}

void Editor::IsSorted(bool sorted) {
    if (sorted) {
        applyBtn->SetLabel("Apply Load Order");
    } else {
        recalcBtn->Show(false);
        applyBtn->SetLabel("Save Changes");
    }
    Layout();
}

void Editor::OnPluginSelect(wxListEvent& event) {
    //Create Plugin object for selected plugin.
    boss::Plugin plugin = boss::Plugin(string(pluginList->GetItemText(event.GetIndex()).ToUTF8()));
    
    //Check if the selected plugin is the same as the current plugin.
    if (currentPlugin != plugin) {
        //Check if there are edits made to the current plugin compared to its original.
        if (IsCurrentPluginEdited()) {
            //Save changes.
            boss::Plugin diff = currentPlugin.DiffMetadata(GetOriginal(currentPlugin, false));
            
            vector<boss::Plugin>::iterator it = std::find(_editedPlugins.begin(), _editedPlugins.end(), diff);

            if (it != _editedPlugins.end())
                *it = diff;
            else
                _editedPlugins.push_back(diff);
        }

        plugin = GetOriginal(plugin, true);

        //Now fill editor fields with new plugin's info.
        pluginText->SetLabelText(wxString(plugin.Name().c_str(), wxConvUTF8));

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
            loadAfterList->InsertItem(i, wxString(it->Name().c_str(), wxConvUTF8));
            loadAfterList->SetItem(i, 1, wxString(it->DisplayName().c_str(), wxConvUTF8));
            loadAfterList->SetItem(i, 2, wxString(it->Condition().c_str(), wxConvUTF8));
            ++i;
        }

        files = plugin.Reqs();
        i=0;
        for (set<boss::File>::const_iterator it=files.begin(), endit=files.end(); it != endit; ++it) {
            reqsList->InsertItem(i, wxString(it->Name().c_str(), wxConvUTF8));
            reqsList->SetItem(i, 1, wxString(it->DisplayName().c_str(), wxConvUTF8));
            reqsList->SetItem(i, 2, wxString(it->Condition().c_str(), wxConvUTF8));
            ++i;
        }

        files = plugin.Incs();
        i=0;
        for (set<boss::File>::const_iterator it=files.begin(), endit=files.end(); it != endit; ++it) {
            incsList->InsertItem(i, wxString(it->Name().c_str(), wxConvUTF8));
            incsList->SetItem(i, 1, wxString(it->DisplayName().c_str(), wxConvUTF8));
            incsList->SetItem(i, 2, wxString(it->Condition().c_str(), wxConvUTF8));
            ++i;
        }

        list<boss::Message> messages = plugin.Messages();
        i=0;
        for (list<boss::Message>::const_iterator it=messages.begin(), endit=messages.end(); it != endit; ++it) {
            messageList->InsertItem(i, wxString(it->Type().c_str(), wxConvUTF8));
            messageList->SetItem(i, 1, wxString(it->Content().c_str(), wxConvUTF8));
            messageList->SetItem(i, 2, wxString(it->Condition().c_str(), wxConvUTF8));
            messageList->SetItem(i, 2, wxString(it->Language().c_str(), wxConvUTF8));
            ++i;
        }

        set<boss::Tag> tags = plugin.Tags();
        i=0;
        for (set<boss::Tag>::const_iterator it=tags.begin(), endit=tags.end(); it != endit; ++it) {
            if (it->IsAddition())
                tagsList->InsertItem(i, translate("Add"));
            else
                tagsList->InsertItem(i, translate("Remove"));
            tagsList->SetItem(i, 1, wxString(it->Name().c_str(), wxConvUTF8));
            tagsList->SetItem(i, 2, wxString(it->Condition().c_str(), wxConvUTF8));
            ++i;
        }

        currentPlugin = plugin;
    }
}

void Editor::OnPriorityChange(wxSpinEvent& event) {
    currentPlugin.Priority(event.GetPosition());
}

void Editor::OnListBookChange(wxBookCtrlEvent& event) {
    if (event.GetSelection() == 0 || event.GetSelection() == 1) {
        addBtn->SetLabel("Add File");
        editBtn->SetLabel("Edit File");
        removeBtn->SetLabel("Remove File");
    } else if (event.GetSelection() == 2) {
        addBtn->SetLabel("Add Plugin");
        editBtn->SetLabel("Edit Plugin");
        removeBtn->SetLabel("Remove Plugin");
    } else if (event.GetSelection() == 3) {
        addBtn->SetLabel("Add Message");
        editBtn->SetLabel("Edit Message");
        removeBtn->SetLabel("Remove Message");
    } else if (event.GetSelection() == 4) {
        addBtn->SetLabel("Add Bash Tag");
        editBtn->SetLabel("Edit Bash Tag");
        removeBtn->SetLabel("Remove Bash Tag");
    }
    Layout();
}

void Editor::OnEnabledToggle(wxCommandEvent& event) {
    currentPlugin.Enabled(event.IsChecked());
}

void Editor::OnQuit(wxCommandEvent& event) {
    if (event.GetId() == BUTTON_Apply) {
        if (recalcBtn->IsShown()) {
            //Signal that the load order should be written.
        }
        //Save edits to userlist.
    }
    Close();
}

bool Editor::IsCurrentPluginEdited() const {
    boss::Plugin original = GetOriginal(currentPlugin, true);

    return (original.Priority() != currentPlugin.Priority()
         || original.Enabled() != currentPlugin.Enabled()
         || original.LoadAfter() != currentPlugin.LoadAfter()
         || original.Reqs() != currentPlugin.Reqs()
         || original.Incs() != currentPlugin.Incs()
         || original.Messages() != currentPlugin.Messages()
         || original.Tags() != currentPlugin.Tags());
}

boss::Plugin Editor::GetOriginal(const boss::Plugin& plugin, bool withEdits) const {
    //Merges together the base and (saved) edited plugin info.
    boss::Plugin p;
    vector<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

    if (it != _basePlugins.end())
        p = *it;

    if (withEdits) {
        it = std::find(_editedPlugins.begin(), _editedPlugins.end(), plugin);

        if (it != _editedPlugins.end())
            p.Merge(*it, true);
    }

    return p;
}
