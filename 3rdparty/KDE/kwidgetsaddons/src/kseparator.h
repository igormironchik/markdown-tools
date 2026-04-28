/*
    SPDX-FileCopyrightText: 1997 Michael Roth <mroth@wirlweb.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSEPARATOR_H
#define KSEPARATOR_H

#include <QFrame>
#include <kwidgetsaddons_export.h>
#include <memory>

/*!
 * \class KSeparator
 * \inmodule KWidgetsAddons
 *
 * \brief Standard horizontal or vertical separator.
 *
 * \image kseparator-horizontal.png "KSeparator Widget with horizontal orientation"
 * \image kseparator-vertical.png "KSeparator Widget with vertical orientation"
 */
class KWIDGETSADDONS_EXPORT KSeparator : public QFrame
{
    Q_OBJECT

    /*!
     * \property KSeparator::orientation
     */
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

public:
    /*!
     * Constructor.
     *
     * \a parent parent object.
     *
     * \a f extra QWidget flags.
     */
    explicit KSeparator(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /*!
     * Constructor.
     *
     * \a orientation Set the orientation of the separator.
     * Possible values are Horizontal or Vertical.
     *
     * \a parent parent object.
     *
     * \a f extra QWidget flags.
     */
    explicit KSeparator(Qt::Orientation orientation, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    ~KSeparator() override;

    /*!
     * Returns the orientation of the separator.
     */
    Qt::Orientation orientation() const;

    /*!
     * Set the orientation of the separator to \a orientation
     *
     * \a orientation Possible values are Vertical and Horizontal.
     */
    void setOrientation(Qt::Orientation orientation);

private:
    std::unique_ptr<class KSeparatorPrivate> const d;
};

#endif // KSEPARATOR_H
