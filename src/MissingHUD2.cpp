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
#include "mhud2_version.h"

INITIALIZE_EASYLOGGINGPP
void InitializeEasyLogging(int argc, char* argv[]);
std::unique_ptr<QCommandLineParser> ParseCommandLine(QApplication *app);

#ifdef QT_STATIC
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

int main(int argc, char* argv[])
{
    // Qt5
    QApplication app(argc, argv);

    // Initialize file logger
    InitializeEasyLogging(argc, argv);
    LOG(INFO) << "========== Missing HUD 2 " << MHUD2_VERSION << " ==========";

    // Initialize BoI DLL Injector
    BoIInjector injector;

    // Load preferences
    MHUD::Prefs mhud_prefs = MHUD::Options::ReadCfgFile(CFG_FILENAME);

    // Handle command line options
    std::unique_ptr<QCommandLineParser> cmd_line = ParseCommandLine(&app);

    // Show GUI
    LoaderGUI gui;
    gui.ConnectSlots(injector);
    gui.UpdatePrefs(mhud_prefs);
    gui.show();

    // Open Isaac automatically if cmd_line says too
    if (cmd_line->isSet("openisaac"))
        gui.RunSteamIsaac();

    // Start the DLL monitoring thread
    injector.Start();

    int ret_code = app.exec();
    LOG(INFO) << "Missing HUD 2 exiting with exit code " << ret_code << ".";
    return ret_code;
}

void InitializeEasyLogging(int argc, char* argv[])
{
    START_EASYLOGGINGPP(argc, argv);

    // EasyLogging++ does not support unicode file paths, we workaround this by changing the working directory
    std::wstring app_dir = QCoreApplication::applicationDirPath().toStdWString();
    SetCurrentDirectoryW(app_dir.c_str());

    // Initialize the file logging module
    std::string log_file = "MHUD2.log";
    std::string conf_file = "MHUD2.log.conf";
    el::Configurations logger_conf(conf_file);
    logger_conf.setGlobally(el::ConfigurationType::Filename, log_file);
    logger_conf.setGlobally(el::ConfigurationType::ToFile, "true");
    el::Loggers::setDefaultConfigurations(logger_conf, true);
}

std::unique_ptr<QCommandLineParser> ParseCommandLine(QApplication *app)
{
    std::unique_ptr<QCommandLineParser> cmd_parser(new QCommandLineParser());

    QCommandLineOption run_isaac("openisaac", "Open Isaac Steam edition automatically when MHUD2 starts.");
    cmd_parser->addOption(run_isaac);

    cmd_parser->process(*app);
    return cmd_parser;
}