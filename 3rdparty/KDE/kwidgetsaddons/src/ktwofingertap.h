/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KTWOFINGERTAP_H
#define KTWOFINGERTAP_H

#include <kwidgetsaddons_export.h>

#include <QGesture>
#include <QGestureRecognizer>
#include <memory>

/*!
 * \class KTwoFingerTap
 * \inmodule KWidgetsAddons
 *
 * \brief A two finger tap gesture.
 *
 * Provides a class for a two finger tap gesture.
 *
 * \note The QGestureManager need a QMainwindow, to delivery the gesture.
 *
 * \since 5.83
 */
class KWIDGETSADDONS_EXPORT KTwoFingerTap : public QGesture
{
    Q_OBJECT

    /*!
     * \property KTwoFingerTap::pos
     */
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

    /*!
     * \property KTwoFingerTap::screenPos
     */
    Q_PROPERTY(QPointF screenPos READ screenPos WRITE setScreenPos)

    /*!
     * \property KTwoFingerTap::scenePos
     */
    Q_PROPERTY(QPointF scenePos READ scenePos WRITE setScenePos)
public:
    /*!
     * The constructor.
     */
    explicit KTwoFingerTap(QObject *parent = nullptr);

    ~KTwoFingerTap() override;

    /*!
     * The position of the gesture, relative to the widget that received the gesture.
     *
     * Note: This is not necessarily the same position as in the widget that grabGesture() uses.
     */
    Q_REQUIRED_RESULT QPointF pos() const;

    /*!
     * Sets the position, relative to the widget.
     *
     * \a pos The position.
     */
    void setPos(QPointF pos);

    /*!
     * Sets the screen position.
     *
     * \a screenPos The screen position.
     */
    Q_REQUIRED_RESULT QPointF screenPos() const;

    /*!
     * Return the start scene position of the gesture.
     */
    void setScreenPos(QPointF screenPos);

    /*!
     * Return the start scene position of the gesture.
     */
    Q_REQUIRED_RESULT QPointF scenePos() const;

    /*!
     * Sets the scene position.
     *
     * \a scenePos The scene position, identical to the screen position for widgets.
     */
    void setScenePos(QPointF scenePos);

private:
    std::unique_ptr<class KTwoFingerTapPrivate> const d;
};

/*!
 * \class KTwoFingerTapRecognizer
 * \inmodule KWidgetsAddons
 * \inheaderfile KTwoFingerTap
 *
 * \brief The recognizer for a two finger tap gesture.
 *
 * Provides the recognizer for a two finger tap gesture.
 *
 * \since 5.83
 */
class KWIDGETSADDONS_EXPORT KTwoFingerTapRecognizer : public QGestureRecognizer
{
public:
    /*!
     * The constructor.
     */
    KTwoFingerTapRecognizer();

    ~KTwoFingerTapRecognizer() override;

    /*!
     * Qt called this member to create a new QGesture object.
     *
     * \a target The target for the gesture.
     *
     * Returns the new QGesture object.
     */
    QGesture *create(QObject *target) override;

    /*!
     * Handles the given event for the watched object and update the gesture object.
     *
     * \a gesture The gesture object.
     *
     * \a watched The watched object.
     *
     * \a event The event.
     *
     * Returns the result reflects how much of the gesture has been recognized.
     */
    Result recognize(QGesture *gesture, QObject *watched, QEvent *event) override;

    /*!
     * Returns the maximum wiggle room for a touch point.
     */
    Q_REQUIRED_RESULT int tapRadius() const;

    /*!
     * Set the maximum wiggle room for a touch point. If \a i is negative, it will be set to null.
     *
     * \a i The maximum wiggle room.
     */
    void setTapRadius(int i);

private:
    std::unique_ptr<class KTwoFingerTapRecognizerPrivate> const d;
    Q_DISABLE_COPY(KTwoFingerTapRecognizer)
};

#endif
