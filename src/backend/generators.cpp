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

#include "generators.h"
#include "helpers.h"
#include "globals.h"
#include "parsers.h"
#include "streams.h"

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

using namespace std;

namespace YAML {
    Emitter& operator << (Emitter& out, const loot::PluginDirtyInfo& rhs) {
        out << BeginMap
            << Key << "crc" << Value << Hex << rhs.CRC() << Dec
            << Key << "util" << Value << rhs.CleaningUtility();

        if (rhs.ITMs() > 0)
            out << Key << "itm" << Value << rhs.ITMs();
        if (rhs.DeletedRefs() > 0)
            out << Key << "udr" << Value << rhs.DeletedRefs();
        if (rhs.DeletedNavmeshes() > 0)
            out << Key << "nav" << Value << rhs.DeletedNavmeshes();

        out << EndMap;

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::Game& rhs) {
        out << BeginMap
            << Key << "type" << Value << loot::Game(rhs.Id()).FolderName()
            << Key << "folder" << Value << rhs.FolderName()
            << Key << "name" << Value << rhs.Name()
            << Key << "master" << Value << rhs.Master()
            << Key << "repo" << Value << rhs.RepoURL()
            << Key << "branch" << Value << rhs.RepoBranch()
            << Key << "path" << Value << rhs.GamePath().string()
            << Key << "registry" << Value << rhs.RegistryKey()
            << EndMap;

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::MessageContent& rhs) {
        out << BeginMap;

        out << Key << "lang" << Value << loot::Language(rhs.Language()).Locale();

        out << Key << "str" << Value << rhs.Str();

        out << EndMap;

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::Message& rhs) {
        out << BeginMap;

        if (rhs.Type() == loot::Message::say)
            out << Key << "type" << Value << "say";
        else if (rhs.Type() == loot::Message::warn)
            out << Key << "type" << Value << "warn";
        else
            out << Key << "type" << Value << "error";

        if (rhs.Content().size() == 1)
            out << Key << "content" << Value << rhs.Content().front().Str();
        else
            out << Key << "content" << Value << rhs.Content();

        if (!rhs.Condition().empty())
            out << Key << "condition" << Value << rhs.Condition();

        out << EndMap;

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::File& rhs) {
        if (!rhs.IsConditional() && rhs.DisplayName().empty())
            out << rhs.Name();
        else {
            out << BeginMap
                << Key << "name" << Value << rhs.Name();

            if (rhs.IsConditional())
                out << Key << "condition" << Value << rhs.Condition();

            if (rhs.DisplayName() != rhs.Name())
                out << Key << "display" << Value << rhs.DisplayName();

            out << EndMap;
        }

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::Tag& rhs) {
        if (!rhs.IsConditional()) {
            if (rhs.IsAddition())
                out << rhs.Name();
            else
                out << '-' << rhs.Name();
        }
        else {
            out << BeginMap;
            if (rhs.IsAddition())
                out << Key << "name" << Value << rhs.Name();
            else
                out << Key << "name" << Value << ('-' + rhs.Name());

            out << Key << "condition" << Value << rhs.Condition()
                << EndMap;
        }

        return out;
    }

    Emitter& operator << (Emitter& out, const loot::Location& rhs) {
        if (rhs.Versions().empty())
            out << rhs.URL();
        else {
            out << BeginMap
                << Key << "link" << Value << rhs.URL()
                << Key << "ver" << Value << rhs.Versions()
                << EndMap;
        }
        return out;
    }

    Emitter& operator << (Emitter& out, const loot::Plugin& rhs) {
        if (!rhs.HasNameOnly()) {
            out << BeginMap
                << Key << "name" << Value << rhs.Name();

            if (rhs.IsPriorityExplicit())
                out << Key << "priority" << Value << rhs.Priority();

            if (!rhs.Enabled())
                out << Key << "enabled" << Value << rhs.Enabled();

            if (!rhs.LoadAfter().empty())
                out << Key << "after" << Value << rhs.LoadAfter();

            if (!rhs.Reqs().empty())
                out << Key << "req" << Value << rhs.Reqs();

            if (!rhs.Incs().empty())
                out << Key << "inc" << Value << rhs.Incs();

            if (!rhs.Messages().empty())
                out << Key << "msg" << Value << rhs.Messages();

            if (!rhs.Tags().empty())
                out << Key << "tag" << Value << rhs.Tags();

            if (!rhs.DirtyInfo().empty())
                out << Key << "dirty" << Value << rhs.DirtyInfo();

            if (!rhs.Locations().empty())
                out << Key << "url" << Value << rhs.Locations();

            out << EndMap;
        }

        return out;
    }
}