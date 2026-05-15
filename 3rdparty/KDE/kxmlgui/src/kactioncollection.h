/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KACTIONCOLLECTION_H
#define KACTIONCOLLECTION_H

#include <KStandardAction>
#include <KStandardActions>
#include <kxmlgui_export.h>

#include <QAction>
#include <QObject>

class KXMLGUIClient;
class KConfigGroup;
class QActionGroup;
class QString;

/*!
 * \class KActionCollection
 * \inmodule KXmlGui
 *
 * \brief A container for a set of QAction objects.
 *
 * KActionCollection manages a set of QAction objects.  It
 * allows them to be grouped for organized presentation of configuration to the user,
 * saving + loading of configuration, and optionally for automatic plugging into
 * specified widget(s).
 *
 * Additionally, KActionCollection provides several convenience functions for locating
 * named actions, and actions grouped by QActionGroup.
 *
 * \note If you create your own action collection and need to assign shortcuts
 * to the actions within, you have to call associateWidget() or
 * addAssociatedWidget() to have them working.
 */
class KXMLGUI_EXPORT KActionCollection : public QObject
{
    friend class KXMLGUIClient;

    Q_OBJECT

    /*!
     * \property KActionCollection::configGroup
     */
    Q_PROPERTY(QString configGroup READ configGroup WRITE setConfigGroup)

    /*!
     * \property KActionCollection::configIsGlobal
     */
    Q_PROPERTY(bool configIsGlobal READ configIsGlobal WRITE setConfigGlobal)

public:
    /*!
     * \brief Constructor.
     *
     * Allows specification of a component name other than the default
     * application name, where needed (remember to call setComponentDisplayName() too).
     */
    explicit KActionCollection(QObject *parent, const QString &cName = QString());

    /*!
     * \brief Destructor.
     */
    ~KActionCollection() override;

    /*!
     * \brief Access the list of all action collections in existence for this app.
     */
    static const QList<KActionCollection *> &allCollections();

    /*!
     * \brief Clears the entire action collection, deleting all actions.
     */
    void clear();

    /*!
     * \brief Associate all actions in this collection to the given \a widget.
     *
     * Unlike addAssociatedWidget(), this method only adds all current actions
     * in the collection to the given widget. Any action added after this call
     * will not be added to the given widget automatically.
     *
     * So this is just a shortcut for a foreach loop and a widget->addAction call.
     */
    void associateWidget(QWidget *widget) const;

    /*!
     * \brief Associate all actions in this collection to the given \a widget,
     * including any actions added after this association is made.
     *
     * This does not change the action's shortcut context, so if you need to have the actions only
     * trigger when the widget has focus, you'll need to set the shortcut context on each action
     * to Qt::WidgetShortcut (or better still, Qt::WidgetWithChildrenShortcut with Qt 4.4+)
     */
    void addAssociatedWidget(QWidget *widget);

    /*!
     * \brief Remove an association between all actions in this collection
     * and the given \a widget, i.e. remove those actions from the widget,
     * and stop associating newly added actions as well.
     */
    void removeAssociatedWidget(QWidget *widget);

    /*!
     * \brief Return a list of all associated widgets.
     */
    QList<QWidget *> associatedWidgets() const;

    /*!
     * \brief Clear all associated widgets and remove the actions from those widgets.
     */
    void clearAssociatedWidgets();

    /*!
     * \brief Returns the KConfig group with which settings will be loaded and saved.
     */
    QString configGroup() const;

    /*!
     * \brief Returns whether this action collection's configuration
     * should be global to KDE ( \c true ),
     * or specific to the application ( \c false ).
     */
    bool configIsGlobal() const;

    /*!
     * \brief Sets \a group as the KConfig group with which
     * settings will be loaded and saved.
     */
    void setConfigGroup(const QString &group);

    /*!
     * \brief Set whether this action collection's configuration
     * should be \a global to KDE ( \c true ),
     * or specific to the application ( \c false ).
     */
    void setConfigGlobal(bool global);

    /*!
     * \brief Read all key associations from \a config.
     *
     * If \a config is zero, read all key associations from the
     * application's configuration file KSharedConfig::openConfig(),
     * in the group set by setConfigGroup().
     */
    void readSettings(KConfigGroup *config = nullptr);

