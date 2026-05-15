/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1997 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Sven Radej <radej@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999 Chris Schlaeger <cs@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KXMLGUIWINDOW_H
#define KXMLGUIWINDOW_H

#include "kmainwindow.h"
#include "kxmlguibuilder.h"
#include "kxmlguiclient.h"

class KMenu;
class KXMLGUIFactory;
class KConfig;
class KConfigGroup;
class KToolBar;
class KXmlGuiWindowPrivate;

/*!
 * \class KXmlGuiWindow kxmlguiwindow.h KXmlGuiWindow
 * \inmodule KXmlGui
 *
 * \brief KMainWindow with convenience functions and integration with XmlGui files.
 *
 * This class includes several convenience \c <action>Enabled() functions
 * to toggle the presence of functionality in your main window,
 * including a KCommandBar instance.
 *
 * The \l StandardWindowOptions enum can be used to pass additional options
 * to describe the main window behavior/appearance.
 * Use it in conjunction with setupGUI() to load an appnameui.rc file
 * to manage the main window's actions.
 *
 * setCommandBarEnabled() is set by default.
 *
 * A minimal example can be created with
 * QMainWindow::setCentralWidget() and setupGUI():
 *
 * \code
 * MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent) {
 *   textArea = new KTextEdit();
 *   setCentralWidget(textArea);
 *   setupGUI(Default);
 * }
 * \endcode
 *
 * With this, a ready-made main window with menubar and statusbar is created,
 * as well as two default menus, Settings and Help.
 *
 * Management of QActions is made trivial in combination with
 * KActionCollection and KStandardAction.
 *
 * \code
 * void MainWindow::setupActions() {
 *   QAction *clearAction = new QAction(this);
 *   clearAction->setText(i18n("&Clear"));
 *   clearAction->setIcon(QIcon::fromTheme("document-new"));
 *   KActionCollection::setDefaultShortcut(clearAction, Qt::CTRL + Qt::Key_W);
 *   actionCollection()->addAction("clear", clearAction);
 *   connect(clearAction, &QAction::triggered, textArea, &KTextEdit::clear);
 *   KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
 *   setupGUI(Default, "texteditorui.rc");
 * }
 * \endcode
 *
 * See https://develop.kde.org/docs/use/kxmlgui/ for a tutorial
 * on how to create a simple text editor using KXmlGuiWindow.
 *
 * See https://develop.kde.org/docs/use/session-managment for more information on session management.
 *
 * \sa KMainWindow
 * \sa KActionCollection
 * \sa KStandardAction
 * \sa setupGUI()
 * \sa createGUI()
 * \sa setCommandBarEnabled()
 */

class KXMLGUI_EXPORT KXmlGuiWindow : public KMainWindow, public KXMLGUIBuilder, virtual public KXMLGUIClient
{
    Q_OBJECT

    /*!
     * \property KXmlGuiWindow::hasMenuBar
     */
    Q_PROPERTY(bool hasMenuBar READ hasMenuBar)

    /*!
     * \property KXmlGuiWindow::autoSaveSettings
     */
    Q_PROPERTY(bool autoSaveSettings READ autoSaveSettings)

    /*!
     * \property KXmlGuiWindow::autoSaveGroup
     */
    Q_PROPERTY(QString autoSaveGroup READ autoSaveGroup)

    /*!
     * \property KXmlGuiWindow::standardToolBarMenuEnabled
     */
    Q_PROPERTY(bool standardToolBarMenuEnabled READ isStandardToolBarMenuEnabled WRITE setStandardToolBarMenuEnabled)
    Q_PROPERTY(QStringList toolBars READ toolBarNames)

public:
    /*!
     * \brief Construct a main window.
     *
     * Note that by default a KXmlGuiWindow is created with the
     * Qt::WA_DeleteOnClose attribute set, i.e. it is automatically destroyed
     * when the window is closed. If you do not want this behavior, call:
     *
     * \code
     * window->setAttribute(Qt::WA_DeleteOnClose, false);
     * \endcode
     *
     * KXmlGuiWindows must be created on the heap with 'new', like:
     *
     * \code
     * KXmlGuiWindow *kmw = new KXmlGuiWindow(...);
     * kmw->setObjectName(...);
     * \endcode
     *
     * \note For session management and window management to work
     * properly, all main windows in the application should have a
     * different name. Otherwise, the base class KMainWindow will create
     * a unique name, but it's recommended to explicitly pass a window name that will
     * also describe the type of the window. If there can be several windows of the same
     * type, append '#' (hash) to the name, and KMainWindow will replace it with numbers to make
     * the names unique. For example, for a mail client which has one main window showing
     * the mails and folders, and which can also have one or more windows for composing
     * mails, the name for the folders window should be e.g. "mainwindow" and
     * for the composer windows "composer#".
     *
     * \a parent The widget parent. This is usually \c nullptr,
     * but it may also be the window group leader.
     * In that case, the KXmlGuiWindow becomes a secondary window.
     *
     * \a flags Specify the window flags. The default is none.
     *
     * \sa KMainWindow::KMainWindow
     */
    explicit KXmlGuiWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    /*!
     * \brief Destructor.
     *
     * Will also destroy the toolbars and menubar if needed.
     */
    ~KXmlGuiWindow() override;

