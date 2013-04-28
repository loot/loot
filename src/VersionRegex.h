/*	BOSS
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

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

	$Revision: 1681 $, $Date: 2010-10-17 21:01:25 +0100 (Sun, 17 Oct 2010) $
*/

#ifndef __SUPPORT_VERSIONREGEX__HPP__
#define __SUPPORT_VERSIONREGEX__HPP__


#include <boost/regex.hpp>


namespace boss {

	using namespace boost;

	/// Array used to try each of the expressions defined above using 
	/// an iteration for each of them.
	extern regex* version_checks[];

}

#endif
