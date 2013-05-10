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
    EVT_LIST_ITEM_SELECTED( LIST_Reqs, Editor::OnRowSelect )
    EVT_LIST_ITEM_SELECTED( LIST_Incs, Editor::OnRowSelect )
    EVT_LIST_ITEM_SELECTED( LIST_LoadAfter, Editor::OnRowSelect )
    EVT_LIST_ITEM_SELECTED( LIST_Messages, Editor::OnRowSelect )
    EVT_LIST_ITEM_SELECTED( LIST_BashTags, Editor::OnRowSelect )
    EVT_SPINCTRL( SPIN_Priority, Editor::OnPriorityChange )
    EVT_NOTEBOOK_PAGE_CHANGED( BOOK_Lists, Editor::OnListBookChange )
	EVT_BUTTON ( BUTTON_Apply, Editor::OnQuit )
	EVT_BUTTON ( BUTTON_Cancel, Editor::OnQuit )
    EVT_BUTTON ( BUTTON_AddRow, Editor::OnAddRow )
    EVT_BUTTON ( BUTTON_EditRow, Editor::OnEditRow )
    EVT_BUTTON ( BUTTON_RemoveRow, Editor::OnRemoveRow )
    EVT_BUTTON ( BUTTON_Recalc, Editor::OnRecalc )
    EVT_CHECKBOX ( wxID_ANY, Editor::OnEnabledToggle )
END_EVENT_TABLE()

using namespace std;

Editor::Editor(wxWindow *parent, const wxString& title) : wxFrame(parent, wxID_ANY, title) {

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
    
    addBtn = new wxButton(this, BUTTON_AddRow, translate("Add File"));
    editBtn = new wxButton(this, BUTTON_EditRow, translate("Edit File"));
    removeBtn = new wxButton(this, BUTTON_RemoveRow, translate("Remove File"));
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

    //Initialise control states.
    addBtn->Enable(false);
    editBtn->Enable(false);
    removeBtn->Enable(false);
    prioritySpin->Enable(false);
    enableUserEditsBox->Enable(false);
    
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
    SetIcon(wxIconLocation("BOSS.exe"));

    SetSizerAndFit(bigBox);
}

void Editor::SetList(const std::vector<boss::Plugin>& basePlugins, const std::vector<boss::Plugin>& editedPlugins) {
    _basePlugins = basePlugins;
    _editedPlugins = editedPlugins;
    
    //Fill pluginList with the contents of basePlugins.
    for (int i=0, max=_basePlugins.size(); i < max; ++i) {
        pluginList->InsertItem(i, FromUTF8(_basePlugins[i].Name()));
    }
    pluginList->SetColumnWidth(0, wxLIST_AUTOSIZE);
    Layout();
}

void Editor::IsSorted(bool sorted) {
    if (sorted) {
        applyBtn->SetLabel(translate("Apply Load Order"));
    } else {
        recalcBtn->Show(false);
        applyBtn->SetLabel(translate("Save Changes"));
    }
    Layout();
}

