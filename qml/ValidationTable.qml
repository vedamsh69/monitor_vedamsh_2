import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    anchors.fill: parent

    Rectangle {
        id: mainRectangle
        color: "#F2F2F2"
        anchors.fill: parent

        property int selectedRow: -1

            ListModel {
                id: validatorsModel
                ListElement { name: "XML Schema Validator"; manual: true; build: true }
                ListElement { name: "XML Validator"; manual: true; build: true }
            }

            // ── Header ─────────────────────────────────────────────────────
            Rectangle {
                id: upperRectangle
                width: parent.width
                height: 60
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                border.color: "lightgray"
                color: "#F2F2F2"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Text { text: "Validation"; color: "darkgray"; bottomPadding: 15 }
                    Item { Layout.fillWidth: true }

                    Row {
                        spacing: 15
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        Image { source: "qrc:/resources/images/icons/refresh_arrow.png"; width: 20; height: 20; MouseArea { anchors.fill: parent } }
                        Image { source: "qrc:/resources/images/icons/left_arrow.png"; width: 20; height: 20; MouseArea { anchors.fill: parent } }
                        Image { source: "qrc:/resources/images/icons/down_arrow.png"; width: 20; height: 20; MouseArea { anchors.fill: parent } }
                        Image { source: "qrc:/resources/images/icons/dots.png"; width: 20; height: 20; MouseArea { anchors.fill: parent } }
                    }
                }
            }

            // ── Bottom Bar — declared BEFORE ScrollView ────────────────────
            Rectangle {
                id: bottomBar
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 60
                color: "white"
                border.color: "#E8E9EB"; border.width: 1

                RowLayout {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: 16; spacing: 8

                    Button {
                        id: restoreDefaultsBtn
                        text: "Restore Defaults"
                        implicitWidth: 130; implicitHeight: 35
                        background: Rectangle { radius: 5; color: "white"; border.color: "darkgray"; border.width: 1 }
                        contentItem: Text {
                            text: restoreDefaultsBtn.text; color: "black"
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            allowOverrideCheck.checked = true
                            suspendAllCheck.checked = false
                            saveModifiedCheck.checked = false
                            showConfirmationCheck.checked = true
                            for (var i = 0; i < validatorsModel.count; i++) {
                                validatorsModel.setProperty(i, "manual", true)
                                validatorsModel.setProperty(i, "build", true)
                            }
                            mainRectangle.selectedRow = -1
                        }
                    }

                    Button {
                        id: applyBtn
                        text: "Apply"
                        implicitWidth: 80; implicitHeight: 35
                        background: Rectangle { radius: 5; color: "white"; border.color: "darkgray"; border.width: 1 }
                        contentItem: Text {
                            text: applyBtn.text; color: "black"
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {}
                    }
                }
            }

            // ── ScrollView ─────────────────────────────────────────────────
            ScrollView {
                id: scrollView
                anchors.top: upperRectangle.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: bottomBar.top
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                Item {
                    id: scrollContent
                    width: scrollView.width
                    height: mainCol.implicitHeight + 40

                    ColumnLayout {
                        id: mainCol
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 16
                        anchors.topMargin: 14
                        spacing: 10

                        // ── Checkbox 1: Allow override (pre-checked) ───────
                        CheckBox {
                            id: allowOverrideCheck
                            Layout.fillWidth: true
                            checked: true
                            indicator: Rectangle {
                                implicitWidth: 16; implicitHeight: 16
                                x: allowOverrideCheck.leftPadding
                                y: allowOverrideCheck.height / 2 - height / 2
                                border.color: allowOverrideCheck.checked ? "#DA3450" : "#34495e"
                                border.width: 2; radius: 2
                                color: allowOverrideCheck.checked ? "#DA3450" : "white"
                                Text { anchors.centerIn: parent; text: allowOverrideCheck.checked ? "✓" : ""; color: "white"; font.bold: true; font.pixelSize: 11 }
                            }
                            contentItem: Text {
                                text: "Allow projects to override these preference settings"
                                leftPadding: allowOverrideCheck.indicator.width + allowOverrideCheck.spacing + 4
                                verticalAlignment: Text.AlignVCenter; color: "black"; font.pixelSize: 12
                            }
                        }

                        // ── Checkbox 2: Suspend all validators (unchecked) ─
                        CheckBox {
                            id: suspendAllCheck
                            Layout.fillWidth: true
                            checked: false
                            indicator: Rectangle {
                                implicitWidth: 16; implicitHeight: 16
                                x: suspendAllCheck.leftPadding
                                y: suspendAllCheck.height / 2 - height / 2
                                border.color: suspendAllCheck.checked ? "#DA3450" : "#34495e"
                                border.width: 2; radius: 2
                                color: suspendAllCheck.checked ? "#DA3450" : "white"
                                Text { anchors.centerIn: parent; text: suspendAllCheck.checked ? "✓" : ""; color: "white"; font.bold: true; font.pixelSize: 11 }
                            }
                            contentItem: Text {
                                text: "Suspend all validators"
                                leftPadding: suspendAllCheck.indicator.width + suspendAllCheck.spacing + 4
                                verticalAlignment: Text.AlignVCenter; color: "black"; font.pixelSize: 12
                            }
                        }

                        // ── Checkbox 3: Save modified (unchecked) ──────────
                        CheckBox {
                            id: saveModifiedCheck
                            Layout.fillWidth: true
                            checked: false
                            indicator: Rectangle {
                                implicitWidth: 16; implicitHeight: 16
                                x: saveModifiedCheck.leftPadding
                                y: saveModifiedCheck.height / 2 - height / 2
                                border.color: saveModifiedCheck.checked ? "#DA3450" : "#34495e"
                                border.width: 2; radius: 2
                                color: saveModifiedCheck.checked ? "#DA3450" : "white"
                                Text { anchors.centerIn: parent; text: saveModifiedCheck.checked ? "✓" : ""; color: "white"; font.bold: true; font.pixelSize: 11 }
                            }
                            contentItem: Text {
                                text: "Save all modified resources automatically prior to validating"
                                leftPadding: saveModifiedCheck.indicator.width + saveModifiedCheck.spacing + 4
                                verticalAlignment: Text.AlignVCenter; color: "black"; font.pixelSize: 12
                            }
                        }

                        // ── Checkbox 4: Show confirmation (pre-checked) ────
                        CheckBox {
                            id: showConfirmationCheck
                            Layout.fillWidth: true
                            checked: true
                            indicator: Rectangle {
                                implicitWidth: 16; implicitHeight: 16
                                x: showConfirmationCheck.leftPadding
                                y: showConfirmationCheck.height / 2 - height / 2
                                border.color: showConfirmationCheck.checked ? "#DA3450" : "#34495e"
                                border.width: 2; radius: 2
                                color: showConfirmationCheck.checked ? "#DA3450" : "white"
                                Text { anchors.centerIn: parent; text: showConfirmationCheck.checked ? "✓" : ""; color: "white"; font.bold: true; font.pixelSize: 11 }
                            }
                            contentItem: Text {
                                text: "Show a confirmation dialog when performing manual validations"
                                leftPadding: showConfirmationCheck.indicator.width + showConfirmationCheck.spacing + 4
                                verticalAlignment: Text.AlignVCenter; color: "black"; font.pixelSize: 12
                            }
                        }

                        // ── Info text ──────────────────────────────────────
                        Text {
                            Layout.fillWidth: true
                            text: "The selected validators will run when validation is performed:"
                            color: "black"; font.pixelSize: 12; wrapMode: Text.WordWrap
                        }
                        // ── Validator Table ────────────────────────────────
                        Rectangle {
                            Layout.fillWidth: true
                            height: 450
                            border.color: "darkgray"; border.width: 1
                            color: "white"; radius: 2; clip: true

                            // Table header
                            Rectangle {
                                id: tblHeader
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 30; color: "#F2F2F2"
                                border.color: "lightgray"; border.width: 1

                                Row {
                                    anchors.fill: parent; anchors.leftMargin: 8
                                    Text { width: parent.width*0.45; height: 30; text: "Validator"; font.pixelSize: 12; color: "black"; verticalAlignment: Text.AlignVCenter }
                                    Text { width: parent.width*0.14; height: 30; text: "Manual"; font.pixelSize: 12; color: "black"; verticalAlignment: Text.AlignVCenter; horizontalAlignment: Text.AlignHCenter }
                                    Text { width: parent.width*0.14; height: 30; text: "Build"; font.pixelSize: 12; color: "black"; verticalAlignment: Text.AlignVCenter; horizontalAlignment: Text.AlignHCenter }
                                    Text { width: parent.width*0.27; height: 30; text: "Settings"; font.pixelSize: 12; color: "black"; verticalAlignment: Text.AlignVCenter; horizontalAlignment: Text.AlignHCenter }
                                }
                            }

                            // Table rows
                            Column {
                                anchors.top: tblHeader.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right

                                Repeater {
                                    model: validatorsModel

                                    Rectangle {
                                        width: parent.width; height: 30
                                        color: mainRectangle.selectedRow === index ? "#DA3450" : "white"

                                        Rectangle {
                                            anchors.bottom: parent.bottom
                                            anchors.left: parent.left; anchors.right: parent.right
                                            height: 1; color: "lightgray"
                                        }

                                        Row {
                                            anchors.fill: parent; anchors.leftMargin: 8

                                            Text {
                                                width: parent.width*0.45; height: 30
                                                text: model.name; font.pixelSize: 12
                                                color: mainRectangle.selectedRow === index ? "white" : "black"
                                                verticalAlignment: Text.AlignVCenter
                                            }

                                            Item {
                                                width: parent.width*0.14; height: 30
                                                Rectangle {
                                                    anchors.centerIn: parent
                                                    width: 16; height: 16; radius: 2
                                                    border.color: suspendAllCheck.checked ? "lightgray" : (model.manual ? "#1565C0" : "#34495e")
                                                    border.width: 1
                                                    color: suspendAllCheck.checked ? "#F0F0F0" : (model.manual ? "#1565C0" : "white")
                                                    Text { anchors.centerIn: parent; text: model.manual ? "✓" : ""; color: suspendAllCheck.checked ? "gray" : "white"; font.bold: true; font.pixelSize: 11 }
                                                    MouseArea { anchors.fill: parent; enabled: !suspendAllCheck.checked; onClicked: validatorsModel.setProperty(index, "manual", !model.manual) }
                                                }
                                            }

                                            Item {
                                                width: parent.width*0.14; height: 30
                                                Rectangle {
                                                    anchors.centerIn: parent
                                                    width: 16; height: 16; radius: 2
                                                    border.color: suspendAllCheck.checked ? "lightgray" : (model.build ? "#1565C0" : "#34495e")
                                                    border.width: 1
                                                    color: suspendAllCheck.checked ? "#F0F0F0" : (model.build ? "#1565C0" : "white")
                                                    Text { anchors.centerIn: parent; text: model.build ? "✓" : ""; color: suspendAllCheck.checked ? "gray" : "white"; font.bold: true; font.pixelSize: 11 }
                                                    MouseArea { anchors.fill: parent; enabled: !suspendAllCheck.checked; onClicked: validatorsModel.setProperty(index, "build", !model.build) }
                                                }
                                            }

                                            Item {
                                                width: parent.width*0.27; height: 30
                                                Rectangle {
                                                    anchors.centerIn: parent
                                                    width: 26; height: 22; radius: 3
                                                    color: "white"; border.color: "#C8A000"; border.width: 1
                                                    Text { anchors.centerIn: parent; text: "···"; color: "#C8A000"; font.pixelSize: 12; font.bold: true }
                                                    MouseArea { anchors.fill: parent; onClicked: {} }
                                                }
                                            }
                                        }

                                        MouseArea { anchors.fill: parent; z: -1; onClicked: mainRectangle.selectedRow = (mainRectangle.selectedRow === index) ? -1 : index }
                                    }
                                }
                            }

                            // Description TextField anchored to bottom of table
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 36
                                border.color: "lightgray"; border.width: 1
                                color: "white"

                                TextField {
                                    anchors.fill: parent; anchors.margins: 4
                                    font.pixelSize: 12; color: "black"
                                    background: Rectangle { color: "transparent" }
                                }
                            }
                        }

                        // ── Description area below table ───────────────────
                        Rectangle {
                            Layout.fillWidth: true; height: 36
                            border.color: "darkgray"; border.width: 1
                            color: "white"; radius: 2
                        }

                        // ── Enable All / Disable All ───────────────────────
                        RowLayout {
                            Layout.fillWidth: true; spacing: 8

                            Button {
                                id: enableAllBtn; text: "Enable All"
                                implicitWidth: 90; implicitHeight: 30
                                // Grayed when Suspend All is checked
                                enabled: !suspendAllCheck.checked
                                background: Rectangle {
                                    radius: 4
                                    color: suspendAllCheck.checked ? "#F0F0F0" : "white"
                                    border.color: suspendAllCheck.checked ? "lightgray" : "darkgray"
                                    border.width: 1
                                }
                                contentItem: Text {
                                    text: enableAllBtn.text
                                    color: suspendAllCheck.checked ? "gray" : "black"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 12
                                }
                                // Check all Manual + Build
                                onClicked: {
                                    for (var i = 0; i < validatorsModel.count; i++) {
                                        validatorsModel.setProperty(i, "manual", true)
                                        validatorsModel.setProperty(i, "build", true)
                                    }
                                }
                            }

                            Button {
                                id: disableAllBtn; text: "Disable All"
                                implicitWidth: 90; implicitHeight: 30
                                enabled: !suspendAllCheck.checked
                                background: Rectangle {
                                    radius: 4
                                    color: suspendAllCheck.checked ? "#F0F0F0" : "white"
                                    border.color: suspendAllCheck.checked ? "lightgray" : "darkgray"
                                    border.width: 1
                                }
                                contentItem: Text {
                                    text: disableAllBtn.text
                                    color: suspendAllCheck.checked ? "gray" : "black"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.pixelSize: 12
                                }
                                // Uncheck all Manual + Build
                                onClicked: {
                                    for (var i = 0; i < validatorsModel.count; i++) {
                                        validatorsModel.setProperty(i, "manual", false)
                                        validatorsModel.setProperty(i, "build", false)
                                    }
                                }
                            }

                            Item { Layout.fillWidth: true }
                        }

                        Item { height: 10 }
                    }
                }
            }
        }
    }