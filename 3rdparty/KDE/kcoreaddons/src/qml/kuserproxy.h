/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KUSERPROXY_H
#define KUSERPROXY_H

#include <QObject>
#include <QQmlEngine>
#include <QUrl>

#include <KDirWatch>
#include <KUser>

/*!
 * \qmltype KUser
 * \inqmlmodule org.kde.coreaddons
 *
 * KUser is an object allowing
 * read-only access to the user's name, os and version and the configured
 * user image. This object can be used to personalize user interfaces.
 *
 * Example usage:
 * \code
 *  import org.kde.coreaddons as KCoreAddons
 *  [...]
 *
 *  Item {
 *      [...]
 *      KCoreAddons.KUser {
 *          id: kuser
 *      }
 *
 *      Image {
 *          id: faceIcon
 *          source: kuser.faceIconUrl
 *          [...]
 *      }
 *
 *      Text {
 *          text: kuser.fullName
 *          [...]
 *      }
 *  }
 *  \endcode
 *
 * \brief User provides read-only access to the user's personal information.
 * \sa KUser
 */
class KUserProxy : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(KUser)

    /*!
     * \qmlproperty string KUser::fullName
     * The user's full name
     */
    Q_PROPERTY(QString fullName READ fullName NOTIFY nameChanged)

    /*!
     * \qmlproperty string KUser::loginName
     * The user's login name
     */
    Q_PROPERTY(QString loginName READ loginName NOTIFY nameChanged)

    /*!
     * \qmlproperty url KUser::faceIconUrl
     * The url of the user's configured image (including file:/)
     */
    Q_PROPERTY(QUrl faceIconUrl READ faceIconUrl NOTIFY faceIconUrlChanged)

    /*!
     * \qmlproperty string KUser::os
     * The pretty name indicating operating system and version
     */
    Q_PROPERTY(QString os READ os CONSTANT)

    /*!
     * \qmlproperty string KUser::host
     * The user's the system's hostname
     */
    Q_PROPERTY(QString host READ host CONSTANT)

public:
    KUserProxy(QObject *parent = nullptr);
    ~KUserProxy() override;

    QString fullName() const;

    QString loginName() const;

    QUrl faceIconUrl() const;

    QString os();

    QString host() const;

Q_SIGNALS:
    void nameChanged();

    void faceIconUrlChanged();

private:
    void update(const QString &path);
    KDirWatch m_dirWatch;
    KUser m_user;
    QString m_os;
    bool m_temporaryEmptyFaceIconPath;
};

#endif // KUSERPROXY_H
