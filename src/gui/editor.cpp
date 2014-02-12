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

#include "editor.h"
#include "../backend/generators.h"
#include "../backend/streams.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <wx/clipbrd.h>
#include <wx/msgdlg.h>

using namespace std;

//////////////////////////////
// TextDropTarget class
//////////////////////////////

TextDropTarget::TextDropTarget(wxListView * owner, wxStaticText * name) : targetOwner(owner), targetName(name) {}

bool TextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString &data) {
    if (data == targetName->GetLabelText() || targetOwner->FindItem(-1, data) != wxNOT_FOUND)
        return false;
    targetOwner->InsertItem(targetOwner->GetItemCount(), data);
    targetOwner->SetColumnWidth(0, wxLIST_AUTOSIZE);
    return true;
}

///////////////////////////////////
// Common Editor Class
///////////////////////////////////

CommonEditor::CommonEditor(const std::list<boss::Plugin>& plugins, const boss::Game& game) : _basePlugins(plugins), _game(game) {}

CommonEditor::CommonEditor(const std::list<boss::Plugin>& plugins, const boss::Game& game, std::list<boss::Plugin>& editedPlugins) : _basePlugins(plugins), _game(game), _editedPlugins(editedPlugins) {}

boss::Plugin CommonEditor::GetMasterData(const wxString& plugin) const {
    BOOST_LOG_TRIVIAL(debug) << "Getting hardcoded and masterlist metadata for plugin: " << plugin.ToUTF8();
    boss::Plugin p;
    boss::Plugin p_in(string(plugin.ToUTF8()));

    list<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), p_in);

    if (it != _basePlugins.end())
        p = *it;

    return p;
}

boss::Plugin CommonEditor::GetUserData(const wxString& plugin) const {
    BOOST_LOG_TRIVIAL(debug) << "Getting userlist metadata for plugin: " << plugin.ToUTF8();
    boss::Plugin p(string(plugin.ToUTF8()));

    list<boss::Plugin>::const_iterator it = std::find(_editedPlugins.begin(), _editedPlugins.end(), p);

    if (it != _editedPlugins.end())
        p = *it;

    return p;
}

void CommonEditor::ApplyEdits(const wxString& plugin, wxListView * wxList) {
    BOOST_LOG_TRIVIAL(debug) << "Applying edits to plugin: " << plugin.ToUTF8();

    //Get recorded data.
    boss::Plugin master(GetMasterData(plugin));
    boss::Plugin edited(GetNewData(plugin));

    boss::Plugin diff = master.DiffMetadata(edited);

    list<boss::Plugin>::iterator pos = std::find(_editedPlugins.begin(), _editedPlugins.end(), diff);
    long i = wxList->FindItem(-1, plugin);

    if (!diff.HasNameOnly()) {
        wxList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).Bold());
        if (pos != _editedPlugins.end())
            *pos = diff;
        else
            _editedPlugins.push_back(diff);
    }
    else {
        wxList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
        if (pos != _editedPlugins.end())
            _editedPlugins.erase(pos);  //Prevents unnecessary writes.
    }
}

boss::File CommonEditor::RowToFile(wxListView * list, long row) const {
    return boss::File(
        string(list->GetItemText(row, 0).ToUTF8()),
        string(list->GetItemText(row, 1).ToUTF8()),
        string(list->GetItemText(row, 2).ToUTF8())
        );
}

