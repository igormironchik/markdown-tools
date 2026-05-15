/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2005-2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "config-xmlgui.h"

#include "kactioncollection.h"

#include "debug.h"
#include "kactioncategory.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

#include <KAuthorized>
#include <KConfigGroup>
#if HAVE_GLOBALACCEL
#include <KGlobalAccel>
#endif
#include <KSharedConfig>

#include <QDomDocument>
#include <QGuiApplication>
#include <QList>
#include <QMetaMethod>
#include <QSet>

static bool actionHasGlobalShortcut(const QAction *action)
{
#if HAVE_GLOBALACCEL
    return KGlobalAccel::self()->hasShortcut(action);
#else
    Q_UNUSED(action)
    return false;
#endif
}

struct ActionStorage {
    void addAction(const QString &name, QAction *action)
    {
        Q_ASSERT(std::find(m_actions.begin(), m_actions.end(), action) == m_actions.end());
        m_actions.push_back(action);
        m_names.push_back(name);
        Q_ASSERT(m_names.size() == m_actions.size());
    }

    bool removeAction(QAction *action)
    {
        auto it = std::find(m_actions.begin(), m_actions.end(), action);
        if (it != m_actions.end()) {
            // We can't have the same action twice.
            Q_ASSERT(std::find(it + 1, m_actions.end(), action) == m_actions.end());
            auto idx = std::distance(m_actions.begin(), it);
            m_names.erase(m_names.begin() + idx);
            m_actions.erase(it);
            return true;
        }
        Q_ASSERT(m_names.size() == m_actions.size());
        return false;
    }

    QAction *findAction(const QString &name) const
    {
        auto it = std::find(m_names.begin(), m_names.end(), name);
        if (it != m_names.end() && *it == name) {
            return m_actions[std::distance(m_names.begin(), it)];
        }
        return nullptr;
    }

    void clear()
    {
        m_actions = {};
        m_names = {};
    }

    int size() const
    {
        return m_actions.size();
    }

    template<typename F>
    void foreachAction(F f)
    {
        Q_ASSERT(m_names.size() == m_actions.size());
        for (int i = 0; i < m_actions.size(); ++i) {
            f(m_names.at(i), m_actions.at(i));
        }
    }

    const QList<QAction *> &actions() const
    {
        return m_actions;
    }

private:
    // 1:1 list of names and actions
    QList<QString> m_names;
    QList<QAction *> m_actions;
};

class KActionCollectionPrivate
{
public:
    KActionCollectionPrivate(KActionCollection *qq)
        : q(qq)
        , configIsGlobal(false)
        , connectTriggered(false)
        , connectHovered(false)

    {
    }

    void setComponentForAction(QAction *action)
    {
        const bool hasGlobalShortcut = actionHasGlobalShortcut(action);
        if (!hasGlobalShortcut) {
            action->setProperty("componentName", m_componentName);
            action->setProperty("componentDisplayName", m_componentDisplayName);
        }
    }

    static QList<KActionCollection *> s_allCollections;

    void _k_associatedWidgetDestroyed(QObject *obj);
    void _k_actionDestroyed(QObject *obj);

    bool writeKXMLGUIConfigFile();

    QString m_componentName;
    QString m_componentDisplayName;

    //! Remove a action from our internal bookkeeping. Returns a nullptr if the
    //! action doesn't belong to us.
    QAction *unlistAction(QAction *);

    ActionStorage actionStore;

    KActionCollection *q = nullptr;

    const KXMLGUIClient *m_parentGUIClient = nullptr;

    QString configGroup{QStringLiteral("Shortcuts")};
    bool configIsGlobal : 1;

    bool connectTriggered : 1;
    bool connectHovered : 1;

    QList<QWidget *> associatedWidgets;
};

QList<KActionCollection *> KActionCollectionPrivate::s_allCollections;

KActionCollection::KActionCollection(QObject *parent, const QString &cName)
    : QObject(parent)
    , d(new KActionCollectionPrivate(this))
{
    KActionCollectionPrivate::s_allCollections.append(this);

    setComponentName(cName);
}

KActionCollection::KActionCollection(const KXMLGUIClient *parent)
    : QObject(nullptr)
    , d(new KActionCollectionPrivate(this))
{
    KActionCollectionPrivate::s_allCollections.append(this);

    d->m_parentGUIClient = parent;
    d->m_componentName = parent->componentName();
}

