/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KGUISTANDARDACTION_H
#define KGUISTANDARDACTION_H

#include <QAction>
#include <QList>
#include <QStringList>

#include <KStandardShortcut>
#include <kconfiggui_export.h>

#include <optional>
#include <type_traits>

class QObject;
class QAction;

/*!
 * \namespace KStandardActions
 * \inmodule KConfigGui
 *
 * \brief Convenience methods to access all standard KDE actions.
 *
 * These actions should be used instead of hardcoding menubar and
 * toolbar items.  Using these actions helps your application easily
 * conform to the \l {https://develop.kde.org/hig/} {KDE Human Interface Guidelines}.
 *
 * All of the documentation for QAction holds for KStandardActions
 * also.  When in doubt on how things work, check the QAction
 * documentation first.
 * Please note that calling any of these methods automatically adds the action
 * to the actionCollection() of the QObject given by the 'parent' parameter.
 *
 * Simple Example:
 *
 * In general, using standard actions should be a drop in replacement
 * for regular actions. For example, if you previously had:
 * \code
 * QAction *newAct = new QAction(QIcon::fromTheme("document-new"),
 *                               i18n("&New"),
 *                               this);
 * newAct->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::New).constFirst());
 * connect(newAct, &QAction::triggered, this, &ClassFoo::fileNew);
 * \endcode
 *
 * You can replace it with:
 * \code
 * QAction *newAct = KStandardActions::openNew(this, &ClassFoo::fileNew, this);
 * \endcode
 *
 * Alternatively you can instantiate the action using the StandardAction enums
 * provided.  This author can't think of a reason why you would want to, but, hey,
 * if you do, here's how:
 *
 * \code
 * QAction *newAct = KStandardActions::create(KStandardActions::New, this, &ClassFoo::fileNew, this);
 * \endcode
 *
 * Relationship with KActionCollection from KXMLGui
 *
 * If a KActionCollection is passed as the parent then the action will be
 * automatically added to that collection:
 * \code
 * QAction *cut = KStandardActions::cut(this, &ClassFoo::editCut, actionCollection());
 * \endcode
 *
 * Each action has a unique internal name which can be queried using the
 * name method.  For example KStandardActions::name(KStandardActions::Cut)
 * would return 'edit_cut'.  This name can be used as a unique identifier
 * for the actions.  So if you wanted to add an existing standard action
 * to an action collection you can do so like this:
 * \code
 * QAction *cut = KStandardActions::cut(this, &ClassFoo::editCut, this);
 * actionCollection()->addAction(KStandardActions::name(KStandardActions::Cut), cut);
 * \endcode
 *
 * You can then get a pointer to the action using
 * \code
 * QAction *cutPtr = actionCollection()->action(KStandardActions::name(KStandardActions::Cut));
 * \endcode
 *
 * \since 6.3
 */
