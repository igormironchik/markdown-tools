/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWINDOWSTATESAVER_H
#define KWINDOWSTATESAVER_H

#include <kconfiggroup.h>
#include <kconfiggui_export.h>

#include <QObject>

class QWindow;
class KWindowStateSaverPrivate;

/*!
 * \class KWindowStateSaver
 * \inmodule KConfigGui
 *
 * \brief Saves and restores a window size and (when possible) position.
 *
 * This is useful for retrofitting persisting window geometry on existing windows or dialogs,
 * without having to modify those classes themselves, or having to inherit from them.
 *
 * For this:
 * \list
 * \li Create a new instance of KWindowStateSaver for every window that should have its
 * state persisted
 * \li Pass it the window
 * \li Pass it the config group the state should be stored in
 * \endlist
 *
 * The KWindowStateSaver will restore an existing state and then monitor
 * the window for subsequent changes to persist. It will delete itself once the window is
 * deleted.
 *
 * \code
 * QPrintPreviewDialog previewDialog = ...
 * new KWindowStateSaver(&previewDialog, "printPreviewDialogState");
 * ...
 * previewDialog.exec();
 * \endcode
 *
 * Note that freshly created top-level QWidgets (such as the dialog in the above example)
 * do not have an associated QWindow yet (that is, windowHandle() returns \c nullptr).
 * KWindowStateSaver monitors suchs widgets
 * for any later-associated QWindows to allow for this use case.
 *
 * When implementing your own windows/dialogs, using KWindowConfig directly can be an
 * alternative.
 *
 * \sa KWindowConfig
 * \since 5.92
 */
class KCONFIGGUI_EXPORT KWindowStateSaver : public QObject
{
    Q_OBJECT
public:
    /*!
     * Creates a new window state saver for the given \a window
     * with the \a configGroup that holds the window state.
     * \sa KSharedConfig::openStateConfig
     */
    explicit KWindowStateSaver(QWindow *window, const KConfigGroup &configGroup);
    /*!
     * Creates a new window state saver for the given \a window
     * with the \a configGroupName that holds the window state.
     * \sa KSharedConfig::openStateConfig
     */
    explicit KWindowStateSaver(QWindow *window, const QString &configGroupName);

    /*!
     * Creates a new window state saver for \a widget
     * with the \a configGroup that holds the window state.
     *
     * Use this for widgets that aren't shown yet
     * and would still return \c nullptr from windowHandle().
     */
    template<typename Widget>
    explicit inline KWindowStateSaver(Widget *widget, const KConfigGroup &configGroup);
    /*!
     * Creates a new window state saver for \a widget
     * with the \a configGroupName that holds the window state.
     *
     * Use this for widgets that aren't shown yet
     * and would still return \c nullptr from windowHandle().
     */
    template<typename Widget>
    explicit inline KWindowStateSaver(Widget *widget, const QString &configGroupName);

    ~KWindowStateSaver();

private:
    void timerEvent(QTimerEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    // API used by template code, so technically part of the ABI
    void initWidget(QObject *widget, const std::function<QWindow *()> &windowHandleCallback, const KConfigGroup &configGroup);
    void initWidget(QObject *widget, const std::function<QWindow *()> &windowHandleCallback, const QString &configGroupName);

    // cannot use std::unique_ptr due to the template ctors
    // not seeing the full private class
    KWindowStateSaverPrivate *d = nullptr;
};

template<typename Widget>
KWindowStateSaver::KWindowStateSaver(Widget *widget, const KConfigGroup &configGroup)
    : QObject(widget)
{
    initWidget(
        widget,
        [widget]() {
            return widget->windowHandle();
        },
        configGroup);
}

template<typename Widget>
KWindowStateSaver::KWindowStateSaver(Widget *widget, const QString &configGroupName)
    : QObject(widget)
{
    initWidget(
        widget,
        [widget]() {
            return widget->windowHandle();
        },
        configGroupName);
}

#endif // KWINDOWSTATESAVER_H
