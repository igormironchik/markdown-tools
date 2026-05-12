/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2013 Alex Merry <alex.merry@kdemail.net>
    SPDX-FileCopyrightText: 2013 John Layt <jlayt@kde.org>
    SPDX-FileCopyrightText: 2010 Michael Leupold <lemma@confuego.org>
    SPDX-FileCopyrightText: 2009 Michael Pyne <mpyne@kde.org>
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KFORMAT_H
#define KFORMAT_H

#include <kcoreaddons_export.h>

#include <QLocale>
#include <QSharedPointer>
#include <QString>

class QDate;
class QDateTime;

class KFormatPrivate;

/*
   The code in this class was copied from the old KLocale and modified
   by John Layt (and also Alex Merry) in the KDELIBS 4 to KDE
   Frameworks 5 transition in 2013.

   Albert Astals Cid is the original author of formatSpelloutDuration()
   originally named KLocale::prettyFormatDuration().

   Michael Pyne is the original author of formatByteSize().

   Michael Leupold is the original author of formatRelativeDate(()
   originally part of KFormat::formatDate().
*/

/*!
 * \class KFormat
 * \inmodule KCoreAddons
 *
 * \brief Class for formatting numbers and datetimes.
 *
 * KFormat provides support for formatting numbers and datetimes in
 * formats that are not supported by QLocale.
 * \since 5.0
 */
class KCOREADDONS_EXPORT KFormat final
{
    Q_GADGET

public:
    /*!
     * These binary units are used in KDE by the formatByteSize()
     * function.
     *
     * NOTE: There are several different units standards:
     * 1) SI  (i.e. metric), powers-of-10.
     * 2) IEC, powers-of-2, with specific units KiB, MiB, etc.
     * 3) JEDEC, powers-of-2, used for solid state memory sizing which
     *    is why you see flash cards labels as e.g. 4GB.  These (ab)use
     *    the metric units.  Although JEDEC only defines KB, MB, GB, if
     *    JEDEC is selected all units will be powers-of-2 with metric
     *    prefixes for clarity in the event of sizes larger than 1024 GB.
     *
     * Although 3 different dialects are possible this enum only uses
     * metric names since adding all 3 different names of essentially the same
     * unit would be pointless.  Use BinaryUnitDialect to control the exact
     * units returned.
     *
     * \sa BinaryUnitDialect
     * \sa formatByteSize
     *
     * \value DefaultBinaryUnits Auto-choose a unit such that the result is in the range [0, 1000 or 1024)
     * \value UnitByte B         1 byte
     * \value UnitKiloByte KiB/KB/kB 1024/1000 bytes
     * \value UnitMegaByte MiB/MB/MB 2^20/10^06 bytes
     * \value UnitGigaByte GiB/GB/GB 2^30/10^09 bytes
     * \value UnitTeraByte TiB/TB/TB 2^40/10^12 bytes
     * \value UnitPetaByte PiB/PB/PB 2^50/10^15 bytes
     * \value UnitExaByte EiB/EB/EB 2^60/10^18 bytes
     * \value UnitZettaByte ZiB/ZB/ZB 2^70/10^21 bytes
     * \value UnitYottaByte YiB/YB/YB 2^80/10^24 bytes
     */
    enum BinarySizeUnits {
        DefaultBinaryUnits = -1,

        // The first real unit must be 0 for the current implementation!
        UnitByte,
        UnitKiloByte,
        UnitMegaByte,
        UnitGigaByte,
        UnitTeraByte,
        UnitPetaByte,
        UnitExaByte,
        UnitZettaByte,
        UnitYottaByte,
        UnitLastUnit = UnitYottaByte,
    };

    /*!
     * These units are used in KDE by the formatValue() function.
     *
     * \sa formatValue
     * \since 5.49
     *
     * \value Other other
     * \value Bit bit
     * \value Byte B
     * \value Meter m
     * \value Hertz Hz
     *
     */
    enum class Unit {
        Other,
        Bit,
        Byte,
        Meter,
        Hertz,
    };

