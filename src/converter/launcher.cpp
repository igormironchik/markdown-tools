/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStringList>

#ifdef Q_OS_WIN
// OpenSSL include.
#include <openssl/opensslconf.h>
#endif // Q_OS_WIN

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Launcher of application required OpenSSL."));
    parser.addHelpOption();

    QCommandLineOption executable(QStringLiteral("exe"), QStringLiteral("Executable to launch."), QStringLiteral("exe"));
    parser.addOption(executable);

    QCommandLineOption mode(QStringLiteral("mode"),
                            QStringLiteral("Launch mode: detached | notdetached "),
                            QStringLiteral("mode"),
                            QStringLiteral("notdetached"));
    parser.addOption(mode);

    QCommandLineOption arg(QStringLiteral("arg"), QStringLiteral("Argument to pass to launced process."),
                           QStringLiteral("arg"));
    parser.addOption(arg);

    QCommandLineOption modules(QStringLiteral("modules"),
                               QStringLiteral("Folder with OpenSSL modules."),
                               QStringLiteral("modules"),
                               QStringLiteral("./ossl-modules"));
    parser.addOption(modules);

    QCommandLineOption engines(QStringLiteral("engines"),
                               QStringLiteral("Folder with OpenSSL engines."),
                               QStringLiteral("engines"),
                               QStringLiteral("./engines-3"));
    parser.addOption(engines);

    parser.process(app);

    const auto modulesValue = parser.value(modules);
    const auto enginesValue = parser.value(engines);

#ifdef Q_OS_WIN
#if OPENSSL_VERSION_MAJOR >= 3
    qputenv("OPENSSL_MODULES", modulesValue.toLocal8Bit());
    qputenv("OPENSSL_ENGINES", enginesValue.toLocal8Bit());
#endif
#endif // Q_OS_WIN

    const auto modeValue = parser.value(mode);

    if (parser.isSet(executable)) {
        const auto exeValue = parser.value(executable);
        const QFileInfo info(exeValue);

        if (!info.exists()) {
            qDebug() << "Executable is not exist.";

            return 1;
        }

        QProcess p;
        p.setWorkingDirectory(info.absolutePath());
        p.setProgram(info.absoluteFilePath());

        if (parser.isSet(arg)) {
            p.setArguments(QStringList() << parser.value(arg));
        }

        if (modeValue == QStringLiteral("detached")) {
            if (p.startDetached()) {
                return 0;
            } else {
                qDebug() << "Unable to start detached process." << p.errorString();

                return 1;
            }
        } else {
            p.start();

            if (p.waitForFinished(15 * 60 * 1000)) {
                return p.exitCode();
            } else {
                qDebug() << "Process is not finished." << p.errorString();

                return 1;
            }
        }
    } else {
        qDebug() << "Define executable to launch.";

        return 1;
    }
}