    /*!
     * \brief Creates a standard help menu when calling createGUI()
     * or setupGUI().
     *
     * \a showHelpMenu Whether to create a Help Menu. \c true by default.
     *
     * \sa isHelpMenuEnabled()
     */
    void setHelpMenuEnabled(bool showHelpMenu = true);

    /*!
     * Returns \c true if the help menu is enabled, \c false if setHelpMenuEnabled(false) was set.
     * \sa setHelpMenuEnabled()
     */
    bool isHelpMenuEnabled() const;

    /*!
     *
     */
    virtual KXMLGUIFactory *guiFactory();

    /*!
     * \brief Generates the interface based on a local XML file.
     *
     * This is the function that generates UI elements such as the main menu,
     * toolbar (if any) and statusbar. This is called by setupGUI(Create) as well.
     *
     * Typically, in a regular application, you would use setupGUI()
     * instead, as it sets up the toolbar/shortcut
     * edit actions, among other things.
     *
     * If \a xmlfile is an empty string, this method will try to construct
     * a local XML filename like appnameui.rc where 'appname' is your app's
     * name. Typically that app name is what KXMLGUIClient::componentName()
     * returns. If that file does not exist, then the XML UI code will use only
     * the global (standard) XML file for its layout purposes.
     *
     * \a xmlfile The path (relative or absolute) to the local xmlfile.
     *
     * \sa setupGUI()
     */
    void createGUI(const QString &xmlfile = QString());

    /*!
     * \brief Creates a toggle under the 'Settings' menu to show/hide the available toolbars.
     *
     * The standard toolbar menu toggles the visibility of one or multiple toolbars.
     *
     * If there is only one toolbar configured, a simple 'Show \<toolbar name\>'
     * menu item is shown; if more than one toolbar is configured, a "Shown Toolbars"
     * menu is created instead, with 'Show \<toolbar1 name\>', 'Show \<toolbar2 name\>'
     * ... sub-menu actions.
     *
     * If your application uses a non-default XmlGui resource file, then you can
     * specify the exact position of the menu/menu item by adding a
     * \c <Merge name="StandardToolBarMenuHandler">
     * line to the settings menu section of your resource file ( usually appname.rc ).
     *
     * \a showToolBarMenu Whether to show the standard toolbar menu. \c false by default.
     *
     * \note This function only makes sense before calling createGUI().
     * Using setupGUI(ToolBar) overrides this function.
     *
     * \sa createGUI()
     * \sa setupGUI()
     * \sa KToggleBarAction
     * \sa StandardWindowOption
     * \sa KMainWindow::toolBar()
     * \sa KMainWindow::toolBars()
     * \sa QMainWindow::addToolBar()
     * \sa QMainWindow::removeToolBar()
     * \sa createStandardStatusBarAction()
     */
    void setStandardToolBarMenuEnabled(bool showToolBarMenu);

    /*!
     * \brief Returns whether setStandardToolBarMenuEnabled() was set.
     *
     * \note This function only makes sense if createGUI() was used.
     * This function returns true only if setStandardToolBarMenuEnabled() was set
     * and will return false even if \l StandardWindowOption::ToolBar was used.
     *
     * Returns \c true if setStandardToolBarMenuEnabled() was set, \c false otherwise.
     *
     * \sa createGUI()
     * \sa setupGUI()
     * \sa setStandardToolBarMenuEnabled()
     * \sa StandardWindowOption
     */
    bool isStandardToolBarMenuEnabled() const;

    /*!
     * \brief Creates a toggle under the 'Settings' menu to show/hide the statusbar.
     *
     * Calling this method will create a statusbar if one doesn't already exist.
     *
     * If an application maintains the action on its own (i.e. never calls
     * this function), a connection needs to be made to let KMainWindow
     * know when the hidden/shown status of the statusbar has changed.
     * For example:
     * \code
     * connect(action, &QAction::triggered,
     *         kmainwindow, &KMainWindow::setSettingsDirty);
     * \endcode
     * Otherwise the status might not be saved by KMainWindow.
     *
     * \note This function only makes sense before calling createGUI()
     * or when using setupGUI() without \l StandardWindowOption::StatusBar.
     *
     * \sa createGUI()
     * \sa setupGUI()
     * \sa StandardWindowOption
     * \sa KStandardAction::showStatusbar()
     * \sa setStandardToolBarMenuEnabled()
     * \sa QMainWindow::setStatusBar()
     * \sa QMainWindow::statusBar()
     */
    void createStandardStatusBarAction();

