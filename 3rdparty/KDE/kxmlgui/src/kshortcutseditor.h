/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: Nicolas Hadacek <hadacek@via.ecp.fr>
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 2001, 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSHORTCUTSEDITOR_H
#define KSHORTCUTSEDITOR_H

#include <kxmlgui_export.h>

#include <QWidget>

class KActionCollection;
class KConfig;
class KConfigBase;
class KConfigGroup;
class KGlobalAccel;
class KShortcutsEditorPrivate;

// KShortcutsEditor expects that the list of existing shortcuts is already
// free of conflicts. If it is not, nothing will crash, but your users
// won't like the resulting behavior.

/*!
 * \class KShortcutsEditor
 * \inmodule KXmlGui
 *
 * \brief Widget for configuration of KAccel and KGlobalAccel.
 *
 * Configure dictionaries of key/action associations for QActions,
 * including global shortcuts.
 *
 * The class takes care of all aspects of configuration, including
 * handling key conflicts internally. Connect to the allDefault()
 * slot if you want to set all configurable shortcuts to their
 * default values.
 *
 * \sa KShortcutsDialog
 */
class KXMLGUI_EXPORT KShortcutsEditor : public QWidget
{
    Q_OBJECT

    /*!
     * \property KShortcutsEditor::actionTypes
     */
    Q_PROPERTY(ActionTypes actionTypes READ actionTypes WRITE setActionTypes)

public:
    /*!
     * \enum KShortcutsEditor::ActionType
     *
     * \value WidgetAction
     *        Actions which are triggered by any keypress in a widget
     *        that has the action added to it.
     *
     * \value WindowAction
     *        Actions which are triggered by any keypress in a window
     *        that has the action added to it or its child widget(s).
     *
     * \value ApplicationAction
     *        Actions that are triggered by any keypress in the application.
     *
     * \value GlobalAction
     *        Actions which are triggered by any keypress in the windowing system.
     *
     * \value AllActions
     *        A combination of all available actions.
     *
     * \note Since 5.95, GlobalAction is ignored if there are no actual Global shortcuts in any of the action collections that are added.
     */
    enum ActionType {
        WidgetAction = Qt::WidgetShortcut /*0*/,
        WindowAction = Qt::WindowShortcut /*1*/,
        ApplicationAction = Qt::ApplicationShortcut /*2*/,
        GlobalAction = 4,
        AllActions = 0xffffffff,
    };
    Q_DECLARE_FLAGS(ActionTypes, ActionType)

    /*!
     * \enum KShortcutsEditor::LetterShortcuts
     *
     * \value LetterShortcutsDisallowed
     *        Shortcuts without a modifier are not allowed,
     *        so 'A' would not be valid, whereas 'Ctrl+A' would be.
     *        This only applies to printable characters, however.
     *        'F1', 'Insert' etc. could still be used.
     *
     * \value LetterShortcutsAllowed
     *        Letter shortcuts are allowed.
     */
    enum LetterShortcuts {
        LetterShortcutsDisallowed = 0,
        LetterShortcutsAllowed,
    };

    /*!
     * \brief Constructor.
     *
     * \a collection The KActionCollection to configure.
     *
     * \a parent Parent widget.
     *
     * \a actionTypes Types of actions to display in this widget.
     *
     * \a allowLetterShortcuts Set to LetterShortcutsDisallowed if unmodified
     * alphanumeric keys ('A', '1', etc.) are not permissible shortcuts.
     */
    KShortcutsEditor(KActionCollection *collection,
                     QWidget *parent,
                     ActionTypes actionTypes = AllActions,
                     LetterShortcuts allowLetterShortcuts = LetterShortcutsAllowed);

    /*!
     * \brief Creates a key chooser without a starting action collection.
     *
     * \overload KShortcutsEditor::KShortcutsEditor()
     *
     * \a parent Parent widget.
     *
     * \a actionTypes Types of actions to display in this widget.
     *
     * \a allowLetterShortcuts Set to LetterShortcutsDisallowed if unmodified
     * alphanumeric keys ('A', '1', etc.) are not permissible shortcuts.
     */
    explicit KShortcutsEditor(QWidget *parent, ActionTypes actionTypes = AllActions, LetterShortcuts allowLetterShortcuts = LetterShortcutsAllowed);

    /*!
     *  Destructor
     */
    ~KShortcutsEditor() override;

    /*!
     * \brief Returns whether there are unsaved changes.
     */
    bool isModified() const;

    /*!
     * \brief Removes all action collections from the editor
     */
    void clearCollections();

    /*!
     * \brief Insert an action collection, i.e. add all its actions to the ones
     * already associated with the KShortcutsEditor object.
     *
     * \a title Subtree title of this collection of shortcut.
     */
    void addCollection(KActionCollection *, const QString &title = QString());

    /*!
     * \brief Undo all changes made since the last save().
     *
     * \since 5.75
     */
    void undo();

    /*!
     * \brief Save the changes.
     *
     * This saves the actions to disk.
     * Any KActionCollection objects with the xmlFile() value set will be
     * written to an XML file.  All others will be written to the application's
     * rc file.
     */
    void save();

    /*!
     * \brief Sets the types of actions to display in this widget.
     *
     * \a actionTypes New types of actions.
     *
     * \since 5.0
     */
    void setActionTypes(ActionTypes actionTypes);
    /*!
     * \brief Returns The types of actions currently displayed in this widget.
     * \since 5.0
     */
    ActionTypes actionTypes() const;

Q_SIGNALS:
    /*!
     * \brief Emitted when an action's shortcut has been changed.
     **/
    void keyChange();

public Q_SLOTS:
    /*!
     * \brief Sets all shortcuts to their default values (bindings).
     **/
    void allDefault();

private Q_SLOTS:
    /*!
     * \brief Resize columns to width required.
     * \internal
     */
    KXMLGUI_NO_EXPORT void resizeColumns();

    /*!
     * \brief Opens a printing dialog to print all the shortcuts.
     * \internal
     */
    KXMLGUI_NO_EXPORT void printShortcuts() const;

private:
    /*!
     * \brief Write the current settings to the \a config object.
     * \internal
     *
     * This does not initialize the \a config object. It adds the
     * configuration.
     *
     * \note This will not save the global configuration! globalaccel holds
     * that part of the configuration.
     * \sa writeGlobalConfig()
     *
     * \a config Config object to save to or, or null to use
     * the applications config object.
     */
    KXMLGUI_NO_EXPORT void writeConfiguration(KConfigGroup *config = nullptr) const;

    /*!
     * \brief Export the current setting to configuration \a config.
     * \internal
     *
     * This initializes the configuration object. This will export the global
     * configuration too.
     *
     * \a config Config object that will import the current exported settings.
     */
    KXMLGUI_NO_EXPORT void exportConfiguration(KConfigBase *config) const;

    /*!
     * \brief Import the settings from configuration \a config.
     * \internal
     *
     * This will remove all current settings before importing. All shortcuts
     * are set to QList<QKeySequence>() prior to importing from \a config !
     *
     * \a config Config object that will export the current settings.
     */
    KXMLGUI_NO_EXPORT void importConfiguration(KConfigBase *config);

    friend class KShortcutsDialog;
    friend class KShortcutsEditorPrivate;
    std::unique_ptr<KShortcutsEditorPrivate> const d;
    Q_DISABLE_COPY(KShortcutsEditor)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KShortcutsEditor::ActionTypes)

#endif // KSHORTCUTSEDITOR_H
