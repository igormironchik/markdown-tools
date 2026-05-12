/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2007 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KX11EXTRAS_H
#define KX11EXTRAS_H

#include <QObject>
#include <QWindow>

#include <kwindowsystem_export.h>

#include "netwm_def.h"

class NETWinInfo;
class NETEventFilter;

/*!
 * \qmltype KX11Extras
 * \inqmlmodule org.kde.kwindowsystem
 * \nativetype KX11Extras
 * \brief A collection of functions to obtain information from and manipulate X11 windows.
 *
 * These are generally not applicable to other window systems.
 *
 * \since 5.101
 */
/*!
 * \class KX11Extras
 * \inmodule KWindowSystem
 * \brief A collection of functions to obtain information from and manipulate X11 windows.
 *
 * These are generally not applicable to other window systems.
 *
 * \since 5.101
 */
class KWINDOWSYSTEM_EXPORT KX11Extras : public QObject
{
    Q_OBJECT

    /*!
     * \qmlproperty bool KX11Extras::compositingActive
     * \brief Whether desktop compositing is active.
     */
    /*!
     * \property KX11Extras::compositingActive
     * \brief Whether desktop compositing is active.
     */
    Q_PROPERTY(bool compositingActive READ compositingActive NOTIFY compositingChanged)

public:
    /*! */
    static KX11Extras *self();

    /*!
     * Returns the list of all toplevel windows currently managed by the
     * window manager in the order of creation.
     *
     * Please do not rely on indexes of this list:
     * Whenever you enter Qt's event loop in your
     * application, it may happen that entries are removed or added.
     * Your module should perhaps work on a copy of this list and verify a
     * window with hasWId() before any operations.
     *
     * Iteration over this list can be done easily with:
     * \code
     *  QList<WId> windows = KWindowSystem::windows();
     *  for (auto it = windows.cbegin(), end = windows.cend(); it != end; ++it) {
     *     ... do something here,  (*it) is the current WId.
     *  }
     * \endcode
     */
    static QList<WId> windows();

    /*!
     * Returns whether \a id is still managed at present.
     */
    static bool hasWId(WId id);

    /*!
     * Returns the list of all toplevel windows currently managed by the
     * window manager in the current stacking order (from lower to
     * higher).
     *
     * May be useful for pagers.
     */
    static QList<WId> stackingOrder();

    /*!
     * Returns the currently active window id, or 0 if no window is active.
     */
    static WId activeWindow();

    /*!
     * Requests that window \a win is activated, with the optional
     * X server timestamp \a time of the user activity that caused this request.
     *
     * There are two ways how to activate a window, by calling
     * activateWindow() and forceActiveWindow(). Generally,
     * applications shouldn't make attempts to explicitly activate
     * their windows, and instead let the user to activate them.
     * In the special cases where this may be needed, applications
     * should use activateWindow(). Window manager may consider whether
     * this request wouldn't result in focus stealing, which
     * would be obtrusive, and may refuse the request.
     *
     * The usage of forceActiveWindow() is meant only for pagers
     * and similar tools, which represent direct user actions
     * related to window manipulation.
     * Except for rare cases, this request will be always honored,
     * and normal applications are forbidden to use it.
     */
    static void activateWindow(WId win, long time = 0);

    /*!
     * Sets window \a win to be the active window, with the optional
     * X server timestamp \a time of the user activity that caused this request.
     *
     * Note that this should be called only in special cases, applications
     * shouldn't force themselves or other windows to be the active
     * window. Generally, this call should used only by pagers
     * and similar tools. See the explanation in description
     * of activateWindow().
     */
    static void forceActiveWindow(WId win, long time = 0);