boss::Tag CommonEditor::RowToTag(wxListView * list, long row) const {
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

boss::PluginDirtyInfo CommonEditor::RowToPluginDirtyInfo(wxListView * list, long row) const {
    string text(list->GetItemText(row, 0).ToUTF8());
    uint32_t crc = strtoul(text.c_str(), NULL, 16);
    return boss::PluginDirtyInfo(
        crc,
        atoi(string(list->GetItemText(row, 1).ToUTF8()).c_str()),
        atoi(string(list->GetItemText(row, 2).ToUTF8()).c_str()),
        atoi(string(list->GetItemText(row, 3).ToUTF8()).c_str()),
        string(list->GetItemText(row, 4).ToUTF8()));
}


///////////////////////////////////
// Mini Editor Class
///////////////////////////////////


MiniEditor::MiniEditor(wxWindow *parent, const wxString& title, const std::list<boss::Plugin>& plugins, const boss::Game& game) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), CommonEditor(plugins, game) {
    //Initialise editing panel.
    editingPanel = new wxPanel(this, wxID_ANY);

    //Initialise controls.
    pluginText = new wxStaticText(editingPanel, wxID_ANY, "");
    descText = new wxStaticText(this, wxID_ANY, translate("Please submit any edits made for reasons other than personal preference to the BOSS team so that they may be included in the masterlist."));
    prioritySpin = new wxSpinCtrl(editingPanel, wxID_ANY, "0");
    prioritySpin->SetRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    filterCheckbox = new wxCheckBox(editingPanel, wxID_ANY, translate("Show only conflicting plugins."));
    
    pluginList = new wxListView(this, LIST_Plugins, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    loadAfterList = new wxListView(editingPanel, LIST_LoadAfter, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    loadAfterList->SetDropTarget(new TextDropTarget(loadAfterList, pluginText));

    removeBtn = new wxButton(editingPanel, BUTTON_RemoveRow, translate("Remove Plugin"));

    //Set up list columns.
    pluginList->AppendColumn(translate("Plugin"));
    pluginList->AppendColumn(translate("Priority"));

    loadAfterList->AppendColumn(translate("Filename"));
    loadAfterList->AppendColumn(translate("Display Name"));
    loadAfterList->AppendColumn(translate("Condition"));
    loadAfterList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    loadAfterList->SetColumnWidth(1, 0);  //Hide this from the user.
    loadAfterList->SetColumnWidth(2, 0);  //Hide this from the user.
    
    //Initialise control states.
    removeBtn->Enable(false);
    prioritySpin->Enable(false);
    filterCheckbox->Enable(false);

    //Make plugin name bold text.
    pluginText->SetFont(pluginText->GetFont().Bold());

    //Set up event handling.
    Bind(wxEVT_LIST_ITEM_SELECTED, &MiniEditor::OnPluginSelect, this, LIST_Plugins);
    Bind(wxEVT_LIST_BEGIN_DRAG, &MiniEditor::OnDragStart, this, LIST_Plugins);
    Bind(wxEVT_LIST_ITEM_SELECTED, &MiniEditor::OnRowSelect, this, LIST_LoadAfter);
    Bind(wxEVT_BUTTON, &MiniEditor::OnRemoveRow, this, BUTTON_RemoveRow);
    Bind(wxEVT_CHECKBOX, &MiniEditor::OnFilterToggle, this);
    Bind(wxEVT_BUTTON, &MiniEditor::OnApply, this, wxID_APPLY);
    Bind(wxEVT_SIZE, &MiniEditor::OnResize, this);

    //Set up editing panel layout.
    wxBoxSizer * mainBox = new wxBoxSizer(wxVERTICAL);
    mainBox->Add(pluginText, 0, wxTOP | wxBOTTOM, 10);
    mainBox->Add(filterCheckbox, 0, wxTOP | wxBOTTOM, 10);

    wxBoxSizer * hbox2 = new wxBoxSizer(wxHORIZONTAL);
    hbox2->Add(new wxStaticText(editingPanel, wxID_ANY, translate("Priority: ")), 0, wxRIGHT, 5);
    hbox2->Add(prioritySpin);
    mainBox->Add(hbox2, 0, wxEXPAND | wxBOTTOM, 10);

    wxStaticBoxSizer * staticBox = new wxStaticBoxSizer(wxVERTICAL, editingPanel, translate("Load After"));
    staticBox->Add(loadAfterList, 1, wxEXPAND);
    staticBox->Add(removeBtn, 0, wxEXPAND | wxALIGN_RIGHT | wxTOP, 5);
    mainBox->Add(staticBox, 1, wxEXPAND);

    editingPanel->SetSizerAndFit(mainBox);
    editingPanel->Layout();

    //Set up rest of layout.
    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer * hBox = new wxBoxSizer(wxHORIZONTAL);
    hBox->Add(pluginList, 1, wxEXPAND | wxALL, 10);
    hBox->Add(editingPanel, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 10);
    bigBox->Add(hBox, 1, wxEXPAND | wxALL, 5);

    bigBox->Add(descText, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 15);

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
        else if (it->LoadsBSA(_game)) {
            pluginList->SetItemTextColour(i, wxColour(0, 142, 219));
        }
        ++i;
    }
    pluginList->SetColumnWidth(0, wxLIST_AUTOSIZE);
    pluginList->SetColumnWidth(1, 0);
    descText->Wrap(pluginList->GetClientSize().GetWidth());

    SetBackgroundColour(wxColour(255, 255, 255));
    SetIcon(wxIconLocation("BOSS.exe"));

    editingPanel->Hide();
    SetSizerAndFit(bigBox);
}

void MiniEditor::OnPluginSelect(wxListEvent& event) {
    //Create Plugin object for selected plugin.
    wxString selectedPlugin = pluginList->GetItemText(event.GetIndex());
    wxString currentPlugin = pluginText->GetLabelText();

    //Check if the selected plugin is the same as the current plugin.
    if (selectedPlugin != currentPlugin) {
        BOOST_LOG_TRIVIAL(debug) << "User selected plugin: " << selectedPlugin.ToUTF8();

        //Apply any current edits.
        if (!currentPlugin.empty()) {
            ApplyEdits(currentPlugin, pluginList);
            //Also update the item's priority value in the plugins list in case it has changed.
            pluginList->SetItem(pluginList->FindItem(-1, currentPlugin), 1, FromUTF8(boss::IntToString(prioritySpin->GetValue())));
        }

        //Merge metadata.
        boss::Plugin plugin = GetMasterData(selectedPlugin);
        plugin.MergeMetadata(GetUserData(selectedPlugin));

        //Now fill editor fields with new plugin's info and update control states.
        BOOST_LOG_TRIVIAL(debug) << "Filling editor fields with plugin info.";
        pluginText->SetLabelText(FromUTF8(plugin.Name()));

        prioritySpin->SetValue(plugin.Priority());

        loadAfterList->DeleteAllItems();
        set<boss::File> files = plugin.LoadAfter();
        int i = 0;
        for (set<boss::File>::const_iterator it = files.begin(), endit = files.end(); it != endit; ++it) {
            loadAfterList->InsertItem(i, FromUTF8(it->Name()));
            ++i;
        }
        if (loadAfterList->GetItemCount() == 0)
            loadAfterList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            loadAfterList->SetColumnWidth(0, wxLIST_AUTOSIZE);

        //Set control states.
        prioritySpin->Enable(true);
        filterCheckbox->Enable(true);
        removeBtn->Enable(false);
    }

    //Change layout.
    if (pluginList->GetColumnWidth(1) == 0)
        pluginList->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    if (!editingPanel->IsShown())
        editingPanel->Show();

    //Do all the horrible fitting. Still haven't managed to set a min size for the editing controls.
    //Layout() doesn't do anything extra, it automatically gets called by the windows.
    //InvalidateBestSize() doesn't do anything useful for the panel or the window.
    pluginList->InvalidateBestSize();  //Makes the priority column visible without scrolling.
    editingPanel->Fit();  //Fits the editing panel to the pluginText length.
    editingPanel->SetMinSize(editingPanel->GetSize());  //Makes sure that the editing panel is not cut off when Fit() is called below.
    Fit();
    int width = descText->GetSize().GetWidth();
    descText->SetLabel(translate("Please submit any edits made for reasons other than personal preference to the BOSS team so that they may be included in the masterlist."));
    descText->Wrap(width);
}

void MiniEditor::OnFilterToggle(wxCommandEvent& event) {
    //First need to merge the base and edited plugin lists so that the right priority values get displayed.

    list<boss::Plugin> plugins(_basePlugins);
    for (list<boss::Plugin>::const_iterator it = _editedPlugins.begin(); it != _editedPlugins.end(); ++it) {
        list<boss::Plugin>::iterator pos = std::find(plugins.begin(), plugins.end(), *it);
        pos->MergeMetadata(*it);
    }

    //Disable list selection.
    if (event.IsChecked())
        Unbind(wxEVT_LIST_ITEM_SELECTED, &MiniEditor::OnPluginSelect, this, LIST_Plugins);
    else
        Bind(wxEVT_LIST_ITEM_SELECTED, &MiniEditor::OnPluginSelect, this, LIST_Plugins);

    pluginList->Freeze();
    if (event.IsChecked()) {
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));
        list<boss::Plugin>::const_iterator pos = std::find(plugins.begin(), plugins.end(), plugin);

        if (pos != plugins.end()) {
            pluginList->DeleteAllItems();

            bool loadsBSA = pos->LoadsBSA(_game);

            int i = 0;
            for (list<boss::Plugin>::const_iterator it = plugins.begin(); it != plugins.end(); ++it) {
                //Want to filter to show only those the selected plugin can load after validly, and which also either conflict with it,
                //or which load a BSA (if the selected plugin loads a BSA).
                if (*it == *pos || !it->MustLoadAfter(*pos) && (pos->DoFormIDsOverlap(*it) || (loadsBSA && it->LoadsBSA(_game)))) {
                    pluginList->InsertItem(i, FromUTF8(it->Name()));
                    pluginList->SetItem(i, 1, FromUTF8(boss::IntToString(it->Priority())));
                    if (it->FormIDs().empty()) {
                        pluginList->SetItemTextColour(i, wxColour(122, 122, 122));
                    }
                    else if (it->LoadsBSA(_game)) {
                        pluginList->SetItemTextColour(i, wxColour(0, 142, 219));
                    }
                    if (std::find(_editedPlugins.begin(), _editedPlugins.end(), *it) != _editedPlugins.end()) {
                        pluginList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).Bold());
                    }
                    ++i;
                }
            }
        }
    }
    else {
        pluginList->DeleteAllItems();
        int i = 0;
        for (list<boss::Plugin>::const_iterator it = plugins.begin(); it != plugins.end(); ++it) {
            pluginList->InsertItem(i, FromUTF8(it->Name()));
            pluginList->SetItem(i, 1, FromUTF8(boss::IntToString(it->Priority())));
            if (it->FormIDs().empty()) {
                pluginList->SetItemTextColour(i, wxColour(122, 122, 122));
            }
            else if (it->LoadsBSA(_game)) {
                pluginList->SetItemTextColour(i, wxColour(0, 142, 219));
            }
            if (std::find(_editedPlugins.begin(), _editedPlugins.end(), *it) != _editedPlugins.end()) {
                pluginList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).Bold());
            }
            ++i;
        }
    }

    //Now re-select the current plugin in the list.
    pluginList->Select(pluginList->FindItem(-1, pluginText->GetLabelText()));
    pluginList->Thaw();
    Refresh();
}

