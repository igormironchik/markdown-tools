/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WAYLANDCLIPBOARD_H
#define WAYLANDCLIPBOARD_H

#include "ksystemclipboard.h"

#include <memory>

class DataControlDevice;
class DataControlDeviceManager;
class ClipboardThread;

class WaylandClipboard : public KSystemClipboard
{
    Q_OBJECT
public:
    ~WaylandClipboard();
    static WaylandClipboard *create(QObject *parent);
    void setMimeData(QMimeData *mime, QClipboard::Mode mode) override;
    void clear(QClipboard::Mode mode) override;
    const QMimeData *mimeData(QClipboard::Mode mode) const override;

    bool isValid();

private:
    WaylandClipboard(QObject *parent);
    std::unique_ptr<ClipboardThread> m_thread;
    std::unique_ptr<DataControlDeviceManager> m_manager;
    std::unique_ptr<DataControlDevice> m_device;
};

#endif
