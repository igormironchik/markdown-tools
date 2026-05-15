/*
    SPDX-FileCopyrightText: 2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksutils.h"

#include <QDebug>

using namespace Solid::Backends::UDisks2;

QByteArray Utils::sanitizeValue(const QByteArray &value)
{
    QByteArray blob = value;
    // UDisks2 sends us null terminated strings, make sure to strip the extranous \0 in favor of the implicit \0.
    // Otherwise comparision becomes unnecessarily complicated because 'foo\0' != 'foo'. QByteArrays are implicitly
    // terminated already.
    while (blob.endsWith('\0')) {
        blob.chop(1);
    }
    return blob;
}

QVariant Utils::sanitizeValue(const QVariant &value)
{
    if (value.userType() == QMetaType::QByteArray) {
        return sanitizeValue(value.toByteArray());
    } else {
        return value;
    }
}

QVariantMap Utils::sanitizeValue(const QVariantMap &map)
{
    QVariantMap ret = map;
    for (QVariant &value : ret) {
        value = sanitizeValue(value);
    }
    return ret;
}
