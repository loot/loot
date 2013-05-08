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

Editor::Editor(const wxString title, wxFrame *parent) : wxFrame(parent, wxID_ANY, title) {

    //Initialise child windows.
    filesBook = new wxNotebook(this, wxID_ANY);
    wxNotebook * messagesBook = new wxNotebook(this, wxID_ANY);

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
    messagesBook->AddPage(tagsTab, translate("Bash Tags"));

    //Initialise controls.
    pluginList = new wxListBox(this, LIST_Plugins);
    pluginText = new wxStaticText(this, wxID_ANY, "");
    prioritySpin = new wxSpinCtrl(this, SPIN_Priority, "0");
    enableUserEditsBox = new wxCheckBox(this, wxID_ANY, translate("Enable User Changes"));
    
    addFileBtn = new wxButton(this, BUTTON_AddFile, translate("Add File"));
    editFileBtn = new wxButton(this, BUTTON_EditFile, translate("Edit File"));
    removeFileBtn = new wxButton(this, BUTTON_RemoveFile, translate("Remove File"));
    addMsgBtn = new wxButton(messagesTab, BUTTON_AddMessage, translate("Add Message"));
    editMsgBtn = new wxButton(messagesTab, BUTTON_EditMessage, translate("Edit Message"));
    removeMsgBtn = new wxButton(messagesTab, BUTTON_RemoveMessage, translate("Remove Message"));
    addTagBtn = new wxButton(tagsTab, BUTTON_AddTag, translate("Add Tag"));
    editTagBtn = new wxButton(tagsTab, BUTTON_EditTag, translate("Edit Tag"));
    removeTagBtn = new wxButton(tagsTab, BUTTON_RemoveTag, translate("Remove Tag"));
    saveEditsBtn = new wxButton(this, BUTTON_SaveEdits, translate("Save Edits"));
    undoEditsBtn = new wxButton(this, BUTTON_UndoEdits, translate("Undo Edits"));
    recalcBtn = new wxButton(this, BUTTON_Recalc, translate("Recalculate Load Order"));
    applyBtn = new wxButton(this, BUTTON_Apply, translate("Apply Load Order"));
    cancelBtn = new wxButton(this, BUTTON_Cancel, translate("Cancel"));

    reqsList = new wxListCtrl(reqsTab, LIST_Reqs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    incsList = new wxListCtrl(incsTab, LIST_Incs, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    loadAfterList = new wxListCtrl(loadAfterTab, LIST_LoadAfter, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    messageList = new wxListCtrl(messagesTab, LIST_Messages, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    tagsList = new wxListCtrl(tagsTab, LIST_BashTags, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

    //Set up list columns.
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

    bigBox->Add(vbox1, 0, wxEXPAND|wxALL, 5);

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
    tabBox4->Add(messageList, 0, wxEXPAND|wxALL, 5);

    wxBoxSizer * hbox3 = new wxBoxSizer(wxHORIZONTAL);
    hbox3->Add(addMsgBtn, 0, wxALL, 10);
    hbox3->Add(editMsgBtn, 0, wxALL, 10);
    hbox3->Add(removeMsgBtn, 0, wxALL, 10);
    tabBox4->Add(hbox3);
    
    messagesTab->SetSizer(tabBox4);

    wxBoxSizer * tabBox5 = new wxBoxSizer(wxVERTICAL);
    tabBox5->Add(tagsList, 0, wxEXPAND|wxALL, 5);

    wxBoxSizer * hbox4 = new wxBoxSizer(wxHORIZONTAL);
    hbox4->Add(addTagBtn, 0, wxTOP|wxLEFT|wxRIGHT, 10);
    hbox4->Add(editTagBtn, 0, wxTOP|wxLEFT|wxRIGHT, 10);
    hbox4->Add(removeTagBtn, 0, wxTOP|wxLEFT|wxRIGHT, 10);
    tabBox5->Add(hbox4);

    tagsTab->SetSizer(tabBox5);

    mainBox->Add(messagesBook, 1, wxEXPAND);

    mainBox->AddSpacer(20);

    wxBoxSizer * hbox5 = new wxBoxSizer(wxHORIZONTAL);
    hbox5->Add(enableUserEditsBox, 0, wxALL, 5);
    hbox5->Add(saveEditsBtn, 0, wxALL, 5);
    hbox5->Add(undoEditsBtn, 0, wxALL, 5);
    mainBox->Add(hbox5);

    mainBox->AddSpacer(20);

    wxBoxSizer * hbox6 = new wxBoxSizer(wxHORIZONTAL);
    hbox6->Add(recalcBtn, 0, wxALL, 5);
    hbox6->Add(applyBtn, 0, wxALL, 5);
    hbox6->Add(cancelBtn, 0, wxALL, 5);
    mainBox->Add(hbox6);

    bigBox->Add(mainBox, 1, wxEXPAND|wxALL, 5);
    
    SetBackgroundColour(wxColour(255,255,255));

    SetSizerAndFit(bigBox);
}
