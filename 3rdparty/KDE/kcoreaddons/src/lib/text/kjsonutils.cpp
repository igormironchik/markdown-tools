/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kjsonutils.h"

#include <QJsonObject>

static QString getDefaultLocaleName()
{
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    if (QLocale() == QLocale::system()) {
        // If the default locale hasn't been changed then
        // On Windows and Apple OSs, we cannot use QLocale::system() if an application-specific
        // language was set by kxmlgui because Qt ignores LANGUAGE on Windows and Apple OSs.
        if (const auto firstLanguage = qEnvironmentVariable("LANGUAGE").section(u':', 0, 0, QString::SectionSkipEmpty); !firstLanguage.isEmpty()) {
            return firstLanguage;
        }
        // Also prefer the configured display language over the system language
        if (const auto languages = QLocale::system().uiLanguages(); !languages.isEmpty()) {
            // uiLanguages() uses dashes as separator, but KConfig assumes underscores
            return languages.value(0).replace(u'-', u'_');
        }
    }
#endif
    return QLocale().name();
}

QJsonValue KJsonUtils::readTranslatedValue(const QJsonObject &jo, const QString &key, const QJsonValue &defaultValue)
{
    QString languageWithCountry = getDefaultLocaleName();
    auto it = jo.constFind(key + QLatin1Char('[') + languageWithCountry + QLatin1Char(']'));
    if (it != jo.constEnd()) {
        return it.value();
    }
    const QStringView language = QStringView(languageWithCountry).mid(0, languageWithCountry.indexOf(QLatin1Char('_')));
    it = jo.constFind(key + QLatin1Char('[') + language + QLatin1Char(']'));
    if (it != jo.constEnd()) {
        return it.value();
    }
    // no translated value found -> check key
    it = jo.constFind(key);
    if (it != jo.constEnd()) {
        return jo.value(key);
    }
    return defaultValue;
}

QString KJsonUtils::readTranslatedString(const QJsonObject &jo, const QString &key, const QString &defaultValue)
{
    return KJsonUtils::readTranslatedValue(jo, key, defaultValue).toString(defaultValue);
}
