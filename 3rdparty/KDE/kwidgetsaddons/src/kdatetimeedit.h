/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDATETIMEEDIT_H
#define KDATETIMEEDIT_H

#include <kwidgetsaddons_export.h>

#include <QLocale>
#include <QTimeZone>
#include <QWidget>
#include <memory>

/*!
 * \class KDateTimeEdit
 * \inmodule KWidgetsAddons
 *
 * \brief A widget for editing date and time.
 */
class KWIDGETSADDONS_EXPORT KDateTimeEdit : public QWidget
{
    Q_OBJECT

    /*!
     * \property KDateTimeEdit::date
     */
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged USER true)

    /*!
     * \property KDateTimeEdit::time
     */
    Q_PROPERTY(QTime time READ time WRITE setTime NOTIFY timeChanged USER true)

    /*!
     * \property KDateTimeEdit::timeListInterval
     */
    Q_PROPERTY(int timeListInterval READ timeListInterval WRITE setTimeListInterval)

    /*!
     * \property KDateTimeEdit::options
     */
    Q_PROPERTY(Options options READ options WRITE setOptions)

public:
    /*!
     * Options provided by the widget
     * \sa options
     * \sa setOptions
     * \sa Options
     *
     * \value ShowCalendar If the Calendar System edit is displayed
     * \value ShowDate If the Date is displayed
     * \value ShowTime If the Time is displayed
     * \value ShowTimeZone If the Time Zone is displayed
     * \value EditDate Allow the user to manually edit the date
     * \value EditTime Allow the user to manually edit the time
     * \value SelectCalendar Allow the user to select a calendar
     * \value SelectDate Allow the user to select a date
     * \value SelectTime Allow the user to select a time
     * \value SelectTimeZone Allow the user to select a time zone
     * \value DatePicker Show a date picker
     * \value DateKeywords Show date keywords
     * \value ForceTime The entered time can only be a selected time
     * \value WarnOnInvalid Show a warning on focus out if the date or time is invalid
     */
    enum Option {
        ShowCalendar = 0x00001,
        ShowDate = 0x00002,
        ShowTime = 0x00004,
        ShowTimeZone = 0x00008,
        // EditCalendar     = 0x00010
        EditDate = 0x00020,
        EditTime = 0x00040,
        // EditTimeZone     = 0x00080
        SelectCalendar = 0x00100,
        SelectDate = 0x00200,
        SelectTime = 0x00400,
        SelectTimeZone = 0x00800,
        DatePicker = 0x01000,
        DateKeywords = 0x02000,
        ForceTime = 0x04000,
        WarnOnInvalid = 0x08000,
    };
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    /*!
     * Create a new KDateTimeEdit widget
     */
    explicit KDateTimeEdit(QWidget *parent = nullptr);

    ~KDateTimeEdit() override;

    /*!
     * Return the currently set widget options
     */
    Options options() const;

    /*!
     * Return the currently selected date, time and time zone
     */
    QDateTime dateTime() const;

    /*!
     * Return the currently selected date
     */
    QDate date() const;

    /*!
     * Return the currently selected time
     */
    QTime time() const;

    /*!
     * Return the currently selected time zone
     */
    QTimeZone timeZone() const;

    /*!
     * Returns the list of Calendar Locales displayed.
     */
    QList<QLocale> calendarLocalesList() const;

    /*!
     * Return the current minimum date and time
     */
    QDateTime minimumDateTime() const;

    /*!
     * Return the current maximum date and time
     */
    QDateTime maximumDateTime() const;

    /*!
     * Return the currently set date display format
     *
     * By default this is the Short Format
     */
    QLocale::FormatType dateDisplayFormat() const;

    /*!
     * Return the map of dates listed in the drop-down and their displayed
     * string forms.
     *
     * \sa setDateMap()
     */
    QMap<QDate, QString> dateMap() const;

    /*!
     * Return the currently set time format
     *
     * By default this is the Short Format
     */
    QLocale::FormatType timeDisplayFormat() const;

    /*!
     * Return the time list interval able to be selected
     */
    int timeListInterval() const;

    /*!
     * Return the list of times able to be selected in the drop-down.
     *
     * \sa setTimeList()
     * \sa timeListInterval()
     * \sa setTimeListInterval()
     */
    QList<QTime> timeList() const;

    /*!
     * Return the list of time zones able to be selected
     */
    QList<QTimeZone> timeZones() const;

    /*!
     * Return if the current user input is valid
     *
     * If the user input is null then it is not valid
     *
     * \sa isNull()
     */
    bool isValid() const;

    /*!
     * Return if the current user input is null
     *
     * \sa isValid()
     */
    bool isNull() const;

    /*!
     * Return if the current user input date is valid
     *
     * If the user input date is null then it is not valid
     *
     * \sa isNullDate()
     */
    bool isValidDate() const;

    /*!
     * Return if the current user input date is null
     *
     * \sa isValidDate()
     */
    bool isNullDate() const;
    /*!
     * Return if the current user input time is valid
     *
     * If the user input time is null then it is not valid
     *
     * \sa isNullTime()
     */
    bool isValidTime() const;

    /*!
     * Return if the current user input time is null
     *
     * \sa isValidTime()
     */
    bool isNullTime() const;

