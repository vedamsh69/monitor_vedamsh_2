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

Popup {
    id: matchAnalysesDialog
    width: 500
    height: 400
    anchors.centerIn: Overlay.overlay
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property var selectedEntities: []
    property var availableEntities: [
        { name: "DataWriter_1", kind: "DataWriter", id: 1 },
        { name: "DataReader_1", kind: "DataReader", id: 2 },
        { name: "DataWriter_2", kind: "DataWriter", id: 3 },
        { name: "DomainParticipant_1", kind: "DomainParticipant", id: 4 }
    ]

    background: Rectangle {
        color: "#ffffff"
        border.color: Theme.lightGrey
        border.width: 1
        radius: 5
    }

    function openMatchAnalysesView(entities) {
        var component = Qt.createComponent("MatchAnalysesView.qml")
        if (component.status === Component.Ready) {
            var window = component.createObject(null, {
                selectedEntity: entities[0]
            })
            if (window !== null) {
                window.show()
                matchAnalysesDialog.close()
            } else {
                console.error("Error creating Match Analyses View window")
            }
        } else if (component.status === Component.Error) {
            console.error("Error loading MatchAnalysesView:", component.errorString())
        }
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        Text {
            text: qsTr("Match Analyses View - Select Entities")
            font.bold: true
            font.pixelSize: 12
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            border.color: Theme.lightGrey
            border.width: 1
            radius: 3

            ListView {
                id: entitiesList
                anchors.fill: parent
                anchors.margins: 5
                model: matchAnalysesDialog.availableEntities
                spacing: 5
                clip: true

                delegate: Item {
                    width: entitiesList.width
                    height: 35

                    CheckBox {
                        id: entityCheckBox
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.name + " (" + modelData.kind + ")"
                        checked: matchAnalysesDialog.selectedEntities.includes(modelData.name)

                        onCheckedChanged: {
                            if (checked) {
                                if (!matchAnalysesDialog.selectedEntities.includes(modelData.name)) {
                                    matchAnalysesDialog.selectedEntities.push(modelData.name)
                                }
                            } else {
                                var index = matchAnalysesDialog.selectedEntities.indexOf(modelData.name)
                                if (index > -1) {
                                    matchAnalysesDialog.selectedEntities.splice(index, 1)
                                }
                            }
                        }
                    }
                }
            }
        }

        Text {
            text: "Selected: " + matchAnalysesDialog.selectedEntities.length + " entity/entities"
            font.pixelSize: 11
            color: Theme.darkGrey
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: qsTr("Select All")
                Layout.preferredWidth: 100
                onClicked: {
                    matchAnalysesDialog.selectedEntities = []
                    for (var i = 0; i < matchAnalysesDialog.availableEntities.length; i++) {
                        matchAnalysesDialog.selectedEntities.push(matchAnalysesDialog.availableEntities[i].name)
                    }
                }
            }

            Button {
                text: qsTr("Clear All")
                Layout.preferredWidth: 100
                onClicked: {
                    matchAnalysesDialog.selectedEntities = []
                }
            }

            Rectangle {
                Layout.fillWidth: true
                color: "transparent"
            }

            Button {
                text: qsTr("Open View")
                Layout.preferredWidth: 100
                onClicked: {
                    if (selectedEntities.length > 0) {
                        openMatchAnalysesView(selectedEntities)
                    }
                }
            }

            Button {
                text: qsTr("Cancel")
                Layout.preferredWidth: 80
                onClicked: matchAnalysesDialog.close()
            }
        }
    }
}
