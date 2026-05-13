/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2026 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KXMLGUI_UTILS_H
#define KXMLGUI_UTILS_H

#include <QString>

// Case insensitive equality without calling toLower which allocates a new string
[[nodiscard]] static inline bool equals(QStringView a, QStringView b)
{
    return a.compare(b, Qt::CaseInsensitive) == 0;
}

[[nodiscard]] static inline bool equals(QStringView a, std::string_view b)
{
    return a.compare(QLatin1String(b.data(), b.size()), Qt::CaseInsensitive) == 0;
}

#endif