namespace KStandardActions
{
/*!
 * The standard menubar and toolbar actions.
 * \omitvalue ActionNone,
 * \value New Create a new document or window.
 * \value Open Open an existing file.
 * \value OpenRecent Open a recently used document.
 * \value Save Save the current document.
 * \value SaveAs Save the current document under a different name.
 * \value Revert Revert the current document to the last saved version.
 * \value Close Close the current document.
 * \value Print Print the current document.
 * \value PrintPreview Show a print preview of the current document.
 * \value Mail Send the current document by mail.
 * \value Quit Quit the program.
 * \value Undo Undo the last operation.
 * \value Redo Redo the last operation.
 * \value Cut Cut selected area and store it in the clipboard.
 * \value Copy Copy selected area and store it in the clipboard.
 * \value Paste Paste the contents of clipboard at the current mouse or cursor.
 * \value SelectAll Select all elements in the current document.
 * \value Deselect Deselect any selected elements in the current document.
 * \value Find Initiate a 'find' request in the current document.
 * \value FindNext Find the next instance of a stored 'find'
 * \value FindPrev Find a previous instance of a stored 'find'.
 * \value Replace Find and replace matches.
 * \value ActualSize View the document at its actual size.
 * \value FitToPage Fit the document view to the size of the current window.
 * \value FitToWidth Fit the document view to the width of the current window.
 * \value FitToHeight Fit the document view to the height of the current window.
 * \value ZoomIn Zoom in the current document.
 * \value ZoomOut Zoom out the current document.
 * \value Zoom Select the current zoom level.
 * \value Redisplay Redisplay or redraw the document.
 * \value Up Move up (web style menu).
 * \value Back Move back (web style menu).
 * \value Forward Move forward (web style menu).
 * \value Home Go to the "Home" position or document.
 * \value Prior Scroll up one page.
 * \value Next Scroll down one page.
 * \value Goto Jump to some specific location in the document.
 * \value GotoPage Go to a specific page.
 * \value GotoLine Go to a specific line.
 * \value FirstPage Jump to the first page.
 * \value LastPage Jump to the last page.
 * \value DocumentBack Move back (document style menu).
 * \value DocumentForward Move forward (document style menu).
 * \value AddBookmark Add the current page to the bookmarks tree.
 * \value EditBookmarks Edit the application bookmarks.
 * \value Spelling Pop up the spell checker.
 * \value ShowMenubar Show/Hide the menubar.
 * \value ShowToolbar Show/Hide the toolbar.
 * \value ShowStatusbar Show/Hide the statusbar.
 * \value KeyBindings Display the configure key bindings dialog.
 * \value Preferences Display the preferences/options dialog.
 * \value ConfigureToolbars Display the toolbar configuration dialog.
 * \value HelpContents Display the handbook of the application.
 * \value WhatsThis Trigger the What's This cursor.
 * \value ReportBug Open up the Report Bug dialog.
 * \value AboutApp Display the application's About box.
 * \value AboutKDE Display the About KDE dialog.
 * \value ConfigureNotifications Display the notifications configuration dialog.
 * \value FullScreen Switch to/from full screen mode.
 * \value Clear Clear the content of the focus widget.
 * \value SwitchApplicationLanguage Display the Switch Application Language dialog.
 * \value DeleteFile Permanently deletes files or folders.
 * \value RenameFile Renames files or folders.
 * \value MoveToTrash Moves files or folders to the trash.
 * \value Donate Open donation page on kde.org.
 * \value HamburgerMenu Opens a menu that substitutes the menubar.
 */
enum StandardAction {
    ActionNone,
    // File Menu
    New,
    Open,
    OpenRecent,
    Save,
    SaveAs,
    Revert,
    Close,
    Print,
    PrintPreview,
    Mail,
    Quit,
    // Edit Menu
    Undo,
    Redo,
    Cut,
    Copy,
    Paste,
    SelectAll,
    Deselect,
    Find,
    FindNext,
    FindPrev,
    Replace,
    // View Menu
    ActualSize,
    FitToPage,
    FitToWidth,
    FitToHeight,
    ZoomIn,
    ZoomOut,
    Zoom,
    Redisplay,
    // Go Menu
    Up,
    Back,
    Forward,
    Home,
    Prior,
    Next,
    Goto,
    GotoPage,
    GotoLine,
    FirstPage,
    LastPage,
    DocumentBack,
    DocumentForward,
    // Bookmarks Menu
    AddBookmark,
    EditBookmarks,
    // Tools Menu
    Spelling,
    // Settings Menu
    ShowMenubar,
    ShowToolbar,
    ShowStatusbar,
    KeyBindings,
    Preferences,
    ConfigureToolbars,
    // Help Menu
    HelpContents,
    WhatsThis,
    ReportBug,
    AboutApp,
    AboutKDE,
    // Other standard actions
    ConfigureNotifications,
    FullScreen,
    Clear,
    SwitchApplicationLanguage,
    DeleteFile,
    RenameFile,
    MoveToTrash,
    Donate,
    HamburgerMenu
    // To keep in sync with KConfigWidgets::KStandardAction
};

KCONFIGGUI_EXPORT QAction *_kgui_createInternal(StandardAction id, QObject *parent);

/*!
 * Creates an action corresponding to one of the
 * KStandardActions::StandardAction actions, which is connected to the given
 * object and \a slot, and is owned by \a parent.
 *
 * If not explicitly specified, \a connectionType will be AutoConnection for all actions
 * except for ConfigureToolbars it will be QueuedConnection.
 *
 * \sa create(StandardAction, const QObject *, const char *, QObject *)
 */
template<class Receiver, class Func>
inline QAction *create(StandardAction id, const Receiver *recvr, Func slot, QObject *parent, std::optional<Qt::ConnectionType> connectionType = std::nullopt)
{
    QAction *action = _kgui_createInternal(id, parent);
    // ConfigureToolbars is special because of bug #200815
    const Qt::ConnectionType defaultConnectionType = (id == ConfigureToolbars) ? Qt::QueuedConnection : Qt::AutoConnection;
    QObject::connect(action, &QAction::triggered, recvr, slot, connectionType.value_or(defaultConnectionType));
    return action;
}

/*!
 * This will return the internal name of a given standard action.
 */
KCONFIGGUI_EXPORT QString name(StandardAction id);

/*!
 * Returns a list of all actionIds.
 */
KCONFIGGUI_EXPORT QList<StandardAction> actionIds();

/*!
 * Returns the standardshortcut associated with \a actionId.
 *
 * \a id The identifier whose associated shortcut is wanted.
 */
KCONFIGGUI_EXPORT KStandardShortcut::StandardShortcut shortcutForActionId(StandardAction id);

/*!
 * Create a new document or window.
 */
template<class Receiver, class Func>
inline QAction *openNew(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(New, recvr, slot, parent);
}

/*!
 * Open an existing file.
 */
template<class Receiver, class Func>
inline QAction *open(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Open, recvr, slot, parent);
}