void MiniEditor::OnRowSelect(wxListEvent& event) {
    boss::File file(string(loadAfterList->GetItemText(event.GetIndex(), 0).ToUTF8()));
    boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

    list<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

    if (it != _basePlugins.end())
        plugin = *it;
    else
        BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

    set<boss::File> loadAfter = plugin.LoadAfter();

    if (loadAfter.find(file) == loadAfter.end()) {
        BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was not found in base plugin metadata. Removal enabled.";
        removeBtn->Enable(true);
    }
    else {
        BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was found in base plugin metadata. Removal disabled.";
        removeBtn->Enable(false);
    }
}

void MiniEditor::OnRemoveRow(wxCommandEvent& event) {
    loadAfterList->DeleteItem(loadAfterList->GetFirstSelected());
    removeBtn->Enable(false);
}

void MiniEditor::OnDragStart(wxListEvent& event) {
    wxTextDataObject data(pluginList->GetItemText(event.GetItem()));
    wxDropSource dropSource(pluginList);
    dropSource.SetData(data);
    wxDragResult result = dropSource.DoDragDrop();
}

void MiniEditor::OnApply(wxCommandEvent& event) {
    //Apply any current edits.
    wxString currentPlugin = pluginText->GetLabelText();
    if (!currentPlugin.empty())
        ApplyEdits(currentPlugin, pluginList);

    EndModal(event.GetId());
}

