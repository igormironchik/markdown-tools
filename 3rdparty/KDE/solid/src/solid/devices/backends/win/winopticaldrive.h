/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WINOPTICALDRIVE_H
#define WINOPTICALDRIVE_H

#include <solid/devices/ifaces/opticaldrive.h>

#include "winstoragedrive.h"

struct _FEATURE_DATA_PROFILE_LIST_EX;
typedef _FEATURE_DATA_PROFILE_LIST_EX FEATURE_DATA_PROFILE_LIST_EX;

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinOpticalDrive : public WinStorageDrive, virtual public Solid::Ifaces::OpticalDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDrive)
public:
    WinOpticalDrive(WinDevice *device);
    ~WinOpticalDrive() override;

    Solid::OpticalDrive::MediumTypes supportedMedia() const override;

    int readSpeed() const override;

    int writeSpeed() const override;

    QList<int> writeSpeeds() const override;

    bool eject() override;

Q_SIGNALS:
    void ejectPressed(const QString &udi) override;

    void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi) override;

    void ejectRequested(const QString &udi) override;

private:
    Solid::OpticalDrive::MediumTypes m_supportedTypes;
};

class MediaProfiles // TODO: cleanup
{
public:
    MediaProfiles();
    ulong profile;
    Solid::OpticalDrive::MediumTypes type;
    QString name;
    bool active;

    static QMap<ulong, MediaProfiles> profiles(const QString &drive);

private:
    MediaProfiles(ulong profile, Solid::OpticalDrive::MediumTypes type, QString name = QString());

    MediaProfiles(FEATURE_DATA_PROFILE_LIST_EX *feature);
    bool isNull();

    static const MediaProfiles getProfile(ulong id);
};
}
}
}

#endif // WINOPTICALDRIVE_H
