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

#include "MHUD_Options.h"

std::map<std::string,MHUD::Prefs> MHUD::Options::cfg_files_;

MHUD::Prefs MHUD::Options::ReadCfgFile(std::string cfg_file_name, bool use_cache)
{
    if (cfg_files_.count(cfg_file_name) > 0 && use_cache)
        return cfg_files_[cfg_file_name];

    // Read the preferences structure from the config file (if it exists)
    QSettings cfg_file(QString::fromStdString(cfg_file_name), QSettings::IniFormat);
    MHUD::Prefs cfg_prefs;
    cfg_prefs.show_tears_fired = cfg_file.value("show_tears_fired", false).toBool();
    cfg_prefs.show_shot_height = cfg_file.value("show_shot_height", false).toBool();
    cfg_prefs.split_deal_chance = cfg_file.value("split_deal_chance", true).toBool();
    cfg_prefs.stat_precision = cfg_file.value("stat_precision", 2).toInt();

    // Cache the result
    cfg_files_[cfg_file_name] = cfg_prefs;
    return cfg_prefs;
}

void MHUD::Options::SaveCfgFile(std::string cfg_file_name, MHUD::Prefs new_prefs)
{
    // Update cache for the file if it exists
    if (cfg_files_.count(cfg_file_name) > 0)
    {
        cfg_files_[cfg_file_name] = new_prefs;
    }

    QSettings cfg_file(QString::fromStdString(cfg_file_name), QSettings::IniFormat);
    cfg_file.setValue("show_tears_fired", new_prefs.show_tears_fired);
    cfg_file.setValue("show_shot_height", new_prefs.show_shot_height);
    cfg_file.setValue("split_deal_chance", new_prefs.split_deal_chance);
    cfg_file.setValue("stat_precision", new_prefs.stat_precision);
}
