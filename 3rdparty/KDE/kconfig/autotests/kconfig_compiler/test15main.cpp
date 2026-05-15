/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: MIT
*/
#include "test15.h"

int main(int, char **)
{
    QStandardPaths::setTestModeEnabled(true);
    Test15::self();
    return 0;
}