    /*!
     * These prefixes are used in KDE by the formatValue()
     * function.
     *
     * IEC prefixes are only defined for integral units of information, e.g.
     * bits and bytes.
     *
     * \sa BinarySizeUnits
     * \sa formatValue
     * \since 5.49
     *
     * \value AutoAdjust Auto-choose a unit such that the result is in the range [0, 1000 or 1024)
     * \value Yocto --/-/y  10^-24
     * \value Zepto --/-/z  10^-21
     * \value Atto --/-/a  10^-18
     * \value Femto --/-/f  10^-15
     * \value Pico --/-/p  10^-12
     * \value Nano --/-/n  10^-9
     * \value Micro --/-/µ  10^-6
     * \value Milli --/-/m  10^-3
     * \value Centi --/-/c  0.01
     * \value Deci --/-/d  0.1
     * \value Unity  ""      1
     * \value Deca --/-/da 10
     * \value Hecto --/-/h  100
     * \value Kilo Ki/K/k  1024/1000
     * \value Mega Mi/M/M  2^20/10^06
     * \value Giga Gi/G/G  2^30/10^09
     * \value Tera Ti/T/T  2^40/10^12
     * \value Peta Pi/P/P  2^50/10^15
     * \value Exa Ei/E/E  2^60/10^18
     * \value Zetta Zi/Z/Z  2^70/10^21
     * \value Yotta Yi/Y/Y  2^80/10^24
     */
    enum class UnitPrefix {
        AutoAdjust = -128,

        Yocto = 0,
        Zepto,
        Atto,
        Femto,
        Pico,
        Nano,
        Micro,
        Milli,
        Centi,
        Deci,
        Unity,
        Deca,
        Hecto,
        Kilo,
        Mega,
        Giga,
        Tera,
        Peta,
        Exa,
        Zetta,
        Yotta,
    };

    /*!
     * This enum chooses what dialect is used for binary units.
     *
     * Note: Although JEDEC abuses the metric prefixes and can therefore be
     * confusing, it has been used to describe *memory* sizes for quite some time
     * and programs should therefore use either Default, JEDEC, or IEC 60027-2
     * for memory sizes.
     *
     * On the other hand network transmission rates are typically in metric so
     * Default, Metric, or IEC (which is unambiguous) should be chosen.
     *
     * Normally choosing DefaultBinaryDialect is the best option as that uses
     * the user's selection for units.  If the user has not selected a preference,
     * IECBinaryDialect will typically be used.
     *
     * \value DefaultBinaryDialect Used if no specific preference
     * \value IECBinaryDialect KiB, MiB, etc. 2^(10*n)
     * \value JEDECBinaryDialect KB, MB, etc. 2^(10*n)
     * \value MetricBinaryDialect SI Units, kB, MB, etc. 10^(3*n)
     * \omitvalue LastBinaryDialect
     *
     * \sa BinarySizeUnits
     * \sa formatByteSize
     */
    enum BinaryUnitDialect {
        DefaultBinaryDialect = -1,
        IECBinaryDialect,
        JEDECBinaryDialect,
        MetricBinaryDialect,
        LastBinaryDialect = MetricBinaryDialect,
    };

    /*!
     * Format flags for formatDuration()
     *
     * \value DefaultDuration Default formatting in localized 1:23:45 format
     * \value InitialDuration Default formatting in localized 1h23m45s format
     * \value ShowMilliseconds Include milliseconds in format, e.g. 1:23:45.678
     * \value HideSeconds Hide the seconds, e.g. 1:23 or 1h23m, overrides ShowMilliseconds
     * \value FoldHours Fold the hours into the minutes, e.g. 83:45 or 83m45s, overrides HideSeconds
     * \value [since 6.11] AbbreviatedDuration Use abbreviated units (e.g. 1 hr 23 min), as a middle ground between just unit initials (InitialDuration) and
     * fully spelled out units (formatSpelloutDuration). This always includes up to 2 components.
     */
    enum DurationFormatOption {
        DefaultDuration = 0x0,
        InitialDuration = 0x1,
        ShowMilliseconds = 0x2,
        HideSeconds = 0x4,
        FoldHours = 0x8,
        AbbreviatedDuration = 0x10,
    };
    Q_DECLARE_FLAGS(DurationFormatOptions, DurationFormatOption)
    Q_FLAG(DurationFormatOptions)