    /*!
     * \brief Import all configurable global key associations from \a config.
     *
     * \since 4.1
     */
    void importGlobalShortcuts(KConfigGroup *config);

    /*!
     * \brief Export the current configurable global key associations to \a config.
     *
     * If \a writeDefaults is set to true, default settings will also be written.
     *
     * \since 4.1
     */
    void exportGlobalShortcuts(KConfigGroup *config, bool writeDefaults = false) const;

    /*!
     * \brief Write the current configurable key associations to \a config. What the
     * function does if \a config is zero depends. If this action collection
     * belongs to a KXMLGUIClient the setting are saved to the kxmlgui
     * definition file. If not the settings are written to the applications
     * config file.
     *
     * \note \a oneAction and \a writeDefaults have no meaning for the kxmlgui
     * configuration file.
     *
     * \a config Config object to save to, or null.
     *
     * \a writeDefaults set to true to write settings which are already at defaults.
     *
     * \a oneAction pass an action here if you just want to save the values for one action, eg.
     *                  if you know that action is the only one which has changed.
     */
    void writeSettings(KConfigGroup *config = nullptr, bool writeDefaults = false, QAction *oneAction = nullptr) const;

    /*!
     * \brief The number of actions in the collection.
     *
     * This is equivalent to actions().count().
     */
    int count() const;

    /*!
     * \brief Returns whether the action collection is empty or not.
     */
    bool isEmpty() const;

    /*!
     * \brief Returns the QAction* at position \a index in the action collection.
     *
     * This is equivalent to actions().value(index).
     */
    QAction *action(int index) const;

    /*!
     * \brief Get an action with the given \a name from the action collection.
     *
     * This won't return the action for the menus defined using a "<Menu>" tag
     * in XMLGUI files (e.g. "<Menu name="menuId">" in "applicationNameui.rc").
     *
     * To access menu actions defined like this, use for example...
     *
     * \code
     * qobject_cast<QMenu *>(guiFactory()->container("menuId", this));
     * \endcode
     *
     * ...after having called setupGUI() or createGUI().
     *
     * Returns a pointer to the QAction in the collection that matches the parameters or
     * null if nothing matches.
     */
    Q_INVOKABLE QAction *action(const QString &name) const;

    /*!
     * \brief Returns the list of QActions that belong to this action collection.
     *
     * The list is guaranteed to be in the same order the action were put into
     * the collection.
     */
    QList<QAction *> actions() const;

    /*!
     * \brief Returns the list of QActions without an QAction::actionGroup()
     * that belong to this action collection.
     */
    const QList<QAction *> actionsWithoutGroup() const;

    /*!
     * \brief Returns the list of all QActionGroups associated
     * with actions in this action collection.
     */
    const QList<QActionGroup *> actionGroups() const;

    /*!
     * \brief Sets the \a componentName associated with this action collection.
     *
     * \warning Don't call this method on a KActionCollection that contains
     * actions. This is not supported.
     *
     * \a componentName The name which is to be associated with this action collection,
     * or QString() to indicate the app name. This is used to load/save settings into XML files.
     * KXMLGUIClient::setComponentName takes care of calling this.
     */
    void setComponentName(const QString &componentName);

    /*! \brief The component name with which this class is associated. */
    QString componentName() const;

    /*!
     * \brief Set the component \a displayName associated with this action collection.
     *
     * This can be done, for example, for the toolbar editor.
     *
     * KXMLGUIClient::setComponentName takes care of calling this.
     */
    void setComponentDisplayName(const QString &displayName);

    /*! \brief The display name for the associated component. */
    QString componentDisplayName() const;

    /*!
     * \brief The parent KXMLGUIClient, or null if not available.
     */
    const KXMLGUIClient *parentGUIClient() const;

Q_SIGNALS:
    /*!
     * \brief Indicates that \a action was inserted into this action collection.
     */
    void inserted(QAction *action);

    /*!
     * Emitted when an action has been inserted into, or removed from, this action collection.
     * \since 5.66
     */
    void changed();

    /*!
     * \brief Indicates that \a action was hovered.
     */
    void actionHovered(QAction *action);

