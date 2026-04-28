/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDATECOMBOBOX_H
#define KDATECOMBOBOX_H

#include <kwidgetsaddons_export.h>

#include <QComboBox>
#include <QLocale>
#include <memory>

/*!
 * \class KDateComboBox
 * \inmodule KWidgetsAddons
 *
 * \brief A combobox for dates.
 */
class KWIDGETSADDONS_EXPORT KDateComboBox : public QComboBox
{
    Q_OBJECT

    /*!
     * \property KDateComboBox::date
     */
    Q_PROPERTY(QDate date READ date WRITE setDate NOTIFY dateChanged USER true)

    /*!
     * \property KDateComboBox::minimumDate
     */
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate RESET resetMinimumDate)

    /*!
     * \property KDateComboBox::maximumDate
     */
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate RESET resetMaximumDate)

    /*!
     * \property KDateComboBox::options
     */
    Q_PROPERTY(Options options READ options WRITE setOptions)

public:
    /*!
     * Options provided by the widget
     * \sa options()
     * \sa setOptions()
     *
     * \value EditDate Allow the user to manually edit the date in the combo line edit
     * \value SelectDate Allow the user to select the date from a drop-down menu
     * \value DatePicker Show a date picker in the drop-down
     * \value DateKeywords Show date keywords in the drop-down
     * \value WarnOnInvalid Show a warning on focus out if the date is invalid
     */
    enum Option {
        EditDate = 0x0001,
        SelectDate = 0x0002,
        DatePicker = 0x0004,
        DateKeywords = 0x0008,
        WarnOnInvalid = 0x0010,
    };
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    /*!
     * Create a new KDateComboBox widget
     *
     * By default the EditDate, SelectDate, DatePicker and DateKeywords options
     * are enabled, the ShortDate format is used and the date is set to the
     * current date.
     */
    explicit KDateComboBox(QWidget *parent = nullptr);

    ~KDateComboBox() override;

    /*!
     * Return the currently selected date
     */
    QDate date() const;

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
     * Return the currently set widget options
     */
    Options options() const;

    /*!
     * Return the currently set date display format
     *
     * By default this is the Short Format
     */
    QLocale::FormatType displayFormat() const;

    /*!
     * Return the current minimum date
     */
    QDate minimumDate() const;

    /*!
     * Return the current maximum date
     */
    QDate maximumDate() const;

    /*!
     * Return the map of dates listed in the drop-down and their displayed
     * string forms.
     *
     * \sa setDateMap()
     */
    QMap<QDate, QString> dateMap() const;

Q_SIGNALS:

    /*!
     * Signal if the date has been manually entered (by typing a date and losing focus, or pressing Enter)
     * or selected by the user (using the popup selector, or up, down, page up, page down keys, or the mouse wheel).
     *
     * The emitted date may be invalid.
     *
     * \a date the new date
     */
    void dateEntered(const QDate &date);

    /*!
     * Signal if the date has been changed either manually by the user
     * or programmatically.
     *
     * The emitted date may be invalid.
     *
     * \a date the new date
     */
    void dateChanged(const QDate &date);

    /*!
     * Signal if the date is being manually edited by the user.
     *
     * The emitted date may be invalid, or may not yet be what the user intends as the final date.
     *
     * \a date the new date
     */
    void dateEdited(const QDate &date);

public Q_SLOTS:

    /*!
     * Set the currently selected date
     *
     * You can set an invalid date or a date outside the valid range, validity
     * checking is only done via isValid().
     *
     * \a date the new date
     */
    void setDate(const QDate &date);

    /*!
     * Set the new widget options
     *
     * \a options the new widget options
     */
    void setOptions(Options options);

    /*!
     * Sets the date format to display.
     *
     * By default is the Short Format.
     *
     * \a format the date format to use
     */
    void setDisplayFormat(QLocale::FormatType format);

    /*!
     * Set the valid date range to be applied by isValid().
     *
     * Both dates must be valid and the minimum date must be less than or equal
     * to the maximum date, otherwise the date range will not be set.
     *
     * \a minDate the minimum date
     *
     * \a maxDate the maximum date
     *
     * \a minWarnMsg the minimum warning message
     *
     * \a maxWarnMsg the maximum warning message
     */
    void setDateRange(const QDate &minDate, const QDate &maxDate, const QString &minWarnMsg = QString(), const QString &maxWarnMsg = QString());

    /*!
     * Reset the minimum and maximum date to the default values.
     * \sa setDateRange()
     */
    void resetDateRange();

    /*!
     * Set the minimum allowed date.
     *
     * If the date is invalid, or greater than current maximum,
     * then the minimum will not be set.
     *
     * \a minDate the minimum date
     *
     * \a minWarnMsg the minimum warning message
     *
     * \sa minimumDate()
     * \sa maximumDate()
     * \sa setMaximumDate()
     * \sa setDateRange()
     */
    void setMinimumDate(const QDate &minDate, const QString &minWarnMsg = QString());

    /*!
     * Reset the minimum date to the default.
     *
     * The default is to have no minimum date.
     */
    void resetMinimumDate();

    /*!
     * Set the maximum allowed date.
     *
     * If the date is invalid, or less than current minimum,
     * then the maximum will not be set.
     *
     * \a maxDate the maximum date
     *
     * \a maxWarnMsg the maximum warning message
     *
     * \sa minimumDate()
     * \sa maximumDate()
     * \sa setMaximumDate()
     * \sa setDateRange()
     */
    void setMaximumDate(const QDate &maxDate, const QString &maxWarnMsg = QString());

    /*!
     * Reset the maximum date to the default
     *
     * The default is to have no maximum date.
     */
    void resetMaximumDate();

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

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void showPopup() override;
    void hidePopup() override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    /*!
     * Assign the date for the widget.
     *
     * Virtual to allow sub-classes to apply extra validation rules.
     *
     * \a date the new date
     */
    virtual void assignDate(const QDate &date);

private:
    friend class KDateComboBoxPrivate;
    std::unique_ptr<class KDateComboBoxPrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDateComboBox::Options)

#endif // KDATECOMBOBOX_H
