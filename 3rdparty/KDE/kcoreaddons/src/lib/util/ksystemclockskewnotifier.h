/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <kcoreaddons_export.h>

#include <QObject>
#include <memory>

class KSystemClockSkewNotifierPrivate;

/*!
 * \class KSystemClockSkewNotifier
 * \inmodule KCoreAddons
 *
 * \brief The KSystemClockSkewNotifier class provides a way for monitoring system clock changes.
 *
 * The KSystemClockSkewNotifier class makes it possible to detect discontinuous changes to
 * the system clock. Such changes are usually initiated by the user adjusting values
 * in the Date and Time KCM or calls made to functions like settimeofday().
 *
 * \since 6.15
 */
class KCOREADDONS_EXPORT KSystemClockSkewNotifier : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

public:
    explicit KSystemClockSkewNotifier(QObject *parent = nullptr);
    ~KSystemClockSkewNotifier() override;

    /*!
     * Returns \c true if the notifier is active; otherwise returns \c false.
     */
    bool isActive() const;

    /*!
     * Sets the active status of the clock skew notifier to \a active.
     *
     * The skewed() signal won't be emitted while the notifier is inactive.
     *
     * The notifier is inactive by default.
     *
     * \sa activeChanged()
     */
    void setActive(bool active);

Q_SIGNALS:
    /*!
     * This signal is emitted whenever the active property is changed.
     */
    void activeChanged();

    /*!
     * This signal is emitted whenever the system clock is changed.
     */
    void skewed();

private:
    std::unique_ptr<KSystemClockSkewNotifierPrivate> d;
};