    /*!
     * \enum KXmlGuiWindow::StandardWindowOption
     *
     * Use these options for the first argument of setupGUI().
     * \sa setupGUI()
     *
     * \value ToolBar
     *        Adds action(s) to show/hide the toolbar(s) and adds a menu
     *        action to configure the toolbar(s).
     *        \sa setStandardToolBarMenuEnabled()
     *        \sa isStandardToolBarMenuEnabled()
     * \value Keys
     *        Adds an action in the 'Settings' menu to open the configure
     *        keyboard shortcuts dialog.
     * \value StatusBar
     *        Adds an action to show/hide the statusbar in the 'Settings' menu.
     *        Note that setting this value will create a statusbar
     *        if one doesn't already exist.
     *        \sa createStandardStatusBarAction()
     * \value Save
     *        Autosaves (and loads) the toolbar/menubar/statusbar settings and
     *        window size using the default name.
     *        Like KMainWindow::setAutoSaveSettings(), enabling this causes the application
     *        to save state data upon close in a KConfig-managed configuration file.
     *        Typically you want to let the default window size be determined by
     *        the widgets' size hints. Make sure that setupGUI() is called after
     *        all the widgets are created (including QMainWindow::setCentralWidget())
     *        so that the default size is managed properly.
     *        \sa KMainWindow::setAutoSaveSettings()
     *        \sa KConfig
     * \value Create
     *        Calls createGUI() once ToolBar, Keys and Statusbar have been
     *        taken care of.
     *        \sa createGUI()
     * \value Default
     *        Sets all of the above options as true.
     *
     * \note  In the case of KXmlGuiWindow::Create, when using KParts::MainWindow,
     *        remove this flag from the setupGUI() call, since you'll be using
     *        createGUI(part) instead:
     * \code
     * setupGUI(ToolBar | Keys | StatusBar | Save);
     * \endcode
     */
    enum StandardWindowOption {
        ToolBar = 1,
        Keys = 2,
        StatusBar = 4,
        Save = 8,
        Create = 16,
        Default = ToolBar | Keys | StatusBar | Save | Create,
    };
    Q_FLAG(StandardWindowOption)
    Q_DECLARE_FLAGS(StandardWindowOptions, StandardWindowOption)

    /*!
     * \brief Configures the current window and its actions in the typical KDE
     * fashion.
     *
     * You can specify which window options/features are going to be set up using
     * \a options, see the \l StandardWindowOptions enum for more details.
     *
     * \code
     * MainWindow::MainWindow(QWidget* parent) : KXmlGuiWindow(parent){
     *   textArea = new KTextEdit();
     *   setCentralWidget(textArea);
     *   setupGUI(Default, "appnameui.rc");
     * }
     * \endcode
     *
     * Use a bitwise OR (|) to select multiple enum choices for setupGUI()
     * (except when using StandardWindowOptions::Default).
     *
     * \code
     * setupGUI(Save | Create, "appnameui.rc");
     * \endcode
     *
     * Typically this function replaces createGUI(),
     * but it is possible to call setupGUI(Create) together with helper functions
     * such as setStandardToolBarMenuEnabled() and createStandardStatusBarAction().
     *
     * \warning To use createGUI() and setupGUI()
     * for the same window, you must avoid using
     * \l StandardWindowOption::Create. Prefer using only setupGUI().
     *
     * \note When \l StandardWindowOption::Save is used,
     * this method will restore the state of the application
     * window (toolbar, dockwindows positions ...etc), so you need to have
     * added all your actions to your UI before calling this
     * method.
     *
     * \a options A combination of \l StandardWindowOptions to specify
     * UI elements to be present in your application window.
     *
     * \a xmlfile The relative or absolute path to the local xmlfile.
     * If this is an empty string, the code will look for a local XML file
     * appnameui.rc, where 'appname' is the name of your app. See the note
     * about the xmlfile argument in createGUI().
     *
     * \sa StandardWindowOption
     */
    void setupGUI(StandardWindowOptions options = Default, const QString &xmlfile = QString());

    /*!
     * \brief This is an overloaded function.
     *
     * \a defaultSize A manually specified window size that overrides the saved size.
     *
     * \a options A combination of \l StandardWindowOptions to specify
     * UI elements to be present in your application window.
     *
     * \a xmlfile The relative or absolute path to the local xmlfile.
     * \sa setupGUI()
     */
    void setupGUI(const QSize &defaultSize, StandardWindowOptions options = Default, const QString &xmlfile = QString());

