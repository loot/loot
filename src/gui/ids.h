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

#ifndef __LOOT_GUI_IDS__
#define __LOOT_GUI_IDS__

#include <string>
#include <boost/format.hpp>
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#       include "wx/wx.h"
#endif


enum {
	//Main window.
    OPTION_EditMetadata = wxID_HIGHEST + 1, // declares an id which will be used to call our button
    OPTION_ViewLastReport,
    OPTION_SortPlugins,
    MENU_ViewDebugLog,
    MENU_ShowSettings,
    MENU_RedatePlugins,
    //Settings window.
    LIST_Games,
    BUTTON_AddGame,
    BUTTON_EditGame,
    BUTTON_RemoveGame,
    //Editor window.
    LIST_Plugins,
    LIST_Reqs,
    LIST_Incs,
    LIST_LoadAfter,
    LIST_Messages,
    LIST_BashTags,
    LIST_DirtyInfo,
    LIST_MessageContent,
    BUTTON_AddRow,
    BUTTON_EditRow,
    BUTTON_RemoveRow,
    BUTTON_AddContent,
    BUTTON_EditContent,
    BUTTON_RemoveContent,
    BUTTON_Apply,
    BUTTON_Cancel,
    BOOK_Lists,
    MENU_CopyName,
    MENU_CopyMetadata,
    MENU_ClearMetadata,
    //Main window - dynamically created IDs.
    MENU_LowestDynamicGameID,
    LIST_LoadOrder,
    BUTTON_MoveUp,
    BUTTON_MoveDown,
};

wxString translate(const std::string& str);

wxString FromUTF8(const std::string& str);

wxString FromUTF8(const boost::format& f);

#endif