    /*!
     * Format flags for formatTime()
     *
     * \value DoNotAddTimeZone Equivalent to the QLocale time formatting, does not add any timezone information
     * \value AddTimezoneAbbreviation Adds the (translated) timezone abbreviation when timezone information is present, e.g. "12:23 CET"
     * \value AddTimezoneAbbreviationIfNeeded Adds the (translated) timezone abbreviation when timezone information is present and differs from the current
     * local timezone.
     *
     * \since 6.15
     */
    enum TimeFormatOption {
        DoNotAddTimeZone = 0x0,
        AddTimezoneAbbreviation = 0x1,
        AddTimezoneAbbreviationIfNeeded = 0x2,
    };
    Q_DECLARE_FLAGS(TimeFormatOptions, TimeFormatOption)
    Q_FLAG(TimeFormatOptions)

    /*!
     * Formatting options for formatDistance()
     *
     * \since 6.11
     *
     * \value LocaleDistanceUnits Automatically select metric or imperial units based on the current locale
     * \value MetricDistanceUnits Force the use of metric unites regardless of the current locale
     */
    enum DistanceFormatOption {
        LocaleDistanceUnits = 0x0,
        MetricDistanceUnits = 0x1,
    };
    Q_DECLARE_FLAGS(DistanceFormatOptions, DistanceFormatOption)
    Q_FLAG(DistanceFormatOptions)

    /*!
     * Constructs a KFormat.
     *
     * \a locale the locale to use, defaults to the system locale
     */
    explicit KFormat(const QLocale &locale = QLocale());

    KFormat(const KFormat &other);

    KFormat &operator=(const KFormat &other);

    ~KFormat();

    /*!
     * Converts \a size from bytes to the appropriate string representation
     * using the binary unit dialect \a dialect and the specific units \a units.
     *
     * Example:
     * \code
     * QString metric, iec, jedec, small;
     * metric = formatByteSize(1000, 1, KFormat::MetricBinaryDialect, KFormat::UnitKiloByte);
     * iec    = formatByteSize(1024, 1, KFormat::IECBinaryDialect, KFormat::UnitKiloByte);
     * jedec  = formatByteSize(1024, 1, KFormat::JEDECBinaryDialect, KFormat::UnitKiloByte);
     * small  = formatByteSize(100);
     * // metric == "1.0 kB", iec == "1.0 KiB", jedec == "1.0 KB", small == "100 B"
     * \endcode
     *
     * \a size size in bytes
     *
     * \a precision number of places after the decimal point to use.  KDE uses
     *        1 by default so when in doubt use 1.  Whenever KFormat::UnitByte is used
     *        (either explicitly or autoselected from KFormat::DefaultBinaryUnits),
     *        the fractional part is always omitted.
     *
     * \a dialect binary unit standard to use.  Use DefaultBinaryDialect to
     *        use the localized user selection unless you need to use a specific
     *        unit type (such as displaying a flash memory size in JEDEC).
     *
     * \a units specific unit size to use in result.  Use
     *        DefaultBinaryUnits to automatically select a unit that will return
     *        a sanely-sized number.
     *
     * Returns converted size as a translated string including the units.
     *         E.g. "1.23 KiB", "2 GB" (JEDEC), "4.2 kB" (Metric).
     * \sa BinarySizeUnits
     * \sa BinaryUnitDialect
     */

    QString formatByteSize(double size,
                           int precision = 1,
                           KFormat::BinaryUnitDialect dialect = KFormat::DefaultBinaryDialect,
                           KFormat::BinarySizeUnits units = KFormat::DefaultBinaryUnits) const;

    /*!
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent, e.g. 1:23:45
     *
     * \a msecs Time duration in milliseconds
     *
     * \a options options to use in the duration format
     *
     * Returns converted duration as a string - e.g. "1:23:45" "1h23m"
     */

    QString formatDuration(quint64 msecs, KFormat::DurationFormatOptions options = KFormat::DefaultDuration) const;

    /*!
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent to the requested decimal places.
     *
     * e.g. given formatDuration(60000), returns "1.0 minutes"
     *
     * \a msecs Time duration in milliseconds
     *
     * \a decimalPlaces Decimal places to round off to, defaults to 2
     *
     * Returns converted duration as a string - e.g. "5.5 seconds" "23.0 minutes"
     */

