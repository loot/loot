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
END_EVENT_TABLE()

using namespace std;

Editor::Editor(const wxString title, wxFrame *parent) : wxFrame(parent, wxID_ANY, title) {

    //Initialise child windows.
    filesBook = new wxNotebook(this, wxID_ANY);
    messagesBook = new wxNotebook(this, wxID_ANY);

    wxPanel * reqsTab = new wxPanel(filesBook);
    wxPanel * incsTab = new wxPanel(filesBook);
    wxPanel * loadAfterTab = new wxPanel(filesBook);
    wxPanel * messagesTab = new wxPanel(messagesBook);
    wxPanel * tagsTab = new wxPanel(messagesBook);

    //Tie together notebooks and panels.
    filesBook->AddPage(reqsTab, translate("Requirements"), true);
    filesBook->AddPage(incsTab, translate("Incompatibilities"));
    filesBook->AddPage(loadAfterTab, translate("Load After"));
    
    messagesBook->AddPage(messagesTab, translate("Messages"), true);
    messagesBook->AddPage(tagsTab, translate("Bash Tag Suggestions"));

    //Initialise controls.
    pluginText = new wxStaticText(this, wxID_ANY, "");
    prioritySpin = new wxSpinCtrl(this, SPIN_Priority, "0");
    prioritySpin->SetRange(-10,10);
    enableUserEditsBox = new wxCheckBox(this, wxID_ANY, translate("Enable User Changes"));
    
    addFileBtn = new wxButton(this, BUTTON_AddFile, translate("Add File"));
    editFileBtn = new wxButton(this, BUTTON_EditFile, translate("Edit File"));
    removeFileBtn = new wxButton(this, BUTTON_RemoveFile, translate("Remove File"));
    addMsgBtn = new wxButton(this, BUTTON_AddMessage, translate("Add Message"));
    editMsgBtn = new wxButton(this, BUTTON_EditMessage, translate("Edit Message"));
    removeMsgBtn = new wxButton(this, BUTTON_RemoveMessage, translate("Remove Message"));
    saveEditsBtn = new wxButton(this, BUTTON_SaveEdits, translate("Save Edits"));
    undoEditsBtn = new wxButton(this, BUTTON_UndoEdits, translate("Undo Edits"));
    recalcBtn = new wxButton(this, BUTTON_Recalc, translate("Recalculate Load Order"));
    applyBtn = new wxButton(this, BUTTON_Apply, translate("Apply Load Order"));
    cancelBtn = new wxButton(this, BUTTON_Cancel, translate("Cancel"));

    pluginList = new wxListCtrl(this, LIST_Plugins, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    reqsList = new wxListCtrl(reqsTab, LIST_Reqs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    incsList = new wxListCtrl(incsTab, LIST_Incs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    loadAfterList = new wxListCtrl(loadAfterTab, LIST_LoadAfter, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    messageList = new wxListCtrl(messagesTab, LIST_Messages, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    tagsList = new wxListCtrl(tagsTab, LIST_BashTags, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);

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

    wxBoxSizer *vbox1 = new wxBoxSizer(wxVERTICAL);
    vbox1->Add(new wxStaticText(this, wxID_ANY, translate("Plugins")), 0, wxALL, 5);
    vbox1->Add(pluginList, 1, wxEXPAND|wxALL, 5);

    bigBox->Add(vbox1, 1, wxEXPAND|wxALL, 5);

    wxBoxSizer * mainBox = new wxBoxSizer(wxVERTICAL);

    mainBox->Add(pluginText, 0, wxALL, 5);

    wxBoxSizer * hbox1 = new wxBoxSizer(wxHORIZONTAL);

    hbox1->Add(new wxStaticText(this, wxID_ANY, translate("Priority: ")), 0, wxALL, 5);
    hbox1->Add(prioritySpin, 0, wxALL, 5);

    mainBox->Add(hbox1);

    wxBoxSizer * tabBox1 = new wxBoxSizer(wxVERTICAL);
    tabBox1->Add(reqsList, 1, wxEXPAND);
    reqsTab->SetSizer(tabBox1);

    wxBoxSizer * tabBox2 = new wxBoxSizer(wxVERTICAL);
    tabBox2->Add(incsList, 1, wxEXPAND);
    incsTab->SetSizer(tabBox2);
    
    wxBoxSizer * tabBox3 = new wxBoxSizer(wxVERTICAL);
    tabBox3->Add(loadAfterList, 1, wxEXPAND);
    loadAfterTab->SetSizer(tabBox3);

    mainBox->Add(filesBook, 1, wxEXPAND);

    wxBoxSizer * hbox2 = new wxBoxSizer(wxHORIZONTAL);
    hbox2->Add(addFileBtn, 0, wxALL, 10);
    hbox2->Add(editFileBtn, 0, wxALL, 10);
    hbox2->Add(removeFileBtn, 0, wxALL, 10);
    mainBox->Add(hbox2);
   
    wxBoxSizer * tabBox4 = new wxBoxSizer(wxVERTICAL);
    tabBox4->Add(messageList, 0, wxEXPAND);
    messagesTab->SetSizer(tabBox4);

    wxBoxSizer * tabBox5 = new wxBoxSizer(wxVERTICAL);
    tabBox5->Add(tagsList, 0, wxEXPAND);
    tagsTab->SetSizer(tabBox5);

    mainBox->Add(messagesBook, 1, wxEXPAND);

    wxBoxSizer * hbox3 = new wxBoxSizer(wxHORIZONTAL);
    hbox3->Add(addMsgBtn, 0, wxTOP|wxLEFT|wxRIGHT, 10);
    hbox3->Add(editMsgBtn, 0, wxTOP|wxLEFT|wxRIGHT, 10);
    hbox3->Add(removeMsgBtn, 0, wxTOP|wxLEFT|wxRIGHT, 10);
    mainBox->Add(hbox3);

    mainBox->AddSpacer(20);

    wxBoxSizer * hbox5 = new wxBoxSizer(wxHORIZONTAL);
    hbox5->Add(enableUserEditsBox, 0, wxALL, 5);
    hbox5->Add(saveEditsBtn, 0, wxALL, 5);
    hbox5->Add(undoEditsBtn, 0, wxALL, 5);
    mainBox->Add(hbox5, 0, wxBOTTOM, 20);

    sortingButtons = new wxBoxSizer(wxHORIZONTAL);
    sortingButtons->Add(recalcBtn, 0, wxALL, 5);
    sortingButtons->Add(applyBtn, 0, wxALL, 5);
    sortingButtons->Add(cancelBtn, 0, wxALL, 5);
    mainBox->Add(sortingButtons);

    bigBox->Add(mainBox, 2, wxEXPAND|wxALL, 5);
    
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
    sortingButtons->ShowItems(sorted);
    Layout();
}

void Editor::OnPluginSelect(wxListEvent& event) {
    //Create Plugin object for selected plugin.
    boss::Plugin plugin = boss::Plugin(string(pluginList->GetItemText(event.GetIndex()).ToUTF8()));
    
    //Check if the selected plugin is the same as the current plugin.
    if (currentPlugin != plugin) {
        //Check if there are edits made to the current plugin compared to its original.
        if (IsCurrentPluginEdited()) {
            //Ask if changes should be saved.
            wxMessageDialog *dlg = new wxMessageDialog(this,
                translate("The current edits are not saved. Do you want to save them?"), 
                translate("BOSS: Metadata Editor"), wxYES_NO);

            if (dlg->ShowModal() == wxID_YES) {
                //Save changes.
                boss::Plugin diff = currentPlugin.DiffMetadata(GetOriginal(currentPlugin, false));
                
                vector<boss::Plugin>::iterator it = std::find(_editedPlugins.begin(), _editedPlugins.end(), diff);

                if (it != _editedPlugins.end())
                    *it = diff;
                else
                    _editedPlugins.push_back(diff);
            }
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