/*!
 * Save the current document.
 */
template<class Receiver, class Func>
inline QAction *save(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Save, recvr, slot, parent);
}

/*!
 * Save the current document under a different name.
 */
template<class Receiver, class Func>
inline QAction *saveAs(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(SaveAs, recvr, slot, parent);
}

/*!
 * Revert the current document to the last saved version
 * (essentially will undo all changes).
 */
template<class Receiver, class Func>
inline QAction *revert(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Revert, recvr, slot, parent);
}

/*!
 * Close the current document.
 */
template<class Receiver, class Func>
inline QAction *close(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Close, recvr, slot, parent);
}

/*!
 * Print the current document.
 */
template<class Receiver, class Func>
inline QAction *print(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Print, recvr, slot, parent);
}

/*!
 * Show a print preview of the current document.
 */
template<class Receiver, class Func>
inline QAction *printPreview(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(PrintPreview, recvr, slot, parent);
}

/*!
 * Mail this document.
 */
template<class Receiver, class Func>
inline QAction *mail(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Mail, recvr, slot, parent);
}

/*!
 * Quit the program.
 *
 * Note that you probably want to connect this action to either QWidget::close()
 * or QApplication::closeAllWindows(), but not QApplication::quit(), so that
 * KMainWindow::queryClose() is called on any open window (to warn the user
 * about unsaved changes for example).
 */
template<class Receiver, class Func>
inline QAction *quit(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Quit, recvr, slot, parent);
}

/*!
 * Undo the last operation.
 */
template<class Receiver, class Func>
inline QAction *undo(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Undo, recvr, slot, parent);
}

/*!
 * Redo the last operation.
 */
template<class Receiver, class Func>
inline QAction *redo(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Redo, recvr, slot, parent);
}

/*!
 * Cut selected area and store it in the clipboard.
 */
template<class Receiver, class Func>
inline QAction *cut(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Cut, recvr, slot, parent);
}

/*!
 * Copy the selected area into the clipboard.
 */
template<class Receiver, class Func>
inline QAction *copy(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Copy, recvr, slot, parent);
}

