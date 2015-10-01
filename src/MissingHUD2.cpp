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

#include <QtWidgets>
#include <QtPlugin>
#include <easylogging++.h>

#include "LoaderGUI.h"
#include "BoIInjector.h"

INITIALIZE_EASYLOGGINGPP
void InitializeEasyLogging(int argc, char* argv[]);

#ifdef QT_STATIC
    Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif

int main(int argc, char* argv[])
{
    // Qt5
    QApplication app(argc, argv);

    // Initialize file logger
    InitializeEasyLogging(argc, argv);
    LOG(INFO) << "========== MissingHUD2 starting. ==========";

    // Initialize BoI DLL Injector
    BoIInjector injector;

    // Show GUI
    LoaderGUI gui;
    gui.ConnectSlots(injector);
    gui.show();

    // Start the DLL monitoring thread
    injector.Start();

    int ret_code = app.exec();
    LOG(INFO) << "MissingHUD2 exiting with exit code " << ret_code << ".";
    return ret_code;
}

void InitializeEasyLogging(int argc, char* argv[])
{
    std::string app_dir = QCoreApplication::applicationDirPath().toStdString();
    std::string log_file = app_dir + "/MHUD2.log";
    std::string conf_file = app_dir + "/MHUD2.log.conf";

    START_EASYLOGGINGPP(argc, argv);
    el::Configurations logger_conf(conf_file);
    logger_conf.setGlobally(el::ConfigurationType::Filename, log_file);
    logger_conf.setGlobally(el::ConfigurationType::ToFile, "true");

    el::Loggers::setDefaultConfigurations(logger_conf, true);
}