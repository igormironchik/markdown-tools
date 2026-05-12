/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#include "waylandinhibition_p.h"

#include "kguiaddons_debug.h"

#include <QDebug>
#include <QGuiApplication>
#include <QSharedPointer>
#include <QWaylandClientExtensionTemplate>
#include <QWindow>
#include <qpa/qplatformwindow_p.h>

#include "qwayland-keyboard-shortcuts-inhibit-unstable-v1.h"

class ShortcutsInhibitor : public QtWayland::zwp_keyboard_shortcuts_inhibitor_v1
{
public:
    ShortcutsInhibitor(::zwp_keyboard_shortcuts_inhibitor_v1 *id)
        : QtWayland::zwp_keyboard_shortcuts_inhibitor_v1(id)
    {
    }

    ~ShortcutsInhibitor() override
    {
        destroy();
    }

    void zwp_keyboard_shortcuts_inhibitor_v1_active() override
    {
        m_active = true;
    }

    void zwp_keyboard_shortcuts_inhibitor_v1_inactive() override
    {
        m_active = false;
    }

    bool isActive() const
    {
        return m_active;
    }

private:
    bool m_active = false;
};

class ShortcutsInhibitManager : public QWaylandClientExtensionTemplate<ShortcutsInhibitManager>, public QtWayland::zwp_keyboard_shortcuts_inhibit_manager_v1
{
public:
    ShortcutsInhibitManager()
        : QWaylandClientExtensionTemplate<ShortcutsInhibitManager>(1)
    {
        initialize();
    }
    ~ShortcutsInhibitManager() override
    {
        if (isInitialized()) {
            destroy();
        }
    }

    void startInhibition(QWindow *window)
    {
        if (m_inhibitions.contains(window)) {
            return;
        }
        auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
        if (!waylandApp || !waylandWindow) {
            return;
        }

        auto seat = waylandApp->lastInputSeat();
        auto surface = waylandWindow->surface();

        if (!seat || !surface) {
            return;
        }
        m_inhibitions[window].reset(new ShortcutsInhibitor(inhibit_shortcuts(surface, seat)));
    }

    bool isInhibited(QWindow *window) const
    {
        return m_inhibitions.contains(window);
    }

    void stopInhibition(QWindow *window)
    {
        m_inhibitions.remove(window);
    }

    QHash<QWindow *, QSharedPointer<ShortcutsInhibitor>> m_inhibitions;
};

static std::shared_ptr<ShortcutsInhibitManager> theManager()
{
    static std::weak_ptr<ShortcutsInhibitManager> managerInstance;
    std::shared_ptr<ShortcutsInhibitManager> ret = managerInstance.lock();
    if (!ret) {
        ret = std::make_shared<ShortcutsInhibitManager>();
        managerInstance = ret;
    }
    return ret;
}

WaylandInhibition::WaylandInhibition(QWindow *window)
    : ShortcutInhibition()
    , m_window(window)
    , m_manager(theManager())
{
}

WaylandInhibition::~WaylandInhibition() = default;

bool WaylandInhibition::shortcutsAreInhibited() const
{
    return m_manager->isInhibited(m_window);
}

void WaylandInhibition::enableInhibition()
{
    if (!m_manager->isActive()) {
        qCInfo(KGUIADDONS_LOG) << "The compositor does not support the keyboard-shortcuts-inhibit-unstable-v1 protocol. Inhibiting shortcuts will not work.";
        return;
    }
    m_manager->startInhibition(m_window);
}

void WaylandInhibition::disableInhibition()
{
    if (!m_manager->isActive()) {
        return;
    }
    m_manager->stopInhibition(m_window);
}