KActionCollection::~KActionCollection()
{
    KActionCollectionPrivate::s_allCollections.removeAll(this);

    delete d;
}

void KActionCollection::clear()
{
    const QList<QAction *> copy = d->actionStore.actions();
    qDeleteAll(copy);
    d->actionStore.clear();
}

QAction *KActionCollection::action(const QString &name) const
{
    if (!name.isEmpty()) {
        return d->actionStore.findAction(name);
    }
    return nullptr;
}

QAction *KActionCollection::action(int index) const
{
    // ### investigate if any apps use this at all
    return actions().value(index);
}

int KActionCollection::count() const
{
    return d->actionStore.size();
}

bool KActionCollection::isEmpty() const
{
    return count() == 0;
}

void KActionCollection::setComponentName(const QString &cName)
{
    for (auto *a : d->actionStore.actions()) {
        if (actionHasGlobalShortcut(a)) {
            // Its component name is part of an action's signature in the context of
            // global shortcuts and the semantics of changing an existing action's
            // signature are, as it seems, impossible to get right.
            qCWarning(DEBUG_KXMLGUI) << "KActionCollection::setComponentName does not work on a KActionCollection containing actions with global shortcuts!"
                                     << cName;
            break;
        }
    }

    if (!cName.isEmpty()) {
        d->m_componentName = cName;
    } else {
        d->m_componentName = QCoreApplication::applicationName();
    }
}

QString KActionCollection::componentName() const
{
    return d->m_componentName;
}

void KActionCollection::setComponentDisplayName(const QString &displayName)
{
    d->m_componentDisplayName = displayName;
}

QString KActionCollection::componentDisplayName() const
{
    if (!d->m_componentDisplayName.isEmpty()) {
        return d->m_componentDisplayName;
    }
    if (!QGuiApplication::applicationDisplayName().isEmpty()) {
        return QGuiApplication::applicationDisplayName();
    }
    return QCoreApplication::applicationName();
}

const KXMLGUIClient *KActionCollection::parentGUIClient() const
{
    return d->m_parentGUIClient;
}

QList<QAction *> KActionCollection::actions() const
{
    return d->actionStore.actions();
}

const QList<QAction *> KActionCollection::actionsWithoutGroup() const
{
    QList<QAction *> ret;
    for (auto *action : std::as_const(d->actionStore.actions())) {
        if (!action->actionGroup()) {
            ret.append(action);
        }
    }
    return ret;
}

const QList<QActionGroup *> KActionCollection::actionGroups() const
{
    QSet<QActionGroup *> set;
    for (auto *action : std::as_const(d->actionStore.actions())) {
        if (action->actionGroup()) {
            set.insert(action->actionGroup());
        }
    }
    return set.values();
}

QAction *KActionCollection::addAction(const QString &name, QAction *action)
{
    if (!action) {
        return action;
    }

    const QString objectName = action->objectName();
    QString indexName = name;

    if (indexName.isEmpty()) {
        // No name provided. Use the objectName.
        indexName = objectName;

    } else {
        // A name was provided. Check against objectName.
        if ((!objectName.isEmpty()) && (objectName != indexName)) {
            // The user specified a new name and the action already has a
            // different one. The objectName is used for saving shortcut
            // settings to disk. Both for local and global shortcuts.
            qCDebug(DEBUG_KXMLGUI) << "Registering action " << objectName << " under new name " << indexName;
            // If there is a global shortcuts it's a very bad idea.
#if HAVE_GLOBALACCEL
            if (KGlobalAccel::self()->hasShortcut(action)) {
                // In debug mode assert
                Q_ASSERT(!KGlobalAccel::self()->hasShortcut(action));
                // In release mode keep the old name
                qCCritical(DEBUG_KXMLGUI) << "Changing action name from " << objectName << " to " << indexName
                                          << "\nignored because of active global shortcut.";
                indexName = objectName;
            }
#endif
        }

        // Set the new name
        action->setObjectName(indexName);
    }

    // No name provided and the action had no name. Make one up. This will not
    // work when trying to save shortcuts. Both local and global shortcuts.
    if (indexName.isEmpty()) {
        indexName = QString::asprintf("unnamed-%p", (void *)action);
        action->setObjectName(indexName);
    }

    // From now on the objectName has to have a value. Else we cannot safely
    // remove actions.
    Q_ASSERT(!action->objectName().isEmpty());

    // look if we already have THIS action under THIS name ;)
    auto oldAction = d->actionStore.findAction(indexName);
    if (oldAction == action) {
        return action;
    }

    if (!KAuthorized::authorizeAction(indexName)) {
        // Disable this action
        action->setEnabled(false);
        action->setVisible(false);
        action->blockSignals(true);
    }

    // Check if we have another action under this name
    if (oldAction) {
        takeAction(oldAction);
    }

    // Remove if we have this action under a different name.
    // Not using takeAction because we don't want to remove it from categories,
    // and because it has the new name already.
    d->actionStore.removeAction(action);

    // Add action to our lists.
    d->actionStore.addAction(indexName, action);

    for (QWidget *widget : std::as_const(d->associatedWidgets)) {
        widget->addAction(action);
    }

    connect(action, &QObject::destroyed, this, [this](QObject *obj) {
        d->_k_actionDestroyed(obj);
    });

    d->setComponentForAction(action);

    if (d->connectHovered) {
        connect(action, &QAction::hovered, this, &KActionCollection::slotActionHovered);
    }

    if (d->connectTriggered) {
        connect(action, &QAction::triggered, this, &KActionCollection::slotActionTriggered);
    }

    Q_EMIT inserted(action);
    Q_EMIT changed();
    return action;
}

