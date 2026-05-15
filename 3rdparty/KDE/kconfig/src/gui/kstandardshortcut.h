/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Stefan Taferner <taferner@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KSTANDARDSHORTCUT_H
#define KSTANDARDSHORTCUT_H

#include <QKeySequence>
#include <QString>

#include <kconfiggui_export.h>

/*!
 * \namespace KStandardShortcut
 * \inmodule KConfigGui
 *
 * \brief Convenient methods for access to the common accelerator keys in
 * the key configuration.
 *
 * These are the standard keybindings that should
 * be used in all KDE applications. They will be configurable,
 * so do not hardcode the default behavior.
 */
namespace KStandardShortcut
{
// STUFF WILL BREAK IF YOU DON'T READ THIS!!!
/*
 *Always add new std-accels to the end of this enum, never in the middle!
 *Don't forget to add the corresponding entries in g_infoStandardShortcut[] in kstandardshortcut.cpp, too.
 *Values of elements here and positions of the corresponding entries in
 *the big array g_infoStandardShortcut[] ABSOLUTELY MUST BE THE SAME.
 * !!!    !!!!   !!!!!    !!!!
 *    !!!!    !!!     !!!!    !!!!
 *
 * Other Rules:
 *
 * - Never change the name of an existing shortcut
 * - Never translate the name of a shortcut
 */

/*!
 * Defines the identifier of all standard accelerators.
 * \value AccelNone
 * \value Open Open file.
 * \value New Create a new document.
 * \value Close Close current document.
 * \value Save Save current document.
 * \value Print Print current document.
 * \value Quit Quit the program.
 * \value Undo Undo last operation.
 * \value Redo Redo last operation.
 * \value Cut Cut selected area and store it in the clipboard.
 * \value Copy Copy selected area into the clipboard.
 * \value Paste Paste contents of clipboard at mouse/cursor position.
 * \value PasteSelection Paste the selection at mouse/cursor position.
 * \value SelectAll Select all.
 * \value Deselect Deselect any selected elements.
 * \value DeleteWordBack Delete a word back from mouse/cursor position.
 * \value DeleteWordForward Delete a word forward from mouse/cursor position.
 * \value Find Initiate a 'find' request in the current document.
 * \value FindNext Find the next instance of a stored 'find'.
 * \value FindPrev Find a previous instance of a stored 'find'.
 * \value Replace Find and replace matches.
 * \value Home Go to home page.
 * \value Begin Go to beginning of the document.
 * \value End Go to end of the document.
 * \value Prior Scroll up one page.
 * \value Next Scroll down one page.
 * \value Up Up.
 * \value Back Back.
 * \value Forward Forward.
 * \value Reload Reload.
 * \value BeginningOfLine Go to beginning of current line.
 * \value EndOfLine Go to end of current line.
 * \value GotoLine Go to line.
 * \value BackwardWord BackwardWord.
 * \value ForwardWord ForwardWord.
 * \value AddBookmark Add current page to bookmarks.
 * \value ZoomIn Zoom in.
 * \value ZoomOut Zoom out.
 * \value FullScreen Full Screen mode.
 * \value ShowMenubar Show Menu Bar.
 * \value TabNext Next Tab.
 * \value TabPrev Previous Tab.
 * \value Help Help the user in the current situation.
 * \value WhatsThis What's This button.
 * \value TextCompletion Complete text in input widgets.
 * \value PrevCompletion Iterate through a list when completion returns multiple items.
 * \value NextCompletion Iterate through a list when completion returns multiple items.
 * \value SubstringCompletion Find a string within another string or list of strings.
 * \value RotateUp Help users iterate through a list of entries.
 * \value RotateDown Help users iterate through a list of entries.
 * \value OpenRecent Open a recently used document.
 * \value SaveAs Save the current document under a different name.
 * \value Revert Revert the current document to the last saved version.
 * \value PrintPreview Show a print preview of the current document.
 * \value Mail Send the current document by mail.
 * \value Clear Clear the content of the focus widget.
 * \value ActualSize View the document at its actual size.
 * \value FitToPage Fit the document view to the size of the current window.
 * \value FitToWidth Fit the document view to the width of the current window.
 * \value FitToHeight Fit the document view to the height of the current window.
 * \value Zoom Select the current zoom level.
 * \value Goto Jump to some specific location in the document.
 * \value GotoPage Go to a specific page.
 * \value DocumentBack Move back (document style menu).
 * \value DocumentForward Move forward (document style menu).
 * \value EditBookmarks Edit the application bookmarks.
 * \value Spelling Pop up the spell checker.
 * \value ShowToolbar Show/Hide the toolbar.
 * \value ShowStatusbar Show/Hide the statusbar.
 * \value KeyBindings Display the configure key bindings dialog.
 * \value Preferences Display the preferences/options dialog.
 * \value ConfigureToolbars Display the toolbar configuration dialog.
 * \value ConfigureNotifications Display the notifications configuration dialog.
 * \value ReportBug Display the Report Bug dialog.
 * \value SwitchApplicationLanguage Display the Switch Application Language dialog.
 * \value AboutApp Display the application's About dialog.
 * \value AboutKDE Display the About KDE dialog.
 * \value [since 5.25] DeleteFile Permanently delete files or folders.
 * \value [since 5.25] RenameFile Rename files or folders.
 * \value [since 5.25] MoveToTrash Move files or folders to the trash.
 * \value [since 5.26] Donate Open donation page on kde.org.
 * \value [since 5.70] ShowHideHiddenFiles Toggle showing or hiding hidden files
 * \value [since 5.74] CreateFolder Create a folder.
 * \value [since 6.0] OpenMainMenu Open a main menu like the menu bar or a hamburger menu. Necessary for accessibility.  * \value [since 6.0] OpenContextMenu
 * Open a context menu for the object with keyboard focus. Necessary for accessibility.  */
enum StandardShortcut {
    // C++ requires that the value of an enum symbol be one more than the previous one.
    // This means that everything will be well-ordered from here on.
    AccelNone = 0,
    // File menu
    Open,
    New,
    Close,
    Save,
    // The Print item
    Print,
    Quit,
    // Edit menu
    Undo,
    Redo,
    Cut,
    Copy,
    Paste,
    PasteSelection,
    SelectAll,
    Deselect,
    DeleteWordBack,
    DeleteWordForward,
    Find,
    FindNext,
    FindPrev,
    Replace,
    // Navigation
    Home,
    Begin,
    End,
    Prior,
    Next,
    Up,
    Back,
    Forward,
    Reload,
    // Text Navigation
    BeginningOfLine,
    EndOfLine,
    GotoLine,
    BackwardWord,
    ForwardWord,
    // View parameters
    AddBookmark,
    ZoomIn,
    ZoomOut,
    FullScreen,
    ShowMenubar,
    // Tabular navigation
    TabNext,
    TabPrev,
    // Help menu
    Help,
    WhatsThis,
    // Text completion
    TextCompletion,
    PrevCompletion,
    NextCompletion,
    SubstringCompletion,
    RotateUp,
    RotateDown,
    OpenRecent,
    SaveAs,
    Revert,
    PrintPreview,
    Mail,
    Clear,
    ActualSize,
    FitToPage,
    FitToWidth,
    FitToHeight,
    Zoom,
    Goto,
    GotoPage,
    DocumentBack,
    DocumentForward,
    EditBookmarks,
    Spelling,
    ShowToolbar,
    ShowStatusbar,
    KeyBindings,
    Preferences,
    ConfigureToolbars,
    ConfigureNotifications,
    ReportBug,
    SwitchApplicationLanguage,
    AboutApp,
    AboutKDE,
    DeleteFile,
    RenameFile,
    MoveToTrash,
    Donate,
    ShowHideHiddenFiles,
    CreateFolder,
    OpenMainMenu,
    OpenContextMenu,
    // Insert new items here!

