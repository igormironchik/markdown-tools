/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include "ksystemclockskewnotifierengine_p.h"

class KDBusSystemClockSkewNotifierEngine : public KSystemClockSkewNotifierEngine
{
    Q_OBJECT

public:
    static std::shared_ptr<KDBusSystemClockSkewNotifierEngine> create();

    KDBusSystemClockSkewNotifierEngine();
};
