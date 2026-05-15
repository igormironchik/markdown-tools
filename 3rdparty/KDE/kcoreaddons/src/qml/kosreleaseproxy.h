// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>
#include <QQmlEngine>

#include "kosrelease.h"

// TODO KF7: remove final on KOSRelease class declaration so we can more easily extend it without having to proxy all functions

/*!
 * \qmltype KOSRelease
 * \inqmlmodule org.kde.coreaddons
 * \nativetype KOSRelease
 * \since 6.11
 *
 * \brief Parses /etc/os-release files.
 *
 * This is a QML singleton
 *
 * \qml
 * import QtQuick
 * import org.kde.coreaddons
 *
 * Item {
 *     Component.onCompleted: console.log(KOSRelease.name)
 * }
 * \endqml
 */
class KOSReleaseProxy : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(KOSRelease)
    QML_SINGLETON

    /*!
     * \qmlproperty string KOSRelease::name
     */
    Q_PROPERTY(QString name READ name CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::version
     */
    Q_PROPERTY(QString version READ version CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::id
     */
    Q_PROPERTY(QString id READ id CONSTANT)

    /*!
     * \qmlproperty list<string> KOSRelease::idLike
     */
    Q_PROPERTY(QStringList idLike READ idLike CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::versionCodename
     */
    Q_PROPERTY(QString versionCodename READ versionCodename CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::versionId
     */
    Q_PROPERTY(QString versionId READ versionId CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::prettyName
     */
    Q_PROPERTY(QString prettyName READ prettyName CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::ansiColor
     */
    Q_PROPERTY(QString ansiColor READ ansiColor CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::cpeName
     */
    Q_PROPERTY(QString cpeName READ cpeName CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::homeUrl
     */
    Q_PROPERTY(QString homeUrl READ homeUrl CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::documentationUrl
     */
    Q_PROPERTY(QString documentationUrl READ documentationUrl CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::supportUrl
     */
    Q_PROPERTY(QString supportUrl READ supportUrl CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::bugReportUrl
     */
    Q_PROPERTY(QString bugReportUrl READ bugReportUrl CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::privacyPolicyUrl
     */
    Q_PROPERTY(QString privacyPolicyUrl READ privacyPolicyUrl CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::buildId
     */
    Q_PROPERTY(QString buildId READ buildId CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::variant
     */
    Q_PROPERTY(QString variant READ variant CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::variantId
     */
    Q_PROPERTY(QString variantId READ variantId CONSTANT)

    /*!
     * \qmlproperty string KOSRelease::logo
     */
    Q_PROPERTY(QString logo READ logo CONSTANT)

public:
    using QObject::QObject;

    [[nodiscard]] QString name() const;
    [[nodiscard]] QString version() const;
    [[nodiscard]] QString id() const;
    [[nodiscard]] QStringList idLike() const;
    [[nodiscard]] QString versionCodename() const;
    [[nodiscard]] QString versionId() const;
    [[nodiscard]] QString prettyName() const;
    [[nodiscard]] QString ansiColor() const;
    [[nodiscard]] QString cpeName() const;
    [[nodiscard]] QString homeUrl() const;
    [[nodiscard]] QString documentationUrl() const;
    [[nodiscard]] QString supportUrl() const;
    [[nodiscard]] QString bugReportUrl() const;
    [[nodiscard]] QString privacyPolicyUrl() const;
    [[nodiscard]] QString buildId() const;
    [[nodiscard]] QString variant() const;
    [[nodiscard]] QString variantId() const;
    [[nodiscard]] QString logo() const;

    /*!
     * \qmlmethod list<string> KOSRelease::extraKeys
     */
    Q_INVOKABLE [[nodiscard]] QStringList extraKeys() const;

    /*!
     * \qmlmethod string KOSRelease::extraValue(string key)
     */
    Q_INVOKABLE [[nodiscard]] QString extraValue(const QString &key) const;

private:
    KOSRelease os;
};
