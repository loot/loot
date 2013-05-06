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

#ifndef __BOSS_GUI_IDS__
#define __BOSS_GUI_IDS__

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
    MENU_Quit,
	MENU_OpenMainReadMe,
	MENU_OpenSyntaxReadMe,
	MENU_OpenAPIReadMe,
	MENU_OpenVersionHistory,
	MENU_OpenLicenses,
	MENU_ShowAbout,
	MENU_ShowSettings,
	MENU_Oblivion,
	MENU_Nehrim,
	MENU_Skyrim,
	MENU_Fallout3,
	MENU_FalloutNewVegas,
    //Settings window.
    OPTION_OKExitSettings,
	OPTION_CancelExitSettings
};

wxString translate(const std::string& str);

wxString FromUTF8(const std::string& str);

wxString FromUTF8(const boost::format& f);

#endif
