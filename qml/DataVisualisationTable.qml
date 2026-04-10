import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    anchors.fill: parent

    Rectangle {
        id: mainRectangle
        color: "#F2F2F2"
        anchors.fill: parent

        // ─── Header Bar ───────────────────────────────────────────────
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

                Text {
                    text: "Data Visualization General Preferences"
                    color: "darkgray"
                    bottomPadding: 15
                }

                Item { Layout.fillWidth: true }

                Row {
                    spacing: 15
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                    Image {
                        source: "qrc:/resources/images/icons/refresh_arrow.png"
                        width: 20
                        height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/left_arrow.png"
                        width: 20
                        height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/down_arrow.png"
                        width: 20
                        height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/dots.png"
                        width: 20
                        height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                }
            }
        }

        // ─── Content Area ─────────────────────────────────────────────
        Rectangle {
            id: lowerRectangle
            width: parent.width
            height: parent.height - upperRectangle.height
            anchors.top: upperRectangle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            border.color: "lightgray"
            color: "#F2F2F2"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 14

                // ── Checkbox 1: Enable in-memory caching ───────────────
                CheckBox {
                    id: cachingCheckBox
                    Layout.fillWidth: true
                    checked: true

                    indicator: Rectangle {
                        implicitWidth: 16
                        implicitHeight: 16
                        x: cachingCheckBox.leftPadding
                        y: parent.height / 2 - height / 2
                        border.color: "#34495e"
                        border.width: 1
                        color: "white"
                        radius: 2

                        Text {
                            anchors.centerIn: parent
                            text: cachingCheckBox.checked ? "✓" : ""
                            color: "#34495e"
                            font.bold: true
                            font.pixelSize: 11
                        }
                    }

                    contentItem: Text {
                        text: "Enable in-memory caching for less CPU usage and higher throughput."
                        leftPadding: cachingCheckBox.indicator.width + cachingCheckBox.spacing + 4
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                        color: "black"
                    }
                }

                // ── Row: Maximum UI updating rate ──────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: "Maximum UI updating rate (Hz)"
                        verticalAlignment: Text.AlignVCenter
                        color: "black"
                    }

                    TextField {
                        id: uiUpdateRateField
                        Layout.fillWidth: true
                        height: 28
                        text: "20"
                        leftPadding: 8
                        verticalAlignment: TextInput.AlignVCenter
                        validator: IntValidator { bottom: 1; top: 9999 }

                        background: Rectangle {
                            border.color: uiUpdateRateField.activeFocus ? "#DA3450" : "darkgray"
                            border.width: 1
                            color: "white"
                            radius: 2
                        }
                    }
                }

                // ── Row: Maximum default key columns ───────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: "Maximum default key columns"
                        verticalAlignment: Text.AlignVCenter
                        color: "black"
                    }

                    TextField {
                        id: maxKeyColumnsField
                        Layout.fillWidth: true
                        height: 28
                        text: "16"
                        leftPadding: 8
                        verticalAlignment: TextInput.AlignVCenter
                        validator: IntValidator { bottom: 1; top: 9999 }

                        background: Rectangle {
                            border.color: maxKeyColumnsField.activeFocus ? "#DA3450" : "darkgray"
                            border.width: 1
                            color: "white"
                            radius: 2
                        }
                    }
                }

                // ── Checkbox 2: Auto subscribe ─────────────────────────
                CheckBox {
                    id: autoSubscribeCheckBox
                    Layout.fillWidth: true
                    checked: true

                    indicator: Rectangle {
                        implicitWidth: 16
                        implicitHeight: 16
                        x: autoSubscribeCheckBox.leftPadding
                        y: parent.height / 2 - height / 2
                        border.color: "#34495e"
                        border.width: 1
                        color: "white"
                        radius: 2

                        Text {
                            anchors.centerIn: parent
                            text: autoSubscribeCheckBox.checked ? "✓" : ""
                            color: "#34495e"
                            font.bold: true
                            font.pixelSize: 11
                        }
                    }

                    contentItem: Text {
                        text: "Automatically subscribe to previously subscribed Topics on restart."
                        leftPadding: autoSubscribeCheckBox.indicator.width + autoSubscribeCheckBox.spacing + 4
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                        color: "black"
                    }
                }

                // ── Indented description label ─────────────────────────
                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: 22
                    text: "Switch to Data Visualization perspective when data visualization features are invoked"
                    wrapMode: Text.WordWrap
                    color: "black"
                }

                // ── Radio buttons: Always | Prompt | Never ─────────────
                ButtonGroup {
                    id: perspectiveGroup
                    exclusive: true
                    buttons: [alwaysRadio, promptRadio, neverRadio]
                }

                Row {
                    Layout.leftMargin: 22
                    spacing: 10

                    RadioButton {
                        id: alwaysRadio
                        text: "Always"
                        checked: false

                        indicator: Rectangle {
                            implicitWidth: 16
                            implicitHeight: 16
                            radius: width / 2
                            x: 10
                            y: 6
                            border.color: "#34495e"
                            border.width: 1
                            color: alwaysRadio.checked ? "#DA3450" : "transparent"
                        }
                    }

                    RadioButton {
                        id: promptRadio
                        text: "Prompt"
                        checked: true

                        indicator: Rectangle {
                            implicitWidth: 16
                            implicitHeight: 16
                            radius: width / 2
                            x: 10
                            y: 6
                            border.color: "#34495e"
                            border.width: 1
                            color: promptRadio.checked ? "#DA3450" : "transparent"
                        }
                    }

                    RadioButton {
                        id: neverRadio
                        text: "Never"
                        checked: false

                        indicator: Rectangle {
                            implicitWidth: 16
                            implicitHeight: 16
                            radius: width / 2
                            x: 10
                            y: 6
                            border.color: "#34495e"
                            border.width: 1
                            color: neverRadio.checked ? "#DA3450" : "transparent"
                        }
                    }
                }

                // ── String character encoding label ────────────────────
                Text {
                    Layout.fillWidth: true
                    text: "String character encoding:"
                    color: "black"
                }

                // ── Radio buttons: UTF-8 | ISO-8859-1 ─────────────────
                ButtonGroup {
                    id: encodingGroup
                    exclusive: true
                    buttons: [utf8Radio, isoRadio]
                }

                Row {
                    spacing: 10

                    RadioButton {
                        id: utf8Radio
                        text: "UTF-8"
                        checked: true

                        indicator: Rectangle {
                            implicitWidth: 16
                            implicitHeight: 16
                            radius: width / 2
                            x: 10
                            y: 6
                            border.color: "#34495e"
                            border.width: 1
                            color: utf8Radio.checked ? "#DA3450" : "transparent"
                        }
                    }

                    RadioButton {
                        id: isoRadio
                        text: "ISO-8859-1"
                        checked: false

                        indicator: Rectangle {
                            implicitWidth: 16
                            implicitHeight: 16
                            radius: width / 2
                            x: 10
                            y: 6
                            border.color: "#34495e"
                            border.width: 1
                            color: isoRadio.checked ? "#DA3450" : "transparent"
                        }
                    }
                }

                // ── Vertical spacer ────────────────────────────────────
                Item { Layout.fillHeight: true }

                // ── Bottom buttons: Restore Defaults | Apply ───────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Item { Layout.fillWidth: true }

                    Button {
                        id: restoreDefaultsButton
                        text: "Restore Defaults"
                        height: 35
                        width: 130

                        background: Rectangle {
                            radius: 5
                            color: "white"
                            border.color: "darkgray"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: restoreDefaultsButton.text
                            color: "black"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {}
                    }

                    Button {
                        id: applyButton
                        text: "Apply"
                        height: 35
                        width: 80

                        background: Rectangle {
                            radius: 5
                            color: "white"
                            border.color: "darkgray"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: applyButton.text
                            color: "black"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {}
                    }
                }
            }
        }
    }
}