    /*!
     * \qmlmethod void KX11Extras::forceActiveWindow(Window *window, long time = 0)
     * Sets \a window to be the active window, with the optional
     * X server timestamp \a time of the user activity that caused this request.
     *
     * Note that this should be called only in special cases, applications
     * shouldn't force themselves or other windows to be the active
     * window. Generally, this call should used only by pagers
     * and similar tools. See the explanation in the description
     * of KX11Extras::activateWindow().
     *
     * \since 6.0
     */
    Q_INVOKABLE static void forceActiveWindow(QWindow *window, long time = 0);

    /*!
     * Returns true if a compositing manager is running (i.e. ARGB windows
     * are supported, effects will be provided, etc.).
     */
    static bool compositingActive();

    /*!
     * Returns the current virtual desktop.
     */
    static int currentDesktop();

    /*!
     * Returns the number of virtual desktops.
     */
    static int numberOfDesktops();

    /*!
     * Convenience function to set the current desktop to \a desktop number.
     *
     * See NETRootInfo.
     */
    static void setCurrentDesktop(int desktop);

    /*!
     * Sets window id \a win to be present on all virtual desktops if \a b
     * is true. Otherwise the window lives only on one single desktop.
     */
    static void setOnAllDesktops(WId win, bool b);

    /*!
     * Moves window id \a win to \a desktop.
     */
    static void setOnDesktop(WId win, int desktop);

    /*!
     * Moves window id \a win to the list of \a activities UUIDs.
     *
     * \sa KWindowInfo::activities
     */
    static void setOnActivities(WId win, const QStringList &activities);

    /*!
     * Returns an icon for window \a win.
     *
     * If  \a width and \a height are specified, the best icon for the requested
     * size is returned.
     *
     * If \a scale is true, the icon is smooth-scaled to have exactly
     * the requested size.
     */
    static QPixmap icon(WId win, int width = -1, int height = -1, bool scale = false);

    /*!
     * \enum KX11Extras::IconSource
     * Masks specifying from which sources to read an icon.
     *
     * They are tried from the best until an icon is found.
     * \value NETWM Property from the window manager specification.
     * \value WMHints From WMHints property.
     * \value ClassHint Load icon after getting name from the classhint.
     * \value XApp Load the standard X icon (last fallback).
     */
    enum IconSource {
        NETWM = 1,
        WMHints = 2,
        ClassHint = 4,
        XApp = 8,
    };
    /*!
     * \overload
     *
     * Overloaded variant that allows specifying from which sources the icon should be read.
     * You should usually prefer the simpler variant which tries all possibilities to get
     * an icon.
     *
     * \a win The id of the window.
     * \a width The desired width, or -1.
     * \a height The desired height, or -1.
     * \a scale If true the icon will be scaled to the desired size. Otherwise the
     *          icon will not be modified.
     * \a flags OR-ed flags from the IconSource enum.
     */
    static QPixmap icon(WId win, int width, int height, bool scale, int flags);

    /*!
     * \overload
     *
     * Overloaded variant that allows passing in the NETWinInfo \a info to use for reading the
     * information. This variant is only useful on the X11 platform, other platforms do not
     * use NETWinInfo and delegate to the variant without NETWinInfo. Though if compiled with
     * X11 support the X11 variant is used on other platforms if \a info is not \c nullptr.
     * This can be used by applications using e.g. platform wayland but also connecting to an
     * XServer.
     *
     * The NETWinInfo must be constructed with properties:
     *
     * \list
     * \li NET::WMIcon to use the IconSource flag NETWM
     * \li NET::WM2IconPixmap to use the IconSource flag WMHints
     * \li NET::WM2WindowClass to use the IconSource flag ClassHint
     * \endlist
     *
     * \a win The id of the window.
     *
     * \a width The desired width, or -1.
     *
     * \a height The desired height, or -1.
     *
     * \a scale If true the icon will be scaled to the desired size. Otherwise the
     *          icon will not be modified.
     *
     * \a flags OR-ed flags from the IconSource enum.
     */
    static QPixmap icon(WId win, int width, int height, bool scale, int flags, NETWinInfo *info);