const std::list<boss::Plugin>& MiniEditor::GetEditedPlugins() const {
    return _editedPlugins;
}

boss::Plugin MiniEditor::GetNewData(const wxString& plugin) const {
    //Get new data. Because most of it is missing in the MiniEditor, start with the existing data.
    boss::Plugin edited(GetMasterData(plugin));
    edited.Priority(prioritySpin->GetValue());
    set<boss::File> files; 
    for (int i = 0, max = loadAfterList->GetItemCount(); i < max; ++i) {
        files.insert(RowToFile(loadAfterList, i));
    }
    edited.LoadAfter(files);

    return edited;
}

void MiniEditor::OnResize(wxSizeEvent& event) {
    descText->SetLabel(translate("Please submit any edits made for reasons other than personal preference to the BOSS team so that they may be included in the masterlist."));
    descText->Wrap(GetClientSize().GetWidth()-30);
    event.Skip();
}

///////////////////////////////////
// Editor Class
///////////////////////////////////

Editor::Editor(wxWindow *parent, const wxString& title, const std::string userlistPath, const std::list<boss::Plugin>& basePlugins, std::list<boss::Plugin>& editedPlugins, const unsigned int language, const boss::Game& game) : wxFrame(parent, wxID_ANY, title), _userlistPath(userlistPath), CommonEditor(basePlugins, game, editedPlugins) {

    //Initialise child windows.
    listBook = new wxNotebook(this, BOOK_Lists);

    wxPanel * reqsTab = new wxPanel(listBook);
    wxPanel * incsTab = new wxPanel(listBook);
    wxPanel * loadAfterTab = new wxPanel(listBook);
    wxPanel * messagesTab = new wxPanel(listBook);
    wxPanel * tagsTab = new wxPanel(listBook);
    wxPanel * dirtyTab = new wxPanel(listBook);

    //Initialise controls.
    pluginText = new wxStaticText(this, wxID_ANY, "");
    prioritySpin = new wxSpinCtrl(this, wxID_ANY, "0");
    prioritySpin->SetRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
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
    dirtyList = new wxListView(dirtyTab, LIST_DirtyInfo, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    messageList = new MessageList(messagesTab, LIST_Messages, language);

    pluginMenu = new wxMenu();

    //Tie together notebooks and panels.
    listBook->AddPage(reqsTab, translate("Requirements"), true);
    listBook->AddPage(incsTab, translate("Incompatibilities"));
    listBook->AddPage(loadAfterTab, translate("Load After"));
    listBook->AddPage(messagesTab, translate("Messages"));
    listBook->AddPage(tagsTab, translate("Bash Tags"));
    listBook->AddPage(dirtyTab, translate("Dirty Info"));

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

    dirtyList->AppendColumn(translate("CRC"));
    dirtyList->AppendColumn(translate("ITM Count"));
    dirtyList->AppendColumn(translate("UDR Count"));
    dirtyList->AppendColumn(translate("Deleted Navmesh Count"));
    dirtyList->AppendColumn(translate("Cleaning Utility"));

    //Set up plugin right-click menu.
    pluginMenu->Append(MENU_CopyName, translate("Copy Name"));
    pluginMenu->Append(MENU_CopyMetadata, translate("Copy Metadata As Text"));
    pluginMenu->Append(MENU_ClearMetadata, translate("Remove All User-Added Metadata"));

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
    Bind(wxEVT_LIST_ITEM_SELECTED, &Editor::OnPluginSelect, this, LIST_Plugins);
    Bind(wxEVT_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_Reqs);
    Bind(wxEVT_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_Incs);
    Bind(wxEVT_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_LoadAfter);
    Bind(wxEVT_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_Messages);
    Bind(wxEVT_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_BashTags);
    Bind(wxEVT_LIST_ITEM_SELECTED, &Editor::OnRowSelect, this, LIST_DirtyInfo);
    Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &Editor::OnListBookChange, this, BOOK_Lists);
    Bind(wxEVT_BUTTON, &Editor::OnQuit, this, BUTTON_Apply);
    Bind(wxEVT_BUTTON, &Editor::OnQuit, this, BUTTON_Cancel);
    Bind(wxEVT_BUTTON, &Editor::OnAddRow, this, BUTTON_AddRow);
    Bind(wxEVT_BUTTON, &Editor::OnEditRow, this, BUTTON_EditRow);
    Bind(wxEVT_BUTTON, &Editor::OnRemoveRow, this, BUTTON_RemoveRow);
    Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &Editor::OnPluginListRightClick, this);
    Bind(wxEVT_MENU, &Editor::OnPluginCopyName, this, MENU_CopyName);
    Bind(wxEVT_MENU, &Editor::OnPluginCopyMetadata, this, MENU_CopyMetadata);
    Bind(wxEVT_MENU, &Editor::OnPluginClearMetadata, this, MENU_ClearMetadata);

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

    wxBoxSizer * tabBox6 = new wxBoxSizer(wxVERTICAL);
    tabBox6->Add(dirtyList, 1, wxEXPAND);
    dirtyTab->SetSizer(tabBox6);

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
    int i = 0;
    for (list<boss::Plugin>::const_iterator it = _basePlugins.begin(); it != _basePlugins.end(); ++it) {
        pluginList->InsertItem(i, FromUTF8(it->Name()));
        if (it->LoadsBSA(_game)) {
            pluginList->SetItemTextColour(i, wxColour(0, 142, 219));
        }
        if (std::find(_editedPlugins.begin(), _editedPlugins.end(), *it) != _editedPlugins.end()) {
            pluginList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).Bold());
        }
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
        BOOST_LOG_TRIVIAL(debug) << "User selected plugin: " << selectedPlugin.ToUTF8();

        //Apply any current edits.
        if (!currentPlugin.empty())
            ApplyEdits(currentPlugin, pluginList);

        //Merge metadata.
        boss::Plugin plugin = GetMasterData(selectedPlugin);
        plugin.MergeMetadata(GetUserData(selectedPlugin));

        //Now fill editor fields with new plugin's info and update control states.
        BOOST_LOG_TRIVIAL(debug) << "Filling editor fields with plugin info.";
        pluginText->SetLabelText(FromUTF8(plugin.Name()));

        prioritySpin->SetValue(plugin.Priority());

        enableUserEditsBox->SetValue(plugin.Enabled());

        loadAfterList->DeleteAllItems();
        reqsList->DeleteAllItems();
        incsList->DeleteAllItems();
        messageList->DeleteAllItems();
        tagsList->DeleteAllItems();
        dirtyList->DeleteAllItems();

        set<boss::File> files = plugin.LoadAfter();
        int i=0;
        for (set<boss::File>::const_iterator it=files.begin(), endit=files.end(); it != endit; ++it) {
            loadAfterList->InsertItem(i, FromUTF8(it->Name()));
            loadAfterList->SetItem(i, 1, FromUTF8(it->DisplayName()));
            loadAfterList->SetItem(i, 2, FromUTF8(it->Condition()));
            ++i;
        }
        if (loadAfterList->GetItemCount() == 0)
            loadAfterList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            loadAfterList->SetColumnWidth(0, wxLIST_AUTOSIZE); 

        files = plugin.Reqs();
        i=0;
        for (set<boss::File>::const_iterator it=files.begin(), endit=files.end(); it != endit; ++it) {
            reqsList->InsertItem(i, FromUTF8(it->Name()));
            reqsList->SetItem(i, 1, FromUTF8(it->DisplayName()));
            reqsList->SetItem(i, 2, FromUTF8(it->Condition()));
            ++i;
        }
        if (reqsList->GetItemCount() == 0)
            reqsList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            reqsList->SetColumnWidth(0, wxLIST_AUTOSIZE);

        files = plugin.Incs();
        i=0;
        for (set<boss::File>::const_iterator it=files.begin(), endit=files.end(); it != endit; ++it) {
            incsList->InsertItem(i, FromUTF8(it->Name()));
            incsList->SetItem(i, 1, FromUTF8(it->DisplayName()));
            incsList->SetItem(i, 2, FromUTF8(it->Condition()));
            ++i;
        }
        if (incsList->GetItemCount() == 0)
            incsList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            incsList->SetColumnWidth(0, wxLIST_AUTOSIZE);

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
        if (tagsList->GetItemCount() == 0)
            tagsList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            tagsList->SetColumnWidth(0, wxLIST_AUTOSIZE);

        set<boss::PluginDirtyInfo> dirtyInfo = plugin.DirtyInfo();
        i=0;
        for (set<boss::PluginDirtyInfo>::const_iterator it=dirtyInfo.begin(), endit=dirtyInfo.end(); it != endit; ++it) {
            dirtyList->InsertItem(i, FromUTF8(boss::IntToHexString(it->CRC())));
            dirtyList->SetItem(i, 1, FromUTF8(boss::IntToString(it->ITMs())));
            dirtyList->SetItem(i, 2, FromUTF8(boss::IntToString(it->UDRs())));
            dirtyList->SetItem(i, 3, FromUTF8(boss::IntToString(it->DeletedNavmeshes())));
            dirtyList->SetItem(i, 4, FromUTF8(it->CleaningUtility()));
            ++i;
        }
        if (dirtyList->GetItemCount() == 0)
            dirtyList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            dirtyList->SetColumnWidth(0, wxLIST_AUTOSIZE);

        //Set control states.
        prioritySpin->Enable(true);
        enableUserEditsBox->Enable(true);
        addBtn->Enable(true);
        editBtn->Enable(false);
        removeBtn->Enable(false);
    }
    Fit();
}

