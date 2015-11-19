// Copyright 2015 Trevor Meehl
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//         http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
//         limitations under the License.

#ifndef MISSINGHUD2_MHUDPREFS_H
#define MISSINGHUD2_MHUDPREFS_H

#include <map>

#ifdef QT_CORE_LIB
#include <QApplication>
#include <QSettings>
#endif

#define CFG_FILENAME    "MHUD2.prefs"

#define CFG_USE_CACHE   true
#define CFG_NO_CACHE    false

namespace MHUD
{

struct Prefs
{
    bool show_tears_fired = false;
    bool show_shot_height = false;
    bool split_deal_chance = true;
    int stat_precision = 2;
};

class Options
{
public:
    static MHUD::Prefs ReadCfgFile(std::string cfg_file_name, bool use_cache = true);
    static void SaveCfgFile(std::string cfg_file_name, MHUD::Prefs new_prefs);

private:
    static std::map<std::string,MHUD::Prefs> cfg_files_;
};

}

#endif //MISSINGHUD2_MHUDPREFS_H
