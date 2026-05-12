/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: MIT
*/
#include "test17.h"

int main(int, char **)
{
    QStandardPaths::setTestModeEnabled(true);
    Test17::self();
    return 0;
}