Q_SIGNALS:

    /*!
     * Signal if the date or time has been manually entered by the user.
     *
     * The returned date and time may be invalid.
     *
     * \a dateTime the new date, time and time zone
     */
    void dateTimeEntered(const QDateTime &dateTime);

    /*!
     * Signal if the date or time has been changed either manually by the user
     * or programmatically.
     *
     * The returned date and time may be invalid.
     *
     * \a dateTime the new date, time and time zone
     */
    void dateTimeChanged(const QDateTime &dateTime);

    /*!
     * Signal if the date or time is being manually edited by the user.
     *
     * The returned date and time may be invalid.
     *
     * \a dateTime the new date, time and time zone
     */
    void dateTimeEdited(const QDateTime &dateTime);

    /*!
     * Signal if the Calendar Locale has been manually entered by the user.
     *
     * \a calendarLocale the new calendar locale
     */
    void calendarEntered(const QLocale &calendarLocale);

    /*!
     * Signal if the Calendar Locale has been changed either manually by the user
     * or programmatically.
     *
     * \a calendarLocale the new calendar locale
     */
    void calendarChanged(const QLocale &calendarLocale);

    /*!
     * Signal if the date has been manually entered by the user.
     *
     * The returned date may be invalid.
     *
     * \a date the new date
     */
    void dateEntered(const QDate &date);

    /*!
     * Signal if the date has been changed either manually by the user
     * or programmatically.
     *
     * The returned date may be invalid.
     *
     * \a date the new date
     */
    void dateChanged(const QDate &date);

    /*!
     * Signal if the date is being manually edited by the user.
     *
     * The returned date may be invalid.
     *
     * \a date the new date
     */
    void dateEdited(const QDate &date);

    /*!
     * Signal if the time has been manually entered by the user.
     *
     * The returned time may be invalid.
     *
     * \a time the new time
     */
    void timeEntered(const QTime &time);

    /*!
     * Signal if the time has been changed either manually by the user
     * or programmatically.
     *
     * The returned time may be invalid.
     *
     * \a time the new time
     */
    void timeChanged(const QTime &time);

    /*!
     * Signal if the time is being manually edited by the user.
     *
     * The returned time may be invalid.
     *
     * \a time the new time
     */
    void timeEdited(const QTime &time);

    /*!
     * Signal if the time zone has been changed manually by the user.
     *
     * \a zone the new time zone
     */
    void timeZoneEntered(const QTimeZone &zone);

    /*!
     * Signal if the time zone has been changed either manually by the user
     * or programmatically.
     *
     * \a zone the new time zone
     */
    void timeZoneChanged(const QTimeZone &zone);

