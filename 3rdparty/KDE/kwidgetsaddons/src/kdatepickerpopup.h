/*
  SPDX-FileCopyrightText: 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDATEPICKERPOPUP_H
#define KDATEPICKERPOPUP_H

#include <kwidgetsaddons_export.h>

#include <QDate>
#include <QMenu>

#include <memory>

class KDatePicker;
class KDatePickerPopupPrivate;

/*!
 * \class KDatePickerPopup
 * \inmodule KWidgetsAddons
 *
 * \brief This menu helps the user to select a date quickly.
 *
 * This menu helps the user to select a date quickly. It offers various
 * modes of selecting, e.g. with a KDatePicker or with words like "Tomorrow".
 *
 * The available modes are:
 * \list
 * \li NoDate: A menu-item with "No Date". If chosen, the datepicker will emit
 *     a null QDate.
 * \li DatePicker: Shows a KDatePicker-widget.
 * \li Words: Shows items like "Today", "Tomorrow" or "Next Week".
 * \endlist
 *
 * \since 5.94
 */
class KWIDGETSADDONS_EXPORT KDatePickerPopup : public QMenu
{
    Q_OBJECT

    /*!
     * \property KDatePickerPopup::modes
     */
    Q_PROPERTY(Modes modes READ modes WRITE setModes)

public:
    /*!
     * Describes the available selection modes.
     *
     * \value NoDate A menu-item with "No Date". Will always return an invalid date.
     * \value DatePicker A menu-item with a KDatePicker.
     * \value Words A menu-item with list of words that describe a date.
     */
    enum Mode {
        NoDate = 1,
        DatePicker = 2,
        Words = 4
    };
    Q_DECLARE_FLAGS(Modes, Mode)

    /*!
     * Creates a new date picker popup.
     *
     * \a modes The selection modes that shall be offered
     *
     * \a date The initial date of date picker widget.
     *
     * \a parent The parent object.
     */
    explicit KDatePickerPopup(Modes modes = DatePicker, QDate date = QDate::currentDate(), QWidget *parent = nullptr);

    ~KDatePickerPopup() override;

    /*!
     * Returns the currently used selection modes.
     */
    Modes modes() const;

    /*!
     * Set the selection modes to use.
     */
    void setModes(Modes modes);

    /*!
     * Sets the range of dates that can be accepted.
     *
     * Invalid dates can be used to define open-ended ranges.
     * If both values are valid, the minimum date must be less than
     * or equal to the maximum date, otherwise the date range will
     * not be set.
     *
     * \a minDate the minimum date
     *
     * \a maxDate the maximum date
     */
    void setDateRange(const QDate &minDate, const QDate &maxDate);

    /*!
     * Return the map of dates listed in the drop-down and their displayed
     * string forms.
     *
     * \sa setDateMap()
     */
    QMap<QDate, QString> dateMap() const;

    /*!
     * Sets the list of dates in the drop-down menu that the user can select from
     * and the text string to display for each date, e.g. "2010-01-01" and "Yesterday".
     *
     * The list of date/string pairs is used as-is (invalid or duplicate dates aren't removed),
     * and the order will not be changed (the map is sorted by key); also and the minimum and
     * maximum dates will not be affected.
     *
     * The \a dateMap is keyed by the date to be listed and the value is the
     * string to be displayed.  If you want the date to be displayed in the
     * default date format then the string should be null.  If you want a
     * separator to be displayed then set the string to "separator".
     *
     * \a dateMap the map of dates the user can select from
     *
     * \sa dateMap()
     */
    void setDateMap(const QMap<QDate, QString> &dateMap);

    /*!
     * Returns the used KDatePicker object.
     */
    Q_REQUIRED_RESULT KDatePicker *datePicker() const;

public Q_SLOTS:
    /*!
     * Sets the current \a date.
     */
    void setDate(QDate date);

Q_SIGNALS:
    /*!
     * This signal is emitted whenever the user has selected a new date.
     *
     * \a date The new date.
     */
    void dateChanged(const QDate &date);

private:
    std::unique_ptr<KDatePickerPopupPrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDatePickerPopup::Modes)

#endif
