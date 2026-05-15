/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include <QObject>

#include <memory>

#include <kdecoration3/private/kdecoration3_private_export.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KDecoration3 API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QRect;

namespace KDecoration3
{
class Decoration;
class DecorationSettings;
class DecorationSettingsPrivate;
class DecoratedWindow;
class DecoratedWindowPrivate;

class KDECORATIONS_PRIVATE_EXPORT DecorationBridge : public QObject
{
    Q_OBJECT
public:
    ~DecorationBridge() override;

    virtual std::unique_ptr<DecoratedWindowPrivate> createClient(DecoratedWindow *client, Decoration *decoration) = 0;
    virtual std::unique_ptr<DecorationSettingsPrivate> settings(DecorationSettings *parent) = 0;

protected:
    explicit DecorationBridge(QObject *parent = nullptr);
};

} // namespace

Q_DECLARE_METATYPE(KDecoration3::DecorationBridge *)