void KActionCollection::addActions(const QList<QAction *> &actions)
{
    for (QAction *action : actions) {
        addAction(action->objectName(), action);
    }
}

void KActionCollection::removeAction(QAction *action)
{
    delete takeAction(action);
}

QAction *KActionCollection::takeAction(QAction *action)
{
    if (!d->unlistAction(action)) {
        return nullptr;
    }

    // Remove the action from all widgets
    for (QWidget *widget : std::as_const(d->associatedWidgets)) {
        widget->removeAction(action);
    }

    action->disconnect(this);

    Q_EMIT changed();
    return action;
}

QAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QObject *receiver, const char *member)
{
    QAction *action = KStandardAction::create(actionType, receiver, member, this);
    return action;
}

QAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver, const char *member)
{
    // pass 0 as parent, because if the parent is a KActionCollection KStandardAction::create automatically
    // adds the action to it under the default name. We would trigger the
    // warning about renaming the action then.
    QAction *action = KStandardAction::create(actionType, receiver, member, nullptr);
    // Give it a parent for gc.
    action->setParent(this);
    // Remove the name to get rid of the "rename action" warning above
    action->setObjectName(name);
    // And now add it with the desired name.
    return addAction(name, action);
}

QAction *KActionCollection::addAction(KStandardActions::StandardAction actionType)
{
    // Use implementation from KConfigWidgets instead of KConfigGui
    // as it provides tighter integration with QtWidgets applications.
    // KStandardAction automatically adds it to the collection.
    QAction *action = KStandardAction::create(static_cast<KStandardAction::StandardAction>(actionType), nullptr, {}, this);
    return action;
}

QAction *KActionCollection::addAction(KStandardActions::StandardAction actionType, const QString &name)
{
    // Use implementation from KConfigWidgets instead of KConfigGui
    // as it provides tighter integration with QtWidgets applications.
    QAction *action = KStandardAction::create(static_cast<KStandardAction::StandardAction>(actionType), nullptr, {}, nullptr);
    action->setParent(this);
    action->setObjectName(name);
    return addAction(name, action);
}

QAction *KActionCollection::addAction(const QString &name, const QObject *receiver, const char *member)
{
    QAction *a = new QAction(this);
    if (receiver && member) {
        connect(a, SIGNAL(triggered(bool)), receiver, member);
    }
    return addAction(name, a);
}

QKeySequence KActionCollection::defaultShortcut(QAction *action)
{
    const QList<QKeySequence> shortcuts = defaultShortcuts(action);
    return shortcuts.isEmpty() ? QKeySequence() : shortcuts.first();
}

QList<QKeySequence> KActionCollection::defaultShortcuts(QAction *action)
{
    return action->property("defaultShortcuts").value<QList<QKeySequence>>();
}

void KActionCollection::setDefaultShortcut(QAction *action, const QKeySequence &shortcut)
{
    setDefaultShortcuts(action, QList<QKeySequence>() << shortcut);
}

void KActionCollection::setDefaultShortcuts(QAction *action, const QList<QKeySequence> &shortcuts)
{
    action->setShortcuts(shortcuts);
    action->setProperty("defaultShortcuts", QVariant::fromValue(shortcuts));
}