    /*!
     * \brief Indicates that \a action was triggered.
     */
    void actionTriggered(QAction *action);

protected:
    /// Overridden to perform connections when someone wants to know whether an action was highlighted or triggered
    void connectNotify(const QMetaMethod &signal) override;

protected Q_SLOTS:
    virtual void slotActionTriggered();

private Q_SLOTS:
    KXMLGUI_NO_EXPORT void slotActionHovered();

public:
    /*!
     * \brief Add an \a action under the given \a name to the collection.
     *
     * Inserting an action that was previously inserted under a different name will replace the
     * old entry, i.e. the action will not be available under the old name anymore but only under
     * the new one.
     *
     * Inserting an action under a name that is already used for another action will replace
     * the other action in the collection (but will not delete it).
     *
     * If KAuthorized::authorizeAction() reports that the action is not
     * authorized, it will be disabled and hidden.
     *
     * The ownership of the action object is not transferred.
     * If the action is destroyed it will be removed automatically from the KActionCollection.
     *
     * \a name The name by which the action be retrieved again from the collection.
     *
     * \a action The action to add.
     *
     * Returns the same as the action given as parameter. This is just for convenience
     * (chaining calls) and consistency with the other addAction methods, you can also
     * simply ignore the return value.
     */
    Q_INVOKABLE QAction *addAction(const QString &name, QAction *action);

    /*!
     * \brief Adds a list of \a actions to the collection.
     *
     * The objectName of the actions is used as their internal name in the collection.
     *
     * The ownership of the action objects is not transferred.
     * If the action is destroyed it will be removed automatically from the KActionCollection.
     *
     * Uses addAction(const QString&, QAction*).
     *
     * \sa addAction()
     * \since 5.0
     */
    void addActions(const QList<QAction *> &actions);

    /*!
     * \brief Removes an \a action from the collection and deletes it.
     */
    void removeAction(QAction *action);

    /*!
     * \brief Removes an \a action from the collection.
     *
     * The ownership of the action object is not changed.
     */
    QAction *takeAction(QAction *action);

    /*!
     * \brief Creates a new standard action, adds it to the collection and connects the
     * action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is also returned.
     *
     * \note Using KStandardAction::OpenRecent will cause a different signal than
     * triggered(bool) to be used, see KStandardAction for more information.
     *
     * The action can be retrieved later from the collection by its standard name as per
     * KStandardAction::stdName.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * \a actionType The standard action type of the action to create.
     *
     * \a receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     *
     * \a member The SLOT to connect the triggered(bool) signal to.  Leave nullptr if no
     *               connection is desired.
     *
     * Returns new action of the given type ActionType.
     */
    QAction *addAction(KStandardAction::StandardAction actionType, const QObject *receiver = nullptr, const char *member = nullptr);

    /*!
     * \brief Creates a new standard action, adds to the collection under the given name
     * and connects the action's triggered(bool) signal to the specified
     * receiver/member. The newly created action is also returned.
     *
     * \note Using KStandardAction::OpenRecent will cause a different signal than
     * triggered(bool) to be used, see KStandardAction for more information.
     *
     * The action can be retrieved later from the collection by the specified name.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * \a actionType The standard action type of the action to create.
     *
     * \a name The name by which the action be retrieved again from the collection.
     *
     * \a receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     *
     * \a member The SLOT to connect the triggered(bool) signal to.  Leave nullptr if no
     *               connection is desired.
     *
     * Returns new action of the given type ActionType.
     */
    QAction *addAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver = nullptr, const char *member = nullptr);

    /*!
     * \brief This is the same as addAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver, const char *member)
     * but using new style connect syntax.
     *
     * \a actionType The standard action type of the action to create.
     *
     * \a name The name by which the action be retrieved again from the collection.
     *
     * \a receiver The QObject to connect the triggered(bool) signal to.
     *
     * \a slot The slot or lambda to connect the triggered(bool) signal to.
     *
     * Returns new action of the given type ActionType.
     *
     * \sa addAction(KStandardAction::StandardAction, const QString &, const QObject *, const char *)
     * \since 5.80
     */
#ifdef Q_QDOC
    template<class Receiver, class Func>
    inline QAction *addAction(KStandardAction::StandardAction actionType, const QString &name, const Receiver *receiver, Func slot)
