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

#include "viewer.h"
#include "../backend/helpers.h"

Viewer::Viewer(wxWindow *parent, const wxString& title, const wxString& url, wxPoint pos, wxSize size, YAML::Node& settings) : wxFrame(parent, wxID_ANY, title, pos, size), _settings(settings) {
    web = wxWebView::New(this, wxID_ANY, url);

    web->Bind(wxEVT_WEBVIEW_NAVIGATING, &Viewer::OnNavigationStart, this);
    web->Bind(wxEVT_CHAR_HOOK, &Viewer::OnKeyUp, this);
    Bind(wxEVT_CLOSE_WINDOW, &Viewer::OnClose, this);

    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    topsizer->Add(web, 1, wxEXPAND);

    SetSizer(topsizer);
    SetSize(800,600);
    SetIcon(wxIconLocation("LOOT.exe"));
}

void Viewer::OnNavigationStart(wxWebViewEvent& event) {
    wxLaunchDefaultBrowser(event.GetURL());
    event.Veto();
}

void Viewer::OnKeyUp(wxKeyEvent &event) {
    if (event.GetUnicodeKey() != 0x46 || event.GetModifiers() != wxMOD_CONTROL)  // "F"
        return;

    wxFindReplaceDialog * findDia = new wxFindReplaceDialog(this, &data, translate("Find in Page"));

    findDia->Bind(wxEVT_FIND, &Viewer::OnFind, this);
    findDia->Bind(wxEVT_FIND_NEXT, &Viewer::OnFind, this);

    findDia->Show();
}

void Viewer::OnFind(wxFindDialogEvent &event) {
    int dialogFlags = event.GetFlags();
    int findFlags = wxWEBVIEW_FIND_WRAP | wxWEBVIEW_FIND_HIGHLIGHT_RESULT;

    if ((dialogFlags & wxFR_DOWN) == 0)
        findFlags = findFlags | wxWEBVIEW_FIND_BACKWARDS;
    if (dialogFlags & wxFR_WHOLEWORD)
        findFlags = findFlags | wxWEBVIEW_FIND_ENTIRE_WORD;
    if (dialogFlags & wxFR_MATCHCASE)
        findFlags = findFlags | wxWEBVIEW_FIND_MATCH_CASE;


    web->Find(event.GetFindString(), findFlags);
}

void Viewer::OnClose(wxCloseEvent &event) {
    //Record window settings.
    YAML::Node node;
    node["height"] = GetSize().GetHeight();
    node["width"] = GetSize().GetWidth();
    node["xPos"] = GetPosition().x;
    node["yPos"] = GetPosition().y;

    _settings["windows"]["viewer"] = node;

    Destroy();
}