bool KActionCollection::isShortcutsConfigurable(QAction *action)
{
    // Considered as true by default
    const QVariant value = action->property("isShortcutConfigurable");
    return value.isValid() ? value.toBool() : true;
}

void KActionCollection::setShortcutsConfigurable(QAction *action, bool configurable)
{
    action->setProperty("isShortcutConfigurable", configurable);
}

QString KActionCollection::configGroup() const
{
    return d->configGroup;
}

void KActionCollection::setConfigGroup(const QString &group)
{
    d->configGroup = group;
}

bool KActionCollection::configIsGlobal() const
{
    return d->configIsGlobal;
}

void KActionCollection::setConfigGlobal(bool global)
{
    d->configIsGlobal = global;
}

void KActionCollection::importGlobalShortcuts(KConfigGroup *config)
{
#if HAVE_GLOBALACCEL
    Q_ASSERT(config);
    if (!config || !config->exists()) {
        return;
    }

    d->actionStore.foreachAction([config](const QString &actionName, QAction *action) {
        if (!action) {
            return;
        }

        if (isShortcutsConfigurable(action)) {
            QString entry = config->readEntry(actionName, QString());
            if (!entry.isEmpty()) {
                KGlobalAccel::self()->setShortcut(action, QKeySequence::listFromString(entry), KGlobalAccel::NoAutoloading);
            } else {
                QList<QKeySequence> defaultShortcut = KGlobalAccel::self()->defaultShortcut(action);
                KGlobalAccel::self()->setShortcut(action, defaultShortcut, KGlobalAccel::NoAutoloading);
            }
        }
    });
#else
    Q_UNUSED(config);
#endif
}

void KActionCollection::readSettings(KConfigGroup *config)
{
    KConfigGroup cg(KSharedConfig::openConfig(), configGroup());
    if (!config) {
        config = &cg;
    }

    if (!config->exists()) {
        return;
    }

    d->actionStore.foreachAction([config](const QString &actionName, QAction *action) {
        if (!action) {
            return;
        }

        if (isShortcutsConfigurable(action)) {
            QString entry = config->readEntry(actionName, QString());
            if (!entry.isEmpty()) {
                action->setShortcuts(QKeySequence::listFromString(entry));
            } else {
                action->setShortcuts(defaultShortcuts(action));
            }
        }
    });

    // qCDebug(DEBUG_KXMLGUI) << " done";
}

void KActionCollection::exportGlobalShortcuts(KConfigGroup *config, bool writeAll) const
{
#if HAVE_GLOBALACCEL
    Q_ASSERT(config);
    if (!config) {
        return;
    }

    d->actionStore.foreachAction([config, this, writeAll](const QString &actionName, QAction *action) {
        if (!action) {
            return;
        }
        // If the action name starts with unnamed- spit out a warning. That name
        // will change at will and will break loading writing
        if (actionName.startsWith(QLatin1String("unnamed-"))) {
            qCCritical(DEBUG_KXMLGUI) << "Skipped exporting Shortcut for action without name " << action->text() << "!";
            return;
        }

        if (isShortcutsConfigurable(action) && KGlobalAccel::self()->hasShortcut(action)) {
            bool bConfigHasAction = !config->readEntry(actionName, QString()).isEmpty();
            bool bSameAsDefault = (KGlobalAccel::self()->shortcut(action) == KGlobalAccel::self()->defaultShortcut(action));
            // If we're using a global config or this setting
            //  differs from the default, then we want to write.
            KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;
            if (configIsGlobal()) {
                flags |= KConfigGroup::Global;
            }
            if (writeAll || !bSameAsDefault) {
                QString s = QKeySequence::listToString(KGlobalAccel::self()->shortcut(action));
                if (s.isEmpty()) {
                    s = QStringLiteral("none");
                }
                qCDebug(DEBUG_KXMLGUI) << "\twriting " << actionName << " = " << s;
                config->writeEntry(actionName, s, flags);
            }
            // Otherwise, this key is the same as default
            //  but exists in config file.  Remove it.
            else if (bConfigHasAction) {
                qCDebug(DEBUG_KXMLGUI) << "\tremoving " << actionName << " because == default";
                config->deleteEntry(actionName, flags);
            }
        }
    });

    config->sync();
#else
    Q_UNUSED(config);
    Q_UNUSED(writeAll);
#endif
}