void Editor::OnPluginListRightClick(wxListEvent& event) {
    PopupMenu(pluginMenu);
}

void Editor::OnPluginCopyName(wxCommandEvent& event) {
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(pluginList->GetItemText(pluginList->GetFirstSelected())));
        wxTheClipboard->Close();
    }
}

void Editor::OnPluginCopyMetadata(wxCommandEvent& event) {
    wxString selectedPlugin = pluginList->GetItemText(pluginList->GetFirstSelected());
    boss::Plugin plugin = GetUserData(selectedPlugin);

    string text;
    if (plugin.HasNameOnly())
        text = "name: " + plugin.Name();
    else {
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << plugin;
        text = yout.c_str();
    }

    BOOST_LOG_TRIVIAL(info) << "Exported userlist metadata text for \"" << selectedPlugin.ToUTF8() << "\": " << text;
    
    if (!text.empty() && wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(FromUTF8(text)));
        wxTheClipboard->Close();
    }
}

void Editor::OnPluginClearMetadata(wxCommandEvent& event) {
    wxMessageDialog dialog(this, 
                            translate("Are you sure you want to clear all existing user-added metadata from this plugin?"), 
                            translate("BOSS: Warning"), 
                            wxYES_NO | wxCANCEL | wxICON_EXCLAMATION);

    if (dialog.ShowModal() == wxID_YES) {
        long i = pluginList->GetFirstSelected();
        wxString selectedPlugin = pluginList->GetItemText(i);
        boss::Plugin p(string(selectedPlugin.ToUTF8()));

        //Need to clear what's currently in the editor and what's from the userlist.

        list<boss::Plugin>::const_iterator it = std::find(_editedPlugins.begin(), _editedPlugins.end(), p);

        //Delete existing userlist entry.
        if (it != _editedPlugins.end())
            _editedPlugins.erase(it);

        //Also clear any unapplied data. Easiest way to do this is to simulate loading the plugin's data again.
        pluginText->SetLabelText("");
        pluginList->Select(i, false);
        pluginList->Select(i, true);
        pluginList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    }
}

