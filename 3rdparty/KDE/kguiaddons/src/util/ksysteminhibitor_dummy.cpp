// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#include "ksysteminhibitor.h"

class KSystemInhibitorPrivate
{
};

KSystemInhibitor::KSystemInhibitor([[maybe_unused]] const QString &reason, [[maybe_unused]] Types types, [[maybe_unused]] QWindow *window, QObject *parent)
    : QObject(parent)
{
}

KSystemInhibitor::~KSystemInhibitor() = default;

#include "moc_ksysteminhibitor.cpp"
