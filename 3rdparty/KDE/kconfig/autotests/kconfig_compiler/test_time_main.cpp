/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <QGuiApplication>

#include "test_time.h"

int main(int argc, char **argv)
{
    QStandardPaths::setTestModeEnabled(true);
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestTime t = TestTime();
    return 0;
}