void Editor::OnListBookChange(wxBookCtrlEvent& event) {
    BOOST_LOG_TRIVIAL(trace) << "Changed list tab.";
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
    } else if (event.GetSelection() == 5) {
        addBtn->SetLabel(translate("Add Dirty Info"));
        editBtn->SetLabel(translate("Edit Dirty Info"));
        removeBtn->SetLabel(translate("Remove Dirty Info"));
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
        BOOST_LOG_TRIVIAL(debug) << "Adding new file row.";

        FileEditDialog * rowDialog = new FileEditDialog(this, translate("BOSS: Add File/Plugin"));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled adding new file row.";
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
    } else if (listBook->GetSelection() == 3) {
        BOOST_LOG_TRIVIAL(debug) << "Adding new message row.";

        MessageEditDialog * rowDialog = new MessageEditDialog(this, translate("BOSS: Add Message"));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled adding new message row.";
            return;
        }

        if (rowDialog->GetMessage().Content().empty()) {
            BOOST_LOG_TRIVIAL(error) << "No content specified. Row will not be added.";
            wxMessageBox(
                translate("Error: No content specified. Row will not be added."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        }

        messageList->AppendItem(rowDialog->GetMessage());
    } else if (listBook->GetSelection() == 4) {
        BOOST_LOG_TRIVIAL(debug) << "Adding new tag row.";

        TagEditDialog * rowDialog = new TagEditDialog(this, translate("BOSS: Add Tag"));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled adding new tag row.";
            return;
        }

        long i = tagsList->GetItemCount();
        tagsList->InsertItem(i, rowDialog->GetState());
        tagsList->SetItem(i, 1, rowDialog->GetName());
        tagsList->SetItem(i, 2, rowDialog->GetCondition());
    } else if (listBook->GetSelection() == 5) {
        BOOST_LOG_TRIVIAL(debug) << "Adding new dirty info row.";

        DirtInfoEditDialog * rowDialog = new DirtInfoEditDialog(this, translate("BOSS: Add Dirty Info"));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled adding dirty info row.";
            return;
        }

        long i = tagsList->GetItemCount();
        dirtyList->InsertItem(i, rowDialog->GetCRC());
        dirtyList->SetItem(i, 1, FromUTF8(boss::IntToString(rowDialog->GetITMs())));
        dirtyList->SetItem(i, 2, FromUTF8(boss::IntToString(rowDialog->GetUDRs())));
        dirtyList->SetItem(i, 3, FromUTF8(boss::IntToString(rowDialog->GetDeletedNavmeshes())));
        dirtyList->SetItem(i, 4, rowDialog->GetUtility());
    }
}

