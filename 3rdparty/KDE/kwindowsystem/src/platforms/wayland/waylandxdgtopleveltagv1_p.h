/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "qwayland-xdg-toplevel-tag-v1.h"

#include <QtWaylandClient/QWaylandClientExtension>

class WaylandXdgToplevelTagManagerV1 : public QWaylandClientExtensionTemplate<WaylandXdgToplevelTagManagerV1>, public QtWayland::xdg_toplevel_tag_manager_v1
{
    Q_OBJECT

public:
    ~WaylandXdgToplevelTagManagerV1() override;

    static WaylandXdgToplevelTagManagerV1 *self();

private:
    WaylandXdgToplevelTagManagerV1();
};
