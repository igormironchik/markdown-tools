/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2025 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBOOKMARKACTIONPRIVATE_H
#define KBOOKMARKACTIONPRIVATE_H

class KBookmarkActionPrivate
{
public:
    KBookmarkActionPrivate(KBookmarkAction *a, KBookmarkOwner *const o) : q(a), owner(o) {}

    static KBookmarkActionPrivate *get(KBookmarkAction *a) { return a->d; }


    Qt::MouseButtons buttons = Qt::NoButton;
    KBookmarkAction *const q;
    KBookmarkOwner *const owner;
};

#endif
