// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#include "systeminhibitor.h"

void SystemInhibitor::classBegin()
{
}

void SystemInhibitor::componentComplete()
{
    m_ready = true;
    apply();
}

void SystemInhibitor::setEnabled(bool enable)
{
    if (m_enabled == enable) {
        return;
    }
    m_enabled = enable;
    Q_EMIT enabledChanged();

    apply();
}

void SystemInhibitor::apply()
{
    if (!m_ready) {
        return;
    }

    m_inhibitor.reset();
    if (m_enabled) {
        m_inhibitor.emplace(m_reason, m_types, m_window, this);
    }
}
