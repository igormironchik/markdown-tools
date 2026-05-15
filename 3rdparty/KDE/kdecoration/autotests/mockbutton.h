/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../src/decorationbutton.h"

class MockButton : public KDecoration3::DecorationButton
{
    Q_OBJECT
public:
    MockButton(KDecoration3::DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent = nullptr);
    void paint(QPainter *painter, const QRectF &repaintRegion) override;
};
