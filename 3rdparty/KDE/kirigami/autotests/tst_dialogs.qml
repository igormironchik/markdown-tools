/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import QtTest

TestCase {
    name: "KirigamiDialogsTest"
    visible: true
    when: windowShown

    width: 500
    height: 500

    Component {
        id: dialogComponent
        Kirigami.Dialog {
            id: dialog

            readonly property SignalSpy acceptedSpy: SignalSpy {
                target: dialog
                signalName: "accepted"
            }
            readonly property SignalSpy rejectedSpy: SignalSpy {
                target: dialog
                signalName: "rejected"
            }
            readonly property Kirigami.Action kActionA: Kirigami.Action {
                text: "Kirigami Action A"
                property int count: 0
                onTriggered: count += 1;
            }
            readonly property Kirigami.Action kActionB: Kirigami.Action {
                text: "Kirigami Action B"
                visible: false
                property int count: 0
                onTriggered: count += 1;
            }
            readonly property Kirigami.Action kActionC: Kirigami.Action {
                text: "Kirigami Action C"
                property int count: 0
                onTriggered: count += 1;
            }

            title: "Dialog"
            preferredWidth: 400
            customFooterActions: [kActionA, kActionB, kActionC]
        }
    }

    function test_footer_buttons() {
        const dialog = createTemporaryObject(dialogComponent, this, {
            standardButtons: T.Dialog.Ok | T.Dialog.Cancel,
        });
        verify(dialog);
        const { kActionA, kActionB, kActionC, acceptedSpy, rejectedSpy } = dialog;
        verify(acceptedSpy.valid);
        verify(rejectedSpy.valid);

        dialog.open();
        tryCompare(dialog, "opened", true, Kirigami.Units.longDuration * 2);

        const buttonOk = dialog.standardButton(T.Dialog.Ok);
        verify(buttonOk);
        mouseClick(buttonOk);
        compare(acceptedSpy.count, 1);

        dialog.open();
        tryCompare(dialog, "opened", true, Kirigami.Units.longDuration * 2);

        const buttonCancel = dialog.standardButton(T.Dialog.Cancel);
        verify(buttonCancel);
        mouseClick(buttonCancel);
        compare(rejectedSpy.count, 1);

        dialog.open();
        tryCompare(dialog, "opened", true, Kirigami.Units.longDuration * 2);

        const buttonA = dialog.customFooterButton(kActionA);
        verify(buttonA);
        mouseClick(buttonA);
        compare(kActionA.count, 1);

        const buttonB = dialog.customFooterButton(kActionB);
        verify(!buttonB);

        const buttonC = dialog.customFooterButton(kActionC);
        verify(buttonC);
        mouseClick(buttonC);
        compare(kActionC.count, 1);
    }

    Component {
        id: nullActionDialogComponent
        Kirigami.Dialog {
            id: dialog

            readonly property SignalSpy acceptedSpy: SignalSpy {
                target: dialog
                signalName: "accepted"
            }
            readonly property SignalSpy rejectedSpy: SignalSpy {
                target: dialog
                signalName: "rejected"
            }

            title: "Dialog"
            preferredWidth: 400
            visible: true
        }
    }

    function test_null_footer_action() {
        const dialog = createTemporaryObject(nullActionDialogComponent, this);
        verify(dialog);

        compare(dialog.customFooterActions.length, 0);

        let button;

        button = dialog.customFooterButton(null);
        verify(!button);

        dialog.customFooterActions = [null];

        button = dialog.customFooterButton(null);
        verify(!button);
    }
}
