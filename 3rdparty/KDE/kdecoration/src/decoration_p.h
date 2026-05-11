/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once
#include "decoration.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KDecoration3 API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace KDecoration3
{
class Decoration;
class DecorationBridge;
class DecorationButton;
class DecoratedWindow;
class DecorationSettings;
class DecorationShadow;

class Q_DECL_HIDDEN Decoration::Private
{
public:
    Private(Decoration *decoration, const QVariantList &args);

    QMarginsF resizeOnlyBorders;

    Qt::WindowFrameSection sectionUnderMouse;
    void setSectionUnderMouse(Qt::WindowFrameSection section);
    void updateSectionUnderMouse(const QPoint &mousePosition);

    QRectF titleBar;
    QRegion blurRegion;

    void addButton(DecorationButton *button);

    std::shared_ptr<DecorationSettings> settings;
    DecorationBridge *bridge;
    std::shared_ptr<DecoratedWindow> client;
    bool opaque;
    QList<DecorationButton *> buttons;
    std::shared_ptr<DecorationShadow> shadow;
    std::shared_ptr<DecorationState> next;
    std::shared_ptr<DecorationState> current;

private:
    Decoration *q;
};

class PositionerData : public QSharedData
{
public:
    QRectF anchorRect;
};

} // namespace
