/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.kirigami as Kirigami
import QtTest

TestCase {
    id: root

    name: "ScenePositionTest"
    visible: true
    when: windowShown

    width: 300
    height: 300

    Item {
        id: anchorsFillItem
        anchors.fill: parent

        Item {
            id: itemA
            x: 50
            y: 100
        }

        Item {
            id: itemB
            x: 150
            y: 200

            Item {
                id: itemB1
                x: 25
                y: 50
            }
        }

        Item {
            id: itemF
            x: 3.5
            y: 6.25

            Item {
                id: itemF1
                x: -0.25
                y: 1.125
            }
        }

        Item {
            id: itemPlaceholder
            x: 56
            y: 78

            property Item __reparentedItem: Item {
                id: reparentedItem
                parent: null
                x: 12
                y: 34
            }
        }

        Rectangle {
            id: itemTransform

            x: 100
            y: 200
            width: 10
            height: 10
            color: "red"

            scale: 2.0
            rotation: 30
            transform: [
                Rotation {
                    angle: 30
                },
                Scale {
                    xScale: 2.0
                    yScale: 3.0
                },
                Translate {
                    x: 12
                    y: 34
                },
                Matrix4x4 {
                    property real a: Math.PI / 4
                    matrix: Qt.matrix4x4(Math.cos(a), -Math.sin(a), 0, 0,
                                         Math.sin(a),  Math.cos(a), 0, 0,
                                         0,           0,            1, 0,
                                         0,           0,            0, 1)
                }
            ]
        }
    }

    function test_root() {
        compare(root.Kirigami.ScenePosition.x, 0);
        compare(root.Kirigami.ScenePosition.y, 0);

        compare(anchorsFillItem.Kirigami.ScenePosition.x, 0);
        compare(anchorsFillItem.Kirigami.ScenePosition.y, 0);
    }

    function test_not_nested() {
        compare(itemA.Kirigami.ScenePosition.x, itemA.x);
        compare(itemA.Kirigami.ScenePosition.y, itemA.y);

        compare(itemB.Kirigami.ScenePosition.x, itemB.x);
        compare(itemB.Kirigami.ScenePosition.y, itemB.y);
    }

    function test_nested() {
        compare(itemB1.Kirigami.ScenePosition.x, itemB.x + itemB1.x);
        compare(itemB1.Kirigami.ScenePosition.y, itemB.y + itemB1.y);
    }

    function test_floating() {
        compare(itemF1.Kirigami.ScenePosition.x, 3.25);
        compare(itemF1.Kirigami.ScenePosition.y, 7.375);
    }

    function test_reparented() {
        reparentedItem.parent = null;
        compare(reparentedItem.Kirigami.ScenePosition.x, reparentedItem.x);
        compare(reparentedItem.Kirigami.ScenePosition.y, reparentedItem.y);

        itemPlaceholder.x = 56;
        itemPlaceholder.y = 78;
        reparentedItem.parent = itemPlaceholder;
        compare(reparentedItem.Kirigami.ScenePosition.x, itemPlaceholder.x + reparentedItem.x);
        compare(reparentedItem.Kirigami.ScenePosition.y, itemPlaceholder.y + reparentedItem.y);

        itemPlaceholder.x += 10;
        itemPlaceholder.y += 20;
        compare(reparentedItem.Kirigami.ScenePosition.x, itemPlaceholder.x + reparentedItem.x);
        compare(reparentedItem.Kirigami.ScenePosition.y, itemPlaceholder.y + reparentedItem.y);
    }

    function test_transform() {
        // transformations are not supported by ScenePosition
        compare(itemTransform.Kirigami.ScenePosition.x, itemTransform.x);
        compare(itemTransform.Kirigami.ScenePosition.y, itemTransform.y);
    }

    Item {
        id: oldParent
        Item {
            id: reparentedChildItem
        }
    }
    Item {
        id: newParent
    }

    SignalSpy {
        id: reparentingXSpy
        target: reparentedChildItem.Kirigami.ScenePosition
        signalName: "xChanged"
    }
    SignalSpy {
        id: reparentingYSpy
        target: reparentedChildItem.Kirigami.ScenePosition
        signalName: "yChanged"
    }

    function test_disconnectOldAncestors() {
        verify(reparentingXSpy.valid);
        verify(reparentingYSpy.valid);
        reparentingXSpy.clear();
        reparentingYSpy.clear();

        // change position of the item itself
        reparentedChildItem.x = 12;
        compare(reparentingXSpy.count, 1);
        compare(reparentingYSpy.count, 0);
        reparentedChildItem.y = 34;
        compare(reparentingXSpy.count, 1);
        compare(reparentingYSpy.count, 1);

        // change position of the current parent
        oldParent.x = 56;
        compare(reparentedChildItem.Kirigami.ScenePosition.x, 12 + 56);
        compare(reparentingXSpy.count, 2);
        compare(reparentingYSpy.count, 1);
        oldParent.y = 78;
        compare(reparentedChildItem.Kirigami.ScenePosition.y, 34 + 78);
        compare(reparentingXSpy.count, 2);
        compare(reparentingYSpy.count, 2);

        // reparent
        reparentedChildItem.parent = newParent;
        compare(reparentingXSpy.count, 3);
        compare(reparentingYSpy.count, 3);
        compare(reparentedChildItem.Kirigami.ScenePosition.x, 12);
        compare(reparentedChildItem.Kirigami.ScenePosition.y, 34);

        // change position of the new parent
        newParent.x = 11;
        compare(reparentedChildItem.Kirigami.ScenePosition.x, 12 + 11);
        compare(reparentingXSpy.count, 4);
        compare(reparentingYSpy.count, 3);
        newParent.y = 22;
        compare(reparentedChildItem.Kirigami.ScenePosition.y, 34 + 22);
        compare(reparentingXSpy.count, 4);
        compare(reparentingYSpy.count, 4);

        // change position of the item itself (again, but with new parent)
        reparentedChildItem.x = 33;
        compare(reparentedChildItem.Kirigami.ScenePosition.x, 33 + 11);
        compare(reparentingXSpy.count, 5);
        compare(reparentingYSpy.count, 4);
        reparentedChildItem.y = 44;
        compare(reparentedChildItem.Kirigami.ScenePosition.y, 44 + 22);
        compare(reparentingXSpy.count, 5);
        compare(reparentingYSpy.count, 5);

        // change position of the old parent, should not trigger signals of the item anymore
        oldParent.x = 55;
        compare(reparentedChildItem.Kirigami.ScenePosition.x, 33 + 11);
        compare(reparentingXSpy.count, 5);
        compare(reparentingYSpy.count, 5);
        oldParent.y = 66;
        compare(reparentedChildItem.Kirigami.ScenePosition.y, 44 + 22);
        compare(reparentingXSpy.count, 5);
        compare(reparentingYSpy.count, 5);
    }
}