public Q_SLOTS:

    /*!
     * Set the new widget options
     *
     * \a options the new widget options
     */
    void setOptions(Options options);

    /*!
     * Set the currently selected date, time and time zone
     *
     * \a dateTime the new date, time and time zone
     */
    void setDateTime(const QDateTime &dateTime);

    /*!
     * Set the currently selected date
     *
     * \a date the new date
     */
    void setDate(const QDate &date);

    /*!
     * Set the currently selected time
     *
     * \a time the new time
     */
    void setTime(const QTime &time);

    /*!
     * Set the current time zone
     *
     * \a zone the new zone
     */
    void setTimeZone(const QTimeZone &zone);

    /*!
     * Set the minimum and maximum date and time range
     *
     * To enable range checking provide two valid dates.
     * To disable range checking provide two invalid dates, or call
     * clearDateRange;
     *
     * \a minDateTime the minimum date and time
     *
     * \a maxDateTime the maximum date and time
     *
     * \a minWarnMsg the minimum warning message
     *
     * \a maxWarnMsg the maximum warning message
     */
    void
    setDateTimeRange(const QDateTime &minDateTime, const QDateTime &maxDateTime, const QString &minWarnMsg = QString(), const QString &maxWarnMsg = QString());

    /*!
     * Reset the minimum and maximum date and time to the default
     */
    void resetDateTimeRange();

    /*!
     * Set the minimum allowed date.
     *
     * If the date is invalid, or more than current maximum,
     * then the minimum will not be set.
     *
     * \a minDateTime the minimum date
     *
     * \a minWarnMsg the minimum warning message
     *
     * \sa setMaximumDateTime()
     * \sa setDateRange()
     */
    void setMinimumDateTime(const QDateTime &minDateTime, const QString &minWarnMsg = QString());

    /*!
     * Reset the minimum date and time to the default
     */
    void resetMinimumDateTime();

    /*!
     * Set the maximum allowed date.
     *
     * If the date is invalid, or less than current minimum,
     * then the maximum will not be set.
     *
     * \a maxDateTime the maximum date
     *
     * \a maxWarnMsg the maximum warning message
     *
     * \sa setMinimumDateTime()
     * \sa setDateRange()
     */
    void setMaximumDateTime(const QDateTime &maxDateTime, const QString &maxWarnMsg = QString());

    /*!
     * Reset the minimum date and time to the default
     */
    void resetMaximumDateTime();

    /*!
     * Sets the date format to display.
     *
     * By default is the Short Format.
     *
     * \a format the date format to use
     */
    void setDateDisplayFormat(QLocale::FormatType format);

    /*!
     * Set the list of Calendar Locales to display.
     *
     * \a calendarLocales the list of calendar locales to display
     */
    void setCalendarLocalesList(const QList<QLocale> &calendarLocales);

    /*!
     * Set the list of dates able to be selected from the drop-down and the
     * string form to display for those dates, e.g. "2010-01-01" and "Yesterday".
     *
     * Any invalid or duplicate dates will be used, the list will NOT be
     * sorted, and the minimum and maximum date will not be affected.
     *
     * The \a dateMap is keyed by the date to be listed and the value is the
     * string to be displayed.  If you want the date to be displayed in the
     * default date format then the string should be null.  If you want a
     * separator to be displayed then set the string to "separator".
     *
     * \a dateMap the map of dates able to be selected
     *
     * \sa dateMap()
     */
    void setDateMap(QMap<QDate, QString> dateMap);

    /*!
     * Sets the time format to display.
     *
     * By default is the Short Format.
     *
     * \a format the time format to use
     */
    void setTimeDisplayFormat(QLocale::FormatType format);

    /*!
     * Set the interval between times able to be selected from the drop-down.
     *
     * The combo drop-down will be populated with times every \a minutes
     * apart, starting from the minimumTime() and ending at maximumTime().
     *
     * If the ForceInterval option is set then any time manually typed into the
     * combo line edit will be forced to the nearest interval.
     *
     * This interval must be an exact divisor of the valid time range hours.
     * For example with the default 24 hour range \a interval must divide 1440
     * minutes exactly, meaning 1, 6 and 90 are valid but 7, 31 and 91 are not.
     *
     * Setting the time list interval will override any time list previously set
     * via setTimeList().
     *
     * \a minutes the time list interval to display
     *
     * \sa timeListInterval()
     */
    void setTimeListInterval(int minutes);

    /*!
     * Set the list of times able to be selected from the drop-down.
     *
     * Setting the time list will override any time interval previously set via
     * setTimeListInterval().
     *
     * Any invalid or duplicate times will be ignored, and the list will be
     * sorted.
     *
     * The minimum and maximum time will automatically be set to the earliest
     * and latest value in the list.
     *
     * \a timeList the list of times able to be selected
     *
     * \a minWarnMsg the minimum warning message
     *
     * \a maxWarnMsg the maximum warning message
     *
     * \sa timeList()
     */
    void setTimeList(QList<QTime> timeList, const QString &minWarnMsg = QString(), const QString &maxWarnMsg = QString());

    /*!
     * Set the time zones able to be selected
     *
     * \a zones the time zones to display
     */
    void setTimeZones(const QList<QTimeZone> &zones);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    /*!
     * Assign the date, time and time zone for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules,
     * but reimplementations must call the parent method at the end.
     *
     * \a dateTime the new date and time
     */
    virtual void assignDateTime(const QDateTime &dateTime);

    /*!
     * Assign the date for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules,
     * but reimplementations must call the parent method at the end.
     *
     * \a date the new date
     */
    virtual void assignDate(const QDate &date);

    /*!
     * Assign the time for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules,
     * but reimplementations must call the parent method at the end.
     *
     * \a time the new time
     */
    virtual void assignTime(const QTime &time);

    /*!
     * Assign the time zone for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules,
     * but reimplementations must call the parent method at the end.
     *
     * \a zone the new time zone
     */
    void assignTimeZone(const QTimeZone &zone);

private:
    friend class KDateTimeEditPrivate;
    std::unique_ptr<class KDateTimeEditPrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDateTimeEdit::Options)

#endif // KDATETIMEEDIT_H
