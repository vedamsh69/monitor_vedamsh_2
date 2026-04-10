import QtQuick 2.15
import QtQuick.Controls 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2

Dialog {
    id: distributed
    width: 950
    height: 700
    visible: false

    function open() {
        visible = true;
    }

    function close() {
        visible = false;
    }

    property bool isplay: false

    Rectangle {
        id: root
        anchors.fill: parent

        Rectangle {
            id: parentroot
            anchors.fill: parent
            color: "white"
            anchors.bottomMargin: 55

            TabView {
                id: tabView
                anchors.fill: parent
                anchors.margins: 1

                onCurrentIndexChanged: {
                    console.log("Selected Tab Index: " + tabView.currentIndex);
                }

                Tab {
                    title: "Log"
                    Rectangle {
                        width: parent.width
                        height: parent.height
                        color: "white"

                        Tableforlogs {
                            id: tablelog
                            anchors.fill: parent
                            isPlaying: distributed.isplay
                        }
                    }
                }
            }
        }

        Row {
            anchors.top: parentroot.bottom
            anchors.margins: 10
            anchors.left: parentroot.left
            anchors.right: parentroot.right
            spacing: 10

            Button {
                width: 125
                height: 40
                contentItem: Row {
                    spacing: 10
                    Layout.alignment: Qt.AlignVCenter
                    Image {
                        id: playimg
                        width: 24
                        height: 24
                        source: isplay ? "qrc:/resources/images/icons/pause.png" : "qrc:/resources/images/icons/play.png"
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Text {
                        id: playtext
                        text: isplay ? "Pause" : "Play"
                        color: "black"
                        Layout.alignment: Qt.AlignVCenter
                        font.bold: true
                    }
                }

                background: Rectangle {
                    radius: 5
                    color: "white"
                    border.color: "gray"
                    border.width: 1
                }

                onClicked: {
                    isplay = !isplay
                    console.log(isplay ? "Play button clicked." : "Pause button clicked.")
                }
            }

            Button {
                id: exportbutton
                width: 125
                height: 40

                contentItem: Row {
                    spacing: 10
                    Layout.alignment: Qt.AlignVCenter
                    Image {
                        id: exportimg
                        width: 24
                        height: 24
                        source: "qrc:/resources/images/icons/file.png"
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Text {
                        id: exporttext
                        text: qsTr("Export")
                        color: "black"
                        Layout.alignment: Qt.AlignVCenter
                        font.bold: true
                    }
                }

                background: Rectangle {
                    radius: 5
                    color: "white"
                    border.color: "gray"
                    border.width: 1
                }

                onClicked: {
                    console.log("Export button clicked")
                    exportFileDialog.open()
                }
            }
        }
    }

    // File Dialog for Export
    FileDialog {
        id: exportFileDialog
        title: "Export Logs to CSV"
        selectExisting: false
        selectMultiple: false
        nameFilters: ["CSV files (*.csv)", "All files (*)"]
        defaultSuffix: "csv"
        selectFolder: false

        onAccepted: {
            var filePath = fileUrl.toString()
            console.log("Raw fileUrl:", filePath)
            
            // Remove file:// prefix - CORRECTED TO KEEP LEADING SLASH
            if (filePath.startsWith("file:///")) {
                filePath = filePath.substring(7)  // Keep leading /
            } else if (filePath.startsWith("file://")) {
                filePath = filePath.substring(7)
            }
            
            // Add .csv extension if not present
            if (!filePath.endsWith(".csv")) {
                filePath = filePath + ".csv"
            }
            
            console.log("Cleaned filePath:", filePath)
            exportLogs(filePath)
        }

        onRejected: {
            console.log("Export cancelled")
        }

        Component.onCompleted: {
            var timestamp = Qt.formatDateTime(new Date(), "yyyyMMdd_HHmmss")
            selectFile("file:///home/cdac/logs_export_" + timestamp + ".csv")
        }
    }

    // Function to export logs to CSV
    function exportLogs(filePath) {
        console.log("Starting export to:", filePath)

        var totalLogs = dllogmodel.rowCount()
        console.log("Total logs to export:", totalLogs)

        if (totalLogs === 0) {
            exportResultDialog.text = "No logs available to export."
            exportResultDialog.open()
            return
        }

        // Build CSV content
        var csvContent = "Host,Process,Timestamp,Level,Category,Message\n"

        for (var i = 0; i < totalLogs; i++) {
            var log = dllogmodel.getLog(i)
            if (log) {
                csvContent += escapeCSV(log.host || "") + ","
                csvContent += escapeCSV(log.process || "") + ","
                csvContent += escapeCSV(log.timestamp || "") + ","
                csvContent += escapeCSV(log.kind || "") + ","
                csvContent += escapeCSV(log.category || "") + ","
                csvContent += escapeCSV(log.message || "") + "\n"
            }
        }

        // Call dllogmodel's exportToFile method
        var success = dllogmodel.exportToFile(filePath, csvContent)

        if (success) {
            console.log("Export successful!")
            exportResultDialog.text = "Successfully exported " + totalLogs + " logs to:\n" + filePath
        } else {
            console.log("Export failed")
            exportResultDialog.text = "Failed to export logs.\n\nPath: " + filePath + "\n\nCheck terminal for error details."
        }

        exportResultDialog.open()
    }

    // Helper function to escape CSV fields
    function escapeCSV(field) {
        var str = String(field)
        if (str.indexOf(',') !== -1 || str.indexOf('\n') !== -1 || str.indexOf('"') !== -1) {
            str = str.replace(/"/g, '""')
            return '"' + str + '"'
        }
        return str
    }

    // Result Dialog
    Dialog {
        id: exportResultDialog
        title: "Export Status"
        standardButtons: Dialog.Ok

        property alias text: resultText.text

        Label {
            id: resultText
            wrapMode: Text.WordWrap
            width: 400
        }
    }

    // Connect to the dllogmodel's signals
    Connections {
        target: dllogmodel
        function onLogsChanged() {
            Qt.callLater(function() {
                if (distributed.tablelog) {
                    distributed.tablelog.updateIntermediateModel()
                }
            })
        }
    }

    // Initialize the model when the dialog is completed
    Component.onCompleted: {
        Qt.callLater(function() {
            if (distributed.tablelog) {
                distributed.tablelog.updateIntermediateModel()
            }
        })
    }
}