    QString formatDecimalDuration(quint64 msecs, int decimalPlaces = 2) const;

    /*!
     * Given a number of milliseconds, converts that to a spell-out string containing
     * the localized equivalent.
     *
     * e.g. given formatSpelloutDuration(60001) returns "1 minute"
     *      given formatSpelloutDuration(62005) returns "1 minute and 2 seconds"
     *      given formatSpelloutDuration(90060000) returns "1 day and 1 hour"
     *
     * \a msecs Time duration in milliseconds
     *
     * Returns converted duration as a string.
     *         Units not interesting to the user, for example seconds or minutes when the first
     *         unit is day, are not returned because they are irrelevant. The same applies for
     *         seconds when the first unit is hour.
     */
    QString formatSpelloutDuration(quint64 msecs) const;

    /*!
     * Returns a string formatted to a relative date style.
     *
     * If the \a date falls within 2 days before or after the current date
     * then a relative date string will be returned, such as:
     * \list
     * \li Yesterday
     * \li Today
     * \li Tomorrow
     * \li Two days ago
     * \li In two days
     * \endlist
     *
     * If the \a date falls outside this period then the \a format is used.
     *
     * \a date the date to be formatted
     *
     * \a format the date format to use
     *
     * Returns the date as a string
     */
    QString formatRelativeDate(const QDate &date, QLocale::FormatType format) const;

    /*!
     * Returns a string formatted to a relative datetime style.
     *
     * If the \a dateTime falls within 2 days before or after the current date
     * then a relative date string will be returned, such as:
     * \list
     * \li Yesterday at 3:00pm
     * \li Today at 3:00pm
     * \li Tomorrow at 3:00pm
     * \li Two days ago at 3:00pm
     * \li In Two days at 3:00pm
     * \endlist
     *
     * If the \a dateTime falls within one hour of the current time.
     * Then a shorter version is displayed:
     * \list
     * \li Just a moment ago (for within the same minute)
     * \li 15 minutes ago
     * \endlist
     *
     * If the \a dateTime falls outside this period then the date is rendered as:
     * \list
     * \li Monday, 7 September, 2021 at 7:00 PM : date formatted \a format + " at " + time formatted with \a format
     * \endlist
     *
     * With \a format LongFormat, time format used is set to ShortFormat (to omit timezone and seconds).
     *
     * First character is capitalized.
     *
     * \a dateTime the date to be formatted
     *
     * \a format the date format to use
     *
     * Returns the date as a string
     */
    QString formatRelativeDateTime(const QDateTime &dateTime, QLocale::FormatType format) const;

    /*!
     * Returns a time value formatted as string considering the timezone.
     *
     * This behaves like QLocale::toString(QTime, QLocale::FormatType) but can be
     * made to add additional timezone information via \a options.
     *
     * \a dateTime the time to be formatted, as a full QDateTime object so timezone informations
     * remain available.
     * \a format the format used for formatting the time value
     * \a options controls how timezone information should be formatted
     *
     * \since 6.15
     */
    [[nodiscard]] QString formatTime(const QDateTime &dateTime, QLocale::FormatType format, TimeFormatOptions options = KFormat::DoNotAddTimeZone) const;

    /*!
     * Converts \a value to the appropriate string representation
     *
     * Example:
     * \code
     * // sets formatted to "1.0 kbit"
     * auto formatted = format.formatValue(1000, KFormat::Unit::Bit, 1, KFormat::UnitPrefix::Kilo);
     * \endcode
     *
     * \a value value to be formatted
     *
     * \a precision number of places after the decimal point to use.  KDE uses
     *        1 by default so when in doubt use 1.
     *
     * \a unit unit to use in result.
     *
     * \a prefix specific prefix to use in result.  Use UnitPrefix::AutoAdjust
     *        to automatically select an appropriate prefix.
     *
     * \a dialect prefix standard to use.  Use DefaultBinaryDialect to
     *        use the localized user selection unless you need to use a specific
     *        unit type. Only meaningful for KFormat::Unit::Byte, and ignored for
     *        all other units.
     *
     * Returns converted size as a translated string including prefix and unit.
     *         E.g. "1.23 KiB", "2 GB" (JEDEC), "4.2 kB" (Metric), "1.2 kbit".
     * \sa Unit
     * \sa UnitPrefix
     * \sa BinaryUnitDialect
     * \since 5.49
     */
    QString formatValue(double value,
                        KFormat::Unit unit,
                        int precision = 1,
                        KFormat::UnitPrefix prefix = KFormat::UnitPrefix::AutoAdjust,
                        KFormat::BinaryUnitDialect dialect = KFormat::DefaultBinaryDialect) const;

