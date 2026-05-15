/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WLRWAYLANDCLIPBOARD_H
#define WLRWAYLANDCLIPBOARD_H

#include "ksystemclipboard.h"

#include <memory>

class WlrDataControlDevice;
class WlrDataControlDeviceManager;
class WlrKeyboardFocusWatcher;

class WlrWaylandClipboard : public KSystemClipboard
{
public:
    ~WlrWaylandClipboard();
    static WlrWaylandClipboard *create(QObject *parent);
    void setMimeData(QMimeData *mime, QClipboard::Mode mode) override;
    void clear(QClipboard::Mode mode) override;
    const QMimeData *mimeData(QClipboard::Mode mode) const override;

    bool isValid();

private:
    WlrWaylandClipboard(QObject *parent);
    void gainedFocus();
    std::unique_ptr<WlrKeyboardFocusWatcher> m_keyboardFocusWatcher;
    std::unique_ptr<WlrDataControlDeviceManager> m_manager;
    std::unique_ptr<WlrDataControlDevice> m_device;
};

#endif
