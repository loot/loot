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

#include "editor.h"
#include "../backend/generators.h"
#include "../backend/helpers.h"
#include "../backend/streams.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <wx/clipbrd.h>
#include <wx/msgdlg.h>

using namespace std;

//////////////////////////////
// TextDropTarget class
//////////////////////////////

TextDropTarget::TextDropTarget(wxListView * owner, wxControl * name) : targetOwner(owner), targetName(name) {}

bool TextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString &data) {
    if (data == targetName->GetLabelText() || targetOwner->FindItem(-1, data) != wxNOT_FOUND)
        return false;
    targetOwner->InsertItem(targetOwner->GetItemCount(), data);
    targetOwner->SetColumnWidth(0, wxLIST_AUTOSIZE);
    return true;
}

///////////////////////////////////
// EditorPanel Class
///////////////////////////////////

EditorPanel::EditorPanel(wxWindow *parent, const std::list<loot::Plugin>& basePlugins, std::list<loot::Plugin>& editedPlugins, const unsigned int language, const loot::Game& game) : wxPanel(parent, wxID_ANY), _basePlugins(basePlugins), _game(game), _editedPlugins(editedPlugins) {

    //Initialise child windows.
    listBook = new wxNotebook(this, BOOK_Lists);

    reqsTab = new wxPanel(listBook);
    incsTab = new wxPanel(listBook);
    loadAfterTab = new wxPanel(listBook);
    messagesTab = new wxPanel(listBook);
    tagsTab = new wxPanel(listBook);
    dirtyTab = new wxPanel(listBook);

    //Initialise controls.
    prioritySpin = new wxSpinCtrl(this, wxID_ANY, "0");
    prioritySpin->SetRange(-999999, 999999);
    priorityCheckbox = new wxCheckBox(this, wxID_ANY, translate("Compare priority against all other plugins"));
    pluginCheckbox = new wxCheckBox(this, wxID_ANY, "");
    filterCheckbox = new wxCheckBox(this, CHECKBOX_Filter, translate("Show only conflicting plugins"));

    addBtn = new wxButton(this, BUTTON_AddRow, translate("Add File"));
    editBtn = new wxButton(this, BUTTON_EditRow, translate("Edit File"));
    removeBtn = new wxButton(this, BUTTON_RemoveRow, translate("Remove File"));

    pluginList = new wxListView(this, LIST_Plugins, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    reqsList = new wxListView(reqsTab, LIST_Reqs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    incsList = new wxListView(incsTab, LIST_Incs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    loadAfterList = new wxListView(loadAfterTab, LIST_LoadAfter, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    tagsList = new wxListView(tagsTab, LIST_BashTags, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    dirtyList = new wxListView(dirtyTab, LIST_DirtyInfo, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
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
    pluginList->AppendColumn(translate("User Metadata Enabled"));
    pluginList->AppendColumn(translate("Plugin Name"));
    pluginList->AppendColumn(translate("Priority"));
    pluginList->AppendColumn(translate("Global Priority"));

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
    pluginMenu->Append(MENU_CopyName, translate("Copy Plugin Name"));
    pluginMenu->Append(MENU_CopyMetadata, translate("Copy Plugin Metadata As Text"));
    pluginMenu->Append(MENU_ClearPluginMetadata, translate("Remove Plugin User-Added Metadata"));
    pluginMenu->AppendSeparator();
    pluginMenu->Append(MENU_ClearAllMetadata, translate("Remove All User-Added Metadata"));

    //Initialise control states.
    addBtn->Enable(false);
    editBtn->Enable(false);
    removeBtn->Enable(false);
    prioritySpin->Enable(false);
    priorityCheckbox->Enable(false);
    pluginCheckbox->Enable(false);
    filterCheckbox->Enable(false);

    //Make plugin name bold text.
    wxFont font = pluginCheckbox->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    pluginCheckbox->SetFont(font);

    //Set up event handling.
    Bind(wxEVT_LIST_ITEM_SELECTED, &EditorPanel::OnPluginSelect, this, LIST_Plugins);
    Bind(wxEVT_LIST_ITEM_SELECTED, &EditorPanel::OnRowSelect, this, LIST_Reqs);
    Bind(wxEVT_LIST_ITEM_SELECTED, &EditorPanel::OnRowSelect, this, LIST_Incs);
    Bind(wxEVT_LIST_ITEM_SELECTED, &EditorPanel::OnRowSelect, this, LIST_LoadAfter);
    Bind(wxEVT_LIST_ITEM_SELECTED, &EditorPanel::OnRowSelect, this, LIST_Messages);
    Bind(wxEVT_LIST_ITEM_SELECTED, &EditorPanel::OnRowSelect, this, LIST_BashTags);
    Bind(wxEVT_LIST_ITEM_SELECTED, &EditorPanel::OnRowSelect, this, LIST_DirtyInfo);
    Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &EditorPanel::OnListBookChange, this, BOOK_Lists);
    Bind(wxEVT_BUTTON, &EditorPanel::OnAddRow, this, BUTTON_AddRow);
    Bind(wxEVT_BUTTON, &EditorPanel::OnEditRow, this, BUTTON_EditRow);
    Bind(wxEVT_BUTTON, &EditorPanel::OnRemoveRow, this, BUTTON_RemoveRow);
    Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &EditorPanel::OnPluginListRightClick, this);
    Bind(wxEVT_MENU, &EditorPanel::OnPluginCopyName, this, MENU_CopyName);
    Bind(wxEVT_MENU, &EditorPanel::OnPluginCopyMetadata, this, MENU_CopyMetadata);
    Bind(wxEVT_MENU, &EditorPanel::OnPluginClearMetadata, this, MENU_ClearPluginMetadata);
    Bind(wxEVT_MENU, &EditorPanel::OnClearAllMetadata, this, MENU_ClearAllMetadata);
    Bind(wxEVT_LIST_BEGIN_DRAG, &EditorPanel::OnDragStart, this, LIST_Plugins);
    Bind(wxEVT_CHECKBOX, &EditorPanel::OnFilterToggle, this, CHECKBOX_Filter);

    //Set up drag 'n' drop.
    reqsList->SetDropTarget(new TextDropTarget(reqsList, pluginCheckbox));
    incsList->SetDropTarget(new TextDropTarget(incsList, pluginCheckbox));
    loadAfterList->SetDropTarget(new TextDropTarget(loadAfterList, pluginCheckbox));

    //Set up tooltips.
    pluginList->SetToolTip(translate("Select a plugin to edit its load order metadata."));
    reqsList->SetToolTip(translate("Drag and drop a plugin here to add it to the list. The \"Show only conflicting plugins\" checkbox must be checked."));
    incsList->SetToolTip(translate("Drag and drop a plugin here to add it to the list. The \"Show only conflicting plugins\" checkbox must be checked."));
    loadAfterList->SetToolTip(translate("Drag and drop a plugin here to make the selected plugin load after it. The \"Show only conflicting plugins\" checkbox must be checked."));
    prioritySpin->SetToolTip(translate("Plugins with higher priorities will load after plugins with smaller priorities that they conflict with, unless one must explicitly load after the other."));
    pluginCheckbox->SetToolTip(translate("If unchecked, any user-added metadata will be ignored during sorting."));
    editBtn->SetToolTip(translate("Only user-added data may be removed."));
    removeBtn->SetToolTip(translate("Only user-added data may be removed."));
    priorityCheckbox->SetToolTip(translate("Otherwise, priorities are only compared between conflicting plugins."));
    filterCheckbox->SetToolTip(translate("Filters the plugin list to only display plugins which can be loaded after the currently selected plugin, and which either conflict with it, or, if it loads a BSA, also load BSAs. Also enables drag and drop of plugins into the Load After box."));

    //Set up layout.
    wxBoxSizer * bigBox = new wxBoxSizer(wxHORIZONTAL);

    bigBox->Add(pluginList, 0, wxEXPAND | wxALL, 10);

    wxBoxSizer * mainBox = new wxBoxSizer(wxVERTICAL);

    mainBox->Add(pluginCheckbox, 0, wxTOP | wxBOTTOM | wxEXPAND, 10);


    wxBoxSizer * hbox1 = new wxBoxSizer(wxHORIZONTAL);
    hbox1->Add(filterCheckbox, 0, wxALIGN_LEFT | wxRIGHT, 10);
    hbox1->AddStretchSpacer(1);
    hbox1->Add(new wxStaticText(this, wxID_ANY, translate("Priority: ")), 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT, 5);
    hbox1->Add(prioritySpin, 0, wxALIGN_RIGHT);

    mainBox->Add(hbox1, 0, wxEXPAND | wxALIGN_RIGHT | wxTOP | wxBOTTOM, 5);
    mainBox->Add(priorityCheckbox, 0, wxALIGN_RIGHT | wxBOTTOM, 10);

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

    mainBox->Add(listBook, 1, wxEXPAND | wxTOP | wxBOTTOM, 10);

    wxBoxSizer * hbox2 = new wxBoxSizer(wxHORIZONTAL);
    hbox2->Add(addBtn, 0, wxRIGHT, 5);
    hbox2->Add(editBtn, 0, wxLEFT | wxRIGHT, 5);
    hbox2->Add(removeBtn, 0, wxLEFT, 5);
    mainBox->Add(hbox2, 0, wxALIGN_RIGHT);

    bigBox->Add(mainBox, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 10);

    //Fill pluginList with the contents of basePlugins.
    int i = 0;
    for (const auto &plugin : _basePlugins) {
        AddPluginToList(plugin, i);
        ++i;
    }
    pluginList->SetColumnWidth(0, wxLIST_AUTOSIZE);
    pluginList->SetColumnWidth(1, wxLIST_AUTOSIZE);
    pluginList->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
    pluginList->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);

    SetBackgroundColour(wxColour(255, 255, 255));

    SetSizerAndFit(bigBox);
    Layout();
}

void EditorPanel::OnPluginSelect(wxListEvent& event) {
    //Create Plugin object for selected plugin.
    wxString selectedPlugin = pluginList->GetItemText(event.GetIndex(), 1);
    wxString currentPlugin = pluginCheckbox->GetLabelText();

    //Check if the selected plugin is the same as the current plugin.
    if (selectedPlugin != currentPlugin) {
        BOOST_LOG_TRIVIAL(debug) << "User selected plugin: " << selectedPlugin.ToUTF8();

        //Apply any current edits.
        if (!currentPlugin.empty()) {
            ApplyEdits(currentPlugin);
            //Also update plugin list UI.
            long position = FindPlugin(currentPlugin);
            loot::Plugin userEdits = GetUserData(currentPlugin);
            if (!userEdits.HasNameOnly()) {
                if (userEdits.Enabled())
                    pluginList->SetItem(position, 0, FromUTF8("\xE2\x9C\x93"));
                else
                    pluginList->SetItem(position, 0, FromUTF8("\xE2\x9C\x97"));
            }
            //Also update the item's priority value in the plugins list in case it has changed.
            pluginList->SetItem(position, 2, FromUTF8(to_string(prioritySpin->GetValue())));
            if (priorityCheckbox->IsChecked())
                pluginList->SetItem(position, 3, FromUTF8("\xE2\x9C\x93"));
            else
                pluginList->SetItem(position, 3, FromUTF8("\xE2\x9C\x97"));
        }

        //Merge metadata.
        loot::Plugin plugin = GetMasterData(selectedPlugin);
        plugin.MergeMetadata(GetUserData(selectedPlugin));

        //Now fill editor fields with new plugin's info and update control states.
        BOOST_LOG_TRIVIAL(debug) << "Filling editor fields with plugin info.";
        pluginCheckbox->SetLabelText(FromUTF8(plugin.Name()));

        prioritySpin->SetValue(loot::modulo(plugin.Priority(), loot::max_priority));

        if (abs(plugin.Priority()) >= loot::max_priority)
            priorityCheckbox->SetValue(true);
        else
            priorityCheckbox->SetValue(false);

        pluginCheckbox->SetValue(plugin.Enabled());

        loadAfterList->DeleteAllItems();
        reqsList->DeleteAllItems();
        incsList->DeleteAllItems();
        messageList->DeleteAllItems();
        tagsList->DeleteAllItems();
        dirtyList->DeleteAllItems();

        set<loot::File> files = plugin.LoadAfter();
        int i = 0;
        for (const auto &file : files) {
            loadAfterList->InsertItem(i, FromUTF8(file.Name()));
            loadAfterList->SetItem(i, 1, FromUTF8(file.DisplayName()));
            loadAfterList->SetItem(i, 2, FromUTF8(file.Condition()));
            ++i;
        }
        if (loadAfterList->GetItemCount() == 0)
            loadAfterList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            loadAfterList->SetColumnWidth(0, wxLIST_AUTOSIZE);

        files = plugin.Reqs();
        i = 0;
        for (const auto &file : files) {
            reqsList->InsertItem(i, FromUTF8(file.Name()));
            reqsList->SetItem(i, 1, FromUTF8(file.DisplayName()));
            reqsList->SetItem(i, 2, FromUTF8(file.Condition()));
            ++i;
        }
        if (reqsList->GetItemCount() == 0)
            reqsList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            reqsList->SetColumnWidth(0, wxLIST_AUTOSIZE);

        files = plugin.Incs();
        i = 0;
        for (const auto &file : files) {
            incsList->InsertItem(i, FromUTF8(file.Name()));
            incsList->SetItem(i, 1, FromUTF8(file.DisplayName()));
            incsList->SetItem(i, 2, FromUTF8(file.Condition()));
            ++i;
        }
        if (incsList->GetItemCount() == 0)
            incsList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            incsList->SetColumnWidth(0, wxLIST_AUTOSIZE);

        list<loot::Message> messages = plugin.Messages();
        vector<loot::Message> vec(messages.begin(), messages.end());
        messageList->SetItems(vec);

        set<loot::Tag> tags = plugin.Tags();
        i = 0;
        for (const auto &tag : tags) {
            if (tag.IsAddition())
                tagsList->InsertItem(i, State[0]);
            else
                tagsList->InsertItem(i, State[1]);
            tagsList->SetItem(i, 1, FromUTF8(tag.Name()));
            tagsList->SetItem(i, 2, FromUTF8(tag.Condition()));
            ++i;
        }
        if (tagsList->GetItemCount() == 0)
            tagsList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            tagsList->SetColumnWidth(0, wxLIST_AUTOSIZE);

        set<loot::PluginDirtyInfo> dirtyInfo = plugin.DirtyInfo();
        i = 0;
        for (const auto &element : dirtyInfo) {
            dirtyList->InsertItem(i, FromUTF8(loot::IntToHexString(element.CRC())));
            dirtyList->SetItem(i, 1, FromUTF8(to_string(element.ITMs())));
            dirtyList->SetItem(i, 2, FromUTF8(to_string(element.UDRs())));
            dirtyList->SetItem(i, 3, FromUTF8(to_string(element.DeletedNavmeshes())));
            dirtyList->SetItem(i, 4, FromUTF8(element.CleaningUtility()));
            ++i;
        }
        if (dirtyList->GetItemCount() == 0)
            dirtyList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        else
            dirtyList->SetColumnWidth(0, wxLIST_AUTOSIZE);

        //Set control states.
        prioritySpin->Enable(true);
        priorityCheckbox->Enable(true);
        pluginCheckbox->Enable(true);
        filterCheckbox->Enable(true);
        addBtn->Enable(true);
        editBtn->Enable(false);
        removeBtn->Enable(false);
    }
    InvalidateBestSize();  //Makes the priority column visible without scrolling.
    if (GetBestSize().GetHeight() > GetSize().GetHeight() || GetBestSize().GetWidth() > GetSize().GetWidth()) {
        Fit();
    }
}

void EditorPanel::OnPluginListRightClick(wxListEvent& event) {
    PopupMenu(pluginMenu);
}

void EditorPanel::OnPluginCopyName(wxCommandEvent& event) {
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(pluginList->GetItemText(pluginList->GetFirstSelected(), 1)));
        wxTheClipboard->Close();
    }
}

void EditorPanel::OnPluginCopyMetadata(wxCommandEvent& event) {
    wxString selectedPlugin = pluginList->GetItemText(pluginList->GetFirstSelected(), 1);
    loot::Plugin plugin = GetUserData(selectedPlugin);

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

void EditorPanel::OnPluginClearMetadata(wxCommandEvent& event) {
    wxMessageDialog dialog(this,
        translate("Are you sure you want to clear all existing user-added metadata from this plugin?"),
        translate("LOOT: Warning"),
        wxYES_NO | wxCANCEL | wxICON_EXCLAMATION);

    if (dialog.ShowModal() == wxID_YES) {
        long i = pluginList->GetFirstSelected();
        wxString selectedPlugin = pluginList->GetItemText(i, 1);
        loot::Plugin p(string(selectedPlugin.ToUTF8()));

        //Need to clear what's currently in the editor and what's from the userlist.

        list<loot::Plugin>::const_iterator it = std::find(_editedPlugins.begin(), _editedPlugins.end(), p);

        //Delete existing userlist entry.
        if (it != _editedPlugins.end())
            _editedPlugins.erase(it);

        //Also clear any unapplied data. Easiest way to do this is to simulate loading the plugin's data again.
        pluginCheckbox->SetLabelText("");
        pluginList->Select(i, false);
        pluginList->Select(i, true);
        pluginList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    }
}

void EditorPanel::OnClearAllMetadata(wxCommandEvent& event) {
    wxMessageDialog dialog(this,
        translate("Are you sure you want to clear all existing user-added metadata from all plugins?"),
        translate("LOOT: Warning"),
        wxYES_NO | wxCANCEL | wxICON_EXCLAMATION);

    if (dialog.ShowModal() == wxID_YES) {
        //Delete all existing userlist entries.
        _editedPlugins.clear();

        //Also clear any unapplied data. Easiest way to do this is to simulate loading the plugin's data again.
        pluginCheckbox->SetLabelText("");
        long i = pluginList->GetFirstSelected();
        pluginList->Select(i, false);
        pluginList->Select(i, true);
        for (int i = 0, max = pluginList->GetItemCount(); i < max; ++i) {
            pluginList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
        }
    }
}

void EditorPanel::OnListBookChange(wxBookCtrlEvent& event) {
    BOOST_LOG_TRIVIAL(trace) << "Changed list tab.";
    if (listBook->GetPageCount() == 1 || event.GetSelection() == 2) {  //First check is for simple view mode.
        addBtn->SetLabel(translate("Add Plugin"));
        editBtn->SetLabel(translate("Edit Plugin"));
        removeBtn->SetLabel(translate("Remove Plugin"));
    }
    else if (event.GetSelection() == 0 || event.GetSelection() == 1) {
        addBtn->SetLabel(translate("Add File"));
        editBtn->SetLabel(translate("Edit File"));
        removeBtn->SetLabel(translate("Remove File"));
    }
    else if (event.GetSelection() == 3) {
        addBtn->SetLabel(translate("Add Message"));
        editBtn->SetLabel(translate("Edit Message"));
        removeBtn->SetLabel(translate("Remove Message"));
    }
    else if (event.GetSelection() == 4) {
        addBtn->SetLabel(translate("Add Bash Tag"));
        editBtn->SetLabel(translate("Edit Bash Tag"));
        removeBtn->SetLabel(translate("Remove Bash Tag"));
    }
    else if (event.GetSelection() == 5) {
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

void EditorPanel::OnAddRow(wxCommandEvent& event) {
    if (listBook->GetSelection() < 3) {
        BOOST_LOG_TRIVIAL(debug) << "Adding new file row.";

        FileEditDialog * rowDialog = new FileEditDialog(this, translate("LOOT: Add File/Plugin"));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled adding new file row.";
            return;
        }

        wxListView * list;
        if (listBook->GetPageCount() == 1 || listBook->GetSelection() == 2)
            list = loadAfterList;
        else if (listBook->GetSelection() == 0)
            list = reqsList;
        else if (listBook->GetSelection() == 1)
            list = incsList;

        long i = list->GetItemCount();
        list->InsertItem(i, rowDialog->GetName());
        list->SetItem(i, 1, rowDialog->GetDisplayName());
        list->SetItem(i, 2, rowDialog->GetCondition());
    }
    else if (listBook->GetSelection() == 3) {
        BOOST_LOG_TRIVIAL(debug) << "Adding new message row.";

        MessageEditDialog * rowDialog = new MessageEditDialog(this, translate("LOOT: Add Message"));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled adding new message row.";
            return;
        }

        if (rowDialog->GetMessage().Content().empty()) {
            BOOST_LOG_TRIVIAL(error) << "No content specified. Row will not be added.";
            wxMessageBox(
                translate("Error: No content specified. Row will not be added."),
                translate("LOOT: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        }

        messageList->AppendItem(rowDialog->GetMessage());
    }
    else if (listBook->GetSelection() == 4) {
        BOOST_LOG_TRIVIAL(debug) << "Adding new tag row.";

        TagEditDialog * rowDialog = new TagEditDialog(this, translate("LOOT: Add Tag"));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled adding new tag row.";
            return;
        }

        long i = tagsList->GetItemCount();
        tagsList->InsertItem(i, rowDialog->GetState());
        tagsList->SetItem(i, 1, rowDialog->GetName());
        tagsList->SetItem(i, 2, rowDialog->GetCondition());
    }
    else if (listBook->GetSelection() == 5) {
        BOOST_LOG_TRIVIAL(debug) << "Adding new dirty info row.";

        DirtInfoEditDialog * rowDialog = new DirtInfoEditDialog(this, translate("LOOT: Add Dirty Info"));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled adding dirty info row.";
            return;
        }

        long i = tagsList->GetItemCount();
        dirtyList->InsertItem(i, rowDialog->GetCRC());
        dirtyList->SetItem(i, 1, FromUTF8(to_string(rowDialog->GetITMs())));
        dirtyList->SetItem(i, 2, FromUTF8(to_string(rowDialog->GetUDRs())));
        dirtyList->SetItem(i, 3, FromUTF8(to_string(rowDialog->GetDeletedNavmeshes())));
        dirtyList->SetItem(i, 4, rowDialog->GetUtility());
    }
}

void EditorPanel::OnEditRow(wxCommandEvent& event) {
    if (listBook->GetSelection() < 3) {
        BOOST_LOG_TRIVIAL(debug) << "Editing file row.";
        FileEditDialog * rowDialog = new FileEditDialog(this, translate("LOOT: Edit File/Plugin"));

        wxListView * list;
        if (listBook->GetPageCount() == 1 || listBook->GetSelection() == 2)
            list = loadAfterList;
        else if (listBook->GetSelection() == 0)
            list = reqsList;
        else if (listBook->GetSelection() == 1)
            list = incsList;

        long i = list->GetFirstSelected();

        rowDialog->SetValues(list->GetItemText(i, 0), list->GetItemText(i, 1), list->GetItemText(i, 2));

        if (rowDialog->ShowModal() != wxID_OK) {
            BOOST_LOG_TRIVIAL(debug) << "Cancelled editing file row.";
            return;
        }

        list->SetItem(i, 0, rowDialog->GetName());
        list->SetItem(i, 1, rowDialog->GetDisplayName());
        list->SetItem(i, 2, rowDialog->GetCondition());
    }
    else if (listBook->GetSelection() == 3) {
        BOOST_LOG_TRIVIAL(debug) << "Editing message row.";
        MessageEditDialog * rowDialog = new MessageEditDialog(this, translate("LOOT: Edit Message"));

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
                translate("LOOT: Error"),
                wxOK | wxICON_ERROR,
                this);
            return;
        }

        messageList->SetItem(i, rowDialog->GetMessage());
    }
    else if (listBook->GetSelection() == 4) {
        BOOST_LOG_TRIVIAL(debug) << "Editing tag row.";
        TagEditDialog * rowDialog = new TagEditDialog(this, translate("LOOT: Edit Tag"));

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
    }
    else if (listBook->GetSelection() == 5) {
        BOOST_LOG_TRIVIAL(debug) << "Editing dirty info row.";
        DirtInfoEditDialog * rowDialog = new DirtInfoEditDialog(this, translate("LOOT: Edit Dirty Info"));

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
        dirtyList->SetItem(i, 1, FromUTF8(to_string(rowDialog->GetITMs())));
        dirtyList->SetItem(i, 2, FromUTF8(to_string(rowDialog->GetUDRs())));
        dirtyList->SetItem(i, 3, FromUTF8(to_string(rowDialog->GetDeletedNavmeshes())));
        dirtyList->SetItem(i, 4, rowDialog->GetUtility());
    }
}

void EditorPanel::OnRemoveRow(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Removing row.";
    wxListView * list;
    if (listBook->GetPageCount() == 1 || listBook->GetSelection() == 2)
        list = loadAfterList;
    else if (listBook->GetSelection() == 0)
        list = reqsList;
    else if (listBook->GetSelection() == 1)
        list = incsList;
    else if (listBook->GetSelection() == 3)
        list = messageList;
    else if (listBook->GetSelection() == 4)
        list = tagsList;
    else if (listBook->GetSelection() == 5)
        list = dirtyList;

    list->DeleteItem(list->GetFirstSelected());

    editBtn->Enable(false);
    removeBtn->Enable(false);
}

void EditorPanel::OnRowSelect(wxListEvent& event) {
    if (event.GetId() == LIST_Reqs) {

        //Create File object, search the masterlist vector for the plugin and search its reqs for this object.
        loot::File file = RowToFile(reqsList, event.GetIndex());
        loot::Plugin plugin(string(pluginCheckbox->GetLabelText().ToUTF8()));

        list<loot::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<loot::File> reqs = plugin.Reqs();

        if (reqs.find(file) == reqs.end()) {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        }
        else {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    }
    else if (event.GetId() == LIST_Incs) {

        loot::File file = RowToFile(incsList, event.GetIndex());
        loot::Plugin plugin(string(pluginCheckbox->GetLabelText().ToUTF8()));

        list<loot::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<loot::File> incs = plugin.Incs();

        if (incs.find(file) == incs.end()) {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        }
        else {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    }
    else if (event.GetId() == LIST_LoadAfter) {

        loot::File file = RowToFile(loadAfterList, event.GetIndex());
        loot::Plugin plugin(string(pluginCheckbox->GetLabelText().ToUTF8()));

        list<loot::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<loot::File> loadAfter = plugin.LoadAfter();

        if (loadAfter.find(file) == loadAfter.end()) {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        }
        else {
            BOOST_LOG_TRIVIAL(trace) << "File \"" << file.Name() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    }
    else if (event.GetId() == LIST_Messages) {

        loot::Message message = messageList->GetItem(event.GetIndex());
        loot::Plugin plugin(string(pluginCheckbox->GetLabelText().ToUTF8()));

        list<loot::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        list<loot::Message> messages = plugin.Messages();

        if (find(messages.begin(), messages.end(), message) == messages.end()) {
            BOOST_LOG_TRIVIAL(trace) << "Message \"" << message.ChooseContent(loot::Language::any).Str() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        }
        else {
            BOOST_LOG_TRIVIAL(trace) << "Message \"" << message.ChooseContent(loot::Language::any).Str() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    }
    else if (event.GetId() == LIST_BashTags) {

        loot::Tag tag = RowToTag(tagsList, event.GetIndex());
        loot::Plugin plugin(string(pluginCheckbox->GetLabelText().ToUTF8()));

        list<loot::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<loot::Tag> tags = plugin.Tags();

        if (tags.find(tag) == tags.end()) {
            BOOST_LOG_TRIVIAL(trace) << "Bash Tag \"" << tag.Name() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        }
        else {
            BOOST_LOG_TRIVIAL(trace) << "Bash Tag \"" << tag.Name() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }

    }
    else {
        loot::PluginDirtyInfo dirtyData = RowToPluginDirtyInfo(dirtyList, event.GetIndex());
        loot::Plugin plugin(string(pluginCheckbox->GetLabelText().ToUTF8()));

        list<loot::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), plugin);

        if (it != _basePlugins.end())
            plugin = *it;
        else
            BOOST_LOG_TRIVIAL(warning) << "Could not find plugin in base list: " << plugin.Name();

        set<loot::PluginDirtyInfo> dirtyInfo = plugin.DirtyInfo();

        if (dirtyInfo.find(dirtyData) == dirtyInfo.end()) {
            BOOST_LOG_TRIVIAL(trace) << "Dirty info for CRC \"" << dirtyData.CRC() << "\" was not found in base plugin metadata. Editing enabled.";
            editBtn->Enable(true);
            removeBtn->Enable(true);
        }
        else {
            BOOST_LOG_TRIVIAL(trace) << "Dirty info for CRC \"" << dirtyData.CRC() << "\" was found in base plugin metadata. Editing disabled.";
            editBtn->Enable(false);
            removeBtn->Enable(false);
        }
    }
}

loot::Plugin EditorPanel::GetNewData(const wxString& plugin) const {
    BOOST_LOG_TRIVIAL(debug) << "Getting metadata from editor fields for plugin: " << plugin.ToUTF8();
    loot::Plugin p(string(plugin.ToUTF8()));

    p.Enabled(pluginCheckbox->IsChecked());

    int priority = prioritySpin->GetValue();
    if (priorityCheckbox->IsChecked()) {
        if (priority < 0)
            priority -= loot::max_priority;
        else
            priority += loot::max_priority;
    }
    p.Priority(priority);

    set<loot::File> files;
    for (int i = 0, max = reqsList->GetItemCount(); i < max; ++i) {
        files.insert(RowToFile(reqsList, i));
    }
    p.Reqs(files);
    files.clear();

    for (int i = 0, max = incsList->GetItemCount(); i < max; ++i) {
        files.insert(RowToFile(incsList, i));
    }
    p.Incs(files);
    files.clear();

    for (int i = 0, max = loadAfterList->GetItemCount(); i < max; ++i) {
        files.insert(RowToFile(loadAfterList, i));
    }
    p.LoadAfter(files);

    set<loot::Tag> tags;
    for (int i = 0, max = tagsList->GetItemCount(); i < max; ++i) {
        tags.insert(RowToTag(tagsList, i));
    }
    p.Tags(tags);

    set<loot::PluginDirtyInfo> dirtyInfo;
    for (int i = 0, max = dirtyList->GetItemCount(); i < max; ++i) {
        dirtyInfo.insert(RowToPluginDirtyInfo(dirtyList, i));
    }
    p.DirtyInfo(dirtyInfo);

    vector<loot::Message> vec = messageList->GetItems();
    list<loot::Message> messages(vec.begin(), vec.end());
    p.Messages(messages);

    return p;
}

void EditorPanel::OnFilterToggle(wxCommandEvent& event) {
    //First need to merge the base and edited plugin lists so that the right priority values get displayed.

    list<loot::Plugin> plugins(_basePlugins);
    for (const auto &plugin : _editedPlugins) {
        list<loot::Plugin>::iterator pos = std::find(plugins.begin(), plugins.end(), plugin);
        pos->MergeMetadata(plugin);
    }

    //Disable list selection.
    if (event.IsChecked())
        Unbind(wxEVT_LIST_ITEM_SELECTED, &EditorPanel::OnPluginSelect, this, LIST_Plugins);
    else
        Bind(wxEVT_LIST_ITEM_SELECTED, &EditorPanel::OnPluginSelect, this, LIST_Plugins);

    pluginList->Freeze();
    if (event.IsChecked()) {
        loot::Plugin plugin(string(pluginCheckbox->GetLabelText().ToUTF8()));
        list<loot::Plugin>::const_iterator pos = std::find(plugins.begin(), plugins.end(), plugin);

        if (pos != plugins.end()) {
            pluginList->DeleteAllItems();

            bool loadsBSA = pos->LoadsBSA(_game);

            int i = 0;
            for (const auto &plugin : plugins) {
                //Want to filter to show only those the selected plugin can load after validly, and which also either conflict with it,
                //or which load a BSA (if the selected plugin loads a BSA).
                if (plugin == *pos || !plugin.MustLoadAfter(*pos) && (pos->DoFormIDsOverlap(plugin) || (loadsBSA && plugin.LoadsBSA(_game)))) {
                    AddPluginToList(plugin, i);
                    ++i;
                }
            }
        }
    }
    else {
        pluginList->DeleteAllItems();
        int i = 0;
        for (const auto &plugin : plugins) {
            AddPluginToList(plugin, i);
            ++i;
        }
    }

    //Now re-select the current plugin in the list.
    pluginList->Select(FindPlugin(pluginCheckbox->GetLabelText()));
    pluginList->Thaw();
    Refresh();
}

void EditorPanel::OnDragStart(wxListEvent& event) {
    wxTextDataObject data(pluginList->GetItemText(event.GetItem(), 1));
    wxDropSource dropSource(pluginList);
    dropSource.SetData(data);
    wxDragResult result = dropSource.DoDragDrop();
}

void EditorPanel::AddPluginToList(const loot::Plugin& plugin, int position) {
    loot::Plugin userEdits = GetUserData(FromUTF8(plugin.Name()));
    if (!userEdits.HasNameOnly()) {
        if (userEdits.Enabled())
            pluginList->InsertItem(position, FromUTF8("\xE2\x9C\x93"));
        else
            pluginList->InsertItem(position, FromUTF8("\xE2\x9C\x97"));
    }
    else
        pluginList->InsertItem(position, "");

    pluginList->SetItem(position, 1, FromUTF8(plugin.Name()));
    pluginList->SetItem(position, 2, FromUTF8(to_string(loot::modulo(plugin.Priority(), loot::max_priority))));
    if (abs(plugin.Priority()) >= loot::max_priority)
        pluginList->SetItem(position, 3, FromUTF8("\xE2\x9C\x93"));
    else
        pluginList->SetItem(position, 3, FromUTF8("\xE2\x9C\x97"));
    if (plugin.LoadsBSA(_game)) {
        pluginList->SetItemTextColour(position, wxColour(0, 142, 219));
    }
}

void EditorPanel::SetSimpleView(bool on) {
    if (on) {
        listBook->RemovePage(5);
        listBook->RemovePage(4);
        listBook->RemovePage(3);
        listBook->RemovePage(1);
        listBook->RemovePage(0);
        addBtn->Show(false);
        editBtn->Show(false);
        loadAfterList->SetColumnWidth(1, 0);
        loadAfterList->SetColumnWidth(2, 0);
    }
    else {
        listBook->InsertPage(0, reqsTab, translate("Requirements"));
        listBook->InsertPage(1, incsTab, translate("Incompatibilities"));
        listBook->AddPage(messagesTab, translate("Messages"));
        listBook->AddPage(tagsTab, translate("Bash Tags"));
        listBook->AddPage(dirtyTab, translate("Dirty Info"));
        addBtn->Show(true);
        editBtn->Show(true);
        loadAfterList->SetColumnWidth(1, wxLIST_AUTOSIZE);
        loadAfterList->SetColumnWidth(2, wxLIST_AUTOSIZE);
    }
}

void EditorPanel::ApplyCurrentEdits() {
    //Apply any current edits.
    wxString currentPlugin = pluginCheckbox->GetLabelText();
    if (!currentPlugin.empty())
        ApplyEdits(currentPlugin);
}

const std::list<loot::Plugin>& EditorPanel::GetEditedPlugins() const {
    return _editedPlugins;
}

loot::Plugin EditorPanel::GetMasterData(const wxString& plugin) const {
    BOOST_LOG_TRIVIAL(debug) << "Getting hardcoded and masterlist metadata for plugin: " << plugin.ToUTF8();
    loot::Plugin p;
    loot::Plugin p_in(string(plugin.ToUTF8()));

    list<loot::Plugin>::const_iterator it = std::find(_basePlugins.begin(), _basePlugins.end(), p_in);

    if (it != _basePlugins.end())
        p = *it;

    return p;
}

loot::Plugin EditorPanel::GetUserData(const wxString& plugin) const {
    BOOST_LOG_TRIVIAL(debug) << "Getting userlist metadata for plugin: " << plugin.ToUTF8();
    loot::Plugin p(string(plugin.ToUTF8()));

    list<loot::Plugin>::const_iterator it = std::find(_editedPlugins.begin(), _editedPlugins.end(), p);

    if (it != _editedPlugins.end())
        p = *it;

    return p;
}

void EditorPanel::ApplyEdits(const wxString& plugin) {
    BOOST_LOG_TRIVIAL(debug) << "Applying edits to plugin: " << plugin.ToUTF8();

    //Get recorded data.
    loot::Plugin master(GetMasterData(plugin));
    loot::Plugin edited(GetNewData(plugin));

    loot::Plugin diff = master.DiffMetadata(edited);

    list<loot::Plugin>::iterator pos = std::find(_editedPlugins.begin(), _editedPlugins.end(), diff);
    long i = FindPlugin(plugin);

    if (!diff.HasNameOnly()) {
        if (pos != _editedPlugins.end()) {
            if (!pos->DiffMetadata(edited).HasNameOnly())
                pluginList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).Bold());
            *pos = diff;
        }
        else {
            _editedPlugins.push_back(diff);
            pluginList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).Bold());
        }
    }
    else {
        pluginList->SetItemFont(i, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
        if (pos != _editedPlugins.end())
            _editedPlugins.erase(pos);  //Prevents unnecessary writes.
    }
}