    /*!
     * Converts \a value to the appropriate string representation
     *
     * Example:
     * \code
     * QString bits, slow, fast;
     * // sets bits to "1.0 kbit", slow to "1.0 kbit/s" and fast to "12.3 Mbit/s".
     * bits = format.formatValue(1000, QStringLiteral("bit"), 1, KFormat::UnitPrefix::Kilo);
     * slow = format.formatValue(1000, QStringLiteral("bit/s");
     * fast = format.formatValue(12.3e6, QStringLiteral("bit/s");
     * \endcode
     *
     * \a value value to be formatted
     *
     * \a precision number of places after the decimal point to use.  KDE uses
     *        1 by default so when in doubt use 1.
     *
     * \a unit unit to use in result.
     *
     * \a prefix specific prefix to use in result.  Use UnitPrefix::AutoAdjust
     *        to automatically select an appropriate prefix.
     *
     * Returns converted size as a translated string including prefix and unit.
     *         E.g. "1.2 kbit", "2.4 kB", "12.3 Mbit/s"
     * \sa UnitPrefix
     * \since 5.49
     */
    // TODO KF7: remove in favor of the method below
    QString formatValue(double value, const QString &unit, int precision = 1, KFormat::UnitPrefix prefix = KFormat::UnitPrefix::AutoAdjust) const;
    /*!
     * Converts \a value to the appropriate string representation.
     *
     * Example:
     * \code
     * QString iec, jedec, metric;
     * // Sets iec to "1.0 KiB/s", jedec to "1.0 KB/s" and metric to "1.0 kB/s"
     * iec = format.formatValue(1024, QStringLiteral("B/s"), 1, KFormat::UnitPrefix::AutoAdjust, KFormat::IECBinaryDialect);
     * jedec = format.formatValue(1024, QStringLiteral("B/s"), 1, KFormat::UnitPrefix::AutoAdjust, KFormat::JEDECBinaryDialect);
     * metric = format.formatValue(1000, QStringLiteral("B/s"), 1, KFormat::UnitPrefix::AutoAdjust, KFormat::MetricBinaryDialect);
     * \endcode
     *
     * \a value value to be formatted
     *
     * \a precision number of places after the decimal point to use. 1 is used by default; when
     *        in doubt use 1
     *
     * \a unit unit to use in result
     *
     * \a prefix specific prefix to use in result. Use UnitPrefix::AutoAdjust
     *        to automatically select an appropriate prefix
     *
     * \a dialect prefix standard to use. Use DefaultBinaryDialect to
     *        use the localized user selection unless you need to use a specific
     *        unit type
     *
     * Returns converted size as a translated string including prefix and unit.
     *         E.g. "1.2 kbit", "2.4 kB", "12.3 Mbit/s"
     * \sa UnitPrefix
     * \since 5.74
     */
    QString formatValue(double value, const QString &unit, int precision, KFormat::UnitPrefix prefix, KFormat::BinaryUnitDialect dialect) const;

    /*!
     * Format \a distance given in meters in a suitable unit for displaying.
     *
     * For locales using metric units (or when forcing metric units explicitly)
     * this will use meters and kilometers depending on the distance, for locales
     * using imperial units this wil use miles and feet.
     *
     * \a distance Distance value to be formatted, in meters.
     *
     * \a options Formatting options.
     *
     * Returns Formatted distance using locale- and distance-appropirate units.
     *
     * \since 6.11
     */
    [[nodiscard]] QString formatDistance(double distance, KFormat::DistanceFormatOptions = LocaleDistanceUnits) const;

private:
    QSharedDataPointer<KFormatPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KFormat::DurationFormatOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KFormat::DistanceFormatOptions)

#endif // KFORMAT_H
