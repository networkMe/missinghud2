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
    qRegisterMetaType<std::wstring>();

    // Connect GUI signals/slots
    QObject::connect(ui_.UpdateCheck, SIGNAL(clicked(bool)),
                     this, SLOT(CheckForUpdates(bool)));
}

void LoaderGUI::ConnectSlots(BoIInjector &injector)
{
    QObject::connect(&injector, SIGNAL(InjectionStatus(InjectStatus)),
                     this, SLOT(OnInjectionStatusChange(InjectStatus)));
    QObject::connect(&injector, SIGNAL(FatalError(std::wstring)),
                     this, SLOT(OnFatalError(std::wstring)));
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
    if (ui_.UpdateCheck->text().toStdWString() == L"New version available!")
    {
        QDesktopServices::openUrl(QUrl(QString::fromStdWString(L"https://github.com/networkMe/missinghud2/releases")));
        return;
    }

    ui_.UpdateCheck->setText(QString::fromStdWString(L"Checking for updates..."));
    ui_.UpdateCheck->setDisabled(true);

    QNetworkAccessManager *network_mgr = new QNetworkAccessManager(this);
    QObject::connect(network_mgr, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(OnUpdateResponse(QNetworkReply*)));
    QObject::connect(network_mgr, SIGNAL(finished(QNetworkReply*)),
                     network_mgr, SLOT(deleteLater()));

    QNetworkRequest github_req(QUrl(QString::fromStdWString(L"https://api.github.com/repos/networkMe/missinghud2/releases/latest")));
    network_mgr->get(github_req);
}

void LoaderGUI::OnUpdateResponse(QNetworkReply *response)
{
    try
    {
        if (response->error() != QNetworkReply::NoError)
            throw MHUD_Error(L"NetworkReply failed. Maybe GitHub is down?");

        QString str_response = response->readAll();

        QJsonParseError json_parse_result;
        QJsonDocument json_response = QJsonDocument::fromJson(str_response.toUtf8(), &json_parse_result);
        if (json_parse_result.error != QJsonParseError::ParseError::NoError)
            throw MHUD_Error(L"Unable to parse the JSON GitHub replied with.");

        QJsonObject json_obj = json_response.object();
        QJsonObject::iterator tag_member = json_obj.find(QString::fromStdWString(L"tag_name"));
        if (tag_member == json_obj.end())
            throw MHUD_Error(L"Unable to find the latest version number in GitHub's API.");

        std::wstring latest_version = tag_member.value().toString().toStdWString();
        if (latest_version != MHUD2_VERSION)
        {
            LOG(INFO) << L"GitHub reported latest version is " << latest_version <<
                    L" whilst running version is " << MHUD2_VERSION;
            ui_.UpdateCheck->setText(QString::fromStdWString(L"New version available!"));
            ui_.UpdateCheck->setStyleSheet(QString::fromStdWString(L"color: rgb(200, 0, 0);"));
        }
        else
        {
            LOG(INFO) << L"GitHub reported no updates available.";
            ui_.UpdateCheck->setText(QString::fromStdWString(L"No update available"));
        }
    }
    catch(MHUD_Error &e)
    {
        LOG(ERROR) << L"Error occured during update check: " << e.get_error();
        ui_.UpdateCheck->setText(QString::fromStdWString(L"Update check failed"));
    }

    ui_.UpdateCheck->setEnabled(true);
    response->deleteLater();
}

void LoaderGUI::OnInjectionStatusChange(InjectStatus inject_status)
{
    switch (inject_status.inj_result)
    {
        case InjectStatus::Result::OK:
        {
            ui_.InjectionStatus->setText(QString::fromStdWString(L"Injected Missing HUD 2 into active BoI Process."));
        } break;

        case InjectStatus::Result::FAIL:
        {
            ui_.InjectionStatus->setText(QString::fromStdWString(L"Error injecting Missing HUD 2 into active BoI process."));
        } break;

        case InjectStatus::Result::NOT_FOUND:
        {
            ui_.InjectionStatus->setText(QString::fromStdWString(L"No active Binding of Isaac: Rebirth process found."));
        } break;
    }
}

void LoaderGUI::OnFatalError(std::wstring err_msg)
{
    QMessageBox err_box(this);
    err_box.setText(QString::fromStdWString(L"Error occurred."));
    err_box.setInformativeText(QString::fromStdWString(err_msg));
    err_box.setStandardButtons(QMessageBox::Ok);
    err_box.exec();

    QApplication::quit();
}