    /*!
     * Minimizes the window with id \a win.
     *
     * On X11 this follows the protocol described in ICCCM section 4.1.4.
     * \sa unminimizeWindow()
     */
    static void minimizeWindow(WId win);
    /*!
     * Unminimizes the window with id \a win.
     *
     * On X11 this follows the protocol described in ICCCM section 4.1.4.
     * \sa minimizeWindow()
     */
    static void unminimizeWindow(WId win);

    /*!
     * Returns the workarea for the specified \a desktop, or the current
     * work area if no desktop has been specified.
     *
     * The current desktop corresponds to -1.
     */
    static QRect workArea(int desktop = -1);

    /*!
     * Returns the workarea for the specified \a desktop, or the current
     * work area if no desktop has been specified.
     *
     * A list of struts belonging to clients can be specified with \a excludes.
     */
    static QRect workArea(const QList<WId> &excludes, int desktop = -1);

    /*!
     * Returns the name of the specified \a desktop number.
     */
    static QString desktopName(int desktop);

    /*!
     * Sets the \a name of the specified \a desktop number.
     */
    static void setDesktopName(int desktop, const QString &name);

    /*!
     * Reads and returns the contents of the given text
     * property (WM_NAME, WM_ICON_NAME,...).
     */
    static QString readNameProperty(WId window, unsigned long atom);

    /*!
     * Returns true if viewports are mapped to virtual desktops.
     */
    static bool mapViewport();

    /*!
     * Sets the strut of window \a win to \a left_width
     * ranging from \a left_start to \a left_end on the left edge,
     * and simiarly for the other edges. For not reserving a strut, pass 0 as the width.
     * E.g. to reserve 10x10 square in the topleft corner, use e.g.
     * setExtendedStrut( w, 10, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 ).
     *
     * \a win The id of the window.
     *
     * \a left_width Width of the strut at the left edge.
     *
     * \a left_start Starting y coordinate of the strut at the left edge.
     *
     * \a left_end Ending y coordinate of the strut at the left edge.
     *
     * \a right_width Width of the strut at the right edge.
     *
     * \a right_start Starting y coordinate of the strut at the right edge.
     *
     * \a right_end Ending y coordinate of the strut at the right edge.
     *
     * \a top_width Width of the strut at the top edge.
     *
     * \a top_start Starting x coordinate of the strut at the top edge.
     *
     * \a top_end Ending x coordinate of the strut at the top edge.
     *
     * \a bottom_width Width of the strut at the bottom edge.
     *
     * \a bottom_start Starting x coordinate of the strut at the bottom edge.
     *
     * \a bottom_end Ending x coordinate of the strut at the bottom edge.
     */
    static void setExtendedStrut(WId win,
                                 qreal left_width,
                                 qreal left_start,
                                 qreal left_end,
                                 qreal right_width,
                                 qreal right_start,
                                 qreal right_end,
                                 qreal top_width,
                                 qreal top_start,
                                 qreal top_end,
                                 qreal bottom_width,
                                 qreal bottom_start,
                                 qreal bottom_end);
    /*!
     * Convenience function for setExtendedStrut() that automatically makes struts
     * as wide/high as the screen width/height.
     * Sets the strut of window \a win to \a left, \a right, \a top, \a bottom.
     * \sa setExtendedStrut
     */
    static void setStrut(WId win, qreal left, qreal right, qreal top, qreal bottom);

    /*!
     * Sets the type of window \a win to \a windowType.
     * \sa NET::WindowType
     * \since 6.0
     */
    static void setType(WId win, NET::WindowType windowType);

    /*!
     * Clears the state of window \a win from \a state.
     *
     * Possible values are OR'ed combinations of:
     *
     * \list
     * \li NET::Modal
     * \li NET::Sticky
     * \li NET::MaxVert
     * \li NET::MaxHoriz
     * \li NET::Shaded
     * \li NET::SkipTaskbar
     * \li NET::SkipPager
     * \li NET::Hidden
     * \li NET::FullScreen
     * \li NET::KeepAbove
     * \li NET::KeepBelow
     * \li NET::SkipSwitcher
     * \endlist
     * \since 6.0
     */
    static void clearState(WId win, NET::States state);