#else
    template<class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    addAction(KStandardAction::StandardAction actionType, const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        QAction *action = KStandardAction::create(actionType, receiver, slot, nullptr);
        action->setParent(this);
        action->setObjectName(name);
        return addAction(name, action);
    }

    /*!
     * \brief This is the same as addAction(KStandardAction::StandardAction actionType, const QString &name, const Receiver *receiver, Func slot)
     * but using KStandardActions from KConfigGui.
     *
     * \a actionType The standard action type of the action to create.
     *
     * \a name The name by which the action be retrieved again from the collection.
     *
     * \a receiver The QObject to connect the triggered(bool) signal to.
     *
     * \a slot The slot or lambda to connect the triggered(bool) signal to.
     *
     * Returns new action of the given type ActionType.
     * \since 6.3
     */
#ifdef Q_QDOC
    template<class Receiver, class Func>
    inline QAction *addAction(KStandardActions::StandardAction actionType, const QString &name, const Receiver *receiver, Func slot)
#else
    template<class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    addAction(KStandardActions::StandardAction actionType, const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        // Use implementation from KConfigWidgets instead of KConfigGui
        // as it provides tighter integration with QtWidgets applications.
        QAction *action = KStandardAction::create(static_cast<KStandardAction::StandardAction>(actionType), receiver, slot, nullptr);
        action->setParent(this);
        action->setObjectName(name);
        return addAction(name, action);
    }

    /*!
     * Creates a new standard action and adds it to the collection.
     *
     * The newly created action is also returned.
     *
     * \a actionType The standard action type of the action to create.
     *
     * \a name The name by which the action be retrieved again from the collection.
     *
     * \return new action of the given type ActionType.
     *
     * \since 6.15
     */
    QAction *addAction(KStandardActions::StandardAction actionType, const QString &name);

    /*!
     * Creates a new standard action, adds it to the collection and connects the
     * action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is also returned.
     *
     * \note Using KStandardAction::OpenRecent will cause a different signal than
     * triggered(bool) to be used, see KStandardAction for more information.
     *
     * The action can be retrieved later from the collection by its standard name as per
     * KStandardAction::stdName.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * \a actionType The standard action type of the action to create.
     *
     * \a receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     *
     * \a slot The slot or lambda to connect the triggered(bool) signal to.
     *
     * Returns new action of the given type ActionType.
     *
     * \since 6.3
     */
#ifdef Q_QDOC
    template<class Receiver, class Func>
    inline QAction *addAction(KStandardActions::StandardAction actionType, const Receiver *receiver, Func slot)
#else
    template<class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    addAction(KStandardActions::StandardAction actionType, const Receiver *receiver, Func slot)
#endif
    {
        // Use implementation from KConfigWidgets instead of KConfigGui
        // as it provides tighter integration with QtWidgets applications.
        // KStandardAction automatically adds it to the collection.
        QAction *action = KStandardAction::create(static_cast<KStandardAction::StandardAction>(actionType), receiver, slot, this);
        return action;
    }

    /*!
     * Creates a new standard action and adds it to the collection.
     * The newly created action is also returned.
     *
     * The action can be retrieved later from the collection by its standard name as per
     * KStandardAction::stdName.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * \a actionType The standard action type of the action to create.
     *
     * Returns new action of the given type ActionType.
     *
     * \since 6.9
     */
    QAction *addAction(KStandardActions::StandardAction actionType);

    /*!
     * Creates a new action under the given name to the collection and connects
     * the action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is returned.
     *
     * \note KDE prior to 4.2 used the triggered() signal instead of the triggered(bool)
     * signal.
     *
     * Inserting an action that was previously inserted under a different name will replace the
     * old entry, i.e. the action will not be available under the old name anymore but only under
     * the new one.
     *
     * Inserting an action under a name that is already used for another action will replace
     * the other action in the collection.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * \a name The name by which the action be retrieved again from the collection.
     *
     * \a receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     *
     * \a member The SLOT to connect the triggered(bool) signal to.  Leave nullptr if no
     *               connection is desired.
     *
     * Returns new action of the given type ActionType.
     */
    QAction *addAction(const QString &name, const QObject *receiver = nullptr, const char *member = nullptr);

    /*!
     * \brief Creates a new action under the given name, adds it to the collection,
     * and connects the action's triggered(bool) signal to the specified receiver/member.
     *
     * The receiver slot may accept either a bool or no
     * parameters at all (i.e. slotTriggered(bool) or slotTriggered() ).
     * The type of the action is specified by the template parameter ActionType.
     *
     * \note KDE prior to 4.2 connected the triggered() signal instead of the triggered(bool)
     * signal.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * \a name The internal name of the action (e.g. "file-open").
     *
     * \a receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     *
     * \a member The SLOT to connect the triggered(bool) signal to.  Leave nullptr if no
     *               connection is desired.
     *
     * Returns new action of the given type ActionType.
     *
     * \sa addAction()
     */
    template<class ActionType>
    ActionType *add(const QString &name, const QObject *receiver = nullptr, const char *member = nullptr)
    {
        ActionType *a = new ActionType(this);
        if (receiver && member) {
            connect(a, SIGNAL(triggered(bool)), receiver, member);
        }
        addAction(name, a);
        return a;
    }

