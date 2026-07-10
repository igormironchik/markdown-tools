/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QApplication>
#include <QCommandLineParser>
#include <QScreen>
#include <QString>
#include <QTranslator>
#include <QWebEngineUrlScheme>

// md-editor include.
#include "mainwindow.h"

// shared include.
#include "utils.h"

#ifdef MD_BREEZE
#include <KIconTheme>
#endif

int main(int argc,
         char **argv)
{
#ifdef MD_BREEZE
    KIconTheme::initTheme();
#endif

    QWebEngineUrlScheme qrc("qrc");
    qrc.setFlags(QWebEngineUrlScheme::CorsEnabled);
    qrc.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    QWebEngineUrlScheme::registerScheme(qrc);

    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("Igor Mironchik"));
    app.setOrganizationDomain(QStringLiteral("github.com/igormironchik"));
    app.setApplicationName(QStringLiteral("Markdown Editor"));

    initTheme(app);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Markdown editor and viewer."));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("markdown"), QStringLiteral("Markdown file to open."));
    QCommandLineOption view(QStringList()
                                << "v"
                                << "view",
                            QStringLiteral("Open Markdown file in view (HTML preview) mode."));
    QCommandLineOption all(QStringList()
                               << "a"
                               << "all",
                           QStringLiteral("Load all linked Markdown files."));
    QCommandLineOption workingDir(QStringList()
                                      << "w"
                                      << "working-directory",
                                  QStringLiteral("Use set working directory."),
                                  QStringLiteral("dir"));
    parser.addOption(view);
    parser.addOption(all);
    parser.addOption(workingDir);

    parser.process(app);

    const auto args = parser.positionalArguments();

    const auto fileName = (args.isEmpty() ? QString() : args.at(0));

    initSharedResources();
    Q_INIT_RESOURCE(resources);

    QTranslator appTranslator;
    const auto locale = QLocale::system();

    if (!hasEnglish(locale.uiLanguages())) {
        if (appTranslator.load(locale, QStringLiteral("md_"), QString(), QStringLiteral(":/tr/"))) {
            QApplication::installTranslator(&appTranslator);
        }
    }

    QIcon appIcon(QStringLiteral(":/pics/icon_256x256.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_128x128.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_64x64.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_48x48.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_32x32.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_24x24.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_16x16.png"));
    app.setWindowIcon(appIcon);

    MdEditor::MainWindow w;
    const auto screenSize = app.primaryScreen()->availableGeometry().size();
    w.resize(qRound((double)screenSize.width() * 0.85), qRound((double)screenSize.height() * 0.85));

    if (parser.isSet(view) && fileName.isEmpty()) {
        qDebug() << QStringLiteral("File name was not specified.");

        return 0;
    }

    if (parser.isSet(all) && fileName.isEmpty()) {
        qDebug() << QStringLiteral("File name was not specified.");

        return 0;
    }

    MdEditor::StartupState state;
    state.m_fileName = fileName;
    state.m_workingDir = parser.value(workingDir);
    state.m_loadAllLinked = parser.isSet(all);
    state.m_previewMode = parser.isSet(view);

    w.setStartupState(state);

    w.show();

    return QApplication::exec();
}
