/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "main_window.h"

// Qt include.
#include <QApplication>
#include <QCommandLineParser>
#include <QString>
#include <QTranslator>

// shared include.
#include "utils.h"

// MicroTeX include.
#include <latex.h>

#ifdef MD_BREEZE
#include <KIconTheme>
#endif

#if defined(Q_OS_WIN) && defined(MD_BREEZE)
#include <KColorSchemeManager>
#endif

using namespace MdPdf;

int main(int argc,
         char **argv)
{
#ifdef MD_BREEZE
    KIconTheme::initTheme();
#endif

    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("Igor Mironchik"));
    app.setOrganizationDomain(QStringLiteral("github.com/igormironchik"));
    app.setApplicationName(QStringLiteral("Markdown Converter"));

    initTheme(app);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Markdown converter to PDF."));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("markdown"), QStringLiteral("Markdown file to open."));

    parser.process(app);

    const auto args = parser.positionalArguments();

    const auto fileName = (args.isEmpty() ? QString() : args.at(0));

    QIcon appIcon(QStringLiteral(":/pics/icon_256x256.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_128x128.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_64x64.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_48x48.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_32x32.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_24x24.png"));
    appIcon.addFile(QStringLiteral(":/pics/icon_16x16.png"));
    app.setWindowIcon(appIcon);

    tex::LaTeX::init(":/res");

    initSharedResources();

    QTranslator appTranslator;
    const auto locale = QLocale::system();

    if (!hasEnglish(locale.uiLanguages())) {
        if (appTranslator.load(locale, QStringLiteral("md_"), QString(), QStringLiteral(":/tr/"))) {
            QApplication::installTranslator(&appTranslator);
        }
    }

    MainWindow w;
    w.show();

    if (!fileName.isEmpty()) {
        w.setMarkdownFile(fileName);
    }

    return app.exec();
}
