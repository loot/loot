/*  BOSS

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

    $Revision: 3163 $, $Date: 2011-08-21 22:03:18 +0100 (Sun, 21 Aug 2011) $
*/

#ifndef __BOSS_HELPERS__
#define __BOSS_HELPERS__

#include "metadata.h"

#include <stdint.h>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace boss {

    /// Array used to try each of the expressions defined using 
	/// an iteration for each of them.
	extern boost::regex version_checks[7];

    //////////////////////////////////////////////////////////////////////////
    // Helper functions
    //////////////////////////////////////////////////////////////////////////

    //Calculate the CRC of the given file for comparison purposes.
    uint32_t GetCrc32(const boost::filesystem::path& filename);

    //Converts an integer to a string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
    std::string IntToString(const int n);

    //Converts an integer to a hex string using BOOST's Spirit.Karma. Faster than a stringstream conversion.
    std::string IntToHexString(const int n);

    //Converts a boolean to a string representation (true/false)
    std::string BoolToString(const bool b);

    //Check if registry subkey exists.
    bool RegKeyExists(const std::string& keyStr, const std::string& subkey, const std::string& value);

    //Get registry subkey value string.
    std::string RegKeyStringValue(const std::string& keyStr, const std::string& subkey, const std::string& value);

    //Get the local application data path.
    boost::filesystem::path GetLocalAppDataPath();

    //Turns an absolute filesystem path into a valid file:// URL.
    std::string ToFileURL(const boost::filesystem::path& file);

    //Converts between BOSS's global const value and the ISO 639-3 value for a language.
    std::string GetLangString(const unsigned int num);
    unsigned int GetLangNum(const std::string& str);

    //Version class for more robust version comparisons.
    class Version {
    private:
        std::string verString;
    public:
        Version();
        Version(const std::string& ver);
        Version(const boost::filesystem::path& file);
        Version(const Plugin& plugin);

        std::string AsString() const;

        bool operator > (Version);
        bool operator < (Version);
        bool operator >= (Version);
        bool operator <= (Version);
        bool operator == (Version);
        bool operator != (Version);
    };
}

#endif