void Editor::OnPluginSelect(wxListEvent& event) {
    //Create Plugin object for selected plugin.
    boss::Plugin plugin = boss::Plugin(string(pluginList->GetItemText(event.GetIndex()).ToUTF8()));
    
    //Check if the selected plugin is the same as the current plugin.
    if (currentPlugin != plugin) {
        //Check if there are edits made to the current plugin compared to its original.
        if (IsCurrentPluginEdited())
            ApplyCurrentPluginEdits();

        plugin = GetOriginal(plugin, true);

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
        i=0;
        for (list<boss::Message>::const_iterator it=messages.begin(), endit=messages.end(); it != endit; ++it) {
            messageList->InsertItem(i, FromUTF8(it->Type()));
            messageList->SetItem(i, 1, FromUTF8(it->Content()));
            messageList->SetItem(i, 2, FromUTF8(it->Condition()));
            messageList->SetItem(i, 2, FromUTF8(it->Language()));
            ++i;
        }

        set<boss::Tag> tags = plugin.Tags();
        i=0;
        for (set<boss::Tag>::const_iterator it=tags.begin(), endit=tags.end(); it != endit; ++it) {
            if (it->IsAddition())
                tagsList->InsertItem(i, translate("Add"));
            else
                tagsList->InsertItem(i, translate("Remove"));
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

        currentPlugin = plugin;
    }
}

void Editor::OnPriorityChange(wxSpinEvent& event) {
    currentPlugin.Priority(event.GetPosition());
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
    Layout();
}

void Editor::OnEnabledToggle(wxCommandEvent& event) {
    currentPlugin.Enabled(event.IsChecked());
}

void Editor::OnAddRow(wxCommandEvent& event) {
    if (listBook->GetSelection() < 3) {
        FileEditDialog * rowDialog = new FileEditDialog(this, translate("BOSS: Add File/Plugin"));
        rowDialog->ShowModal();
    } else if (listBook->GetSelection() == 3) {
        MessageEditDialog * rowDialog = new MessageEditDialog(this, translate("BOSS: Add Message"));
        rowDialog->ShowModal();
    } else if (listBook->GetSelection() == 4) {
        TagEditDialog * rowDialog = new TagEditDialog(this, translate("BOSS: Add Tag"));
        rowDialog->ShowModal();
    }
}

void Editor::OnEditRow(wxCommandEvent& event) {
    if (listBook->GetSelection() < 3) {
        FileEditDialog * rowDialog = new FileEditDialog(this, translate("BOSS: Edit File/Plugin"));

        
        
        rowDialog->ShowModal();
    } else if (listBook->GetSelection() == 3) {
        MessageEditDialog * rowDialog = new MessageEditDialog(this, translate("BOSS: Edit Message"));
        rowDialog->ShowModal();
    } else if (listBook->GetSelection() == 4) {
        TagEditDialog * rowDialog = new TagEditDialog(this, translate("BOSS: Edit Tag"));
        rowDialog->ShowModal();
    }
}

void Editor::OnRemoveRow(wxCommandEvent& event) {

}

void Editor::OnRecalc(wxCommandEvent& event) {

}

void Editor::OnRowSelect(wxListEvent& event) {
    //Need to check if this row was added by the masterlist or the userlist. If the latter, enable the edit and remove buttons.
}


void Editor::OnQuit(wxCommandEvent& event) {
    if (event.GetId() == BUTTON_Apply) {
        //Save edits to userlist.
  /*      YAML::Emitter yout;
        yout.SetIndent(2);
        yout << YAML::BeginMap
             << YAML::Key << "plugins" << YAML::Value << _editedPlugins
             << YAML::EndMap;

        ofstream out(game.UserlistPath().string().c_str());
        out << yout.c_str();
        out.close();
    */    
        if (recalcBtn->IsShown()) {
            //Signal that the load order should be written.
        }
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

void Editor::ApplyCurrentPluginEdits() {
    boss::Plugin diff = currentPlugin.DiffMetadata(GetOriginal(currentPlugin, false));
    
    vector<boss::Plugin>::iterator it = std::find(_editedPlugins.begin(), _editedPlugins.end(), diff);

    if (it != _editedPlugins.end())
        *it = diff;
    else
        _editedPlugins.push_back(diff);
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

void FileEditDialog::SetValues(const std::string& name, const std::string& display, const std::string& condition) {

}

std::vector<std::string> FileEditDialog::GetValues() const {
    vector<string> values;

    return values;
}

MessageEditDialog::MessageEditDialog(wxWindow *parent, const wxString& title) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {

    wxString Language[] = {
        wxT("None Specified"),
		wxT("English"),
	/*	wxString::FromUTF8("Español"),
		wxT("Deutsch"),
		wxString::FromUTF8("Русский"),
		wxString::FromUTF8("简体中文")*/
	};

    wxString Type[] = {
        wxT("Note"),
        wxT("Warning"),
        wxT("Error")
    };

    //Initialise controls.
    _type = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, Type);
    _language = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, Language);

    _content = new wxTextCtrl(this, wxID_ANY);
    _condition = new wxTextCtrl(this, wxID_ANY);

    wxSizerFlags leftItem(0);
	leftItem.Left();

	wxSizerFlags rightItem(1);
	rightItem.Right().Expand();

    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer * GridSizer = new wxFlexGridSizer(2, 5, 5);
    GridSizer->AddGrowableCol(1,1);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Type:")), leftItem);
	GridSizer->Add(_type, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Content:")), leftItem);
	GridSizer->Add(_content, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Condition:")), leftItem);
	GridSizer->Add(_condition, rightItem);

	GridSizer->Add(new wxStaticText(this, wxID_ANY, translate("Language:")), leftItem);
	GridSizer->Add(_language, rightItem);

    bigBox->Add(GridSizer, 0, wxEXPAND|wxALL, 10);

    bigBox->AddSpacer(10);
    bigBox->AddStretchSpacer(1);

    //Need to add 'OK' and 'Cancel' buttons.
	wxSizer * sizer = CreateSeparatedButtonSizer(wxOK|wxCANCEL);
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND|wxLEFT|wxBOTTOM|wxRIGHT, 15);

    //Set defaults.
    _type->SetSelection(0);
    _language->SetSelection(0);

    SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));
	SetSizerAndFit(bigBox);

}

void MessageEditDialog::SetValues(const std::string& type, const std::string& content, const std::string& condition, const std::string& language) {

}

std::vector<std::string> MessageEditDialog::GetValues() const {
    vector<string> values;

    return values;
}

TagEditDialog::TagEditDialog(wxWindow *parent, const wxString& title) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {

    wxString State[] = {
        wxT("Add"),
        wxT("Remove")
    };

    //Initialise controls.
    _addRemove = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, State);
    
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
	GridSizer->Add(_addRemove, rightItem);

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
    _addRemove->SetSelection(0);

    SetBackgroundColour(wxColour(255,255,255));
    SetIcon(wxIconLocation("BOSS.exe"));
	SetSizerAndFit(bigBox);
}

void TagEditDialog::SetValues(const std::string& addRemove, const std::string& name, const std::string& condition) {

}

std::vector<std::string> TagEditDialog::GetValues() const {
    vector<string> values;

    return values;
}