long EditorPanel::FindPlugin(const wxString& plugin) {
    for (int i = 0, max = pluginList->GetItemCount(); i < max; ++i) {
        if (pluginList->GetItemText(i, 1) == plugin)
            return i;
    }
    return -1;
}

loot::File EditorPanel::RowToFile(wxListView * list, long row) const {
    return loot::File(
        string(list->GetItemText(row, 0).ToUTF8()),
        string(list->GetItemText(row, 1).ToUTF8()),
        string(list->GetItemText(row, 2).ToUTF8())
        );
}

loot::Tag EditorPanel::RowToTag(wxListView * list, long row) const {
    string name = string(list->GetItemText(row, 1).ToUTF8());

    if (list->GetItemText(row, 0) == State[1])
        return loot::Tag(
        name,
        false,
        string(list->GetItemText(row, 2).ToUTF8())
        );
    else
        return loot::Tag(
        name,
        true,
        string(list->GetItemText(row, 2).ToUTF8())
        );
}

loot::PluginDirtyInfo EditorPanel::RowToPluginDirtyInfo(wxListView * list, long row) const {
    string text(list->GetItemText(row, 0).ToUTF8());
    uint32_t crc = strtoul(text.c_str(), nullptr, 16);
    return loot::PluginDirtyInfo(
        crc,
        atoi(string(list->GetItemText(row, 1).ToUTF8()).c_str()),
        atoi(string(list->GetItemText(row, 2).ToUTF8()).c_str()),
        atoi(string(list->GetItemText(row, 3).ToUTF8()).c_str()),
        string(list->GetItemText(row, 4).ToUTF8()));
}


