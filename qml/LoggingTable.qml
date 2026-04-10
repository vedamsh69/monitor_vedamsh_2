import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    anchors.fill: parent

    Rectangle {
        id: mainRectangle
        color: "#F2F2F2"
        anchors.fill: parent

        // ── Header Bar ─────────────────────────────────────────────────
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
                    text: "Logging"
                    color: "darkgray"
                    bottomPadding: 15
                }

                Item { Layout.fillWidth: true }

                Row {
                    spacing: 15
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                    Image {
                        source: "qrc:/resources/images/icons/refresh_arrow.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/left_arrow.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/down_arrow.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                    Image {
                        source: "qrc:/resources/images/icons/dots.png"
                        width: 20; height: 20
                        MouseArea { anchors.fill: parent; onClicked: {} }
                    }
                }
            }
        }

        // ── Content Area ───────────────────────────────────────────────
        Rectangle {
            id: lowerRectangle
            anchors.top: upperRectangle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: bottomBar.top
            color: "#F2F2F2"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                // ── Console Log Verbosity Row ──────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: "Console Log Verbosity :"
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                    }

                    ComboBox {
                        id: consoleLogVerbosityCombo
                        Layout.fillWidth: true
                        height: 36
                        model: ["Warn", "Debug", "Info", "Error", "Silent"]
                        currentIndex: 0

                        background: Rectangle {
                            radius: 4
                            color: "white"
                            border.color: "darkgray"
                            border.width: 1
                        }

                        contentItem: Text {
                            leftPadding: 10
                            text: consoleLogVerbosityCombo.displayText
                            verticalAlignment: Text.AlignVCenter
                            color: "black"
                        }

                        delegate: ItemDelegate {
                            width: consoleLogVerbosityCombo.width
                            contentItem: Text {
                                text: modelData
                                color: "black"
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 10
                            }
                            highlighted: consoleLogVerbosityCombo.highlightedIndex === index
                            background: Rectangle {
                                color: highlighted ? "#DA3450" : "white"
                            }
                        }

                        popup: Popup {
                            y: consoleLogVerbosityCombo.height
                            width: consoleLogVerbosityCombo.width
                            padding: 0
                            contentItem: ListView {
                                implicitHeight: contentHeight
                                model: consoleLogVerbosityCombo.delegateModel
                                clip: true
                            }
                            background: Rectangle {
                                border.color: "darkgray"
                                border.width: 1
                                color: "white"
                            }
                        }
                    }
                }

                // ── Distributed Logger Settings ────────────────────────
                Rectangle {
                    id: distributedLoggerField
                    Layout.fillWidth: true
                    height: 110
                    border.color: "gray"
                    color: "#F2F2F2"

                    Label {
                        text: "Distributed Logger Settings"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        z: 1
                        background: Rectangle {
                            color: lowerRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        anchors.topMargin: 14
                        spacing: 0

                        // Process Log Length row
                        Rectangle {
                            Layout.fillWidth: true
                            height: 44
                            color: "transparent"
                            border.color: "lightgray"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                spacing: 0

                                Text {
                                    text: "Process Log Length:"
                                    color: "black"
                                    verticalAlignment: Text.AlignVCenter
                                    Layout.preferredWidth: 160
                                }

                                TextField {
                                    id: distProcessLogField
                                    Layout.fillWidth: true
                                    height: 44
                                    text: "2048"
                                    leftPadding: 8
                                    verticalAlignment: TextInput.AlignVCenter
                                    validator: IntValidator { bottom: 0; top: 999999 }
                                    background: Rectangle { color: "transparent" }
                                }

                                // Minus button
                                Rectangle {
                                    width: 36
                                    height: 44
                                    color: "transparent"
                                    border.color: "lightgray"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: "−"
                                        font.pixelSize: 16
                                        color: "black"
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var v = parseInt(distProcessLogField.text) || 0
                                            distProcessLogField.text = Math.max(v - 1, 0).toString()
                                        }
                                    }
                                }

                                // Plus button
                                Rectangle {
                                    width: 36
                                    height: 44
                                    color: "transparent"
                                    border.color: "lightgray"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: "+"
                                        font.pixelSize: 16
                                        color: "black"
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var v = parseInt(distProcessLogField.text) || 0
                                            distProcessLogField.text = (v + 1).toString()
                                        }
                                    }
                                }
                            }
                        }

                        // System Log Length row
                        Rectangle {
                            Layout.fillWidth: true
                            height: 44
                            color: "transparent"
                            border.color: "lightgray"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                spacing: 0

                                Text {
                                    text: "System Log Length:"
                                    color: "black"
                                    verticalAlignment: Text.AlignVCenter
                                    Layout.preferredWidth: 160
                                }

                                TextField {
                                    id: distSystemLogField
                                    Layout.fillWidth: true
                                    height: 44
                                    text: "4096"
                                    leftPadding: 8
                                    verticalAlignment: TextInput.AlignVCenter
                                    validator: IntValidator { bottom: 0; top: 999999 }
                                    background: Rectangle { color: "transparent" }
                                }

                                Rectangle {
                                    width: 36; height: 44
                                    color: "transparent"
                                    border.color: "lightgray"; border.width: 1
                                    Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 16; color: "black" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var v = parseInt(distSystemLogField.text) || 0
                                            distSystemLogField.text = Math.max(v - 1, 0).toString()
                                        }
                                    }
                                }

                                Rectangle {
                                    width: 36; height: 44
                                    color: "transparent"
                                    border.color: "lightgray"; border.width: 1
                                    Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 16; color: "black" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var v = parseInt(distSystemLogField.text) || 0
                                            distSystemLogField.text = (v + 1).toString()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // ── Security Logger Settings ───────────────────────────
                Rectangle {
                    id: securityLoggerField
                    Layout.fillWidth: true
                    height: 110
                    border.color: "gray"
                    color: "#F2F2F2"

                    Label {
                        text: "Security Logger Settings"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        z: 1
                        background: Rectangle {
                            color: lowerRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        anchors.topMargin: 14
                        spacing: 0

                        Rectangle {
                            Layout.fillWidth: true
                            height: 44
                            color: "transparent"
                            border.color: "lightgray"; border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                spacing: 0

                                Text {
                                    text: "Process Log Length:"
                                    color: "black"
                                    verticalAlignment: Text.AlignVCenter
                                    Layout.preferredWidth: 160
                                }

                                TextField {
                                    id: secProcessLogField
                                    Layout.fillWidth: true
                                    height: 44
                                    text: "2048"
                                    leftPadding: 8
                                    verticalAlignment: TextInput.AlignVCenter
                                    validator: IntValidator { bottom: 0; top: 999999 }
                                    background: Rectangle { color: "transparent" }
                                }

                                Rectangle {
                                    width: 36; height: 44
                                    color: "transparent"
                                    border.color: "lightgray"; border.width: 1
                                    Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 16; color: "black" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var v = parseInt(secProcessLogField.text) || 0
                                            secProcessLogField.text = Math.max(v - 1, 0).toString()
                                        }
                                    }
                                }

                                Rectangle {
                                    width: 36; height: 44
                                    color: "transparent"
                                    border.color: "lightgray"; border.width: 1
                                    Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 16; color: "black" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var v = parseInt(secProcessLogField.text) || 0
                                            secProcessLogField.text = (v + 1).toString()
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 44
                            color: "transparent"
                            border.color: "lightgray"; border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                spacing: 0

                                Text {
                                    text: "System Log Length:"
                                    color: "black"
                                    verticalAlignment: Text.AlignVCenter
                                    Layout.preferredWidth: 160
                                }

                                TextField {
                                    id: secSystemLogField
                                    Layout.fillWidth: true
                                    height: 44
                                    text: "4096"
                                    leftPadding: 8
                                    verticalAlignment: TextInput.AlignVCenter
                                    validator: IntValidator { bottom: 0; top: 999999 }
                                    background: Rectangle { color: "transparent" }
                                }

                                Rectangle {
                                    width: 36; height: 44
                                    color: "transparent"
                                    border.color: "lightgray"; border.width: 1
                                    Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 16; color: "black" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var v = parseInt(secSystemLogField.text) || 0
                                            secSystemLogField.text = Math.max(v - 1, 0).toString()
                                        }
                                    }
                                }

                                Rectangle {
                                    width: 36; height: 44
                                    color: "transparent"
                                    border.color: "lightgray"; border.width: 1
                                    Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 16; color: "black" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            var v = parseInt(secSystemLogField.text) || 0
                                            secSystemLogField.text = (v + 1).toString()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // ── Local DDS Logger Settings ──────────────────────────
                Rectangle {
                    id: localDdsLoggerField
                    Layout.fillWidth: true
                    height: 280
                    border.color: "gray"
                    color: "#F2F2F2"

                    Label {
                        text: "Local DDS Logger Settings"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        z: 1
                        background: Rectangle {
                            color: lowerRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }

                    // Reusable verbosity options
                    property var verbosityOptions: ["Default", "Warning", "Debug", "Info", "Error", "Silent"]

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        anchors.topMargin: 14
                        spacing: 0

                        // Helper component for each verbosity row
                        Repeater {
                            model: [
                                { label: "Console Log Verbosity :", defaultVal: "Default"  },
                                { label: "Platform Verbosity :",     defaultVal: "Warning"  },
                                { label: "Communications Verbosity :", defaultVal: "Warning" },
                                { label: "Database Verbosity :",     defaultVal: "Warning"  },
                                { label: "Entities Verbosity :",     defaultVal: "Warning"  },
                                { label: "API Verbosity :",          defaultVal: "Warning"  }
                            ]

                            delegate: Rectangle {
                                Layout.fillWidth: true
                                height: 44
                                color: "transparent"
                                border.color: "lightgray"
                                border.width: 1

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 10
                                    spacing: 10

                                    Text {
                                        text: modelData.label
                                        color: "black"
                                        verticalAlignment: Text.AlignVCenter
                                        Layout.preferredWidth: 190
                                    }

                                    ComboBox {
                                        id: verbosityCombo
                                        Layout.fillWidth: true
                                        height: 32
                                        model: localDdsLoggerField.verbosityOptions
                                        currentIndex: {
                                            var idx = localDdsLoggerField.verbosityOptions.indexOf(modelData.defaultVal)
                                            return idx >= 0 ? idx : 0
                                        }

                                        background: Rectangle {
                                            radius: 4
                                            color: "white"
                                            border.color: "darkgray"
                                            border.width: 1
                                        }

                                        contentItem: Text {
                                            leftPadding: 10
                                            text: verbosityCombo.displayText
                                            verticalAlignment: Text.AlignVCenter
                                            color: "black"
                                        }

                                        delegate: ItemDelegate {
                                            width: verbosityCombo.width
                                            contentItem: Text {
                                                text: modelData
                                                color: "black"
                                                verticalAlignment: Text.AlignVCenter
                                                leftPadding: 10
                                            }
                                            highlighted: verbosityCombo.highlightedIndex === index
                                            background: Rectangle {
                                                color: highlighted ? "#DA3450" : "white"
                                            }
                                        }

                                        popup: Popup {
                                            y: verbosityCombo.height
                                            width: verbosityCombo.width
                                            padding: 0
                                            contentItem: ListView {
                                                implicitHeight: contentHeight
                                                model: verbosityCombo.delegateModel
                                                clip: true
                                            }
                                            background: Rectangle {
                                                border.color: "darkgray"
                                                border.width: 1
                                                color: "white"
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Vertical spacer
                Item { Layout.fillHeight: true }
            }
        }

        // ── Bottom Bar: Restore Defaults + Apply ───────────────────────
        Rectangle {
            id: bottomBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "white"
            border.color: "#E8E9EB"
            border.width: 1

            RowLayout {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 16
                spacing: 8

                Button {
                    id: restoreDefaultsBtn
                    text: "Restore Defaults"
                    implicitWidth: 130
                    implicitHeight: 35

                    background: Rectangle {
                        radius: 5
                        color: "white"
                        border.color: "darkgray"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: restoreDefaultsBtn.text
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        distProcessLogField.text = "2048"
                        distSystemLogField.text  = "4096"
                        secProcessLogField.text  = "2048"
                        secSystemLogField.text   = "4096"
                        consoleLogVerbosityCombo.currentIndex = 0
                    }
                }

                Button {
                    id: applyBtn
                    text: "Apply"
                    implicitWidth: 80
                    implicitHeight: 35

                    background: Rectangle {
                        radius: 5
                        color: "white"
                        border.color: "darkgray"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: applyBtn.text
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