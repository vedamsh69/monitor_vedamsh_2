import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3 

Item {
    id: fileloggertab
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        anchors.margins: 10

        // Top control row
        RowLayout {
            id: rowforbutton
            Layout.fillWidth: true
            spacing: 10

            Button {
                id: findbutton
                text: fileLoggerModel.running ? "Stop" : "Find"
                implicitHeight: 25
                implicitWidth: 65
                onClicked: {
                    if (fileLoggerModel.running) {
                        fileLoggerModel.stopLogging()
                    } else {
                        finddialog.open()
                    }
                }

                background: Rectangle {
                    color: "white"
                    border.color: findbutton.down ? "#81c784" : "#c2c2c2"
                    border.width: 1
                    radius: 2
                }

                contentItem: Text {
                    text: findbutton.text
                    font: findbutton.font
                    color: "black"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }

            Label {
                text: "Queue Size: "
            }

            SpinBox {
                id: queueSizeSpinBox
                editable: true
                value: 512
                from: 512
                to: 2320

                onValueChanged: {
                    fileLoggerModel.setQueueSize(value)
                }

                background: Rectangle {
                    implicitWidth: 85
                    implicitHeight: 21
                    border.color: queueSizeSpinBox.focus ? "#b5e2ff" : "gray"
                    radius: 1
                }

                up.indicator: null
                down.indicator: null

                Layout.preferredWidth: 80

                Rectangle {
                    anchors {
                        left: parent.right
                        verticalCenter: parent.verticalCenter
                    }
                    width: 20
                    height: parent.height
                    color: "transparent"

                    Column {
                        anchors.fill: parent
                        spacing: 1

                        Button {
                            width: parent.width
                            height: parent.height / 2 - 0.5
                            text: "▲"
                            onClicked: queueSizeSpinBox.increase()
                        }

                        Button {
                            width: parent.width
                            height: parent.height / 2 - 0.5
                            text: "▼"
                            onClicked: queueSizeSpinBox.decrease()
                        }
                    }
                }

                font.pixelSize: height * 0.5
            }

            // Add spacer to push everything left
            Item {
                Layout.fillWidth: true
            }
        }

        // Log file path row
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Log File Path: "
            }

            Button {
                id: browseButton
                text: "Browse"
                implicitHeight: 30
                implicitWidth: 75
                
                onClicked: {
                    fileDialog.folder = "file:///home/cdac/indedds/monitor/src/DataBase"
                    fileDialog.open()
                }

                background: Rectangle {
                    color: "white"
                    border.color: browseButton.down ? "#81c784" : "#c2c2c2"
                    border.width: 1
                    radius: 2
                }

                contentItem: Text {
                    text: browseButton.text
                    font: browseButton.font
                    color: "black"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }

            TextField {
                id: logFilePathTextField
                Layout.fillWidth: true
                readOnly: true
                placeholderText: "No file selected"
                text: fileLoggerModel.running ? fileLoggerModel.filePath : "No file selected"
            }
        }

        // Separator line (like RTI)
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#E0E0E0"
            Layout.topMargin: 5
            Layout.bottomMargin: 5
        }

        // Data display section with Grid Layout (RTI-style)
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: false
            columns: 2
            columnSpacing: 20
            rowSpacing: 12
            Layout.leftMargin: 10
            Layout.rightMargin: 10

            // Running
            Label {
                text: "Running:"
                font.bold: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
            Label {
                text: fileLoggerModel.running ? "true" : "false"
                color: fileLoggerModel.running ? "green" : "black"
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                horizontalAlignment: Text.AlignRight
            }

            // File Path
            Label {
                text: "File Path:"
                font.bold: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
            Label {
                text: fileLoggerModel.filePath || ""
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideMiddle
            }

            // File Size
            Label {
                text: "File size:"
                font.bold: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
            Label {
                text: fileLoggerModel.fileSize || "0"
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                horizontalAlignment: Text.AlignRight
            }

            // Messages Written
            Label {
                text: "Messages Written:"
                font.bold: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
            Label {
                text: fileLoggerModel.messagesWritten || "0"
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                horizontalAlignment: Text.AlignRight
            }

            // Messages Dropped Count
            Label {
                text: "Messages Dropped Count:"
                font.bold: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
            Label {
                text: fileLoggerModel.messagesDropped.toString()
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                horizontalAlignment: Text.AlignRight
            }

            // Last Exception
            Label {
                text: "Last Exception:"
                font.bold: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
            Label {
                text: fileLoggerModel.lastException || "None"
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                horizontalAlignment: Text.AlignRight
                wrapMode: Text.WordWrap
                color: fileLoggerModel.lastException ? "red" : "black"
            }

            // Max Queue Size
            Label {
                text: "Max Queue Size:"
                font.bold: false
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
            Label {
                text: fileLoggerModel.maxQueueSize.toString()
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                horizontalAlignment: Text.AlignRight
            }
        }

        // Queue Size Progress Bar (separate row like RTI)
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.topMargin: 5
            spacing: 10

            Label {
                text: "Queue Size:"
                font.bold: false
                Layout.preferredWidth: 140  // Match label width above
            }

            ProgressBar {
                Layout.fillWidth: true
                from: 0
                to: 100
                value: fileLoggerModel.queuePercentage

                background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: 20
                    color: "#e0e0e0"
                    radius: 3
                    border.color: "#c0c0c0"
                    border.width: 1
                }

                contentItem: Item {
                    implicitWidth: 200
                    implicitHeight: 20

                    Rectangle {
                        width: parent.parent.visualPosition * parent.width
                        height: parent.height
                        radius: 2
                        color: "#4caf50"
                    }
                }
            }

            Label {
                text: fileLoggerModel.currentQueueSize + " of " + 
                      (fileLoggerModel.maxQueueSize * 100) + " : " + 
                      fileLoggerModel.queuePercentage + "%"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                font.pixelSize: 12
            }
        }

        // Fill remaining space
        Item {
            Layout.fillHeight: true
        }
    }

    // File selection dialog
    FileDialog {
        id: fileDialog
        title: "Choose log file or directory"
        folder: shortcuts.home
        selectMultiple: false
        selectFolder: false  // Allow file selection
        selectExisting: true
        nameFilters: ["Log files (*.log *.db)", "All files (*)"]

        onAccepted: {
            if (fileUrls.length > 0) {
                var path = fileUrls[0].toString()
                logFilePathTextField.text = path
                if (fileLoggerModel.loadLogFile(path)) {
                    console.log("Log file loaded successfully")
                } else {
                    console.log("Failed to load log file")
                }
            }
        }
    }

    // Auto-refresh timer (like MessageTab)
    Timer {
        interval: 1000 // 1 second refresh
        running: fileLoggerModel.running
        repeat: true
        onTriggered: {
            fileLoggerModel.refreshStats()
        }
    }
}