    /*!
     * Returns a pointer to the main window's action responsible for the toolbar's menu.
     */
    QAction *toolBarMenuAction();

    /*!
     * \internal for KToolBar
     */
    void setupToolbarMenuActions();

    /*!
     * Returns a list of all toolbars for this window in string form
     * \sa KMainWindow::toolBars()
     * \since 6.10
     */
    QStringList toolBarNames() const;

    /*!
     *
     */
    void finalizeGUI(bool force);
    using KXMLGUIBuilder::finalizeGUI;

    // reimplemented for internal reasons
    void applyMainWindowSettings(const KConfigGroup &config) override;

    /*!
     * \brief Enable a KCommandBar to list and quickly execute actions.
     *
     * A KXmlGuiWindow by default automatically creates a KCommandBar,
     * but it is inaccessible unless createGUI() or setupGUI(Create) is used.
     *
     * It provides a HUD-like menu that lists all QActions in your application
     * and can be activated via Ctrl+Atl+i or via an action in the 'Help' menu.
     *
     * If you need more than a global set of QActions listed for your application,
     * use KCommandBar directly instead.
     *
     * \a showCommandBar Whether to show the command bar. \c true by default.
     *
     * \since 5.83
     *
     * \sa KCommandBar
     * \sa KCommandBar::setActions()
     * \sa isCommandBarEnabled()
     */
    void setCommandBarEnabled(bool showCommandBar);

    /*!
     * \brief Whether a KCommandBar was set.
     *
     * Returns \c true by default, \c false if setCommandBarEnabled(false) was set.
     *
     * \since 5.83
     * \sa setCommandBarEnabled()
     */
    bool isCommandBarEnabled() const;

public Q_SLOTS:
    /*!
     * \brief Show a standard configure toolbar dialog.
     *
     * This slot can be connected directly to the action to configure the toolbar.
     *
     * Example code:
     * \code
     * KStandardAction::configureToolbars(this, &KXmlGuiWindow::configureToolbars, actionCollection);
     * \endcode
     */
    virtual void configureToolbars();

    /*!
     * \brief Applies a state change
     *
     * Reimplement this to enable and disable actions as defined in the XmlGui rc file.
     *
     * \a newstate The state change to be applied.
     */
    virtual void slotStateChanged(const QString &newstate);

    /*!
     * \brief Applies a state change
     *
     * Reimplement this to enable and disable actions as defined in the XmlGui rc file.
     *
     * This function can "reverse" the state (disable the actions which should be
     * enabled, and vice-versa) if specified.
     *
     * \a newstate The state change to be applied.
     *
     * \a reverse Whether to reverse \a newstate or not.
     */
    void slotStateChanged(const QString &newstate, bool reverse);

    /*!
     * \brief Checks the visual state of a given toolbar. If an invalid name is
     * given, the method will always return false.
     *
     * This does not utilize KMainWindow::toolBar(), as that method would
     * create a new toolbar in the case of an invalid name, which is not
     * something that should be freely accessable via the dbus.
     *
     * \a name The internal name of the toolbar.
     *
     * Returns whether the toolbar with the specified name is currently visible
     *
     * \sa toolBarNames()
     * \sa setToolBarVisible()
     * \sa KMainWindow::toolBar()
     * \since 6.10
     */
    bool isToolBarVisible(const QString &name);

    /*!
     * \brief Sets the visibility of a given toolbar. If an invalid name is
     * given, nothing happens.
     *
     * \a name The internal name of the toolbar.
     *
     * \a visible The new state of the toolbar's visibility.
     * \since 6.10
     */
    void setToolBarVisible(const QString &name, bool visible);

protected:
    /*!
     * Reimplemented to return the \a event QEvent::Polish in order to adjust the object name
     * if needed, once all constructor code for the main window has run.
     *
     * Also reimplemented to catch when a QDockWidget is added or removed.
     */
    bool event(QEvent *event) override;

    /*!
     * \brief Checks if there are actions using the same shortcut.
     *
     * This is called automatically from createGUI().
     *
     * \since 5.30
     */
    void checkAmbiguousShortcuts();

protected Q_SLOTS:
    /*!
     * \brief Rebuilds the GUI after KEditToolBar changes the toolbar layout.
     * \sa configureToolbars()
     */
    virtual void saveNewToolbarConfig();

private:
    Q_DECLARE_PRIVATE(KXmlGuiWindow)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KXmlGuiWindow::StandardWindowOptions)

#endif
