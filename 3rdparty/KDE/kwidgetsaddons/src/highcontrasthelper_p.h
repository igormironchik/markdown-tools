/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2025 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef HIGHCONTRASTHELPER_P_H
#define HIGHCONTRASTHELPER_P_H

// Standalone (pure Qt) functionality needed internally in more than
// one source file.

/*!
 * \internal
 *
 * Returns whether a high-contrast color scheme is currently in use.
 *
 * This can be used to avoid painting UI elements with colors that would
 * cause bad contrast. This function returns true if high-contrast is
 * active in the platform and if the default color scheme is in use.
 *
 * \note For Qt 6.9 and earlier high-contrast is only detected on Windows.
 */

bool isHighContrastColorSchemeInUse();

#endif
