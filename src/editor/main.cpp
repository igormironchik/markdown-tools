/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QApplication>
#include <QCommandLineParser>
#include <QScreen>
#include <QString>
#include <QWebEngineUrlScheme>

// md-editor include.
#include "mainwindow.hpp"

// shared include.
#include "utils.hpp"

int main(int argc, char **argv)
{
    QWebEngineUrlScheme qrc("qrc");
    qrc.setFlags(QWebEngineUrlScheme::CorsEnabled);
    qrc.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    QWebEngineUrlScheme::registerScheme(qrc);

    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("Igor Mironchik"));
    app.setOrganizationDomain(QStringLiteral("github.com/igormironchik"));
    app.setApplicationName(QStringLiteral("Markdown Editor"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Markdown editor and viewer."));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("markdown"), QStringLiteral("Markdown file to open."));
    QCommandLineOption view(QStringList() << "v"
                                          << "view",
                            QStringLiteral("Open Markdown file in view (HTML preview) mode."));
    QCommandLineOption all(QStringList() << "a"
                                         << "all",
                           QStringLiteral("Load all linked Markdown files."));
    parser.addOption(view);
    parser.addOption(all);

    parser.process(app);

    const auto args = parser.positionalArguments();

    const auto fileName = (args.isEmpty() ? QString() : args.at(0));

    initSharedResources();
    Q_INIT_RESOURCE(resources);

    QIcon appIcon(QStringLiteral(":/icon/icon_256x256.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_128x128.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_64x64.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_48x48.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_32x32.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_24x24.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_16x16.png"));
    app.setWindowIcon(appIcon);

    MdEditor::MainWindow w;
    const auto screenSize = app.primaryScreen()->availableGeometry().size();
    w.resize(qRound((double)screenSize.width() * 0.85), qRound((double)screenSize.height() * 0.85));

    if (!fileName.isEmpty()) {
        w.openFile(fileName);
    }

    if (parser.isSet(view)) {
        w.openInPreviewMode(parser.isSet(all));
    } else if (parser.isSet(all)) {
        w.loadAllLinkedFiles();
    }

    if (parser.isSet(view) && fileName.isEmpty()) {
        return 0;
    }

    w.show();

    return QApplication::exec();
}