    StandardShortcutCount, // number of standard shortcuts
};

/*!
 * Categories in which the standard shortcuts can be classified
 * \since 5.74
 * \value InvalidCategory
 * \value File
 * \value Edit
 * \value Navigation
 * \value View
 * \value Settings
 * \value Help
 */
enum class Category {
    InvalidCategory = -1,
    File,
    Edit,
    Navigation,
    View,
    Settings,
    Help,
};

/*!
 * Returns the keybinding for \a accel.
 *
 * On X11, if QApplication was initialized with GUI disabled, the
 * default keybinding will always be returned.
 *
 * \a id the id of the accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &shortcut(StandardShortcut id);

/*!
 * Returns a unique name for the given accel.
 *
 * \a id the id of the accelerator
 */
KCONFIGGUI_EXPORT QString name(StandardShortcut id);

/*!
 * Returns a localized label for user-visible display.
 *
 * \a id the id of the accelerator
 */
KCONFIGGUI_EXPORT QString label(StandardShortcut id);

/*!
 * Returns an extended WhatsThis description for the given accelerator.
 *
 * \a id the id of the accelerator
 */
KCONFIGGUI_EXPORT QString whatsThis(StandardShortcut id);

/*!
 * Return the StandardShortcut id of the standard accel action which
 * uses this key sequence, or AccelNone if none of them do.
 *
 * This is used by class KKeyChooser.
 *
 * \a keySeq the key sequence to search
 */
KCONFIGGUI_EXPORT StandardShortcut find(const QKeySequence &keySeq);

/*!
 * Return the StandardShortcut id of the standard accelerator action which
 * has \a name as its name, or AccelNone if none of them do.
 *
 * \a name the name as returned by  name(StandardShortcut id)
 *
 * \since 5.71
 */
KCONFIGGUI_EXPORT StandardShortcut findByName(const QString &name);

/*!
 * Returns the hardcoded default shortcut for \a id.
 *
 * This does not take into account the user's configuration.
 *
 * \a id the id of the accelerator
 */
KCONFIGGUI_EXPORT QList<QKeySequence> hardcodedDefaultShortcut(StandardShortcut id);

/*!
 * Saves the new shortcut \a cut for standard accel \a id.
 */
KCONFIGGUI_EXPORT void saveShortcut(StandardShortcut id, const QList<QKeySequence> &newShortcut);

/*!
 * Returns the appropriate category for the given StandardShortcut \a id.
 * \since 5.73
 */
KCONFIGGUI_EXPORT Category category(StandardShortcut id);

/*!
 * Open file. Default: Ctrl-o
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &open();

/*!
 * Create a new document (or whatever). Default: Ctrl-n
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &openNew();

/*!
 * Close current document. Default: Ctrl-w
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &close();

/*!
 * Save current document. Default: Ctrl-s
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &save();

/*!
 * Print current document. Default: Ctrl-p
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &print();

/*!
 * Quit the program. Default: Ctrl-q
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &quit();

/*!
 * Undo last operation. Default: Ctrl-z
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &undo();

/*!
 * Redo last operation. Default: Shift-Ctrl-z
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &redo();

/*!
 * Cut selected area and store it in the clipboard. Default: Ctrl-x
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &cut();

/*!
 * Copy selected area into the clipboard. Default: Ctrl-c
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &copy();

/*!
 * Paste contents of clipboard at mouse/cursor position. Default: Ctrl-v
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &paste();

/*!
 * Paste the selection at mouse/cursor position. Default: Ctrl-Shift-Insert
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &pasteSelection();

/*!
 * Select all. Default: Ctrl-A
 *
 * Returns the shortcut of the standard accelerator
 **/
KCONFIGGUI_EXPORT const QList<QKeySequence> &selectAll();

/*!
 * Delete a word back from mouse/cursor position. Default: Ctrl-Backspace
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &deleteWordBack();

/*!
 * Delete a word forward from mouse/cursor position. Default: Ctrl-Delete
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &deleteWordForward();

/*!
 * Initiate a 'find' request in the current document. Default: Ctrl-f
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &find();

/*!
 * Find the next instance of a stored 'find' Default: F3
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &findNext();

/*!
 * Find a previous instance of a stored 'find'. Default: Shift-F3
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &findPrev();

/*!
 * Find and replace matches. Default: Ctrl-r
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &replace();

/*!
 * Zoom in. Default: Ctrl-Plus
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &zoomIn();

/*!
 * Zoom out. Default: Ctrl-Minus
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &zoomOut();

/*!
 * Go to home page. Default: Alt-Home
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &home();

/*!
 * Go to beginning of the document. Default: Ctrl-Home
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &begin();

/*!
 * Go to end of the document. Default: Ctrl-End
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &end();

/*!
 * Go to beginning of current line. Default: Home
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &beginningOfLine();

/*!
 * Go to end of current line. Default: End
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &endOfLine();

/*!
 * Scroll up one page. Default: Prior
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &prior();

/*!
 * Scroll down one page. Default: Next
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &next();

/*!
 * Go to line. Default: Ctrl+G
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &gotoLine();

/*!
 * Add current page to bookmarks. Default: Ctrl+B
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &addBookmark();

/*!
 * Next Tab. Default: Ctrl-<
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &tabNext();

/*!
 * Previous Tab. Default: Ctrl->
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &tabPrev();

/*!
 * Full Screen Mode. Default: Ctrl+Shift+F
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &fullScreen();

/*!
 * Help the user in the current situation. Default: F1
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &help();

/*!
 * Complete text in input widgets. Default Ctrl+E
 *
 * Returns the shortcut of the standard accelerator
 **/
KCONFIGGUI_EXPORT const QList<QKeySequence> &completion();

/*!
 * Iterate through a list when completion returns
 * multiple items. Default: Ctrl+Up
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &prevCompletion();

/*!
 * Iterate through a list when completion returns
 * multiple items. Default: Ctrl+Down
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &nextCompletion();

/*!
 * Find a string within another string or list of strings.
 * Default: Ctrl-T
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &substringCompletion();

/*!
 * Help users iterate through a list of entries. Default: Up
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &rotateUp();

/*!
 * Help users iterate through a list of entries. Default: Down
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &rotateDown();

/*!
 * What's This button. Default: Shift+F1
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &whatsThis();

/*!
 * Reload. Default: F5
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &reload();

/*!
 * Up. Default: Alt+Up
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &up();

/*!
 * Back. Default: Alt+Left
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &back();

/*!
 * Forward. Default: ALT+Right
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &forward();

/*!
 * BackwardWord. Default: Ctrl+Left
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &backwardWord();

/*!
 * ForwardWord. Default: Ctrl+Right
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &forwardWord();

/*!
 * Show Menu Bar.  Default: Ctrl-M
 *
 * Returns the shortcut of the standard accelerator
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &showMenubar();

/*!
 * Permanently delete files or folders. Default: Shift+Delete
 *
 * Returns the shortcut of the standard accelerator
 * \since 5.25
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &deleteFile();

/*!
 * Rename files or folders. Default: F2
 *
 * Returns the shortcut of the standard accelerator
 * \since 5.25
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &renameFile();

/*!
 * Create a folder. Default: Ctrl+Shift+N
 *
 * Returns the shortcut of the standard accelerator
 * \since 5.74
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &createFolder();

/*!
 * Moves files or folders to the trash. Default: Delete
 *
 * Returns the shortcut of the standard accelerator
 * \since 5.25
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &moveToTrash();

/*!
 * Opens the app's settings window. Default: Ctrl+Shift+Comma
 *
 * Returns the shortcut of the standard accelerator
 * \since 5.64
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &preferences();

/*!
 * Shows or hides hidden files. Defaults: Ctrl+H, Alt+.
 *
 * Returns the shortcut of the standard accelerator
 * \since 5.70
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &showHideHiddenFiles();

/*!
 * Open a main menu like the menu bar or a hamburger menu.
 * Necessary for accessibility. Default: F10
 *
 * Returns the shortcut of the standard accelerator
 * \since 6.0
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &openMainMenu();

/*!
 * Open a context menu for the object with keyboard focus.
 * Necessary for accessibility. Default: Menu, Shift+F10
 *
 * Returns the shortcut of the standard accelerator
 * \since 6.0
 */
KCONFIGGUI_EXPORT const QList<QKeySequence> &openContextMenu();
}

#endif // KSTANDARDSHORTCUT_H
