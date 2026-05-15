/*
    SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WAYLANDXDGDIALOGV1_P_H
#define WAYLANDXDGDIALOGV1_P_H

#include "qwayland-xdg-dialog-v1.h"

#include <QObject>
#include <QtWaylandClient/QWaylandClientExtension>

class WaylandXdgDialogV1 : public QObject, public QtWayland::xdg_dialog_v1
{
    Q_OBJECT
public:
    explicit WaylandXdgDialogV1(::xdg_dialog_v1 *object);
    ~WaylandXdgDialogV1() override;
};

class WaylandXdgDialogWmV1 : public QWaylandClientExtensionTemplate<WaylandXdgDialogWmV1>, public QtWayland::xdg_wm_dialog_v1
{
public:
    ~WaylandXdgDialogWmV1() override;

    static WaylandXdgDialogWmV1 &self();

    WaylandXdgDialogV1 *getDialog(struct ::xdg_toplevel *toplevel);

private:
    WaylandXdgDialogWmV1();
};

#endif // WAYLANDXDGDIALOGV1_P_H