/*!
 *
 *
 * \brief This is the same as add(const QString &name, const QObject *receiver, const char *member)
 * but using new style connect syntax.
 *
 * \a name The internal name of the action (e.g. "file-open").
 *
 * \a receiver The QObject to connect the triggered(bool) signal to.
 *
 * \a slot The slot or lambda to connect the triggered(bool) signal to.
 *
 * Returns new action of the given type ActionType.
 *
 * \sa add(const QString &, const QObject *, const char *)
 * \since 5.28
 */
#ifdef Q_QDOC
    template<class ActionType, typename Receiver, typename Func>
    ActionType *add(const QString &name, const Receiver *receiver, Func slot)
#else
    template<class ActionType, class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, ActionType>::type *
    add(const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        ActionType *a = new ActionType(this);
        connect(a, &QAction::triggered, receiver, slot);
        addAction(name, a);
        return a;
    }

/*!
 * \brief This is the same as addAction(const QString &name, const QObject *receiver, const char *member)
 * but using new style connect syntax.
 *
 * \a name The internal name of the action (e.g. "file-open").
 *
 * \a receiver The QObject to connect the triggered(bool) signal to.
 *
 * \a slot The slot or lambda to connect the triggered(bool) signal to.
 *
 * Returns new action of the given type ActionType.
 *
 * \sa addAction(const QString &, const QObject *, const char *)
 * \since 5.28
 */
#ifdef Q_QDOC
    template<class Receiver, class Func>
    inline QAction *addAction(const QString &name, const Receiver *receiver, Func slot)
#else
    template<class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    addAction(const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        return add<QAction>(name, receiver, slot);
    }

    /*!
     * \brief Returns the default primary shortcut for the given \a action.
     *
     * \since 5.0
     */
    static QKeySequence defaultShortcut(QAction *action);

    /*!
     * \brief Returns the default shortcuts for the given \a action.
     *
     * \since 5.0
     */
    static QList<QKeySequence> defaultShortcuts(QAction *action);

    /*!
     * \brief Sets the default \a shortcut for the given \a action.
     *
     * Since 5.2, this also calls action->setShortcut(shortcut),
     * i.e. the default shortcut is made active initially.
     *
     * \since 5.0
     */
    static void setDefaultShortcut(QAction *action, const QKeySequence &shortcut);

    /*!
     * \brief Sets the default \a shortcuts in their specified shortcutContext()
     * for the given \a action.
     *
     * Since 5.2, this also calls action->setShortcuts(shortcuts),
     * i.e. the default shortcut is made active initially.

     * \since 5.0
     */
    Q_INVOKABLE static void setDefaultShortcuts(QAction *action, const QList<QKeySequence> &shortcuts);

    /*!
     * \brief Returns true if the given \a action shortcuts may be configured by the user.
     *
     * \since 5.0
     */
    static bool isShortcutsConfigurable(QAction *action);

    /*!
     * \brief Sets whether the \a action shortcuts may be \a configurable
     * by the user.
     *
     * \since 5.0
     */
    static void setShortcutsConfigurable(QAction *action, bool configurable);

private:
    KXMLGUI_NO_EXPORT explicit KActionCollection(const KXMLGUIClient *parent); // used by KXMLGUIClient

    friend class KActionCollectionPrivate;
    class KActionCollectionPrivate *const d;
};

#endif
