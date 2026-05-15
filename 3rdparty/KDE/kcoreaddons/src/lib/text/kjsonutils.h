/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJSONUTILS_H
#define KJSONUTILS_H

#include "kcoreaddons_export.h"

#include <QJsonValue>

class QJsonObject;
class QString;

namespace KJsonUtils
{
/*!
 * \namespace KJsonUtils
 * \inmodule KCoreAddons
 *
 * Reads a value from \a jo but unlike QJsonObject::value() it allows different entries for each locale
 *
 * This is done by appending the locale identifier in brackets to the key (e.g. "[de_DE]" or "[es]")
 *
 * When looking for a key "foo" with German (Germany) locale we will first attempt to read "foo[de_DE]",
 * if that does not exist "foo[de]", finally falling back to "foo" if that also doesn't exist.
 *
 * Returns the translated value for \a key from \a jo or \a defaultValue if \a key was not found
 * \since 5.88
 */
KCOREADDONS_EXPORT QJsonValue readTranslatedValue(const QJsonObject &jo, const QString &key, const QJsonValue &defaultValue = QJsonValue());

/*!
 * Returns the translated value of \a key from \a jo as a string or \a defaultValue if \a key was not found
 * or the value for \a key is not of type string
 *
 * \since 5.88
 */
KCOREADDONS_EXPORT QString readTranslatedString(const QJsonObject &jo, const QString &key, const QString &defaultValue = QString());
}

#endif // KJSONUTILS_H
