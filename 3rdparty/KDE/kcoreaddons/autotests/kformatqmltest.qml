/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQml
import QtTest 1.2
import org.kde.coreaddons as KCoreAddons

TestCase {
    name: "KFormatQml"

    function test_formatTime() {
        if (_obj.systemTimeZone !== "Asia/Kolkata") {
            skip("timezone change not supported on this platform");
        }
        compare(KCoreAddons.Format.formatTime(_obj, "euTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "12:34 CEST");
        compare(KCoreAddons.Format.formatTime(_obj, "indiaTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "12:34");
        compare(KCoreAddons.Format.formatTime(_obj, "utcTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviationIfNeeded), "12:34 UTC");

        compare(KCoreAddons.Format.formatTime(_obj, "indiaTime", Locale.ShortFormat, KCoreAddons.FormatTypes.AddTimezoneAbbreviation), "12:34 IST");
        compare(KCoreAddons.Format.formatTime(_obj, "utcTime", Locale.ShortFormat), "12:34");
        compare(KCoreAddons.Format.formatTime(_obj, "euTime"), "12:34");

        compare(KCoreAddons.Format.formatTime(_obj, "invalid", Locale.ShortFormat), "");
        compare(KCoreAddons.Format.formatTime(null, "invalid", Locale.ShortFormat), "");
    }
}