void Editor::OnEditRow(wxCommandEvent& event) {
    if (listBook->GetSelection() < 3) {
        BOOST_LOG_TRIVIAL(debug) << "Editing file row.";
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

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled editing file row.";
            return;
        }

        list->SetItem(i, 0, rowDialog->GetName());
        list->SetItem(i, 1, rowDialog->GetDisplayName());
        list->SetItem(i, 2, rowDialog->GetCondition());
    } else if (listBook->GetSelection() == 3) {
        BOOST_LOG_TRIVIAL(debug) << "Editing message row.";
        MessageEditDialog * rowDialog = new MessageEditDialog(this, translate("BOSS: Edit Message"));

        long i = messageList->GetFirstSelected();

        rowDialog->SetMessage(messageList->GetItem(i));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled editing message row.";
            return;
        }

        if (rowDialog->GetMessage().Content().empty()) {
            BOOST_LOG_TRIVIAL(error) << "No content specified. Row will not be edited.";
            wxMessageBox(
                translate("Error: No content specified. Row will not be edited."),
                translate("BOSS: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        }

        messageList->SetItem(i, rowDialog->GetMessage());
    } else if (listBook->GetSelection() == 4) {
        BOOST_LOG_TRIVIAL(debug) << "Editing tag row.";
        TagEditDialog * rowDialog = new TagEditDialog(this, translate("BOSS: Edit Tag"));

        long i = tagsList->GetFirstSelected();

        int stateNo;
        if (tagsList->GetItemText(i, 0) == State[0])
            stateNo = 0;
        else
            stateNo = 1;

        rowDialog->SetValues(stateNo, tagsList->GetItemText(i, 1), tagsList->GetItemText(i, 2));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled editing tag row.";
            return;
        }

        tagsList->SetItem(i, 0, rowDialog->GetState());
        tagsList->SetItem(i, 1, rowDialog->GetName());
        tagsList->SetItem(i, 2, rowDialog->GetCondition());
    } else if (listBook->GetSelection() == 5) {
        BOOST_LOG_TRIVIAL(debug) << "Editing dirty info row.";
        DirtInfoEditDialog * rowDialog = new DirtInfoEditDialog(this, translate("BOSS: Edit Dirty Info"));

        long i = tagsList->GetFirstSelected();

        rowDialog->SetValues(tagsList->GetItemText(i, 0),
                             atoi(string(tagsList->GetItemText(i, 1)).c_str()),
                             atoi(string(tagsList->GetItemText(i, 2)).c_str()),
                             atoi(string(tagsList->GetItemText(i, 3)).c_str()),
                             tagsList->GetItemText(i, 4));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled editing tag row.";
            return;
        }

        dirtyList->SetItem(i, 0, rowDialog->GetCRC());
        dirtyList->SetItem(i, 1, FromUTF8(boss::IntToString(rowDialog->GetITMs())));
        dirtyList->SetItem(i, 2, FromUTF8(boss::IntToString(rowDialog->GetUDRs())));
        dirtyList->SetItem(i, 3, FromUTF8(boss::IntToString(rowDialog->GetDeletedNavmeshes())));
        dirtyList->SetItem(i, 4, rowDialog->GetUtility());
    }
}

