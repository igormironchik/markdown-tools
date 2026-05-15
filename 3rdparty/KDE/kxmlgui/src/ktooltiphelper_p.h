/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later OR BSD-2-Clause
*/

#ifndef KTOOLTIPHELPER_P_H
#define KTOOLTIPHELPER_P_H

#include <QObject>

#include <QPointer>
#include <QRect>
#include <QTimer>

class KToolTipHelper;

class QAction;
class QHelpEvent;
class QMenu;

/*!
 * \brief The private class of KToolTipHelper used for the PIMPL idiom.
 * \inmodule KXmlGui
 * \internal
 */
class KToolTipHelperPrivate : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Singleton implementation for KToolTipHelper and
     * NOT of this class (KToolTipHelperPrivate).
     */
    static KToolTipHelper *instance();

    explicit KToolTipHelperPrivate(KToolTipHelper *qq);

    ~KToolTipHelperPrivate() override;

    /*! \sa KToolTipHelper::eventFilter() */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /*! \sa KToolTipHelper::whatsThisHintOnly() */
    static const QString whatsThisHintOnly();

    /*!
     * \brief Makes sure submenus that show up do not mess with tooltips appearing in menus.
     *
     * This is somewhat of a workaround for Qt not posting QEvent::ToolTips when the
     * cursor wasn't moved *after* a submenu hides.
     *
     * Returns false.
     */
    bool handleHideEvent(QObject *watched, QEvent *event);

    /*!
     * \brief Returns \c true if the key press is used to
     * expand a tooltip, \c false otherwise.
     */
    bool handleKeyPressEvent(QEvent *event);

    /*!
     * Is called from handleToolTipEvent() to handle a QEvent::ToolTip in a menu.
     * This method will show the tooltip of the action that is hovered at a nice
     * position.
     *
     * \a menu      The menu that a tooltip is requested for.
     *
     * \a helpEvent The QEvent::ToolTip that was cast to a QHelpEvent.
     */
    bool handleMenuToolTipEvent(QMenu *menu, QHelpEvent *helpEvent);

    /*!
     * \brief Returns \c false if no special handling of the tooltip event
     * seems necessary, \c true otherwise.
     *
     * \a watched The object that is receiving the QHelpEvent.
     *
     * \a helpEvent The QEvent::ToolTip that was cast to a QHelpEvent.
     */
    bool handleToolTipEvent(QObject *watched, QHelpEvent *helpEvent);

    /*!
     * \brief Handles links being clicked in whatsThis.
     *
     * Returns true.
     */
    bool handleWhatsThisClickedEvent(QEvent *event);

    /*!
     * \brief The name is slightly misleading because it will only post events for QMenus.
     * \sa handleHideEvent()
     */
    void postToolTipEventIfCursorDidntMove() const;

    /*!
     * \brief Shows a tooltip that contains a whatsThisHint
     * at the location \a globalPos.
     *
     * If \a tooltip is empty, only a whatsThisHint is shown.
     *
     * The parameter usage is identical to that of QToolTip::showText. The only difference
     * is that this method doesn't need a QWidget *w parameter because that one is already
     * retrieved in handleToolTipEvent() prior to calling this method.
     *
     * \sa QToolTip::showText()
     */
    void showExpandableToolTip(const QPoint &globalPos, const QString &toolTip = QString(), const QRect &rect = QRect());

public:
    KToolTipHelper *const q;

private:
    /*! \brief An action in a menu a tooltip was requested for. */
    QPointer<QAction> m_action;
    /*!
     * \brief The global position where the last tooltip
     * that had a whatsThisHint was displayed.
     */
    QPoint m_lastExpandableToolTipGlobalPos;
    /*! \brief The last widget a QEvent::tooltip was sent for. */
    QPointer<QWidget> m_widget;
    /*! \brief Whether or not the last tooltip was expandable. */
    bool m_lastToolTipWasExpandable = false;

    /*!
     * \brief The global position of where the cursor was when the last
     * QEvent::HideEvent for a menu occurred.
     *
     * \sa handleHideEvent() */
    QPoint m_cursorGlobalPosWhenLastMenuHid;
    /*!
     * \brief Calls postToolTipEventIfCursorDidntMove().
     * \sa handleHideEvent()
     */
    QTimer m_toolTipTimeout;

    static KToolTipHelper *s_instance;
};

/*
 * This method checks if string "a" is sufficiently different
 * from string "b", barring characters like periods, ampersands
 * and other characters.
 *
 * Used for determining if tooltips are different from their icon name counterparts.
 *
 * Returns true if the string "a" is similar to "b" and false otherwise.
 */
bool isTextSimilar(const QString &a, const QString &b);

#endif // KTOOLTIPHELPER_P_H
