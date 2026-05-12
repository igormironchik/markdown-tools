/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef FORMATS_H
#define FORMATS_H

#include <KFormat>
#include <QObject>
#include <QQmlEngine>

namespace FormatTypes
{
Q_NAMESPACE
QML_NAMED_ELEMENT(FormatTypes)
QML_FOREIGN_NAMESPACE(KFormat)
}

/*!
 * \qmltype Format
 * \inqmlmodule org.kde.coreaddons
 *
 * \brief Provides support for formatting numbers and datetimes in
 * formats that are not supported by QLocale.
 *
 * \sa KFormat
 */
class Formats : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Format)
    QML_SINGLETON

public:
    /*!
     * \qmlmethod string Format::formatByteSize(double size, int precision = 1)
     *
     * Converts \a size from bytes to the appropriate string representation
     */
    Q_INVOKABLE QString formatByteSize(double size, int precision = 1) const;

    /*!
     * \qmlmethod string Format::formatDuration(int msecs, KFormat::DurationFormatOptions options = KFormat::DefaultDuration)
     *
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent, e.g. 1:23:45
     */
    Q_INVOKABLE QString formatDuration(quint64 msecs, KFormat::DurationFormatOptions options = KFormat::DefaultDuration) const;

    /*!
     * \qmlmethod string Format::formatDecimalDuration(int msecs, int decimalPlaces = 2)
     *
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent to the requested decimal places.
     *
     * e.g. given formatDuration(60000), returns "1.0 minutes"
     */
    Q_INVOKABLE QString formatDecimalDuration(quint64 msecs, int decimalPlaces = 2) const;

    /*!
     * \qmlmethod string Format::formatSpelloutDuration(int msecs)
     *
     * Given a number of milliseconds, converts that to a spell-out string containing
     * the localized equivalent.
     *
     * e.g. given formatSpelloutDuration(60001) returns "1 minute"
     *      given formatSpelloutDuration(62005) returns "1 minute and 2 seconds"
     *      given formatSpelloutDuration(90060000) returns "1 day and 1 hour"
     *
     * Units not interesting to the user, for example seconds or minutes when the first
     * unit is day, are not returned because they are irrelevant. The same applies for
     * seconds when the first unit is hour.
     *
     */
    Q_INVOKABLE QString formatSpelloutDuration(quint64 msecs) const;

    /*!
     * \qmlmethod string Format::formatRelativeDate(date date, format)
     *
     * Returns a string formatted to a relative date style.
     *
     * If the date falls within 2 days before or after the current date
     * then a relative date string will be returned, such as:
     * \list
     * \li Yesterday
     * \li Today
     * \li Tomorrow
     * \li Two days ago
     * \li In Two days
     * \endlist
     * If the date falls outside this period then the format is used
     */
    Q_INVOKABLE QString formatRelativeDate(const QDate &date, QLocale::FormatType format) const;

    /*!
     * \qmlmethod string Format::formatRelativeDate(QDateTime date, format)
     *
     * Returns a string formatted to a relative datetime style.
     *
     * If the dateTime falls within 2 days before or after the current date
     * then a relative date string will be returned, such as:
     * \list
     * \li Yesterday, 3:00pm
     * \li Today, 3:00pm
     * \li Tomorrow, 3:00pm
     * \li Two days ago, 3:00pm
     * \li In Two days, 3:00pm
     * \endlist
     *
     * If the datetime falls outside this period then the format is used
     */
    Q_INVOKABLE QString formatRelativeDateTime(const QDateTime &dateTime, QLocale::FormatType format) const;

    /*!
     * \qmlmethod string Format::formatTime(var object, string propertyName, int format, int options)
     *
     * Applies time formatting to the QDateTime value found in the property \a propertyName in object \a object.
     * This indirection is necessary to avoid the QDateTime value to ever be exposed to QML's JS runtime,
     * which would destroy its timezone information.
     * The formatting itself is done via KFormat::formatTime, see that for \a format and \a options.
     *
     * \since 6.16
     */
    Q_INVOKABLE [[nodiscard]] QString formatTime(const QVariant &obj,
                                                 const QString &propertyName,
                                                 QLocale::FormatType format = QLocale::ShortFormat,
                                                 KFormat::TimeFormatOptions options = KFormat::DoNotAddTimeZone) const;

    /*!
     * \qmlmethod string Format::formatDistance(real distance, enumeration options)
     *
     * Formats a distance value given in meters in appropriate units for
     * displaying.
     *
     * Unless explicitly forced to metric units this uses units approrpiate for
     * the current locale.
     */
    Q_INVOKABLE [[nodiscard]] QString formatDistance(double distance, KFormat::DistanceFormatOptions options = KFormat::LocaleDistanceUnits) const;

private:
    const KFormat m_format;
};

#endif
