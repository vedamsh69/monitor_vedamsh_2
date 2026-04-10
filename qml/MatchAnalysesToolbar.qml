// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima Fast DDS Monitor.
//
// eProsima Fast DDS Monitor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eProsima Fast DDS Monitor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eProsima Fast DDS Monitor. If not, see <https://www.gnu.org/licenses/>.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Theme 1.0

Rectangle {
    id: matchAnalysesToolbar
    height: 40
    color: Theme.lightGrey
    border.bottom: 1
    border.color: Theme.grey

    // Signals
    signal toggleShowMismatches()
    signal expandAll()
    signal collapseAll()
    signal toggleLinkSelection()

    // Properties
    property bool showMismatches: false
        property bool linkWithSelection: true

            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 8

                // Toggle Mismatches Button
                ToolButton {
                    id: toggleMismatchesButton
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32

                    background: Rectangle {
                        color: toggleMismatchesButton.hovered ? Theme.eProsimaLightBlue : Theme.lightGrey
                        border.width: matchAnalysesToolbar.showMismatches ? 2 : 0
                        border.color: matchAnalysesToolbar.showMismatches ? Theme.eProsimaDarkBlue : "transparent"
                        radius: 3
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: "Show/Hide Mismatches Only"
                    ToolTip.delay: 500

                    contentItem: Text {
                        text: "✗"
                        color: matchAnalysesToolbar.showMismatches ? Theme.eProsimaDarkBlue : Theme.darkGrey
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        matchAnalysesToolbar.showMismatches = !matchAnalysesToolbar.showMismatches
                        matchAnalysesToolbar.toggleShowMismatches()
                    }
                }

                // Expand All Button
                ToolButton {
                    id: expandAllButton
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32

                    background: Rectangle {
                        color: expandAllButton.hovered ? Theme.eProsimaLightBlue : Theme.lightGrey
                        radius: 3
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: "Expand All"
                    ToolTip.delay: 500

                    contentItem: Text {
                        text: "▼"
                        color: Theme.eProsimaDarkBlue
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: matchAnalysesToolbar.expandAll()
                }

                // Collapse All Button
                ToolButton {
                    id: collapseAllButton
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32

                    background: Rectangle {
                        color: collapseAllButton.hovered ? Theme.eProsimaLightBlue : Theme.lightGrey
                        radius: 3
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: "Collapse All"
                    ToolTip.delay: 500

                    contentItem: Text {
                        text: "▶"
                        color: Theme.eProsimaDarkBlue
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: matchAnalysesToolbar.collapseAll()
                }

                Rectangle {
                    Layout.fillWidth: true
                    color: "transparent"
                }

                // Link with Selection Button
                ToolButton {
                    id: linkSelectionButton
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32

                    background: Rectangle {
                        color: linkSelectionButton.hovered ? Theme.eProsimaLightBlue : Theme.lightGrey
                        border.width: matchAnalysesToolbar.linkWithSelection ? 2 : 0
                        border.color: matchAnalysesToolbar.linkWithSelection ? Theme.eProsimaDarkBlue : "transparent"
                        radius: 3
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: "Link with Selection"
                    ToolTip.delay: 500

                    contentItem: Text {
                        text: "🔗"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        matchAnalysesToolbar.linkWithSelection = !matchAnalysesToolbar.linkWithSelection
                        matchAnalysesToolbar.toggleLinkSelection()
                    }
                }
            }
        }
