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

#include "sorting.h"
#include "../backend/helpers.h"

using namespace std;

MiniEditor::MiniEditor(wxWindow *parent, const wxString& title, const std::list<boss::Plugin>& plugins, const boss::Game& game) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), _basePlugins(plugins) {
    //Initialise controls.
    pluginText = new wxStaticText(this, wxID_ANY, "");
    prioritySpin = new wxSpinCtrl(this, wxID_ANY, "0");
    prioritySpin->SetRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    filterCheckbox = new wxCheckBox(this, wxID_ANY, translate("Show only conflicting plugins."));

    pluginList = new wxListView(this, LIST_Plugins, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    loadAfterList = new wxListView(this, LIST_LoadAfter, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);

    removeBtn = new wxButton(this, BUTTON_RemoveRow, translate("Remove"));

    wxMenu * pluginMenu = new wxMenu();

    //Set up list columns.
    pluginList->AppendColumn(translate("Plugin"));
    pluginList->AppendColumn(translate("Priority"));
    loadAfterList->AppendColumn(translate("Filename"));

    //Set up plugin right-click menu.
    pluginMenu->Append(MENU_CopyName, translate("Copy Into Load After List"));

    //Initialise control states.
    removeBtn->Enable(false);
    prioritySpin->Enable(false);
    filterCheckbox->Enable(false);

    //Make plugin name bold text.
    pluginText->SetFont(pluginText->GetFont().Bold());
    
    //Set up layout.
    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer * hBox = new wxBoxSizer(wxHORIZONTAL);

    hBox->Add(pluginList, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer * mainBox = new wxBoxSizer(wxVERTICAL);

    mainBox->Add(pluginText, 0, wxTOP | wxBOTTOM, 10);
    mainBox->Add(filterCheckbox, 0, wxTOP | wxBOTTOM, 10);

    wxBoxSizer * hbox2 = new wxBoxSizer(wxHORIZONTAL); 
    hbox2->Add(new wxStaticText(this, wxID_ANY, translate("Priority: ")), 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT, 5);
    hbox2->Add(prioritySpin, 0, wxALIGN_RIGHT);
    mainBox->Add(hbox2, 0, wxEXPAND | wxALIGN_RIGHT | wxTOP | wxBOTTOM, 5);

    mainBox->Add(loadAfterList, 1, wxEXPAND);
    mainBox->Add(removeBtn, 0, wxTOP | wxBOTTOM | wxALIGN_RIGHT, 10);

    hBox->Add(mainBox, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 10);
    bigBox->Add(hBox, 1, wxEXPAND);

    //Need to add 'Yes' and 'No' buttons.
    wxSizer * sizer = CreateSeparatedButtonSizer(wxAPPLY | wxCANCEL);

    //Now add buttons to window sizer.
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, 15);

    //Fill pluginList with the contents of basePlugins.
    int i = 0;
    for (list<boss::Plugin>::const_iterator it = _basePlugins.begin(); it != _basePlugins.end(); ++it) {
        pluginList->InsertItem(i, FromUTF8(it->Name()));
        pluginList->SetItem(i, 1, FromUTF8(boss::IntToString(it->Priority())));
        if (it->FormIDs().empty()) {
            pluginList->SetItemTextColour(i, wxColour(122, 122, 122));
        }
        else if (it->LoadsBSA(game)) {
            pluginList->SetItemTextColour(i, wxColour(0, 142, 219));
        }
        ++i;
    }
    pluginList->SetColumnWidth(0, wxLIST_AUTOSIZE);

    SetBackgroundColour(wxColour(255, 255, 255));
    SetIcon(wxIconLocation("BOSS.exe"));

    SetSizerAndFit(bigBox);
    Layout();
}

void MiniEditor::OnPluginSelect(wxListEvent& event) {

}

void MiniEditor::OnPluginListRightClick(wxListEvent& event) {

}

void MiniEditor::OnPluginCopy(wxCommandEvent& event) {

}

void MiniEditor::OnFilterToggle(wxCommandEvent& event) {

}

void MiniEditor::OnQuit(wxCommandEvent& event) {

}


LoadOrderPreview::LoadOrderPreview(wxWindow *parent, const wxString title, const std::list<boss::Plugin>& plugins, const boss::Game& game) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

    //Init controls.
    _loadOrder = new wxListView(this, LIST_LoadOrder, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

    //Populate list.
    _loadOrder->AppendColumn(translate("Load Order"));
    size_t i = 0;
    for (list<boss::Plugin>::const_iterator it = plugins.begin(), endit = plugins.end(); it != endit; ++it, ++i) {
        _loadOrder->InsertItem(i, FromUTF8(it->Name()));
        if (it->FormIDs().empty()) {
            _loadOrder->SetItemTextColour(i, wxColour(122, 122, 122));
        }
        else if (it->LoadsBSA(game)) {
            _loadOrder->SetItemTextColour(i, wxColour(0, 142, 219));
        }
    }
    _loadOrder->SetColumnWidth(0, wxLIST_AUTOSIZE);

    //Set up layout.
    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

    bigBox->Add(_loadOrder, 1, wxEXPAND | wxALL, 15);

    bigBox->Add(new wxStaticText(this, wxID_ANY, translate("Do you wish to make any changes to the load order above?")), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 15);

    //Need to add 'Yes' and 'No' buttons.
    wxSizer * sizer = CreateSeparatedButtonSizer(wxYES | wxNO | wxCANCEL);

    //Now add buttons to window sizer.
    if (sizer != NULL)
        bigBox->Add(sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, 15);

    //Now set the layout and sizes.
    SetBackgroundColour(wxColour(255, 255, 255));
    SetIcon(wxIconLocation("BOSS.exe"));
    SetSizerAndFit(bigBox);
}
