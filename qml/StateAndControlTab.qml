import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: stateandcontroltab
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        spacing: 15
        anchors.margins: 15

        // Info message when no database loaded
        Rectangle {
            id: infoMessage
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "#FFF9C4"
            border.color: "#FBC02D"
            visible: !fileLoggerModel.running
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10

                Label {
                    text: "⚠ Please load a log file from the File Logger tab first"
                    font.bold: true
                    color: "#F57C00"
                }
            }
        }

        Rectangle {
            id: parentRectangle
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.color: "#D3D3D3"
            color: "white"
            opacity: fileLoggerModel.running ? 1.0 : 0.5

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 20
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 20

                // Distributed Logger Section
                Rectangle {
                    id: dlRectangle
                    Layout.fillWidth: true
                    Layout.preferredHeight: 130
                    border.color: "blue"
                    color: "#F2F2F2"

                    Label {
                        id: dlLabel
                        text: "Distributed Logger"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        color: "blue"
                        z: 1
                        background: Rectangle {
                            color: dlRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }

                    GridLayout {
                        anchors.fill: parent
                        anchors.topMargin: 20
                        anchors.leftMargin: 15
                        anchors.rightMargin: 15
                        anchors.bottomMargin: 10
                        columns: 2
                        columnSpacing: 20
                        rowSpacing: 8

                        Label {
                            text: "State:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        Label {
                            text: stateControlModel.state
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }

                        Label {
                            text: "Last Update:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        Label {
                            text: stateControlModel.lastUpdate
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }

                        Label {
                            text: "Application Kind:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        Label {
                            text: stateControlModel.applicationKind
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }

                        Label {
                            text: "Filter Level:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight
                            spacing: 10

                            Label {
                                text: logLevelComboBox.currentText
                                Layout.alignment: Qt.AlignVCenter
                            }

                            ComboBox {
                                id: logLevelComboBox
                                model: ["Trace", "Silent", "Error", "Warning", "Notice", "Info", "Debug"]
                                currentIndex: model.indexOf(stateControlModel.filterLevel)
                                enabled: fileLoggerModel.running
                                
                                onActivated: {
                                    stateControlModel.setFilterLevel(currentText)
                                }

                                implicitWidth: 200
                                implicitHeight: 25

                                background: Rectangle {
                                    color: logLevelComboBox.enabled ? "white" : "#F0F0F0"
                                    border.color: logLevelComboBox.pressed ? "#81c784" : "#c2c2c2"
                                    border.width: 1
                                    radius: 2
                                }

                                contentItem: Text {
                                    leftPadding: 10
                                    text: logLevelComboBox.displayText
                                    font: logLevelComboBox.font
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                    color: logLevelComboBox.enabled ? "black" : "gray"
                                }
                            }
                        }
                    }
                }

                // Logger Section (same pattern - add enabled: fileLoggerModel.running to all ComboBoxes)
                Rectangle {
                    id: rtiloggerRectangle
                    Layout.fillWidth: true
                    Layout.preferredHeight: 230
                    border.color: "blue"
                    color: "#F2F2F2"

                    Label {
                        id: rtiloggerLabel
                        text: "Logger"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        color: "blue"
                        z: 1
                        background: Rectangle {
                            color: rtiloggerRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }

                    GridLayout {
                        anchors.fill: parent
                        anchors.topMargin: 20
                        anchors.leftMargin: 15
                        anchors.rightMargin: 15
                        anchors.bottomMargin: 10
                        columns: 2
                        columnSpacing: 20
                        rowSpacing: 8

                        // Print Format
                        Label {
                            text: "Print Format:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight
                            spacing: 10

                            Label {
                                text: printformatComboBox.currentText
                            }

                            ComboBox {
                                id: printformatComboBox
                                model: ["Default", "Timestamped", "Verbose", "Verbose Timestamped", "Debug", "Minimal", "Maximal"]
                                currentIndex: model.indexOf(stateControlModel.printFormat)
                                enabled: fileLoggerModel.running
                                
                                onActivated: {
                                    stateControlModel.setPrintFormat(currentText)
                                }

                                implicitWidth: 200
                                implicitHeight: 25

                                background: Rectangle {
                                    color: printformatComboBox.enabled ? "white" : "#F0F0F0"
                                    border.color: printformatComboBox.pressed ? "#81c784" : "#c2c2c2"
                                    border.width: 1
                                    radius: 2
                                }
                            }
                        }

                        // Platform Verbosity
                        Label {
                            text: "Platform Verbosity:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight
                            spacing: 10

                            Label {
                                text: platformverbosityComboBox.currentText
                            }

                            ComboBox {
                                id: platformverbosityComboBox
                                model: ["Trace", "Silent", "Error", "Warning", "Notice", "Info", "Debug"]
                                currentIndex: model.indexOf(stateControlModel.platformVerbosity)
                                enabled: fileLoggerModel.running
                                
                                onActivated: {
                                    stateControlModel.setPlatformVerbosity(currentText)
                                }

                                implicitWidth: 200
                                implicitHeight: 25

                                background: Rectangle {
                                    color: platformverbosityComboBox.enabled ? "white" : "#F0F0F0"
                                    border.color: platformverbosityComboBox.pressed ? "#81c784" : "#c2c2c2"
                                    border.width: 1
                                    radius: 2
                                }
                            }
                        }

                        // Communication Verbosity
                        Label {
                            text: "Communication Verbosity:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight
                            spacing: 10

                            Label {
                                text: communictaionverbosityComboBox.currentText
                            }

                            ComboBox {
                                id: communictaionverbosityComboBox
                                model: ["Trace", "Silent", "Error", "Warning", "Notice", "Info", "Debug"]
                                currentIndex: model.indexOf(stateControlModel.communicationVerbosity)
                                enabled: fileLoggerModel.running
                                
                                onActivated: {
                                    stateControlModel.setCommunicationVerbosity(currentText)
                                }

                                implicitWidth: 200
                                implicitHeight: 25

                                background: Rectangle {
                                    color: communictaionverbosityComboBox.enabled ? "white" : "#F0F0F0"
                                    border.color: communictaionverbosityComboBox.pressed ? "#81c784" : "#c2c2c2"
                                    border.width: 1
                                    radius: 2
                                }
                            }
                        }

                        // Database Verbosity
                        Label {
                            text: "Database Verbosity:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight
                            spacing: 10

                            Label {
                                text: databaseverbosityComboBox.currentText
                            }

                            ComboBox {
                                id: databaseverbosityComboBox
                                model: ["Trace", "Silent", "Error", "Warning", "Notice", "Info", "Debug"]
                                currentIndex: model.indexOf(stateControlModel.databaseVerbosity)
                                enabled: fileLoggerModel.running
                                
                                onActivated: {
                                    stateControlModel.setDatabaseVerbosity(currentText)
                                }

                                implicitWidth: 200
                                implicitHeight: 25

                                background: Rectangle {
                                    color: databaseverbosityComboBox.enabled ? "white" : "#F0F0F0"
                                    border.color: databaseverbosityComboBox.pressed ? "#81c784" : "#c2c2c2"
                                    border.width: 1
                                    radius: 2
                                }
                            }
                        }

                        // Entities Verbosity
                        Label {
                            text: "Entities verbosity:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight
                            spacing: 10

                            Label {
                                text: entitiesverbosityComboBox.currentText
                            }

                            ComboBox {
                                id: entitiesverbosityComboBox
                                model: ["Trace", "Silent", "Error", "Warning", "Notice", "Info", "Debug"]
                                currentIndex: model.indexOf(stateControlModel.entitiesVerbosity)
                                enabled: fileLoggerModel.running
                                
                                onActivated: {
                                    stateControlModel.setEntitiesVerbosity(currentText)
                                }

                                implicitWidth: 200
                                implicitHeight: 25

                                background: Rectangle {
                                    color: entitiesverbosityComboBox.enabled ? "white" : "#F0F0F0"
                                    border.color: entitiesverbosityComboBox.pressed ? "#81c784" : "#c2c2c2"
                                    border.width: 1
                                    radius: 2
                                }
                            }
                        }

                        // API Verbosity
                        Label {
                            text: "API Verbosity:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight
                            spacing: 10

                            Label {
                                text: apiverbosityComboBox.currentText
                            }

                            ComboBox {
                                id: apiverbosityComboBox
                                model: ["Trace", "Silent", "Error", "Warning", "Notice", "Info", "Debug"]
                                currentIndex: model.indexOf(stateControlModel.apiVerbosity)
                                enabled: fileLoggerModel.running
                                
                                onActivated: {
                                    stateControlModel.setApiVerbosity(currentText)
                                }

                                implicitWidth: 200
                                implicitHeight: 25

                                background: Rectangle {
                                    color: apiverbosityComboBox.enabled ? "white" : "#F0F0F0"
                                    border.color: apiverbosityComboBox.pressed ? "#81c784" : "#c2c2c2"
                                    border.width: 1
                                    radius: 2
                                }
                            }
                        }
                    }
                }

                // Command Response Section
                Rectangle {
                    id: crRectangle
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    border.color: "blue"
                    color: "#F2F2F2"

                    Label {
                        id: crLabel
                        text: "Command Response"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.topMargin: -10
                        color: "blue"
                        z: 1
                        background: Rectangle {
                            color: crRectangle.color
                            anchors.fill: parent
                            anchors.leftMargin: -5
                            anchors.rightMargin: -5
                        }
                    }

                    GridLayout {
                        anchors.fill: parent
                        anchors.topMargin: 20
                        anchors.leftMargin: 15
                        anchors.rightMargin: 15
                        anchors.bottomMargin: 10
                        columns: 4
                        columnSpacing: 30
                        rowSpacing: 8

                        Label {
                            text: "Result:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        Label {
                            text: stateControlModel.commandResult || ""
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }
                        Label {
                            text: "Invocation:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        Label {
                            text: stateControlModel.commandInvocation || ""
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }

                        Label {
                            text: "Host ID:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        Label {
                            text: stateControlModel.commandHostId || ""
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }
                        Label {
                            text: "Message:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        Label {
                            text: stateControlModel.commandMessage || ""
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }

                        Label {
                            text: "Last Update:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        Label {
                            text: stateControlModel.commandLastUpdate || ""
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }
                        Label {
                            text: "App ID:"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        Label {
                            text: stateControlModel.commandAppId || ""
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    // Watch for file logger changes
    Connections {
        target: fileLoggerModel
        function onRunningChanged() {
            if (fileLoggerModel.running) {
                // Load state control with same database
                var dbPath = fileLoggerModel.filePath.replace(".log", ".db")
                if (!dbPath.endsWith(".db")) {
                    dbPath = fileLoggerModel.filePath
                }
                console.log("Loading state control from:", dbPath)
                stateControlModel.loadStateFromDatabase(dbPath)
            }
        }
    }

    // Auto-refresh timer (only when file logger is running)
    Timer {
        interval: 2000
        running: fileLoggerModel.running
        repeat: true
        onTriggered: {
            stateControlModel.refreshState()
        }
    }
}
