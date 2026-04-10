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
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.15
import QtQml.Models 2.11
import Theme 1.0

/**
* @brief Match Analyses View - RTI DDS compatible hierarchical QoS compatibility analyzer
*
* Displays hierarchical analysis of QoS compatibility between DataWriters and DataReaders
* with 4-column layout (Name, Offered, Requested, Notes) and toolbar controls
*/
Item {
    id: matchAnalysesView
    anchors.fill: parent

    // Get the Match Analyses Model from Controller
    readonly property var matchAnalysesModel: controller.matchAnalysesModel

        property bool showOnlyMismatches: false
            property bool linkWithSelection: true

                // Component.onCompleted lifecycle
                Component.onCompleted: {
                    console.log("[MatchAnalysesView] Component completed")
                    if (matchAnalysesModel)
                    {
                        console.log("[MatchAnalysesView] ✓ Model available")
                        treeView.model = matchAnalysesModel
                    } else {
                    console.log("[MatchAnalysesView] ⚠ Model not available yet")
                }
            }

            // Watch for model changes
            Connections {
                target: controller
                onMatchAnalysesModelChanged: {
                    console.log("[MatchAnalysesView] Model changed, rebinding...")
                    treeView.model = controller.matchAnalysesModel
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 0
                spacing: 0

                // ========================
                // TOOLBAR SECTION
                // ========================
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    color: Theme.lightGrey
                    border.width: 1
                    border.color: Theme.grey

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 8

                        // Toggle Mismatches Button
                        Rectangle {
                            id: toggleMismatchesButton
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            color: mouseAreaToggle.containsMouse ? Theme.eProsimaLightBlue : Theme.lightGrey
                            border.width: showOnlyMismatches ? 2 : 0
                            border.color: showOnlyMismatches ? Theme.eProsimaDarkBlue : "transparent"
                            radius: 3

                            Text {
                                anchors.centerIn: parent
                                text: "✗"
                                color: showOnlyMismatches ? Theme.eProsimaDarkBlue : Theme.darkGrey
                                font.pixelSize: 16
                                font.bold: true
                            }

                            MouseArea {
                                id: mouseAreaToggle
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    showOnlyMismatches = !showOnlyMismatches
                                    if (matchAnalysesModel && matchAnalysesModel.set_filter_mismatches_only)
                                    {
                                        matchAnalysesModel.set_filter_mismatches_only(showOnlyMismatches)
                                    }
                                }
                            }
                        }

                        // Expand All Button
                        Rectangle {
                            id: expandAllButton
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            color: mouseAreaExpand.containsMouse ? Theme.eProsimaLightBlue : Theme.lightGrey
                            radius: 3

                            Text {
                                anchors.centerIn: parent
                                text: "▼"
                                color: Theme.eProsimaDarkBlue
                                font.pixelSize: 14
                            }

                            MouseArea {
                                id: mouseAreaExpand
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    for (var i = 0; i < (matchAnalysesModel ? matchAnalysesModel.rowCount() : 0); i++) {
                                        treeView.expand(i)
                                    }
                                }
                            }
                        }

                        // Collapse All Button
                        Rectangle {
                            id: collapseAllButton
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            color: mouseAreaCollapse.containsMouse ? Theme.eProsimaLightBlue : Theme.lightGrey
                            radius: 3

                            Text {
                                anchors.centerIn: parent
                                text: "▶"
                                color: Theme.eProsimaDarkBlue
                                font.pixelSize: 14
                            }

                            MouseArea {
                                id: mouseAreaCollapse
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    for (var i = 0; i < (matchAnalysesModel ? matchAnalysesModel.rowCount() : 0); i++) {
                                        treeView.collapse(i)
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            color: "transparent"
                        }

                        // Link with Selection Button
                        Rectangle {
                            id: linkSelectionButton
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            color: mouseAreaLink.containsMouse ? Theme.eProsimaLightBlue : Theme.lightGrey
                            border.width: linkWithSelection ? 2 : 0
                            border.color: linkWithSelection ? Theme.eProsimaDarkBlue : "transparent"
                            radius: 3

                            Text {
                                anchors.centerIn: parent
                                text: "🔗"
                                font.pixelSize: 16
                            }

                            MouseArea {
                                id: mouseAreaLink
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    linkWithSelection = !linkWithSelection
                                }
                            }
                        }
                    }
                }

                // ========================
                // TREE VIEW SECTION
                // ========================
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ffffff"
                    border.color: Theme.lightGrey
                    border.width: 1

                    TreeView {
                        id: treeView
                        anchors.fill: parent
                        anchors.margins: 0
                        model: matchAnalysesModel
                        selectionMode: SelectionMode.SingleSelection
                        frameVisible: false
                        alternatingRowColors: true

                        // Column 1: Name
                        TableViewColumn {
                            title: "Name"
                            role: "name"
                            width: parent.width * 0.30
                            delegate: Item {
                                height: 32
                                width: parent.width

                                Text {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    verticalAlignment: Text.AlignVCenter
                                    text: styleData.value
                                    elide: Text.ElideRight
                                    color: Theme.darkGrey
                                    font.pixelSize: 12
                                }
                            }
                        }

                        // Column 2: Offered
                        TableViewColumn {
                            title: "Offered"
                            role: "offered"
                            width: parent.width * 0.23
                            delegate: Item {
                                height: 32
                                width: parent.width

                                Text {
                                    anchors.fill: parent
                                    anchors.leftMargin: 5
                                    anchors.rightMargin: 5
                                    verticalAlignment: Text.AlignVCenter
                                    text: styleData.value
                                    elide: Text.ElideRight
                                    color: Theme.darkGrey
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // Column 3: Requested
                        TableViewColumn {
                            title: "Requested"
                            role: "requested"
                            width: parent.width * 0.23
                            delegate: Item {
                                height: 32
                                width: parent.width

                                Text {
                                    anchors.fill: parent
                                    anchors.leftMargin: 5
                                    anchors.rightMargin: 5
                                    verticalAlignment: Text.AlignVCenter
                                    text: styleData.value
                                    elide: Text.ElideRight
                                    color: Theme.darkGrey
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // Column 4: Notes
                        TableViewColumn {
                            title: "Notes"
                            role: "notes"
                            width: parent.width * 0.24
                            delegate: Item {
                                height: 32
                                width: parent.width

                                Rectangle {
                                    anchors.fill: parent
                                    color: {
                                        var is_matched = treeView.model ? treeView.model.data(
                                            treeView.model.index(styleData.row, 0),
                                            treeView.model.IsMatchedRole ? treeView.model.IsMatchedRole : 257
                                        ) : true

                                        var is_parent = treeView.model ? treeView.model.data(
                                            treeView.model.index(styleData.row, 0),
                                            treeView.model.IsParentRole ? treeView.model.IsParentRole : 258
                                        ) : false

                                        if (is_parent)
                                        {
                                            return is_matched ? "#ffffff" : "#ffcccc"
                                        }
                                        if (showOnlyMismatches && is_matched)
                                        {
                                            return "#ffffff"
                                        }
                                        return is_matched ? "#e8f5e9" : "#ffcccc"
                                    }
                                    border.color: {
                                        var is_matched = treeView.model ? treeView.model.data(
                                            treeView.model.index(styleData.row, 0),
                                            treeView.model.IsMatchedRole ? treeView.model.IsMatchedRole : 257
                                        ) : true
                                        return is_matched ? "#c8e6c9" : "#ffb3b3"
                                    }
                                    border.width: 0.5

                                    Text {
                                        anchors.fill: parent
                                        anchors.leftMargin: 8
                                        verticalAlignment: Text.AlignVCenter
                                        text: styleData.value
                                        color: {
                                            var is_matched = treeView.model ? treeView.model.data(
                                                treeView.model.index(styleData.row, 0),
                                                treeView.model.IsMatchedRole ? treeView.model.IsMatchedRole : 257
                                            ) : true
                                            return is_matched ? "#388e3c" : "#d32f2f"
                                        }
                                        font.bold: true
                                        font.pixelSize: 11
                                    }
                                }
                            }
                        }
                    }
                }

                // ========================
                // STATUS BAR SECTION
                // ========================
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24
                    color: Theme.lightGrey
                    border.width: 1
                    border.color: Theme.grey

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 10

                        Text {
                            text: "Ready"
                            color: Theme.darkGrey
                            font.pixelSize: 10
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            color: "transparent"
                        }

                        Text {
                            text: "Match Analyses View"
                            color: Theme.darkGrey
                            font.pixelSize: 10
                            font.italic: true
                        }
                    }
                }
            }
        }


