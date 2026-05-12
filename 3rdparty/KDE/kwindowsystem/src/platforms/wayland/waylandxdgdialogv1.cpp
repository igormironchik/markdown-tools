/*
    SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "helpers.h"
#include "waylandxdgdialogv1_p.h"

WaylandXdgDialogV1::WaylandXdgDialogV1(::xdg_dialog_v1 *object)
    : QObject()
    , QtWayland::xdg_dialog_v1(object)
{
}

WaylandXdgDialogV1::~WaylandXdgDialogV1()
{
    if (isQpaAlive()) {
        destroy();
    }
}

WaylandXdgDialogWmV1::WaylandXdgDialogWmV1()
    : QWaylandClientExtensionTemplate<WaylandXdgDialogWmV1>(1)
{
    initialize();
}

WaylandXdgDialogWmV1::~WaylandXdgDialogWmV1()
{
    if (isQpaAlive() && isActive()) {
        destroy();
    }
}

WaylandXdgDialogWmV1 &WaylandXdgDialogWmV1::self()
{
    static WaylandXdgDialogWmV1 s_instance;
    return s_instance;
}

WaylandXdgDialogV1 *WaylandXdgDialogWmV1::getDialog(struct ::xdg_toplevel *toplevel)
{
    return new WaylandXdgDialogV1(get_xdg_dialog(toplevel));
}

#include "moc_waylandxdgdialogv1_p.cpp"
