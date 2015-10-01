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

    // Fix UI size
    this->setFixedSize(this->size());

    // Register Qt5 metatypes (for signals/slots)
    qRegisterMetaType<InjectStatus>();
    qRegisterMetaType<std::string>();

    // Connect GUI signals/slots
    QObject::connect(ui_.UpdateCheck, SIGNAL(clicked(bool)),
                     this, SLOT(CheckForUpdates(bool)));
}

void LoaderGUI::ConnectSlots(BoIInjector &injector)
{
    QObject::connect(&injector, SIGNAL(InjectionStatus(InjectStatus)),
                     this, SLOT(OnInjectionStatusChange(InjectStatus)));
    QObject::connect(&injector, SIGNAL(FatalError(std::string)),
                     this, SLOT(OnFatalError(std::string)));
}

void LoaderGUI::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

#ifdef NDEBUG
    // Check for MHUD2 updates automatically...
    CheckForUpdates();
#endif
}

void LoaderGUI::CheckForUpdates(bool checked)
{
    if (ui_.UpdateCheck->text().toStdString() == "New version available!")
    {
        QDesktopServices::openUrl(QUrl("https://github.com/networkMe/missinghud2/releases"));
        return;
    }

    ui_.UpdateCheck->setText("Checking for updates...");
    ui_.UpdateCheck->setDisabled(true);

    QNetworkAccessManager *network_mgr = new QNetworkAccessManager(this);
    QObject::connect(network_mgr, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(OnUpdateResponse(QNetworkReply*)));
    QObject::connect(network_mgr, SIGNAL(finished(QNetworkReply*)),
                     network_mgr, SLOT(deleteLater()));

    QNetworkRequest github_req(QUrl("https://api.github.com/repos/networkMe/missinghud2/releases/latest"));
    network_mgr->get(github_req);
}

void LoaderGUI::OnUpdateResponse(QNetworkReply *response)
{
    try
    {
        if (response->error() != QNetworkReply::NoError)
            throw std::runtime_error("NetworkReply failed. Maybe GitHub is down?");

        std::string json_response = response->readAll().toStdString();
        rapidjson::Document update_doc;
        if (update_doc.Parse(json_response.c_str()).HasParseError())
            throw std::runtime_error("Unable to parse GitHub's latest release JSON.");

        rapidjson::Value::MemberIterator tag_name = update_doc.FindMember("tag_name");
        if (tag_name == update_doc.MemberEnd())
            throw std::runtime_error("Unable to find the latest version number in GitHub's API.");

        std::string latest_version = tag_name->value.GetString();
        if (latest_version != MHUD2_VERSION)
        {
            LOG(INFO) << "GitHub reported latest version is " << latest_version <<
                    " whilst running version is " << MHUD2_VERSION;
            ui_.UpdateCheck->setText("New version available!");
            ui_.UpdateCheck->setStyleSheet("color: rgb(200, 0, 0);");
        }
        else
        {
            LOG(INFO) << "GitHub reported no updates available.";
            ui_.UpdateCheck->setText("No update available");
        }
    }
    catch (std::runtime_error &e)
    {
        LOG(ERROR) << "Error occured during update check: " << e.what();
        ui_.UpdateCheck->setText("Update check failed");
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
            ui_.InjectionStatus->setText("Injected Missing HUD 2 into BoI Process.");
        } break;

        case InjectStatus::Result::FAIL:
        {
            ui_.InjectionStatus->setText("Error injecting Missing HUD 2 into BoI process.");
        } break;

        case InjectStatus::Result::NOT_FOUND:
        {
            ui_.InjectionStatus->setText("No Binding of Isaac: Rebirth process found.");
        } break;
    }
}

void LoaderGUI::OnFatalError(std::string err_msg)
{
    QMessageBox err_box(this);
    err_box.setText("Error occurred.");
    err_box.setInformativeText((err_msg).c_str());
    err_box.setStandardButtons(QMessageBox::Ok);
    err_box.exec();

    QApplication::quit();
}
