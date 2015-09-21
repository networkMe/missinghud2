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
}

void LoaderGUI::ConnectSlots(BoIInjector &injector)
{
    qRegisterMetaType<InjectStatus>("InjectStatus");
    qRegisterMetaType<std::string>();

    QObject::connect(&injector, SIGNAL(InjectionStatus(InjectStatus)),
                     this, SLOT(OnInjectionStatusChange(InjectStatus)));
    QObject::connect(&injector, SIGNAL(FatalError(std::string)),
                     this, SLOT(OnFatalError(std::string)));
}

void LoaderGUI::OnInjectionStatusChange(InjectStatus inject_status)
{
    switch (inject_status.inj_result)
    {
        case InjectStatus::Result::OK:
        {
            ui_.InjectionStatus->setText("Injected Missing HUD 2 into BoI Process.");

            // Connect to the callbacks of the injected process

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
