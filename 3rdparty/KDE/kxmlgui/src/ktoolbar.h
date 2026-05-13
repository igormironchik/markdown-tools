/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Sven Radej <radej@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997, 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KTOOLBAR_H
#define KTOOLBAR_H

#include <kxmlgui_export.h>

#include <QToolBar>
#include <memory>

class QDomElement;

class KConfigGroup;
class KConfig;
class KMainWindow;
class KXMLGUIClient;

/*!
 * \class KToolBar
 * \inmodule KXmlGui
 *
 * \brief Floatable toolbar with auto resize.
 *
 * A KDE-style toolbar.
 *
 * KToolBar can be used as a standalone widget, but KMainWindow
 * provides easy factories and management of one or more toolbars.
 *
 * KToolBar uses a global config group to load toolbar settings on
 * construction. It will reread this config group on a
 * KApplication::appearanceChanged() signal.
 *
 * KToolBar respects Kiosk settings (see the KAuthorized namespace in the
 * KConfig framework).  In particular, system administrators can prevent users
 * from moving toolbars with the "movable_toolbars" action, and from showing or
 * hiding toolbars with the "options_show_toolbar" action.  For example, to
 * disable both, add the following the application or global configuration:
 * \badcode
 * [KDE Action Restrictions][$i]
 * movable_toolbars=false
 * options_show_toolbar=false
 * \endcode
 *
 * If you can't depend on KXmlGui but still want to integrate better with KDE,
 * you can use QToolBar and:
 * \list
 *      \li Set ToolButtonStyle to Qt::ToolButtonFollowStyle,
 *          this will make QToolBar use the settings for "Main Toolbar".
 *      \li Additionally set QToolBar::setProperty("otherToolbar", true)
 *          to use settings for "Other toolbars"; settings from "Other toolbars"
 *          will only work on widget styles derived from KStyle.
 * \endlist
 */
class KXMLGUI_EXPORT KToolBar : public QToolBar
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor.
     *
     * This constructor takes care of adding the toolbar to the mainwindow,
     * if \a parent is a QMainWindow.
     *
     * Normally KDE applications do not call this directly, they either
     * call KMainWindow::toolBar(), or they use XML-GUI and specify
     * toolbars using XML.
     *
     * \a parent The standard toolbar parent (usually a KMainWindow).
     *
     * \a isMainToolBar True for the "main toolbar", false for other toolbars.
     * Different settings apply.
     *
     * \a readConfig Whether to apply the configuration (global and application-specific)
     */
    explicit KToolBar(QWidget *parent, bool isMainToolBar = false, bool readConfig = true);
    // KDE5: remove. The one below is preferred so that all debug output from init() shows the right objectName already,
    // and so that isMainToolBar() and iconSizeDefault() return correct values during loading too.

    /*!
     * \brief Constructor.
     *
     * This constructor takes care of adding the toolbar to the mainwindow,
     * if \a parent is a QMainWindow.
     *
     * Normally KDE applications do not call this directly, they either
     * call KMainWindow::toolBar(), or they use XML-GUI and specify
     * toolbars using XML.
     *
     * \a objectName The QObject name of this toolbar, required
     * so that QMainWindow can save and load the toolbar position,
     * and so that KToolBar can find out if it's the main toolbar.
     *
     * \a parent The standard toolbar parent (usually a KMainWindow).
     *
     * \a readConfig Whether to apply the configuration (global and application-specific).
     */
    explicit KToolBar(const QString &objectName, QWidget *parent, bool readConfig = true);

    /*!
     * \brief Alternate constructor with additional arguments,
     * e.g. to choose in which area the toolbar should be auto-added.
     * This is rarely used in KDE. When using XMLGUI
     * you can specify this as an xml attribute instead.
     *
     * \a objectName  The QObject name of this toolbar, required so that QMainWindow can save and load the toolbar position.
     *
     * \a parentWindow The window that should be the parent of this toolbar.
     *
     * \a area        The position of the toolbar. Usually Qt::TopToolBarArea.
     *
     * \a newLine     If true, start a new line in the dock for this toolbar.
     *
     * \a isMainToolBar  True for the "main toolbar", false for other toolbars.
     * Different settings apply.
     * \a readConfig  whether to apply the configuration (global and application-specific)
     */
    KToolBar(const QString &objectName,
             QMainWindow *parentWindow,
             Qt::ToolBarArea area,
             bool newLine = false,
             bool isMainToolBar = false,
             bool readConfig = true); // KDE5: remove, I don't think anyone is using this.

    /*!
     * \brief Destroys the toolbar.
     */
    ~KToolBar() override;

    /*!
     * \brief Returns the main window that this toolbar is docked with.
     */
    KMainWindow *mainWindow() const;

    /*!
     * \brief Convenience function to set icon \a size.
     */
    void setIconDimensions(int size);

    /*!
     * \brief Returns the default size for this type of toolbar.
     */
    int iconSizeDefault() const; // KDE5: hide from public API. Doesn't make sense to export this, and it isn't used.

    /*!
     * \brief Save the toolbar settings to group \a cg.
     */
    void saveSettings(KConfigGroup &cg);

    /*!
     * \brief Read the toolbar settings from group \a cg and apply them.
     */
    void applySettings(const KConfigGroup &cg);

    /*!
     * \brief Adds an XML gui \a client that uses this toolbar.
     * \since 4.8.1
     */
    void addXMLGUIClient(KXMLGUIClient *client);

    /*!
     * \brief Removes an XML gui \a client that uses this toolbar.
     * \since 4.8.5
     */
    void removeXMLGUIClient(KXMLGUIClient *client);

    /*!
     * \brief Load state from an XML \a element, called by KXMLGUIBuilder.
     */
    void loadState(const QDomElement &element);

    /*!
     * \brief Save state into an XML \a element, called by KXMLGUIBuilder.
     */
    void saveState(QDomElement &element) const;

    /*!
     * \brief Filters then returns the specified \a event
     * for a given \a watched object.
     *
     * Reimplemented to support context menu activation on disabled tool buttons.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /*!
     * \brief Returns whether the toolbars are currently editable (drag & drop of actions).
     */
    static bool toolBarsEditable();

    /*!
     * \brief Makes all toolbars \a editable via drag & drop of actions.
     *
     * This is called by KEditToolBar and should generally be set to disabled
     * whenever KEditToolBar is not active.
     */
    static void setToolBarsEditable(bool editable);

    /*!
     * \brief Returns whether the toolbars are locked
     * (that is, disallow moving of the toolbars).
     */
    static bool toolBarsLocked();

    /*!
     * \brief Makes all toolbars \a locked
     * (that is, disallow/allow moving of the toolbars).
     */
    static void setToolBarsLocked(bool locked);

    /*!
     * \brief Emits a D-Bus signal to tell all toolbars in all applications
     * that the user settings have changed.
     * \since 5.0
     */
    static void emitToolbarStyleChanged();

protected Q_SLOTS:
    virtual void slotMovableChanged(bool movable);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;
    void actionEvent(QActionEvent *) override;

    // Draggable toolbar configuration
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dropEvent(QDropEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private:
    friend class KToolBarPrivate;
    std::unique_ptr<class KToolBarPrivate> const d;

    Q_PRIVATE_SLOT(d, void slotAppearanceChanged())
    Q_PRIVATE_SLOT(d, void slotContextRight())
    Q_PRIVATE_SLOT(d, void slotContextTextRight())
};

#endif
