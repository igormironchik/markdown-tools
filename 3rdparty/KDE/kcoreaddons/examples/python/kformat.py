#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

from KCoreAddons import KFormat

formatter = KFormat()

print(formatter.formatByteSize(1234567))

print(formatter.formatDecimalDuration(54353534))

print(formatter.formatDistance(3.41, KFormat.MetricDistanceUnits))

print(formatter.formatDuration(1234567, KFormat.AbbreviatedDuration))

print(formatter.formatSpelloutDuration(1234567))
