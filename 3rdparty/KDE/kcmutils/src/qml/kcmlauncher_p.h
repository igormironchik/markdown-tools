/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCMSHELL_H
#define KCMSHELL_H

#include <QObject>
#include <QQmlEngine>

/*!
 * \qmltype KCMLauncher
 * \inqmlmodule org.kde.kcmutils
 *
 * This is a QML singleton.
 */
class KCMLauncher : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public Q_SLOTS:
    /*!
     * \qmlmethod void KCMLauncher::open(list<string> names) const
     */
    void open(const QStringList &names) const;

    /*!
     * \qmlmethod void KCMLauncher::openSystemSettings(string name, list<string> args = []) const
     *
     * Opens the specified module in System Settings. Only a single KCM name may
     * be provided.
     *
     * \code
     * import QtQuick.Controls as QQC2
     * import org.kde.kcmutils as KCMUtils
     *
     * QQC2.Button {
     *     onClicked: KCMUtils.KCMLauncher.openSystemSettings("kcm_kscreen")
     * }
     * \endcode
     *
     * \a name A single kcm name to open in System Settings. Opening multiple
     * KCMs using this function is not supported; to do that, use KCMLauncher.open().
     * \a args Additional arguments to pass to the module.
     *
     * \since 5.71
     */
    void openSystemSettings(const QString &name, const QStringList &args = QStringList()) const;

    /*!
     * \qmlmethod void KCMLauncher::openInfoCenter(string name) const
     *
     * Opens the specified module in InfCenter. Only a single KCM name may
     * be provided.
     *
     * \code
     * import QtQuick.Controls as QQC2
     * import org.kde.kcmutils as KCMUtils
     * QQC2.Button {
     *     onClicked: KCMUtils.KCMLauncher.openInfoCenter("kcm_energy")
     * }
     * \endcode
     *
     * \a name A single kcm name to open in Info Center. Opening multiple
     * KCMs using this function is not supported; to do that, use KCMLauncher.open().
     *
     * \since 5.71
     */
    void openInfoCenter(const QString &name) const;
};

#endif // KCMSHELL_H