/*!
 * Paste the contents of clipboard at the current mouse or cursor
 * position.
 */
template<class Receiver, class Func>
inline QAction *paste(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Paste, recvr, slot, parent);
}

/*!
 * Clear the content of the focus widget
 */
template<class Receiver, class Func>
inline QAction *clear(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Clear, recvr, slot, parent);
}

/*!
 * Select all elements in the current document.
 */
template<class Receiver, class Func>
inline QAction *selectAll(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(SelectAll, recvr, slot, parent);
}

/*!
 * Deselect any selected elements in the current document.
 */
template<class Receiver, class Func>
inline QAction *deselect(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Deselect, recvr, slot, parent);
}

/*!
 * Initiate a 'find' request in the current document.
 */
template<class Receiver, class Func>
inline QAction *find(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Find, recvr, slot, parent);
}

/*!
 * Find the next instance of a stored 'find'.
 */
template<class Receiver, class Func>
inline QAction *findNext(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(FindNext, recvr, slot, parent);
}

/*!
 * Find a previous instance of a stored 'find'.
 */
template<class Receiver, class Func>
inline QAction *findPrev(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(FindPrev, recvr, slot, parent);
}

/*!
 * Find and replace matches.
 */
template<class Receiver, class Func>
inline QAction *replace(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Replace, recvr, slot, parent);
}

/*!
 * View the document at its actual size.
 */
template<class Receiver, class Func>
inline QAction *actualSize(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(ActualSize, recvr, slot, parent);
}

/*!
 * Fit the document view to the size of the current window.
 */
template<class Receiver, class Func>
inline QAction *fitToPage(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(FitToPage, recvr, slot, parent);
}

/*!
 * Fit the document view to the width of the current window.
 */
template<class Receiver, class Func>
inline QAction *fitToWidth(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(FitToWidth, recvr, slot, parent);
}

/*!
 * Fit the document view to the height of the current window.
 */
template<class Receiver, class Func>
inline QAction *fitToHeight(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(FitToHeight, recvr, slot, parent);
}

/*!
 * Zoom in the current document view.
 */
template<class Receiver, class Func>
inline QAction *zoomIn(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(ZoomIn, recvr, slot, parent);
}

/*!
 * Zoom out the current document view.
 */
template<class Receiver, class Func>
inline QAction *zoomOut(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(ZoomOut, recvr, slot, parent);
}

/*!
 * Select the current zoom level.
 */
template<class Receiver, class Func>
inline QAction *zoom(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Zoom, recvr, slot, parent);
}

/*!
 * Redisplay or redraw the document.
 */
template<class Receiver, class Func>
inline QAction *redisplay(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Redisplay, recvr, slot, parent);
}

/*!
 * Move up (web style menu).
 */
template<class Receiver, class Func>
inline QAction *up(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Up, recvr, slot, parent);
}

/*!
 * Move back (web style menu).
 */
template<class Receiver, class Func>
inline QAction *back(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Back, recvr, slot, parent);
}

/*!
 * Move forward (web style menu).
 */
template<class Receiver, class Func>
inline QAction *forward(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Forward, recvr, slot, parent);
}

/*!
 * Go to the "Home" position or document.
 */
template<class Receiver, class Func>
inline QAction *home(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Home, recvr, slot, parent);
}

/*!
 * Scroll up one page.
 */
template<class Receiver, class Func>
inline QAction *prior(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Prior, recvr, slot, parent);
}

/*!
 * Scroll down one page.
 */
template<class Receiver, class Func>
inline QAction *next(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Next, recvr, slot, parent);
}

/*!
 * Jump to some specific location in the document.
 */
template<class Receiver, class Func>
inline QAction *goTo(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Goto, recvr, slot, parent);
}

/*!
 * Go to a specific page.
 */
template<class Receiver, class Func>
inline QAction *gotoPage(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(GotoPage, recvr, slot, parent);
}

/*!
 * Go to a specific line.
 */
template<class Receiver, class Func>
inline QAction *gotoLine(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(GotoLine, recvr, slot, parent);
}

