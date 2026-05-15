/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KLocalizedString>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLibraryInfo>

using namespace Qt::Literals;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    KLocalizedString s; // bypass --as-needed linking

    // verify qttranslations is installed, skip test otherwise
    if (!QFile::exists(QLibraryInfo::path(QLibraryInfo::TranslationsPath) + "/qtbase_fr.qm"_L1)) {
        qWarning("qttranslations not installed, skipping test");
        return 0;
    }

    const auto context = QCoreApplication::arguments().at(1);
    const auto sourceText = QCoreApplication::arguments().at(2);
    const auto expectedText = QCoreApplication::arguments().at(3);
    const auto translatedText = QCoreApplication::translate(context.toUtf8().constData(), sourceText.toUtf8().constData());
    if (expectedText != translatedText) {
        qWarning() << "FAIL! Got" << translatedText << " Expected:" << expectedText;
        return -1;
    }

    return 0;
}
