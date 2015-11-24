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

#ifndef BOISTATSREBORN_LOADERGUI_H
#define BOISTATSREBORN_LOADERGUI_H

#include <QtWidgets>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtWinExtras>

#include <iostream>
#include <thread>
#include <chrono>

#include "ui_LoaderGUI.h"
#include "BoIInjector.h"
#include "MHUD_Options.h"
#include "mhud2_version.h"

Q_DECLARE_METATYPE(std::string);

class LoaderGUI : public QMainWindow
{
    Q_OBJECT

public:
    LoaderGUI(QWidget *parent = 0);

    void ConnectSlots(BoIInjector &injector);
    void UpdatePrefs(MHUD::Prefs mhud_prefs);

signals:
    void NewPrefs(MHUD::Prefs mhud_prefs);

public slots:
    void RunSteamIsaac(bool checked = false);

protected:
    void showEvent(QShowEvent *event);

private slots:
    void CheckForUpdates(bool checked = false);
    void SavePreferences(bool checked = false);
    void ResetPreferences(bool checked = false);
    void OnInjectionStatusChange(InjectStatus s);
    void OnFatalError(std::string err_msg);
    void OnUpdateResponse(QNetworkReply* response);

private:
    Ui::LoaderGUI ui_;
};


#endif //BOISTATSREBORN_LOADERGUI_H
