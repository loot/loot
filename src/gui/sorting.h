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
#ifndef __BOSS_GUI_SORTING__
#define __BOSS_GUI_SORTING__

#include "ids.h"
#include "../backend/metadata.h"

#include <string>
#include <vector>
#include <list>
#include <wx/spinctrl.h>
#include <wx/listctrl.h>

class MiniEditor : public wxDialog {
public:
    MiniEditor(wxWindow *parent, const wxString& title, const std::list<boss::Plugin>& plugins, const boss::Game& game);

    void OnPluginSelect(wxListEvent& event);
    void OnPluginListRightClick(wxListEvent& event);
    void OnPluginCopy(wxCommandEvent& event);
    void OnFilterToggle(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
private:
    wxButton * removeBtn;
    wxListView * pluginList;
    wxListView * loadAfterList;
    wxCheckBox * filterCheckbox;
    wxSpinCtrl * prioritySpin;
    wxStaticText * pluginText;


    const std::list<boss::Plugin>& _basePlugins;
    std::list<boss::Plugin> _editedPlugins;
};

class LoadOrderPreview : public wxDialog {
public:
    LoadOrderPreview(wxWindow *parent, const wxString title, const std::list<boss::Plugin>& plugins, const boss::Game& game);
private:
    wxListView * _loadOrder;
};

#endif
