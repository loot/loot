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

#include "viewer.h"

#include <wx/webview.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

Viewer::Viewer(wxWindow *parent, const wxString& title, const std::string& path) : wxFrame(parent, wxID_ANY, title) {
    wxWebView * web = wxWebView::New(this, wxID_ANY, "file://" + fs::absolute(path).string());

    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    topsizer->Add(web, 1, wxEXPAND);

    SetSizer(topsizer);
    SetSize(800,600);
    SetIcon(wxIconLocation("BOSS.exe"));
}