    /*!
     * Sets the state of window \a win to \a state.
     *
     * Possible values are OR'ed combinations of:
     * \list
     * \li NET::Modal
     * \li NET::Sticky
     * \li NET::MaxVert
     * \li NET::MaxHoriz
     * \li NET::Shaded
     * \li NET::SkipTaskbar
     * \li NET::SkipPager
     * \li NET::Hidden
     * \li NET::FullScreen
     * \li NET::KeepAbove
     * \li NET::KeepBelow
     * \li NET::SkipSwitcher
     * \endlist
     * \since 6.0
     */
    static void setState(WId win, NET::States state);

Q_SIGNALS:

    /*!
     * Switched to another virtual \a desktop.
     */
    void currentDesktopChanged(int desktop);

    /*!
     * A window with the given \a id has been added.
     */
    void windowAdded(WId id);

    /*!
     * A window with the given \a id has been removed.
     */
    void windowRemoved(WId id);

    /*!
     * Hint that <Window> with the given \a id is active (= has focus) now.
     */
    void activeWindowChanged(WId id);

    /*!
     * Desktops have been renamed.
     */
    void desktopNamesChanged();

    /*!
     * The number \a num of desktops changed.
     */
    void numberOfDesktopsChanged(int num);

    /*!
     * The workarea has changed.
     */
    void workAreaChanged();

    /*!
     * Something changed with the struts, may or may not have changed the work area.
     *
     * Usually just using the workAreaChanged() signal is sufficient.
     */
    void strutChanged();

    /*!
     * Emitted when the stacking order of the window changed.
     *
     * The new order can be obtained with stackingOrder().
     */
    void stackingOrderChanged();

    /*!
     * The window with the given \a id changed.
     *
     * Carries the NET::Properties \a properties and
     * NET::Properties2 \a properties2 that were changed.
     */
    void windowChanged(WId id, NET::Properties properties, NET::Properties2 properties2);

    /*!
     * Compositing was \a enabled or disabled.
     *
     * Note that this signal may be emitted before any compositing plugins
     * have been initialized in the window manager.
     *
     * If you need to check if a specific compositing plugin such as the
     * blur effect is enabled, you should track that separately rather
     * than test for it in a slot connected to this signal.
     */
    void compositingChanged(bool enabled);

protected:
    void connectNotify(const QMetaMethod &signal) override;

private:
    friend class KWindowInfo;
    friend class KWindowSystemPrivateX11;
    friend class NETEventFilter;
    friend class MainThreadInstantiator;

    enum FilterInfo {
        INFO_BASIC = 1, // desktop info, not per-window
        INFO_WINDOWS = 2, // also per-window info
    };

    KWINDOWSYSTEM_NO_EXPORT void init(FilterInfo info);
    KWINDOWSYSTEM_NO_EXPORT QPoint desktopToViewport(int desktop, bool absolute);
    KWINDOWSYSTEM_NO_EXPORT int viewportToDesktop(const QPoint &pos);

    KWINDOWSYSTEM_NO_EXPORT QPoint constrainViewportRelativePosition(const QPoint &pos);

    // used in xcb/kwindowsystem.cpp
    static bool showingDesktop();
    static void setShowingDesktop(bool showing);

    /*!
     * \internal
     * Returns mapped virtual desktop for the given window geometry.
     */
    KWINDOWSYSTEM_NO_EXPORT static int viewportWindowToDesktop(const QRect &r);

    KWINDOWSYSTEM_NO_EXPORT NETEventFilter *s_d_func()
    {
        return d.get();
    }
    std::unique_ptr<NETEventFilter> d;
};

#endif