void Editor::OnRemoveRow(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Removing row.";
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

        list<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<boss::File> reqs = plugin.Reqs();

        if (reqs.find(file) == reqs.end()) {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    } else if (event.GetId() == LIST_Incs) {

        boss::File file = RowToFile(incsList, event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        list<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<boss::File> incs = plugin.Incs();

        if (incs.find(file) == incs.end()) {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    } else if (event.GetId() == LIST_LoadAfter) {

        boss::File file = RowToFile(loadAfterList, event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        list<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<boss::File> loadAfter = plugin.LoadAfter();

        if (loadAfter.find(file) == loadAfter.end()) {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    } else if (event.GetId() == LIST_Messages) {

        boss::Message message = messageList->GetItem(event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        list<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        list<boss::Message> messages = plugin.Messages();

        if (find(messages.begin(), messages.end(), message) == messages.end()) {
            BOOST_LOG_TRIVIAL(trace) << "Message \"" << message.ChooseContent(boss::g_lang_any).Str() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            BOOST_LOG_TRIVIAL(trace) << "Message \"" << message.ChooseContent(boss::g_lang_any).Str() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    } else if (event.GetId() == LIST_BashTags) {

        boss::Tag tag = RowToTag(tagsList, event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        list<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<boss::Tag> tags = plugin.Tags();

        if (tags.find(tag) == tags.end()) {
            BOOST_LOG_TRIVIAL(trace) << "Bash Tag \"" << tag.Name() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            BOOST_LOG_TRIVIAL(trace) << "Bash Tag \"" << tag.Name() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    } else {
        boss::PluginDirtyInfo dirtyData = RowToPluginDirtyInfo(dirtyList, event.GetIndex());
        boss::Plugin plugin(string(pluginText->GetLabelText().ToUTF8()));

        list<boss::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<boss::PluginDirtyInfo> dirtyInfo = plugin.DirtyInfo();

        if (dirtyInfo.find(dirtyData) == dirtyInfo.end()) {
            BOOST_LOG_TRIVIAL(trace) << "Dirty info for CRC \"" << dirtyData.CRC() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        } else {
            BOOST_LOG_TRIVIAL(trace) << "Dirty info for CRC \"" << dirtyData.CRC() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }
    }
}

void Editor::OnQuit(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Exiting metadata editor.";
    if (event.GetId() == BUTTON_Apply) {

        //Apply any current edits.
        wxString currentPlugin = pluginText->GetLabelText();
        if (!currentPlugin.empty())
            ApplyEdits(currentPlugin, pluginList);

        BOOST_LOG_TRIVIAL(debug) << "Saving metadata edits to userlist.";

        //Save edits to userlist.
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << YAML::BeginMap
             << YAML::Key << "plugins" << YAML::Value << _editedPlugins
             << YAML::EndMap;

        boost::filesystem::path p(_userlistPath);
        boss::ofstream out(p);
        out << yout.c_str();
        out.close();
    }
    Close();
}

boss::Plugin Editor::GetNewData(const wxString& plugin) const {
    BOOST_LOG_TRIVIAL(debug) << "Getting metadata from editor fields for plugin: " << plugin.ToUTF8();
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

    set<boss::PluginDirtyInfo> dirtyInfo;
    for (int i=0,max=dirtyList->GetItemCount(); i < max; ++i) {
        dirtyInfo.insert(RowToPluginDirtyInfo(dirtyList, i));
    }
    p.DirtyInfo(dirtyInfo);

    vector<boss::Message> vec = messageList->GetItems();
    list<boss::Message> messages(vec.begin(), vec.end());
    p.Messages(messages);

    return p;
}