///////////////////////////////////
// Mini Editor Class
///////////////////////////////////

MiniEditor::MiniEditor(wxWindow *parent, const wxString& title, const std::list<loot::Plugin>& basePlugins, std::list<loot::Plugin>& editedPlugins, const loot::Game& game) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

    //Initialise content.
    editorPanel = new EditorPanel(this, basePlugins, editedPlugins, loot::Language::any, game);
    editorPanel->SetSimpleView(true);
    feedbackText = translate("If LOOT has gotten something wrong, please let the team know. See the Contributing To LOOT section of the readme for details.");

    //Set up event handling.
    Bind(wxEVT_BUTTON, &MiniEditor::OnApply, this, wxID_APPLY);
    Bind(wxEVT_SIZE, &MiniEditor::OnResize, this);

    //Set up layout.
    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

    bigBox->Add(editorPanel, 1, wxEXPAND | wxALL, 10);
    bigBox->Add(descText, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 15);

    //Need to add 'Yes' and 'No' buttons.
    wxSizer * sizer = CreateSeparatedButtonSizer(wxAPPLY | wxCANCEL);

    //Now add buttons to window sizer.
    if (sizer != nullptr)
        bigBox->Add(sizer, 0, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, 15);

    descText->Wrap(editorPanel->GetClientSize().GetWidth());

    SetBackgroundColour(wxColour(255, 255, 255));
    SetIcon(wxIconLocation("LOOT.exe"));

    SetSizerAndFit(bigBox);
}

