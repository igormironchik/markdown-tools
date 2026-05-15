/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2021,2023 Ingo Klöcker <kloecker@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ki18n_logging.h"

#include <QCoreApplication>
#include <QFile>
#include <QLibraryInfo>
#include <QLocale>
#include <QThread>
#include <QTranslator>

#include <memory>

//
// NOTE when changing anything in here, also check whether ECMQmLoader.cpp.in in ECM
// needs to be changed as well!
//

using namespace Qt::Literals;

[[nodiscard]] static QString translationsPath()
{
#ifndef Q_OS_ANDROID
    return QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    return u"assets://translations/"_s;
#endif
}

static bool loadCatalog(QStringView catalog, QStringView language)
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    const QString fullPath = translationsPath() + '/'_L1 + catalog + language + ".qm"_L1;
    if (!QFile::exists(fullPath)) {
        return false;
    }
    auto translator = std::make_unique<QTranslator>(QCoreApplication::instance());
    if (!translator->load(fullPath)) {
        qCDebug(KI18N) << "Loading catalog failed:" << fullPath;
        return false;
    }
    QCoreApplication::instance()->installTranslator(translator.release());
    return true;
}

// load global Qt translation, needed in KDE e.g. by lots of builtin dialogs (QColorDialog, QFontDialog) that we use
static bool loadTranslation(QStringView language)
{
    // first, try to load the qt_ meta catalog
    if (loadCatalog(u"qt_", language)) {
        return true;
    }
    // if loading the meta catalog failed, then try loading the catalogs
    // it depends on, i.e. qtbase, qtmultimedia separately
    const auto catalogs = {
        u"qtbase_",
        u"qtmultimedia_",
    };
    bool success = false;
    for (const auto &catalog : catalogs) {
        success |= loadCatalog(catalog, language);
    }
    return success;
}

static QStringList getSystemLanguages()
{
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    // On Windows and Apple OSs, we cannot use QLocale::system() if an application-specific
    // language was set by kxmlgui because Qt ignores LANGUAGE on Windows and Apple OSs.
    // The following code is a simplified variant of QSystemLocale::fallbackUiLocale()
    // (in qlocale_unix.cpp) ignoring LC_ALL, LC_MESSAGES, and LANG.
    if (const auto languages = qEnvironmentVariable("LANGUAGE").split(':'_L1, Qt::SkipEmptyParts); !languages.isEmpty()) {
        return languages;
    }
#endif
    return QLocale::system().uiLanguages();
}

static void load()
{
    // The way Qt translation system handles plural forms makes it necessary to
    // have a translation file which contains only plural forms for `en`. That's
    // why we load the `en` translation unconditionally, then load the
    // translation for the current locale to overload it.
    QMetaObject::invokeMethod(QCoreApplication::instance(), [] {
        loadCatalog(u"qt_", u"en");

        auto languages = getSystemLanguages();
        for (qsizetype i = 0; i < languages.size(); ++i) {
            // normalize into the format used in Qt catalog suffixes
            languages[i].replace('-'_L1, '_'_L1);
            // make sure we always also have the generic language variant
            // depending on the platform that might not be in uiLanguages by default
            // insert that after the last country-specific entry for the same language
            const auto idx = languages[i].indexOf('_'_L1);
            if (idx > 0) {
                const QString genericLang = languages[i].left(idx);
                qsizetype j = i + 1;
                for (; j < languages.size(); ++j) {
                    if (!languages[j].startsWith(genericLang)) {
                        break;
                    }
                }
                if (languages[j - 1] != genericLang) {
                    languages.insert(j, genericLang);
                }
            }
        }
        languages.removeDuplicates();
        for (const auto &language : std::as_const(languages)) {
            if (language == "en"_L1 || loadTranslation(language)) {
                break;
            }
        }
    });
}

Q_COREAPP_STARTUP_FUNCTION(load)
