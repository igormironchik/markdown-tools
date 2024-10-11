/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "main_window.hpp"

// Qt include.
#include <QApplication>
#include <QCommandLineParser>
#include <QString>

// shared include.
#include "utils.hpp"

using namespace MdPdf;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Markdown converter to PDF."));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("markdown"), QStringLiteral("Markdown file to open."));

    parser.process(app);

    const auto args = parser.positionalArguments();

    const auto fileName = (args.isEmpty() ? QString() : args.at(0));

    QIcon appIcon(QStringLiteral(":/icon/icon_256x256.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_128x128.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_64x64.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_48x48.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_32x32.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_24x24.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_16x16.png"));
    app.setWindowIcon(appIcon);

    initSharedResources();

    MainWindow w;
    w.show();

    if (!fileName.isEmpty()) {
        w.setMarkdownFile(fileName);
    }

    return app.exec();
}
