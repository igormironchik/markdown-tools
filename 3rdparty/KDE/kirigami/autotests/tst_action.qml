// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import QtTest
import KirigamiTestUtils

TestCase {
    name: "Action"

    Kirigami.Action {
        id: normalAction
    }

    function test_normal(): void {
        compare(normalAction.text, '');
        compare(normalAction.enabled, true);
    }

    Kirigami.Action {
        id: enabledAction
        fromQAction: ActionData.enabledAction
    }

    function test_enabledAction(): void {
        compare(enabledAction.text, 'Enabled Action');
        compare(enabledAction.enabled, true);
    }

    Kirigami.Action {
        id: disabledAction
        fromQAction: ActionData.disabledAction
    }

    function test_disabledAction(): void {
        compare(disabledAction.text, 'Disabled Action');
        compare(disabledAction.enabled, false);
    }
}
