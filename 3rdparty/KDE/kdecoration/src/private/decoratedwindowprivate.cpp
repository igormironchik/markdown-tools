/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "decoratedwindowprivate.h"

#include <QColor>

namespace KDecoration3
{
class Q_DECL_HIDDEN DecoratedWindowPrivate::Private
{
public:
    explicit Private(DecoratedWindow *client, Decoration *decoration);
    DecoratedWindow *client;
    Decoration *decoration;
};

DecoratedWindowPrivate::Private::Private(DecoratedWindow *client, Decoration *decoration)
    : client(client)
    , decoration(decoration)
{
}

DecoratedWindowPrivate::DecoratedWindowPrivate(DecoratedWindow *client, Decoration *decoration)
    : d(new Private(client, decoration))
{
}

DecoratedWindowPrivate::~DecoratedWindowPrivate() = default;

Decoration *DecoratedWindowPrivate::decoration()
{
    return d->decoration;
}

Decoration *DecoratedWindowPrivate::decoration() const
{
    return d->decoration;
}

DecoratedWindow *DecoratedWindowPrivate::window()
{
    return d->client;
}

QColor DecoratedWindowPrivate::color(ColorGroup group, ColorRole role) const
{
    Q_UNUSED(role)
    Q_UNUSED(group)

    return QColor();
}

DecoratedWindowPrivateV2::DecoratedWindowPrivateV2(DecoratedWindow *client, Decoration *decoration)
    : DecoratedWindowPrivate(client, decoration)
{
}

DecoratedWindowPrivateV3::DecoratedWindowPrivateV3(DecoratedWindow *client, Decoration *decoration)
    : DecoratedWindowPrivateV2(client, decoration)
{
}

DecoratedWindowPrivateV4::DecoratedWindowPrivateV4(DecoratedWindow *client, Decoration *decoration)
    : DecoratedWindowPrivateV3(client, decoration)
{
}
}
