// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>

#include "ksysteminhibitor.h"

#include <windows.h>

#include "kguiaddons_debug.h"

using namespace Qt::StringLiterals;

class KSystemInhibitorPrivate
{
public:
    HANDLE m_handle = INVALID_HANDLE_VALUE;
};

KSystemInhibitor::KSystemInhibitor(const QString &reason, Types types, [[maybe_unused]] QWindow *window, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<KSystemInhibitorPrivate>())
{
    REASON_CONTEXT context{};
    context.Version = POWER_REQUEST_CONTEXT_VERSION;
    context.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
    context.Reason.SimpleReasonString = reason.toStdWString().data();

    HANDLE handle = PowerCreateRequest(&context);
    if (handle == INVALID_HANDLE_VALUE) {
        qCWarning(KGUIADDONS_LOG) << "KSystemInhibitor Error: Failed to create power request:" << GetLastError();
        return;
    }

    if (types & KSystemInhibitor::Type::Suspend) {
        if (!PowerSetRequest(handle, PowerRequestExecutionRequired)) {
            qCWarning(KGUIADDONS_LOG) << "KSystemInhibitor Error: Failed to set ExecutionRequired:" << GetLastError();
        }
    }

    if (types & KSystemInhibitor::Type::Idle) {
        if (!PowerSetRequest(handle, PowerRequestDisplayRequired)) {
            qCWarning(KGUIADDONS_LOG) << "KSystemInhibitor Error: Failed to set DisplayRequired:" << GetLastError();
        }
    }

    d->m_handle = handle;
}

KSystemInhibitor::~KSystemInhibitor()
{
    if (d->m_handle != INVALID_HANDLE_VALUE) {
        PowerClearRequest(d->m_handle, PowerRequestExecutionRequired);
        PowerClearRequest(d->m_handle, PowerRequestDisplayRequired);
        CloseHandle(d->m_handle);
    }
}

#include "moc_ksysteminhibitor.cpp"
