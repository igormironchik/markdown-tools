/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.AbstractApplicationWindow {
    id: root

    width: 500
    height: 800
    visible: true

    globalDrawer: Kirigami.GlobalDrawer {
        title: "Widget gallery"
        titleIcon: "applications-graphics"
        actions: [
            Kirigami.Action {
                text: "View"
                icon.name: "view-list-icons"
                Kirigami.Action {
                    text: "action 1"
                }
                Kirigami.Action {
                    text: "action 2"
                }
                Kirigami.Action {
                    text: "action 3"
                }
            },
            Kirigami.Action {
                text: "Sync"
                icon.name: "folder-sync"
                Kirigami.Action {
                    text: "action 4"
                }
                Kirigami.Action {
                    text: "action 5"
                }
            },
            Kirigami.Action {
                text: "Checkable"
                icon.name: "view-list-details"
                checkable: true
                checked: false
                onTriggered: {
                    print("Action checked:" + checked)
                }
            },
            Kirigami.Action {
                text: "Settings"
                icon.name: "configure"
                checkable: true
                //Need to do this, otherwise it breaks the bindings
                property bool current: (root.pageStack as QQC2.StackView).currentItem?.objectName === "settingsPage" ?? false
                onCurrentChanged: {
                    checked = current;
                }
                onTriggered: {
                    (root.pageStack as QQC2.StackView).push(settingsComponent);
                }
            }
        ]

        QQC2.CheckBox {
            checked: true
            text: "Option 1"
        }
        QQC2.CheckBox {
            text: "Option 2"
        }
        QQC2.CheckBox {
            text: "Option 3"
        }
        QQC2.Slider {
            Layout.fillWidth: true
            value: 0.5
        }
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack: QQC2.StackView {
        id: stackView
        anchors.fill: parent
        property int currentIndex: 0
        focus: true
        onCurrentIndexChanged: {
            if (depth > currentIndex + 1) {
                pop(get(currentIndex));
            }
        }
        onDepthChanged: {
            currentIndex = depth-1;
        }
        initialItem: mainPageComponent

        Keys.onReleased: event => {
            if (event.key === Qt.Key_Back ||
                    (event.key === Qt.Key_Left && (event.modifiers & Qt.AltModifier))) {
                event.accepted = true;
                if (root.contextDrawer && root.contextDrawer.drawerOpen) {
                    root.contextDrawer.close();
                } else if (root.globalDrawer && root.globalDrawer.drawerOpen) {
                    root.globalDrawer.close();
                } else {
                    var backEvent = {accepted: false}
                    if (stackView.currentIndex >= 1) {
                        (stackView.currentItem as Kirigami.Page).backRequested(backEvent);
                        if (!backEvent.accepted) {
                            if ((root.pageStack as QQC2.StackView).depth > 1) {
                                (root.pageStack as QQC2.StackView).currentIndex = Math.max(0, stackView.currentIndex - 1);
                                backEvent.accepted = true;
                            } else {
                                Qt.quit();
                            }
                        }
                    }

                    if (!backEvent.accepted) {
                        Qt.quit();
                    }
                }
            }
        }
    }

    Component {
        id: settingsComponent
        Kirigami.Page {
            title: "Settings"
            objectName: "settingsPage"
            Rectangle {
                anchors.fill: parent
            }
        }
    }

    //Main app content
    Component {
        id: mainPageComponent
        MultipleColumnsGallery {}
    }
}
