/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include <KAboutData>
#include <KTextToHTML>

class KTextToHtmlWrapper
{
    Q_GADGET
    Q_INVOKABLE [[nodiscard]] static QString
    convertToHtml(const QString &plainText, const KTextToHTML::Options &options, int maxUrlLen = 4096, int maxAddressLen = 255)
    {
        return KTextToHTML::convertToHtml(plainText, options, maxUrlLen, maxAddressLen);
    }
};

class KCoreAddonsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    /*!
     * \qmltype AboutData
     * \inqmlmodule org.kde.coreaddons
     * \brief This type exposes project metadata set with KAboutData from the C++ side.
     *
     * This singleton exposes the following read-only properties:
     *
     * \list
     * \li KAboutData::displayName as \c AboutData.displayName
     * \li KAboutData::productName as \c AboutData.productName
     * \li KAboutData::componentName as \c AboutData.componentName
     * \li KAboutData::programLogo as \c AboutData.programLogo
     * \li KAboutData::shortDescription as \c AboutData.shortDescription
     * \li KAboutData::homepage as \c AboutData.homepage
     * \li KAboutData::bugAddress as \c AboutData.bugAddress
     * \li KAboutData::version as \c AboutData.version
     * \li KAboutData::otherText as \c AboutData.otherText
     * \li KAboutData::authors as \c AboutData.authors
     * \li KAboutData::credits as \c AboutData.credits
     * \li KAboutData::translators as \c AboutData.translators
     * \li KAboutData::components as \c AboutData.components
     * \li KAboutData::licenses as \c AboutData.licenses
     * \li KAboutData::copyrightStatement as \c AboutData.copyrightStatement
     * \li KAboutData::desktopFileName as \c AboutData.desktopFileName
     * \endlist
     *
     * The KAboutData metadata is expected to be set in C++ (or equivalent KAboutData bindings).
     *
     * \sa KAboutData
     */
    void registerTypes(const char *uri) override
    {
        qmlRegisterSingletonType(uri, 1, 0, "AboutData", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
            return engine->toScriptValue(KAboutData::applicationData());
        });
        qmlRegisterSingletonType(uri, 1, 0, "KTextToHTML", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
            return engine->toScriptValue(KTextToHtmlWrapper());
        });
    }
};

#include "kcoreaddonsplugin.moc"
