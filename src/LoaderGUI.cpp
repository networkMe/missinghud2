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

#include "LoaderGUI.h"

LoaderGUI::LoaderGUI(QWidget *parent) :
    QMainWindow(parent)
{
    ui_.setupUi(this);

    // Register Qt5 metatypes (for signals/slots)
    qRegisterMetaType<InjectStatus>();
    qRegisterMetaType<std::string>();

    // Connect GUI signals/slots
    QObject::connect(ui_.btn_UpdateCheck, SIGNAL(clicked(bool)),
                     this, SLOT(CheckForUpdates(bool)));
    QObject::connect(ui_.btn_ApplyPrefs, SIGNAL(clicked(bool)),
                     this, SLOT(SavePreferences(bool)));
    QObject::connect(ui_.btn_ResetPrefs, SIGNAL(clicked(bool)),
                     this, SLOT(ResetPreferences(bool)));
}

void LoaderGUI::ConnectSlots(BoIInjector &injector)
{
    QObject::connect(this, SIGNAL(NewPrefs(MHUD::Prefs)),
                     &injector, SLOT(SendNewPrefs(MHUD::Prefs)));

    QObject::connect(&injector, SIGNAL(InjectionStatus(InjectStatus)),
                     this, SLOT(OnInjectionStatusChange(InjectStatus)));
    QObject::connect(&injector, SIGNAL(FatalError(std::string)),
                     this, SLOT(OnFatalError(std::string)));
}

void LoaderGUI::UpdatePrefs(MHUD::Prefs mhud_prefs)
{
    ui_.cb_ShowNumTears->setChecked(mhud_prefs.show_tears_fired);
    ui_.cb_ShowShotHeight->setChecked(mhud_prefs.show_shot_height);
    ui_.spinbox_Precision->setValue(mhud_prefs.stat_precision);
}

void LoaderGUI::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // Fix UI size
    this->setFixedSize(this->size());

#ifdef NDEBUG
    // Check for MHUD2 updates automatically...
    CheckForUpdates();
#endif
}

void LoaderGUI::CheckForUpdates(bool checked)
{
    if (ui_.btn_UpdateCheck->text().toStdString() == "New version available!")
    {
        QDesktopServices::openUrl(QUrl(QString::fromStdString("https://github.com/networkMe/missinghud2/releases")));
        return;
    }

    ui_.btn_UpdateCheck->setText(QString::fromStdString("Checking for updates..."));
    ui_.btn_UpdateCheck->setDisabled(true);

    QNetworkAccessManager *network_mgr = new QNetworkAccessManager(this);
    QObject::connect(network_mgr, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(OnUpdateResponse(QNetworkReply*)));
    QObject::connect(network_mgr, SIGNAL(finished(QNetworkReply*)),
                     network_mgr, SLOT(deleteLater()));

    QNetworkRequest github_req(QUrl(QString::fromStdString("https://api.github.com/repos/networkMe/missinghud2/releases/latest")));
    network_mgr->get(github_req);
}

void LoaderGUI::SavePreferences(bool checked)
{
    MHUD::Prefs new_mhud2_prefs;
    new_mhud2_prefs.show_tears_fired = ui_.cb_ShowNumTears->isChecked();
    new_mhud2_prefs.show_shot_height = ui_.cb_ShowShotHeight->isChecked();
    new_mhud2_prefs.stat_precision = ui_.spinbox_Precision->value();
    MHUD::Options::SaveCfgFile(CFG_FILENAME, new_mhud2_prefs);

    // Send the new prefs to the injected DLL
    emit NewPrefs(new_mhud2_prefs);

    QMessageBox prefs_saved(this);
    prefs_saved.setText("Updated preferences!");
    prefs_saved.setStandardButtons(QMessageBox::Ok);
    prefs_saved.exec();
}

void LoaderGUI::ResetPreferences(bool checked)
{
    UpdatePrefs(MHUD::Prefs());
}

void LoaderGUI::OnUpdateResponse(QNetworkReply *response)
{
    try
    {
        if (response->error() != QNetworkReply::NoError)
            throw std::runtime_error("NetworkReply failed. Maybe GitHub is down?");

        QString str_response = response->readAll();

        QJsonParseError json_parse_result;
        QJsonDocument json_response = QJsonDocument::fromJson(str_response.toUtf8(), &json_parse_result);
        if (json_parse_result.error != QJsonParseError::ParseError::NoError)
            throw std::runtime_error("Unable to parse the JSON GitHub replied with.");

        QJsonObject json_obj = json_response.object();
        QJsonObject::iterator tag_member = json_obj.find(QString::fromStdString("tag_name"));
        if (tag_member == json_obj.end())
            throw std::runtime_error("Unable to find the latest version number in GitHub's API.");

        std::string latest_version = tag_member.value().toString().toStdString();
        if (latest_version != MHUD2_VERSION)
        {
            LOG(INFO) << "GitHub reported latest version is " << latest_version <<
                    " whilst running version is " << MHUD2_VERSION;
            ui_.btn_UpdateCheck->setText(QString::fromStdString("New version available! (" + latest_version + ")"));
            ui_.btn_UpdateCheck->setStyleSheet(QString::fromStdString("color: rgb(200, 0, 0);"));
        }
        else
        {
            LOG(INFO) << "GitHub reported no updates available.";
            ui_.btn_UpdateCheck->setText(QString::fromStdString("No update available"));
        }
    }
    catch(std::runtime_error &e)
    {
        LOG(ERROR) << "Error occured during update check: " << e.what();
        ui_.btn_UpdateCheck->setText(QString::fromStdString("Update check failed"));
    }

    ui_.btn_UpdateCheck->setEnabled(true);
    response->deleteLater();
}

void LoaderGUI::OnInjectionStatusChange(InjectStatus inject_status)
{
    switch (inject_status.inj_result)
    {
        case InjectStatus::Result::OK:
        {
            ui_.lbl_InjectStatus->setText(QString::fromStdString("Injected Missing HUD 2 into active BoI Process."));
        } break;

        case InjectStatus::Result::FAIL:
        {
            ui_.lbl_InjectStatus->setText(QString::fromStdString("Error injecting Missing HUD 2 into active BoI process."));
        } break;

        case InjectStatus::Result::NOT_FOUND:
        {
            ui_.lbl_InjectStatus->setText(QString::fromStdString("No active Binding of Isaac process found."));
        } break;
    }
}

void LoaderGUI::OnFatalError(std::string err_msg)
{
    QMessageBox err_box(this);
    err_box.setText(QString::fromStdString("Error occurred."));
    err_box.setInformativeText(QString::fromStdString(err_msg));
    err_box.setStandardButtons(QMessageBox::Ok);
    err_box.exec();

    QApplication::quit();
}