/*!
 * Jump to the first page.
 */
template<class Receiver, class Func>
inline QAction *firstPage(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(FirstPage, recvr, slot, parent);
}

/*!
 * Jump to the last page.
 */
template<class Receiver, class Func>
inline QAction *lastPage(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(LastPage, recvr, slot, parent);
}

/*!
 * Move back (document style menu).
 */
template<class Receiver, class Func>
inline QAction *documentBack(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(DocumentBack, recvr, slot, parent);
}

/*!
 * Move forward (document style menu).
 */
template<class Receiver, class Func>
inline QAction *documentForward(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(DocumentForward, recvr, slot, parent);
}

/*!
 * Add the current page to the bookmarks tree.
 */
template<class Receiver, class Func>
inline QAction *addBookmark(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(AddBookmark, recvr, slot, parent);
}

/*!
 * Edit the application bookmarks.
 */
template<class Receiver, class Func>
inline QAction *editBookmarks(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(EditBookmarks, recvr, slot, parent);
}

/*!
 * Pop up the spell checker.
 */
template<class Receiver, class Func>
inline QAction *spelling(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Spelling, recvr, slot, parent);
}

/*!
 * Display the configure key bindings dialog.
 *
 * Note that you might be able to use the pre-built KXMLGUIFactory's function:
 * \code
 * KStandardActions::keyBindings(guiFactory(), &KXMLGUIFactory::showConfigureShortcutsDialog, actionCollection());
 * \endcode
 */
template<class Receiver, class Func>
inline QAction *keyBindings(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(KeyBindings, recvr, slot, parent);
}

/*!
 * Display the preferences/options dialog.
 */
template<class Receiver, class Func>
inline QAction *preferences(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Preferences, recvr, slot, parent);
}

/*!
 * Display the toolbar configuration dialog.
 */
template<class Receiver, class Func>
inline QAction *configureToolbars(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(ConfigureToolbars, recvr, slot, parent);
}

/*!
 * Display the notifications configuration dialog.
 */
template<class Receiver, class Func>
inline QAction *configureNotifications(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(ConfigureNotifications, recvr, slot, parent);
}

/*!
 * Display the Switch Application Language dialog.
 */
template<class Receiver, class Func>
inline QAction *switchApplicationLanguage(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(SwitchApplicationLanguage, recvr, slot, parent);
}

/*!
 * Display the handbook of the application.
 */
template<class Receiver, class Func>
inline QAction *helpContents(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(HelpContents, recvr, slot, parent);
}

/*!
 * Trigger the What's This cursor.
 */
template<class Receiver, class Func>
inline QAction *whatsThis(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(WhatsThis, recvr, slot, parent);
}

/*!
 * Open up the Report Bug dialog.
 */
template<class Receiver, class Func>
inline QAction *reportBug(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(ReportBug, recvr, slot, parent);
}

/*!
 * Display the application's About box.
 */
template<class Receiver, class Func>
inline QAction *aboutApp(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(AboutApp, recvr, slot, parent);
}

/*!
 * Display the About KDE dialog.
 */
template<class Receiver, class Func>
inline QAction *aboutKDE(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(AboutKDE, recvr, slot, parent);
}

/*!
 * Permanently deletes files or folders.
 */
template<class Receiver, class Func>
inline QAction *deleteFile(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(DeleteFile, recvr, slot, parent);
}

/*!
 * Renames files or folders.
 */
template<class Receiver, class Func>
inline QAction *renameFile(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(RenameFile, recvr, slot, parent);
}

/*!
 * Moves files or folders to the trash.
 */
template<class Receiver, class Func>
inline QAction *moveToTrash(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(MoveToTrash, recvr, slot, parent);
}

/*!
 * Open donation page on kde.org.
 */
template<class Receiver, class Func>
inline QAction *donate(const Receiver *recvr, Func slot, QObject *parent)
{
    return create(Donate, recvr, slot, parent);
}
}

#endif // KSTDACTION_H
