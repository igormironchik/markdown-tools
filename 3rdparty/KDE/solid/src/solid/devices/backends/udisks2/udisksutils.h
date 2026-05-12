/*
    SPDX-FileCopyrightText: 2024 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSUTILS_H
#define UDISKSUTILS_H

#include <QByteArray>
#include <QVariant>
#include <QVariantMap>

namespace Solid
{
namespace Backends
{
namespace UDisks2
{

namespace Utils
{

QByteArray sanitizeValue(const QByteArray &value);
QVariant sanitizeValue(const QVariant &value);
QVariantMap sanitizeValue(const QVariantMap &map);

} // Utils

} // UDisks2
} // Backends
} // Solid

#endif // UDISKSUTILS_H
