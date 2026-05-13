/*
 *  SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: separator
    anchors.fill: parent

    readonly property bool isSeparator: true
    property Item previousColumn
    property Item column
    property Item nextColumn
    readonly property bool inToolBar: parent !== column

    visible: (column.Kirigami.ColumnView.view?.separatorVisible ?? false) && (column.Kirigami.ColumnView.view?.columnResizeMode !== Kirigami.ColumnView.SingleColumn ?? false)

    SeparatorHandle {
        id: leftHandle
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        leadingColumn: LayoutMirroring.enabled ? separator.column : separator.previousColumn
        trailingColumn: LayoutMirroring.enabled ? separator.nextColumn : separator.column
        // If this handle touched the left ColumnView edge, hide it
        visible: leadingColumn && trailingColumn && separator.column.Kirigami.ColumnView.view?.leadingVisibleItem !== separator.column
    }

    SeparatorHandle {
        id: rightHandle
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            rightMargin: -1
        }
        leadingColumn: LayoutMirroring.enabled ? separator.previousColumn : separator.column
        trailingColumn: LayoutMirroring.enabled ? separator.column : separator.nextColumn
        visible: leadingColumn && trailingColumn && separator.column.Kirigami.ColumnView.view?.leadingVisibleItem !== separator.previousColumn
    }

    component SeparatorHandle: Kirigami.Separator {
        id: handle

        property Item leadingColumn
        property Item trailingColumn

        Kirigami.Theme.colorSet: Kirigami.Theme.Header
        Kirigami.Theme.inherit: false

        visible: leadingColumn && trailingColumn

        MouseArea {
            anchors {
                fill: parent
                leftMargin: -Kirigami.Units.smallSpacing
                rightMargin: -Kirigami.Units.smallSpacing
            }

            visible: {
                if (!separator.column.Kirigami.ColumnView.view?.columnResizeMode === Kirigami.ColumnView.DynamicColumns ?? false) {
                    return false;
                }
                if (handle.leadingColumn?.Kirigami.ColumnView.interactiveResizeEnabled ?? false) {
                    return true;
                }
                if (handle.trailingColumn?.Kirigami.ColumnView.interactiveResizeEnabled ?? false) {
                    return true;
                }
                return false;
            }
            cursorShape: Qt.SplitHCursor
            onPressed: mouse => {
                if (handle.leadingColumn) {
                    handle.leadingColumn.Kirigami.ColumnView.interactiveResizing = true;
                }
                if (handle.trailingColumn) {
                    handle.trailingColumn.Kirigami.ColumnView.interactiveResizing = true;
                }
            }
            onReleased: {
                if (handle.leadingColumn) {
                    handle.leadingColumn.Kirigami.ColumnView.interactiveResizing = false;
                }
                if (handle.trailingColumn) {
                    handle.trailingColumn.Kirigami.ColumnView.interactiveResizing = false;
                }
            }
            onCanceled: {
                if (handle.leadingColumn) {
                    handle.leadingColumn.Kirigami.ColumnView.interactiveResizing = false;
                }
                if (handle.trailingColumn) {
                    handle.trailingColumn.Kirigami.ColumnView.interactiveResizing = false;
                }
            }
            onPositionChanged: mouse => {
                const newX = mapToItem(null, mouse.x, 0).x;
                const leadingX = handle.leadingColumn?.mapToItem(null, 0, 0).x ?? 0;
                const trailingX = handle.trailingColumn?.mapToItem(null, 0, 0).x ?? 0;
                const view = separator.column.Kirigami.ColumnView.view;

                let leadingWidth = handle.leadingColumn.width;
                let trailingWidth = handle.trailingColumn.width;

                // Minimum and maximum widths for the leading column
                let leadingMinimumWidth = handle.leadingColumn.Kirigami.ColumnView.minimumWidth;
                if (handle.leadingColumn.Kirigami.ColumnView.fillWidth) {
                    leadingMinimumWidth = view.columnWidth;
                }
                if (leadingMinimumWidth < 0) {
                    leadingMinimumWidth = Kirigami.Units.gridUnit * 8;
                }

                // Minimum and maximum widths for the trailing column
                let trailingMinimumWidth = handle.trailingColumn.Kirigami.ColumnView.minimumWidth;
                if (handle.trailingColumn.Kirigami.ColumnView.fillWidth) {
                    trailingMinimumWidth = view.columnWidth;
                }
                if (trailingMinimumWidth < 0) {
                    trailingMinimumWidth = Kirigami.Units.gridUnit * 8;
                }

                let leadingMaximumWidth = handle.leadingColumn.Kirigami.ColumnView.maximumWidth;
                if (leadingMaximumWidth < 0) {
                    leadingMaximumWidth = leadingWidth + trailingWidth - trailingMinimumWidth;
                }


                let trailingMaximumWidth = handle.trailingColumn.Kirigami.ColumnView.maximumWidth;
                if (trailingMaximumWidth < 0) {
                    trailingMaximumWidth = leadingWidth + trailingWidth - leadingMinimumWidth;
                }


                if (!handle.leadingColumn.Kirigami.ColumnView.fillWidth) {
                    handle.leadingColumn.Kirigami.ColumnView.preferredWidth = Math.max(leadingMinimumWidth,
                                                        Math.min(leadingMaximumWidth,
                                                                 newX - leadingX));
                }
                if (!handle.trailingColumn.Kirigami.ColumnView.fillWidth) {
                    handle.trailingColumn.Kirigami.ColumnView.preferredWidth = Math.max(trailingMinimumWidth,
                                                        Math.min(trailingMaximumWidth,
                                                                 trailingX + handle.trailingColumn.width - newX));
                }
            }
        }
    }
}
