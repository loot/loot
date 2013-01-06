/*  BOSS

    A plugin load order optimiser for games that use the esp/esm plugin system.

    Copyright (C) 2012    WrinklyNinja

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

#include "plugindata.h"

#include <vector>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <new>
#include <boost/filesystem.hpp>

namespace boss {

    PluginData::PluginData() : crc(0) {}

    PluginData::PluginData(std::string filename) : name(filename) {
        //Load by scanning plugin file using libespm.
    }

    PluginData::PluginData(std::string filename, uint32_t fileCRC) : name(filename), crc(fileCRC) {
        //Load by scanning cache file.
        std::string file = name[0] + '/' + name[1] + '/' + name + ".dc";
        //Lowercase the path.
        std::for_each(file.begin(), file.end(), tolower);

        if (!boost::filesystem::exists(file))
            return;

        ifstream ifile(file.c_str(), ios_base::binary);
        ifile.exceptions(ifstream::failbit | ifstream::badbit | ifstream::eofbit);

        //Get the number of entries.
        uint32_t count;
        ifile.seekg(4, ios_base::end);
        ifile.read((char*)&count, 4);

        //Read index.
        CacheIndexEntry * index;
        try {
            index = new CacheIndexEntry[count];
        } catch (std::bad_alloc& e) {
            return;
        }
        ifile.seekg(4 + count * sizeof(CacheIndexEntry), ios_base::end);
        ifile.read((char*)index, count * sizeof(CacheIndexEntry));

        //Search index.
        uint32_t i = 0;
        for (i; i < count; i++) {
            if (index[i].crc == crc)
                break;
        }

        if (i == count)
            return;

        //Read plugin data size.
        uint32_t dataSize;
        ifile.seekg(index[i].offset, ios_base::beg);
        ifile.read((char*)&dataSize, 4);

        //Read plugin data.
        char * data;
        try {
            data = new char[dataSize];
        } catch (std::bad_alloc& e) {
            return;
        }
        ifile.read(data, dataSize);

        //Now we need to read the list of masters.
        uint16_t mastersLen = *(uint16_t*)data;
        std::string master;
        std::vector<std::string> masters;
        for (int i=2; i < mastersLen; i++) {
            if (data[i] != NULL)
                master += data[i];
            else {
                masters.push_back(master);
                master.clear();
            }
        }

        //Now read the FormIDs, building up FormID objects from them and their
        //associated masters.
        for (int i=2+mastersLen; i < dataSize; i += 4) {
            int masterID = data[i+3];  //FormIDs are encoded low to high byte.
            FormID fid;
            fid.plugin = masters[masterID];
            fid.objectIndex = *(uint32_t*)data & 0xFFFFFF;  //Zero the masterID.
            formIDs.insert(fid);
        }

        ifile.close();
    }

    PluginData::Save() const {
        //Save to a cache file.
        std::string outPath = name[0] + '/' + name[1] + '/' + name + ".dc";
        //Lowercase the path.
        std::for_each(outPath.begin(), outPath.end(), tolower);

        uint32_t count = 0;
        CacheIndexEntry * index = NULL;
        if (boost::filesystem::exists(file)) {
            ifstream ifile(outPath.c_str(), ios_base::binary);
            ifile.exceptions(ifstream::failbit | ifstream::badbit | ifstream::eofbit);

            //Get the number of entries.
            ifile.seekg(4, ios_base::end);
            ifile.read((char*)&count, 4);

            //Read index.
            try {
                index = new CacheIndexEntry[count];
            } catch (std::bad_alloc& e) {
                return;
            }
            ifile.seekg(4 + count * sizeof(CacheIndexEntry), ios_base::end);
            ifile.read((char*)&index, count * sizeof(CacheIndexEntry));

            ifile.close();
        }

        /*Split the FormID object list into a FormID number list and a master list.
          This doesn't retain the same ordering of masters as the original data,
          but that doesn't matter. */
        std::map<std::string, int> masters;
        std::list<uint32_t> fids;
        std::string mastersList;
        int id = 0;
        for (std::list<FormID>::iterator it = formIDs.begin(), endIt = formIDs.end(); it != endIt; ++it) {
            std::map<std::string, int>::iterator mit = masters.find(it->plugin);
            if (mit != masters.end())
                fids.insert(it->objectIndex | (mit->second<<24));
            else {
                id++;
                masters.insert(pair<std::string, int>(it->plugin, id);
                fids.insert(it->objectIndex | (id<<24));
                mastersList += it->plugin + '\0';
            }
        }

        ofstream ofile(file.c_str(), ios_base::binary);
        ofile.exceptions(ifstream::failbit | ifstream::badbit | ifstream::eofbit);

        CacheIndexEntry entry;
        entry.crc = crc;

        if (!boost::filesystem::exists(file)) {
            uint8_t nameLength = name.length();
            ofile.write((char*)&nameLenth, 1);
            ofile.write((char*)name.data(), nameLength);
            entry.offset = tellg();
        } else {
            ofile.seekg(4 + count * sizeof(CacheIndexEntry), ios_base::end);
            entry.offset = ofile.tellg();
        }

        //Write out data.
        uint16_t mLen = mastersList.size();
        ofile.write((char*)&mLen, 2);
        ofile.write(mastersList.data(), mLen);
        for (std::list<uint32_t>::iterator it = formIDs.begin(), endIt = formIds.end(); it != endIt; ++it) {
            ofile.write((char*)it, 4);
        }

        //Write out updated index.
        if (index != NULL)
            ofile.write((char*)index, count * sizeof(CacheIndexEntry));
        ofile.write((char*)&entry, sizeof(CacheIndexEntry));
        count++;
        ofile.write((char*)&count, 4);

        ofile.close();
    }

    PluginData::Empty() const {
        return formIDs.empty();
    }

    int PluginData::Overlap(const PluginData& otherPlugin) const {
        int count = 0;
        for (std::list<FormID>::iterator it1 = formIDs.begin(), endIt1 = formIDs.end(); it1 != endIt1; ++it1) {
            for (std::list<FormID>::iterator it2 = otherPlugin.formIDs.begin(), endIt2 = otherPlugin.formIDs.end(); it2 != endIt2; ++it2) {
                if (*it1 == *it2)
                    count++;
            }
        }
    }
}
