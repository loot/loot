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
#ifndef __BOSS_GUI_EDITOR__
#define __BOSS_GUI_EDITOR__

#include "ids.h"
#include "../metadata.h"

#include <vector>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>

class Editor : public wxFrame {
public:
    Editor(const wxString title, wxFrame *parent);

private:
    wxListBox * pluginList;
    
    wxButton * addFileBtn;
    wxButton * editFileBtn;
    wxButton * removeFileBtn;
    
    wxButton * addMsgBtn;
    wxButton * editMsgBtn;
    wxButton * removeMsgBtn;
    
    wxButton * addTagBtn;
    wxButton * editTagBtn;
    wxButton * removeTagBtn;
    
    wxButton * saveEditsBtn;
    wxButton * undoEditsBtn;
    wxButton * recalcBtn;
    wxButton * applyBtn;
    wxButton * cancelBtn;
    
    wxCheckBox * enableUserEditsBox;
    wxSpinCtrl * prioritySpin;
    wxStaticText * pluginText;
    wxNotebook * filesBook;
    wxListCtrl * reqsList;
    wxListCtrl * incsList;
    wxListCtrl * loadAfterList;
    wxListCtrl * messageList;
    wxListCtrl * tagsList;

    std::vector<boss::Plugin> plugins;
};

#endif
