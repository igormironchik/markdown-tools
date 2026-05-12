// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#pragma once

#include <kguiaddons_export.h>

#include <QObject>

class QWindow;

/*!
    \class KSystemInhibitor
    \inmodule KGuiAddons
    \since 6.23

    \brief Inhibit system actions such as logout, suspend, etc.
*/
class KGUIADDONS_EXPORT KSystemInhibitor : public QObject
{
    Q_OBJECT
public:
    /*!
        Inhibition types.

        \value Logout Inhibit logout/shutdown
        \value UserSwitch Inhibit user switching
        \value Suspend Inhibit suspend/hibernate
        \value Idle Inhibit idle actions (e.g. screen locking)
    */
    enum class Type {
        Logout = 1,
        UserSwitch = 2,
        Suspend = 4,
        Idle = 8
    };
    Q_ENUM(Type)
    Q_DECLARE_FLAGS(Types, Type)
    Q_FLAG(Types)

    /*!
        Claim inhibition.

        \a reason is the user-visible reason for the inhibition.

        \a types is a combination of inhibition types.

        \a window is the window for which to apply the inhibition. May be null in which case possible user queries will not be associated with any window.

        \a parent is the parent QObject.
    */
    KSystemInhibitor(const QString &reason, Types types, QWindow *window, QObject *parent = nullptr);

    /*!
        Release inhibition
    */
    ~KSystemInhibitor() override;

    Q_DISABLE_COPY_MOVE(KSystemInhibitor)

private:
    std::unique_ptr<class KSystemInhibitorPrivate> const d;
};
