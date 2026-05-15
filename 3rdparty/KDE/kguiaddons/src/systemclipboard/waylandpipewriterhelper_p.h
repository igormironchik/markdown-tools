// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WAYLANDPIPEWRITERHELPER_P_H
#define WAYLANDPIPEWRITERHELPER_P_H

#include <QtCore/qtypes.h>
#include <chrono>

namespace WaylandPipeWriterHelper
{
enum class SafeWriteResult {
    Ok,
    Timeout,
    Closed,
    Error,
};
SafeWriteResult safeWriteWithTimeout(int fd, const char *data, qsizetype len, qsizetype chunkSize, std::chrono::nanoseconds timeout);
};

#endif // WAYLANDPIPEWRITERHELPER_P_H
