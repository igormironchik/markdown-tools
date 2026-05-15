/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ksystemclockskewnotifierengine_linux.h"

#include <QSocketNotifier>

#include <cerrno>
#include <fcntl.h>
#include <sys/timerfd.h>
#include <unistd.h>

#ifndef TFD_TIMER_CANCEL_ON_SET // only available in newer glib
#define TFD_TIMER_CANCEL_ON_SET (1 << 1)
#endif

std::shared_ptr<KLinuxSystemClockSkewNotifierEngine> KLinuxSystemClockSkewNotifierEngine::create()
{
    int fd = timerfd_create(CLOCK_REALTIME, O_CLOEXEC | O_NONBLOCK);
    if (fd == -1) {
        qWarning("Couldn't create clock skew notifier engine: %s", strerror(errno));
        return nullptr;
    }

    const itimerspec spec = {};
    const int ret = timerfd_settime(fd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &spec, nullptr);
    if (ret == -1) {
        qWarning("Couldn't create clock skew notifier engine: %s", strerror(errno));
        close(fd);
        return nullptr;
    }
    return std::make_shared<KLinuxSystemClockSkewNotifierEngine>(fd);
}

KLinuxSystemClockSkewNotifierEngine::KLinuxSystemClockSkewNotifierEngine(int fd)
    : m_fd(fd)
{
    const QSocketNotifier *notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &KLinuxSystemClockSkewNotifierEngine::handleTimerCancelled);
}

KLinuxSystemClockSkewNotifierEngine::~KLinuxSystemClockSkewNotifierEngine()
{
    close(m_fd);
}

void KLinuxSystemClockSkewNotifierEngine::handleTimerCancelled()
{
    uint64_t expirationCount;
    read(m_fd, &expirationCount, sizeof(expirationCount));

    Q_EMIT skewed();
}

#include "moc_ksystemclockskewnotifierengine_linux.cpp"
