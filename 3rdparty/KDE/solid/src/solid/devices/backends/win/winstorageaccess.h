/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WINSTORAGEACCESS_H
#define WINSTORAGEACCESS_H

#include <solid/devices/ifaces/storageaccess.h>

#include "windevice.h"
#include "wininterface.h"

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinStorageAccess : public WinInterface, public virtual Solid::Ifaces::StorageAccess
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageAccess)
public:
    WinStorageAccess(WinDevice *device);
    ~WinStorageAccess();

    bool isAccessible() const override;

    QString filePath() const override;

    bool isIgnored() const override;

    bool isEncrypted() const override;

    bool setup() override;

    bool teardown() override;

Q_SIGNALS:
    void accessibilityChanged(bool accessible, const QString &udi) override;

    void setupDone(Solid::ErrorType error, QVariant resultData, const QString &udi) override;

    void teardownDone(Solid::ErrorType error, QVariant resultData, const QString &udi) override;

    void setupRequested(const QString &udi) override;

    void teardownRequested(const QString &udi) override;

    void checkRequested(const QString &udi) override;

    void checkDone(Solid::ErrorType error, QVariant errorData, const QString &udi) override;

    void repairRequested(const QString &udi) override;

    void repairDone(Solid::ErrorType error, QVariant errorData, const QString &udi) override;
};
}
}
}

#endif // WINSTORAGEACCESS_H
