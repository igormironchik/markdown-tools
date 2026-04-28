/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KTWOFINGERSWIPE_H
#define KTWOFINGERSWIPE_H

#include <kwidgetsaddons_export.h>

#include <QGesture>
#include <QGestureRecognizer>
#include <memory>

/*!
 * \class KTwoFingerSwipe
 *
 * \brief A two finger swipe gesture.
 *
 * Provides a class for a two finger swipe gesture.
 *
 * Note: The QGestureManager need a QMainwindow, to delivery the gesture.
 *
 * \since 5.83
 */
class KWIDGETSADDONS_EXPORT KTwoFingerSwipe : public QGesture
{
    Q_OBJECT

    /*!
     * \property KTwoFingerSwipe::pos
     */
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

    /*!
     * \property KTwoFingerSwipe::screenPos
     */
    Q_PROPERTY(QPointF screenPos READ screenPos WRITE setScreenPos)

    /*!
     * \property KTwoFingerSwipe::scenePos
     */
    Q_PROPERTY(QPointF scenePos READ scenePos WRITE setScenePos)

    /*!
     * \property KTwoFingerSwipe::swipeAngle
     */
    Q_PROPERTY(qreal swipeAngle READ swipeAngle WRITE setSwipeAngle)
public:
    /*!
     * The constructor.
     */
    explicit KTwoFingerSwipe(QObject *parent = nullptr);

    ~KTwoFingerSwipe() override;

    /*!
     * Returns the start position of the gesture, relative to the widget that received the gesture.
     *
     * \note This is not necessarily the same position as in the widget that grabGesture() uses.
     */
    Q_REQUIRED_RESULT QPointF pos() const;

    /*!
     * Sets the position, relative to the widget.
     *
     * \a pos The position.
     */
    void setPos(QPointF pos);

    /*!
     * Returns the start screen position of the gesture.
     */
    Q_REQUIRED_RESULT QPointF screenPos() const;

    /*!
     * Sets the screen position.
     *
     * \a screenPos The screen position.
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

    /*!
     * Returns the angle of the swipe gesture.
     */
    Q_REQUIRED_RESULT qreal swipeAngle() const;

    /*!
     * Sets the angle of the swipe gesture
     *
     * \a swipeAngle The angle.
     */
    void setSwipeAngle(qreal swipeAngle);

private:
    std::unique_ptr<class KTwoFingerSwipePrivate> const d;
};

/*!
 * \class KTwoFingerSwipeRecognizer
 * \inmodule KWidgetsAddons
 * \inheaderfile KTwoFingerSwipe
 *
 * \brief The recognizer for a two finger swipe gesture.
 *
 * Provides the recognizer for a two finger swipe gesture. To adjust the maximum swipe time
 * and the minimum swipe distance, for a valid swipe gesture:
 * \sa setMaxSwipeTime
 * \sa setSwipeDistance
 *
 * \since 5.83
 */
class KWIDGETSADDONS_EXPORT KTwoFingerSwipeRecognizer : public QGestureRecognizer
{
public:
    /*!
     * The constructor.
     */
    KTwoFingerSwipeRecognizer();

    ~KTwoFingerSwipeRecognizer() override;

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
     * Returns the maximum duration for the swipe gesture, in milliseconds.
     */
    Q_REQUIRED_RESULT int maxSwipeTime() const;

    /*!
     * Set the maximum duration of the swipe gesture. If \a i is negative, it will be set to null.
     *
     * \a i The maximum duration in milliseconds.
     */
    void setMaxSwipeTime(int i);

    /*!
     * Returns the minimum distance for the swipe gesture.
     */
    Q_REQUIRED_RESULT int minSswipeDistance() const;

    /*!
     * Set the minimum distance of the swipe gesture. If \a i is negative, it will be set to null.
     *
     * \a i The minimum distance.
     */
    void setSwipeDistance(int i);

private:
    std::unique_ptr<class KTwoFingerSwipeRecognizerPrivate> const d;
    Q_DISABLE_COPY(KTwoFingerSwipeRecognizer)
};

#endif