void MiniEditor::OnApply(wxCommandEvent& event) {
    editorPanel->ApplyCurrentEdits();
    EndModal(event.GetId());
}

void MiniEditor::OnResize(wxSizeEvent& event) {
    descText->SetLabel(feedbackText);
    descText->Wrap(GetClientSize().GetWidth()-30);
    event.Skip();
}

const std::list<loot::Plugin>& MiniEditor::GetEditedPlugins() const {
    return editorPanel->GetEditedPlugins();
}


///////////////////////////////////
// Full Editor Class
///////////////////////////////////

FullEditor::FullEditor(wxWindow *parent, const wxString& title, const std::string userlistPath, const std::list<loot::Plugin>& basePlugins, std::list<loot::Plugin>& editedPlugins, const unsigned int language, const loot::Game& game) : wxFrame(parent, wxID_ANY, title), _userlistPath(userlistPath) {
    //Set up content.
    editorPanel = new EditorPanel(this, basePlugins, editedPlugins, language, game);
    applyBtn = new wxButton(this, BUTTON_Apply, translate("Save Changes"));
    cancelBtn = new wxButton(this, BUTTON_Cancel, translate("Cancel"));

    //Set up event handling.
    Bind(wxEVT_BUTTON, &FullEditor::OnQuit, this, BUTTON_Apply);
    Bind(wxEVT_BUTTON, &FullEditor::OnQuit, this, BUTTON_Cancel);

    //Set up layout.
    wxBoxSizer * bigBox = new wxBoxSizer(wxVERTICAL);

    bigBox->Add(editorPanel, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer * hbox6 = new wxBoxSizer(wxHORIZONTAL);
    hbox6->Add(applyBtn, 0, wxRIGHT, 10);
    hbox6->Add(cancelBtn);
    bigBox->Add(hbox6, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_RIGHT, 10);

    SetBackgroundColour(wxColour(255, 255, 255));
    SetIcon(wxIconLocation("LOOT.exe"));

    SetSizerAndFit(bigBox);
    Layout();
}

void FullEditor::OnQuit(wxCommandEvent& event) {
    BOOST_LOG_TRIVIAL(debug) << "Exiting metadata editor.";
    if (event.GetId() == BUTTON_Apply) {

        //Apply any current edits.
        editorPanel->ApplyCurrentEdits();

        BOOST_LOG_TRIVIAL(debug) << "Saving metadata edits to userlist.";

        //Save edits to userlist.
        YAML::Emitter yout;
        yout.SetIndent(2);
        yout << YAML::BeginMap
            << YAML::Key << "plugins" << YAML::Value << editorPanel->GetEditedPlugins()
            << YAML::EndMap;

        boost::filesystem::path p(_userlistPath);
        loot::ofstream out(p);
        out << yout.c_str();
        out.close();
    }
    Close();
}
