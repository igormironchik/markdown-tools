// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#pragma once

#include <KSystemInhibitor>

#include <QWindow>
#include <qqml.h>

/*!
    \qmltype SystemInhibitor
    \since 6.23
    \inqmlmodule org.kde.guiaddons
    \brief Inhibit system actions such as logout, suspend, etc.

    \qml
    readonly property SystemInhibitor inhibitor: SystemInhibitor {
        window: ApplicationWindow.window
        types: SystemInhibitor.Suspend | SystemInhibitor.Idle
        enabled: true
        reason: KI18n.i18nc("inhibition reason", "Playing a video")
    }
    \endqml
*/
class SystemInhibitor : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT
    // Grab the enums from KSystemInhibitor into our namespace for consistent QML API - so we can address it as SystemInhibitor.Logout rather than
    // KSystemInhibitor.Logout
    QML_EXTENDED_NAMESPACE(KSystemInhibitor)

    /*!
        \qmlproperty Window SystemInhibitor::window
        The window for which to apply the inhibition. May be null in which case possible user queries will not be associated with any window.
    */
    Q_PROPERTY(QWindow *window MEMBER m_window NOTIFY windowChanged)

    /*!
        \qmlproperty enumeration SystemInhibitor::types
        \qmlenumeratorsfrom KSystemInhibitor::Type
        \required
        The types of inhibition to apply. May be a combination of multiple types.
    */
    Q_PROPERTY(KSystemInhibitor::Types types MEMBER m_types NOTIFY typesChanged REQUIRED)

    /*!
        \qmlproperty string SystemInhibitor::reason
        \required
        The user-facing reason why ths inhibition is in place (e.g. "Playing a video").
    */
    Q_PROPERTY(QString reason MEMBER m_reason NOTIFY reasonChanged REQUIRED)

    /*!
        \qmlproperty bool SystemInhibitor::enabled
        Toggle for enabling/disabling the inhibition. Please note that inhibiting is an asynchronous operation and may be delayed or fail, the property always
        reflects the requested state, not the actual state.
    */
    Q_PROPERTY(bool enabled MEMBER m_enabled WRITE setEnabled NOTIFY enabledChanged)
public:
    using QObject::QObject;

    void classBegin() override;
    void componentComplete() override;
    void setEnabled(bool enable);

Q_SIGNALS:
    void windowChanged();
    void typesChanged();
    void reasonChanged();
    void enabledChanged();

private:
    void apply();

    bool m_ready = false;

    QWindow *m_window = nullptr;
    KSystemInhibitor::Types m_types;
    QString m_reason;
    bool m_enabled = false;

    std::optional<KSystemInhibitor> m_inhibitor;
};