bool KActionCollectionPrivate::writeKXMLGUIConfigFile()
{
    const KXMLGUIClient *kxmlguiClient = q->parentGUIClient();
    // return false if there is no KXMLGUIClient
    if (!kxmlguiClient || kxmlguiClient->xmlFile().isEmpty()) {
        return false;
    }

    qCDebug(DEBUG_KXMLGUI) << "xmlFile=" << kxmlguiClient->xmlFile();

    QString attrShortcut = QStringLiteral("shortcut");

    // Read XML file
    QString sXml(KXMLGUIFactory::readConfigFile(kxmlguiClient->xmlFile(), q->componentName()));
    QDomDocument doc;
    doc.setContent(sXml);

    // Process XML data

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement(doc);

    // now, iterate through our actions
    actionStore.foreachAction([&elem, &attrShortcut, this](const QString &actionName, QAction *action) {
        if (!action) {
            return;
        }

        // If the action name starts with unnamed- spit out a warning and ignore
        // it. That name will change at will and will break loading writing
        if (actionName.startsWith(QLatin1String("unnamed-"))) {
            qCCritical(DEBUG_KXMLGUI) << "Skipped writing shortcut for action " << actionName << "(" << action->text() << ")!";
            return;
        }

        bool bSameAsDefault = (action->shortcuts() == q->defaultShortcuts(action));
        qCDebug(DEBUG_KXMLGUI) << "name = " << actionName << " shortcut = " << QKeySequence::listToString(action->shortcuts())
#if HAVE_GLOBALACCEL
                               << " globalshortcut = " << QKeySequence::listToString(KGlobalAccel::self()->shortcut(action))
#endif
                               << " def = " << QKeySequence::listToString(q->defaultShortcuts(action));

        // now see if this element already exists
        // and create it if necessary (unless bSameAsDefault)
        QDomElement act_elem = KXMLGUIFactory::findActionByName(elem, actionName, !bSameAsDefault);
        if (act_elem.isNull()) {
            return;
        }

        if (bSameAsDefault) {
            act_elem.removeAttribute(attrShortcut);
            // qCDebug(DEBUG_KXMLGUI) << "act_elem.attributes().count() = " << act_elem.attributes().count();
            if (act_elem.attributes().count() == 1) {
                elem.removeChild(act_elem);
            }
        } else {
            act_elem.setAttribute(attrShortcut, QKeySequence::listToString(action->shortcuts()));
        }
    });

    // Write back to XML file
    KXMLGUIFactory::saveConfigFile(doc, kxmlguiClient->localXMLFile(), q->componentName());
    // and since we just changed the xml file clear the dom we have in memory
    // it'll be rebuilt if needed
    const_cast<KXMLGUIClient *>(kxmlguiClient)->setXMLGUIBuildDocument({});
    return true;
}

void KActionCollection::writeSettings(KConfigGroup *config, bool writeAll, QAction *oneAction) const
{
    // If the caller didn't provide a config group we try to save the KXMLGUI
    // Configuration file. If that succeeds we are finished.
    if (config == nullptr && d->writeKXMLGUIConfigFile()) {
        return;
    }

    KConfigGroup cg(KSharedConfig::openConfig(), configGroup());
    if (!config) {
        config = &cg;
    }

    QList<QAction *> writeActions;
    if (oneAction) {
        writeActions.append(oneAction);
    } else {
        writeActions = actions();
    }

    d->actionStore.foreachAction([config, this, writeAll](const QString &actionName, QAction *action) {
        if (!action) {
            return;
        }

        // If the action name starts with unnamed- spit out a warning and ignore
        // it. That name will change at will and will break loading writing
        if (actionName.startsWith(QLatin1String("unnamed-"))) {
            qCCritical(DEBUG_KXMLGUI) << "Skipped saving Shortcut for action without name " << action->text() << "!";
            return;
        }

        // Write the shortcut
        if (isShortcutsConfigurable(action)) {
            bool bConfigHasAction = !config->readEntry(actionName, QString()).isEmpty();
            bool bSameAsDefault = (action->shortcuts() == defaultShortcuts(action));
            // If we're using a global config or this setting
            //  differs from the default, then we want to write.
            KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;

            // Honor the configIsGlobal() setting
            if (configIsGlobal()) {
                flags |= KConfigGroup::Global;
            }

            if (writeAll || !bSameAsDefault) {
                // We are instructed to write all shortcuts or the shortcut is
                // not set to its default value. Write it
                QString s = QKeySequence::listToString(action->shortcuts());
                if (s.isEmpty()) {
                    s = QStringLiteral("none");
                }
                qCDebug(DEBUG_KXMLGUI) << "\twriting " << actionName << " = " << s;
                config->writeEntry(actionName, s, flags);

            } else if (bConfigHasAction) {
                // Otherwise, this key is the same as default but exists in
                // config file. Remove it.
                qCDebug(DEBUG_KXMLGUI) << "\tremoving " << actionName << " because == default";
                config->deleteEntry(actionName, flags);
            }
        }
    });

    config->sync();
}

