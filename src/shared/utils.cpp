/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// shared include.
#include "utils.hpp"

// Qt include.
#include <QtResource>

void initSharedResources()
{
    Q_INIT_RESOURCE(qt);
    Q_INIT_RESOURCE(icon);
}