void KActionCollection::slotActionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        Q_EMIT actionTriggered(action);
    }
}

void KActionCollection::slotActionHovered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        Q_EMIT actionHovered(action);
    }
}

// The downcast from a QObject to a QAction triggers UBSan
// but we're only comparing pointers, so UBSan shouldn't check vptrs
// Similar to https://github.com/itsBelinda/plog/pull/1/files
#if defined(__clang__) || __GNUC__ >= 8
__attribute__((no_sanitize("vptr")))
#endif
void KActionCollectionPrivate::_k_actionDestroyed(QObject *obj)
{
    // obj isn't really a QAction anymore. So make sure we don't do fancy stuff
    // with it.
    QAction *action = static_cast<QAction *>(obj);

    if (!unlistAction(action)) {
        return;
    }

    Q_EMIT q->changed();
}

void KActionCollection::connectNotify(const QMetaMethod &signal)
{
    if (d->connectHovered && d->connectTriggered) {
        return;
    }

    if (signal.methodSignature() == "actionHovered(QAction*)") {
        if (!d->connectHovered) {
            d->connectHovered = true;

            for (auto *action : d->actionStore.actions()) {
                connect(action, &QAction::hovered, this, &KActionCollection::slotActionHovered);
            }
        }

    } else if (signal.methodSignature() == "actionTriggered(QAction*)") {
        if (!d->connectTriggered) {
            d->connectTriggered = true;
            for (auto *action : d->actionStore.actions()) {
                connect(action, &QAction::triggered, this, &KActionCollection::slotActionTriggered);
            }
        }
    }

    QObject::connectNotify(signal);
}

const QList<KActionCollection *> &KActionCollection::allCollections()
{
    return KActionCollectionPrivate::s_allCollections;
}

void KActionCollection::associateWidget(QWidget *widget) const
{
    for (auto *action : d->actionStore.actions()) {
        if (!widget->actions().contains(action)) {
            widget->addAction(action);
        }
    }
}

void KActionCollection::addAssociatedWidget(QWidget *widget)
{
    if (!d->associatedWidgets.contains(widget)) {
        widget->addActions(actions());

        d->associatedWidgets.append(widget);
        connect(widget, &QObject::destroyed, this, [this](QObject *obj) {
            d->_k_associatedWidgetDestroyed(obj);
        });
    }
}

void KActionCollection::removeAssociatedWidget(QWidget *widget)
{
    for (auto *action : d->actionStore.actions()) {
        widget->removeAction(action);
    }

    d->associatedWidgets.removeAll(widget);
    disconnect(widget, &QObject::destroyed, this, nullptr);
}

QAction *KActionCollectionPrivate::unlistAction(QAction *action)
{
    // ATTENTION:
    //   This method is called with an QObject formerly known as a QAction
    //   during _k_actionDestroyed(). So don't do fancy stuff here that needs a
    //   real QAction!

    // Remove the action
    if (!actionStore.removeAction(action)) {
        return nullptr;
    }

    // Remove the action from the categories. Should be only one
    const QList<KActionCategory *> categories = q->findChildren<KActionCategory *>();
    for (KActionCategory *category : categories) {
        category->unlistAction(action);
    }

    return action;
}

QList<QWidget *> KActionCollection::associatedWidgets() const
{
    return d->associatedWidgets;
}

void KActionCollection::clearAssociatedWidgets()
{
    for (QWidget *widget : std::as_const(d->associatedWidgets)) {
        for (auto *action : d->actionStore.actions()) {
            widget->removeAction(action);
        }
    }

    d->associatedWidgets.clear();
}

void KActionCollectionPrivate::_k_associatedWidgetDestroyed(QObject *obj)
{
    associatedWidgets.removeAll(static_cast<QWidget *>(obj));
}

#include "moc_kactioncollection.cpp"
